#ifndef SYNTHINTERFACE_H_INCLUDED
#define SYNTHINTERFACE_H_INCLUDED

#include "cri_audio_effect.h"

#define MEMORY_ALIGN_SIZE							16

// Index
#define DASYNTHBETA_BASE_WAVE_GAIN_INDEX			0
#define DASYNTHBETA_EVEN_WAVE_GAIN_INDEX			1
#define DASYNTHBETA_ODD_WAVE_GAIN_INDEX				2
#define DASYNTHBETA_NUM_PARAMETERS					3

// DefaultValue
#define DASYNTHBETA_BASE_WAVE_GAIN_DEFAULT			1.0f
#define DASYNTHBETA_EVEN_WAVE_GAIN_DEFAULT			0.5f
#define DASYNTHBETA_ODD_WAVE_GAIN_DEFAULT			0.5f

#define BASE_FREQ 440


/* Connect to Interface(written by C-Language */
#ifdef __cplusplus
extern "C" {
#endif
	/* Function Definition */
	const CriAfxInterfaceWithVersion* GetInterfaceWithVersion(void);

	/* Prototype Functions */
	const char* GetEffectName(const CriAfxInterfaceVersion1Tag* interface_version_tag);
	int32_t CalculateWorkSize(const CriAfxConfig* config);
	void* Create(const CriAfxConfig* config, void* work, int32_t work_size);
	void Destroy(void* dsp);
	void Start(void* dsp);
	void Process(void* dsp,
		float const* const* pcm_input, float* const* pcm_output, uint32_t num_samples,
		int32_t is_zero_signal_input, int32_t* is_zero_signal_output);
	void SetParameter(void* dsp, uint32_t parameter_index, float parameter_value);
	float GetParameter(void* dsp, uint32_t parameter_index);
	void ApplyParameters(void* dsp);
	void GetNumProcessChannels(void* dsp, uint32_t* num_input_channels, uint32_t* num_output_channels);

	/* Interface */
	static const CriAfxInterface criInterface = {
		GetEffectName,
		CalculateWorkSize,
		Create,
		Destroy,
		Start,
		Process,
		SetParameter,
		GetParameter,
		ApplyParameters,
		GetNumProcessChannels,
	};
	/* Variable Definition */
	static const CriAfxInterfaceWithVersion criInterfaceWithVersion = {
		/*!
		 * \brief インターフェースバージョン
		 * \par 説明:
		 * 必ずこのソースから参照しているヘッダファイルのマクロを使用して下さい。
		 */
		CRIAFX_INTERFACE_VERSION,
		/*!
		 * \brief インターフェース定義のアドレス
		 */
		&criInterface,
	};
#ifdef __cplusplus
}
#endif

#endif /* SYNTHINTERFACE_H_INCLUDED */
