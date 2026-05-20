#include "KMSynthDllAdapter.h"

#include <cmath>

#include <fmidi/decode_message.h>
#include <fmidi/utils.h>

namespace km_synth
{
KMSynthDllAdapter::KMSynthDllAdapter()
{
}

KMSynthDllAdapter::~KMSynthDllAdapter() = default;

void KMSynthDllAdapter::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);

    sampleRateHz = sampleRate;
    phase = 0.0;

    constexpr double freqHz = 440.0;
    phaseDelta = juce::MathConstants<double>::twoPi * freqHz / sampleRateHz;
}

void KMSynthDllAdapter::reset()
{
    phase = 0.0;
}

// processing
void KMSynthDllAdapter::renderNextBlock(float const* const* pcm_input,
                                        float* const* pcm_output,
                                        uint32_t num_samples,
                                        int32_t is_zero_signal_input,
                                        int32_t* is_zero_signal_output)
{
    const int num_channels = 2;
    juce::AudioBuffer<float> outBuffer(const_cast<float* const*>(pcm_output), num_channels, (int)num_samples);

    const bool passThroughInput = !is_zero_signal_input;
    if (passThroughInput)
    {
        for (int ch = 0; ch < num_channels; ++ch)
        {
            const float* in = pcm_input[ch];
            float* out = outBuffer.getWritePointer(ch);
            if (in != nullptr && out != nullptr)
                juce::FloatVectorOperations::copy(out, in, (int)num_samples);
            else if (out != nullptr)
                juce::FloatVectorOperations::clear(out, (int)num_samples);
        }
    }
    else
    {
        outBuffer.clear(0, (int)num_samples);
    }

    {
        juce::MidiBuffer midi;
        midi.addEvents(midiBuffer, 0, outBuffer.getNumSamples(), 0);
        midiBuffer.clear();
        juce::ignoreUnused(midi);
    }

    for (int sample = 0; sample < (int)num_samples; ++sample)
    {
        const float v = static_cast<float>(std::sin(phase));
        phase += phaseDelta;
        if (phase >= juce::MathConstants<double>::twoPi)
            phase -= juce::MathConstants<double>::twoPi;

        for (int ch = 0; ch < num_channels; ++ch)
        {
            float* out = outBuffer.getWritePointer(ch);
            out[sample] += v;
        }
    }

    // Check if output is silent
    *is_zero_signal_output = isBufferZero(outBuffer) ? 1 : 0;
}

// parameter
void KMSynthDllAdapter::setParameter(int index, float value)
{
    if (index == 0xFFFF)
    {
        if (FMIDI::isValid(value))
        {
            auto vec = FMIDI::decodeMessage(value);
            juce::MidiMessage msg(vec.data(), (int)vec.size());
            midiBuffer.addEvent(msg, 0);
        }
        return;
    }

    juce::ignoreUnused(index, value);
}

float KMSynthDllAdapter::getParameter(int index) const
{
    juce::ignoreUnused(index);
    return NAN;
}

//======================================================================
bool KMSynthDllAdapter::isBufferZero(const juce::AudioBuffer<float>& outBuffer) const
{
    float eps = 1.0e-12f;
    for (int ch = 0; ch < outBuffer.getNumChannels(); ++ch)
    {
        const float* rp = outBuffer.getReadPointer(ch);
        for (int i = 0; i < outBuffer.getNumSamples(); ++i)
        {
            if (std::fabs(rp[i]) > eps)
            {
                return false;
            }
        }
    }
    return true;
}
} // namespace km_synth
