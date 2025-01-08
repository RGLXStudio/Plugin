/*
  ==============================================================================

    Phoenix Saturation Plugin
    Created: 2025-01-08 12:56:21 UTC
    Author:  RGLXStudio

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

// Parameter IDs
#define INPUT_TRIM_ID "input_trim"
#define PROCESS_ID "process"
#define OUTPUT_TRIM_ID "output_trim"
#define BRIGHTNESS_ID "brightness"
#define TYPE_ID "type"

class PhoenixSaturationAudioProcessor : public juce::AudioProcessor,
                                      public juce::AudioProcessorValueTreeState::Listener
{
public:
    // PhoenixProcessor class for audio processing
    class PhoenixProcessor {
    public:
        PhoenixProcessor();
        void setSampleRate(double sampleRate);
        void reset();
        void setMode(float brightness, float type);
        void setProcessing(float amount);
        float processSample(float x)
{
    const float proc = processing * a3;
    
    // Enhanced high-pass filter
    const float x1 = hpf_k * x + 0.9985f * (x - prev_x); // more subtle HPF
    
    // Enhanced pre-saturation with more harmonics
    const float x2 = x1 * (f1 + 0.18f * proc) + x1 * (1.0f + 0.02f * proc * proc);
    
    const float x3 = (!g0) ? x : x2;
    
    // Enhanced saturation stages
    float x4;
    if (model_type == 3) {
        x4 = sat(x2 * proc * 1.35f); // increased from 1.25
    } else {
        x4 = sat(x2 + 0.025f * proc * x2 * x2); // increased from 0.02
    }
    
    const float x5 = sat(x4 * proc * p20 + x3);

    prev_x = x;
    s += (x5 - s) * lpf_k;
    float y = proc * (s - x * p24);

    if (model_type == 3) {
        y *= 0.7f; // increased from 0.65
    }
    
    if (model_type == 4) {
        y = sat(y * 1.25f); // increased from 1.15
    }

    return (y + x) * auto_gain;
}

    private:
        float sat(float x)
{
    switch (sat_type) {
        case 0:  // Opal - Enhanced tanh with more harmonics
            {
                float y = std::tanh(x * 1.5f); // increased from 1.25
                return y + 0.015f * y * y * y; // adds subtle third harmonic
            }
        case 1:  // Gold - Enhanced soft clip with asymmetry
            {
                float pos = x > 0 ? x : x * 0.95f; // slight asymmetry
                return pos / (1.0f + std::abs(pos * 1.25f)); // increased from 1.15
            }
        case 2:  // Sapphire - Enhanced arctangent with more harmonics
            {
                float y = std::atan(x * 1.45f) / 1.45f; // increased from 1.35
                return y + 0.035f * y * y * y; // increased from 0.025
            }
        default:
            return x;
    }
}
        float sr_scale, s, prev_x;
        float hpf_k, lpf_k, a3, f1;
        float p20, p24;
        bool g0;
        int sat_type;
        int model_type;
        float processing;
        float auto_gain_a1, auto_gain_a2, auto_gain;
    };

    PhoenixSaturationAudioProcessor();
    ~PhoenixSaturationAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    
    // Accessor for the parameter state
    juce::AudioProcessorValueTreeState& getState() { return parameters; }

private:
    juce::AudioProcessorValueTreeState parameters;
    PhoenixProcessor leftChannel;
    PhoenixProcessor rightChannel;
    
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    bool prepared = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhoenixSaturationAudioProcessor)
};
