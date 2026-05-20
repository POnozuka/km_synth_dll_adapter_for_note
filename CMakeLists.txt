cmake_minimum_required(VERSION 3.20)
project(KMSynth_CRI_DLL LANGUAGES C CXX)

# 基本設定
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 依存するモジュール: modules/submodules/km_synth
# ルート CMakeLists.txt で add_subdirectory(modules) 済みを確認
# JUCE モジュールのソースグループ設定を有効化
set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON)

# ソースファイルとヘッダファイルを一括管理
file(GLOB_RECURSE CRIDLL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/win/*.cpp"
)

file(GLOB_RECURSE CRIDLL_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/cri/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/win/*.h"
)

add_library(KMSynth_CRI SHARED ${CRIDLL_SOURCES} ${CRIDLL_HEADERS})

# Visual Studio でのソースグループ設定
source_group("Source Files" FILES ${CRIDLL_SOURCES})
source_group("Header Files" FILES ${CRIDLL_HEADERS})

# サブディレクトリごとにグループ分け
source_group("Source Files\\CRI Interface" FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/SynthInterface.cpp"
)
source_group("Source Files\\Windows" FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/win/dllmain.cpp"
)
source_group("Header Files\\CRI Interface" FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/SynthInterface.h"
)
source_group("Header Files\\CRI Public" FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/cri/cri_audio_effect.h"
)

# エクスポートシンボルは .def で管理（GetInterfaceWithVersion のみ）
if(MSVC)
    target_link_options(KMSynth_CRI PRIVATE
        "/DEF:${CMAKE_CURRENT_LIST_DIR}/win/export_symbols.def"
    )
endif()

# インクルードパス
target_include_directories(KMSynth_CRI
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}"
        "${CMAKE_CURRENT_LIST_DIR}/cri" # 追加: CRI 公開ヘッダ
        "${CMAKE_CURRENT_LIST_DIR}/src" # 内部ヘッダ（SynthInterface.h）
)

# プリプロセッサ定義/オプション
if(MSVC)
    target_compile_definitions(KMSynth_CRI PRIVATE _WINDOWS)
    target_compile_options(KMSynth_CRI PRIVATE $<$<CONFIG:Release>:/Gy>)
    target_link_options(KMSynth_CRI PRIVATE
        $<$<CONFIG:Release>:/OPT:REF>
        $<$<CONFIG:Release>:/OPT:ICF>
        $<$<CONFIG:Release>:/DEBUG>
        "/SUBSYSTEM:WINDOWS"
    )
endif()

# 依存ライブラリ
# km_synth は JUCE に依存しており、km_synth をリンクすれば JUCE が解決される
target_link_libraries(KMSynth_CRI PRIVATE km_synth fmidi)

# 出力名（KMSynth_CRI.dll）
set_target_properties(KMSynth_CRI PROPERTIES OUTPUT_NAME "KMSynth_CRI")

# 出力ディレクトリ制御
include(GNUInstallDirs)
set_target_properties(KMSynth_CRI PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>"
)

message(STATUS "[cridll] Build info:")
message(STATUS "  Target               = KMSynth_CRI (DLL target)")
message(STATUS "  Export .def          = ${CMAKE_CURRENT_LIST_DIR}/win/export_symbols.def")
message(STATUS "  Link with            = km_synth")
message(STATUS "  Output DLL           = KMSynth_CRI.dll")
