/*
  ==============================================================================

    Phoenix Saturation Plugin
    Created: 2025-01-14 06:52:24 UTC
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
    PhoenixProcessor() : processing(0.0f), sat_type(0), model_type(0) {}
    void setProcessing(float amount) { processing = amount * 0.01f; }
    void setMode(float brightness, float type) {
        sat_type = static_cast<int>(brightness);
        model_type = static_cast<int>(type);
    }
    void setSampleRate(double) {} // Not used
    void reset() {} // Not used
    float processSample(float x);
private:
    float processing;
    int sat_type;    // Brightness: 0=Opal, 1=Gold, 2=Sapphire
    int model_type;  // Type: 0=Luminescent, 1=Iridescent, etc.
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
