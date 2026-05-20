# KMSynth CRI DLL Adapter

> このリポジトリは以下の note 記事のサンプルコードです。  
> https://note.com/p_onozuka/n/n5ef0a9dc636a

CRIWARE（CRI Atom Craft/ADX LE）のカスタムDSPエフェクト用DLL（`KMSynth_CRI.dll`）を実装するサンプルコードです。  
JUCEを利用したシンセサイザーをCRIWAREのエフェクトプラグインとして組み込む方法を示します。

## 概要

CRIWAREはカスタムDSPエフェクトをDLLとして外部から読み込む仕組みを持っています。  
このリポジトリは、その仕組みに対応したDLLを作成する最小限の実装例です。

- **エフェクト名**: `POnozuka/KMSynth_CRI`
- **チャンネル構成**: ステレオ（2ch）のみ
- **動作確認用サウンド**: 440Hz サイン波をパススルー入力に加算して出力

## ディレクトリ構成

```
.
├── CMakeLists.txt          ビルド定義（CMake）
├── cri/
│   ├── cri_audio_effect.h  CRI提供のエフェクトインターフェースヘッダ（変更不可）
│   └── README.md
├── src/
│   ├── SynthInterface.h    CRIインターフェース関数の宣言・静的テーブル定義
│   ├── SynthInterface.cpp  CRIインターフェース関数の実装（Create/Destroy/Process等）
│   ├── KMSynthDllAdapter.h シンセエンジンとCRIインターフェースをつなぐアダプタクラス
│   └── KMSynthDllAdapter.cpp
└── win/
    ├── dllmain.cpp         DllMain エントリポイント（定型実装）
    └── export_symbols.def  DLLエクスポートシンボル定義（GetInterfaceWithVersion のみ）
```

## 依存関係

| ライブラリ | 役割 |
|---|---|
| [JUCE](https://juce.com/) | AudioBuffer / MidiBuffer / FloatVectorOperations |
| `km_synth` | シンセサイザーコア（本リポジトリの親プロジェクト） |
| [FMIDI](https://github.com/POnozuka/FMIDI) | MIDIメッセージエンコード／デコードユーティリティ |

CMake上では `km_synth` をリンクすることでJUCEの依存も自動解決されます。

## ビルド方法

このリポジトリは親プロジェクトのサブディレクトリとして組み込む構成です。  
親の `CMakeLists.txt` で `add_subdirectory` 済みであることを前提とします。

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

出力先: `build/bin/Release/KMSynth_CRI.dll`

## CRIインターフェースの仕組み

CRIWAREがDLLに要求するエントリポイントは1つだけです。

```c
const CriAfxInterfaceWithVersion* GetInterfaceWithVersion(void);
```

この関数が返す構造体に、以下のコールバック関数ポインタをセットします。

| 関数 | 呼ばれるタイミング |
|---|---|
| `GetEffectName` | エフェクト名の取得（識別子） |
| `CalculateWorkSize` | ワークメモリサイズの計算 |
| `Create` | エフェクトインスタンスの生成 |
| `Destroy` | インスタンスの破棄 |
| `Start` | 再生開始時の初期化 |
| `Process` | オーディオブロック処理（バッファ単位で繰り返し呼ばれる） |
| `SetParameter` / `GetParameter` | パラメータの読み書き |
| `ApplyParameters` | パラメータの一括適用（本実装では即時反映のため未使用） |
| `GetNumProcessChannels` | 入出力チャンネル数の通知 |

### パラメータ経由のMIDI受信

`SetParameter` のインデックス `0xFFFF` はMIDI受信チャンネルとして予約しています。  
`fmidi` でエンコードされたfloat値を渡すと、内部でMidiMessageに変換してバッファに積みます。

## KMSynthDllAdapter とコア音源の接続

`KMSynthDllAdapter` は、CRIインターフェース（`SynthInterface.cpp`）と  
km_synth コア音源エンジンをつなぐアダプタクラスです。

```
CRIWAREランタイム
    │  Process() / SetParameter() / ...
    ▼
SynthInterface.cpp   ← CRIコールバック実装
    │  renderNextBlock() / setParameter() / ...
    ▼
KMSynthDllAdapter    ← アダプタ（このクラスがコアを保持・呼び出す）
    │
    ▼
km_synth コア音源    ← 親プロジェクトのシンセエンジン
```

### 現在の状態（スタブ実装）

本サンプルの `KMSynthDllAdapter` はコア音源との接続を**まだ行っていません**。  
動作確認用として、440Hz固定のサイン波をパススルー入力に加算して出力するスタブを実装しています。

### コア音源を接続する場合の拡張方針

1. `KMSynthDllAdapter.h` のprivateメンバーにコアエンジンのインスタンスを追加する

```cpp
private:
    std::unique_ptr<km_synth::SomeEngine> engine; // コアエンジン
```

2. `prepareToPlay()` でエンジンを初期化し、`renderNextBlock()` から委譲する

```cpp
void KMSynthDllAdapter::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engine->prepare(sampleRate, samplesPerBlock);
}

void KMSynthDllAdapter::renderNextBlock(...)
{
    engine->process(pcm_input, pcm_output, num_samples, ...);
}
```

3. `setParameter()` / `getParameter()` もエンジンのAPIに委譲する

CMake上では既に `km_synth` へのリンクが設定済みのため、  
ヘッダのインクルードとクラスのインスタンス化を追加するだけで接続できます。

## 実装上の注意点

### CalculateWorkSize が 1 を返す理由

CRIWAREの仕様上、`CalculateWorkSize` が 0 を返すと `Create` が呼ばれません。  
インスタンスは `new` でヒープ確保するためワークメモリは実質不要ですが、  
`Create` を呼ばせるために最小値 `1` を返しています。

## ライセンス
本リポジトリのオリジナルコード（`src/` および `win/dllmain.cpp` の実装部分）は  
POnozuka によるものです。 

※cri_audio_effect.hにつきましては非公開となりました。