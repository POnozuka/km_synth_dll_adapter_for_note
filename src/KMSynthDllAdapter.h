#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <cstdint>
#include <limits>

namespace km_synth
{
class KMSynthDllAdapter
{
public:
    KMSynthDllAdapter();
    ~KMSynthDllAdapter();

    // lifecycle
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void reset();

    // processing
    void renderNextBlock(float const* const* pcm_input,
                         float* const* pcm_output,
                         uint32_t num_samples,
                         int32_t is_zero_signal_input,
                         int32_t* is_zero_signal_output);

    // parameter
    void setParameter(int index, float value);
    float getParameter(int index) const;

private:
    double sampleRateHz = 44100.0;
    double phase = 0.0;
    double phaseDelta = 0.0;

    juce::MidiBuffer midiBuffer;

    bool isBufferZero(const juce::AudioBuffer<float>& outBuffer) const;
};
} // namespace km_synth
