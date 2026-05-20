#include <windows.h>
#include <stdio.h>

#include <math.h>
#include <string.h>

#include "KMSynthDllAdapter.h"
#include "SynthInterface.h"


/*! [Interface] */

/***************************************************************************
 *		関数定義
 *      Function Definition
 ***************************************************************************/

 /*! [GetInterfaceWithVersion] */
const CriAfxInterfaceWithVersion* GetInterfaceWithVersion(void)
{
	return &criInterfaceWithVersion;
}
/*! [GetInterfaceWithVersion] */

/*! [GetEffectName] */
const char* GetEffectName(const CriAfxInterfaceVersion1Tag* interface_version_tag)
{
	/* バージョン検知引数はインターフェースのバージョン検知のため、コンパイル時に
	 * 使用されますが、プログラム動作時は使用しません。 */
	 /* 補足）ADX2内部ではNULLが渡されます */
	CRIAFX_UNUSED(interface_version_tag);
	/* エフェクト名の命名規則は "<ベンダー名>/<エフェクト名>" となっています。
	 * エフェクトインターフェースは本関数の戻り値で管理されるため、必ず一意に
	 * 識別可能な名前を付けて下さい。 */
	return "POnozuka/KMSynth_CRI";
}
/*! [GetEffectName] */

/*! [CalculateWorkSize] */
// DASynthBetaObjectのメモリ上のサイズを返す関数
int32_t CalculateWorkSize(const CriAfxConfig* config)
{
	if (config == NULL) {
		return -1;
	}

	// ヒープ確保(new)の場合は0で良い。
	// 0でいいはずなのに0だとCreateが呼ばれないため1にする。
	return 1;
}
/*! [CalculateWorkSize] */

/*! [Create] */
void* Create(const CriAfxConfig* config, void* work, int32_t work_size)
{
	// std::nothrowを使用して、メモリ確保に失敗した場合はnullptrを返す。
	auto* synth = new (std::nothrow) km_synth::KMSynthDllAdapter();
	if (!synth) return nullptr;

	// num_input_channelsはステレオのみ対応
	if(config->num_input_channels != 2) {
		delete synth;
		return nullptr; // ステレオ以外は対応しない
	}

	// num_output_channelsはステレオのみ対応
	if(config->num_output_channels != 2) {
		delete synth;
		return nullptr; // ステレオ以外は対応しない
	}

	// sampling_rateとmax_block_samplesを設定
	synth->prepareToPlay(config->sampling_rate, config->max_block_samples);

	// コンフィグパラメータを設定
	//synth->setNumConfigParameters(config->num_config_parameters);
	//synth->setConfigParameters(config->config_parameters);

	// エフェクト名の確認("POnozuka/KMSynth_CRI"であることを確認)
	if(config->effect_name == nullptr || 
	   strcmp(config->effect_name, "POnozuka/KMSynth_CRI") != 0) {
		delete synth;
		return nullptr; // エフェクト名が不正
	}

	// チャンネルコンフィグ(7.1chとか)はステレオのみ対応
	if(config->channel_config != CRIAFX_CHANNEL_CONFIG_STEREO) {
		delete synth;
		return nullptr; // ステレオ以外は対応しない
	}

	return synth;
}
/*! [Create] */

/*! [Destroy] */
void Destroy(void* dsp)
{
	if (!dsp) return;

	delete static_cast<km_synth::KMSynthDllAdapter*>(dsp);
	CRIAFX_UNUSED(dsp);
}
/*! [Destroy] */

/*! [Start] */
void Start(void* dsp)
{
	auto* synth = static_cast<km_synth::KMSynthDllAdapter*>(dsp);
	synth->reset();
}
/*! [Start] */

/*! [Process] */
void Process(void* dsp,
    float const* const* pcm_input, float* const* pcm_output,
    uint32_t num_samples,
    int32_t is_zero_signal_input, int32_t* is_zero_signal_output)
{
    auto* synth = static_cast<km_synth::KMSynthDllAdapter*>(dsp);
    synth->renderNextBlock(pcm_input, pcm_output, num_samples, is_zero_signal_input, is_zero_signal_output);
}

/*! [Process] */

/*! [SetParameter] */
void SetParameter(void* dsp, uint32_t parameter_index, float parameter_value)
{
	auto* synth = static_cast<km_synth::KMSynthDllAdapter*>(dsp);

	synth->setParameter(parameter_index, parameter_value);
}
/*! [SetParameter] */

/*! [GetParameter] */
float GetParameter(void* dsp, uint32_t parameter_index) {
	auto* synth = static_cast<km_synth::KMSynthDllAdapter*>(dsp);

	return synth->getParameter(parameter_index);
}
/*! [GetParameter] */

/*! [applyParameters] */
void ApplyParameters(void* dsp)
{
	// 即時反映のため未使用。
	// auto* synth = static_cast<km_synth::KMSynthDllAdapter*>(dsp);
}
/*! [applyParameters] */

/************ Tools **************/
/*! [GetNumProcessChannels] */
void GetNumProcessChannels(void* dsp, uint32_t* num_input_channels, uint32_t* num_output_channels)
{
	//auto* synth = static_cast<km_synth::KMSynthCore*>(dsp);

	// ステレオのみ対応
	*num_input_channels = 2;
	*num_output_channels = 2;
}
/*! [GetNumProcessChannels] */
