/*
  ==============================================================================

    Phoenix Saturation Plugin
    Created: 2025-01-08 08:55:04 UTC
    Author:  RGLXStudio

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class PhoenixTapeAudioProcessor : public juce::AudioProcessor,
                                 public juce::AudioProcessorValueTreeState::Listener
{
public:
    PhoenixTapeAudioProcessor();
    ~PhoenixTapeAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    bool isBusesLayoutSupported (const juce::AudioProcessor::BusesLayout& layouts) const override;
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    
    juce::AudioProcessorValueTreeState& getState() { return parameters; }

private:
    juce::AudioProcessorValueTreeState parameters;
    
    // Parameter IDs as static constexpr for better optimization
    static constexpr const char* INPUT_TRIM_ID = "input_trim";
    static constexpr const char* PROCESS_ID = "process";
    static constexpr const char* OUTPUT_TRIM_ID = "output_trim";
    static constexpr const char* BRIGHTNESS_ID = "brightness";
    static constexpr const char* TYPE_ID = "type";

    class PhoenixProcessor
    {
    public:
        PhoenixProcessor();
        void setSampleRate(double sampleRate);
        void reset();
        void setMode(float brightness, float type);
        void setProcessing(float amount);
        float processSample(float x);

    private:
        float sat(float x);
        
        // Processing parameters
        float sr_scale;
        float s;
        float prev_x;
        float hpf_k;
        float lpf_k;
        float a3;
        float f1;
        float p20;
        float p24;
        bool g0;
        int sat_type;
        int model_type;
        float processing;
        float auto_gain_a1;
        float auto_gain_a2;
        float auto_gain;
    };

    PhoenixProcessor leftChannel, rightChannel;
    double currentSampleRate{44100.0};
    int currentBlockSize{512};
    std::atomic<bool> prepared{false};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhoenixTapeAudioProcessor)
};
