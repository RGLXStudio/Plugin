/*
  ==============================================================================

    Phoenix Tape Plugin
    Created: 2025-01-08 07:06:18 UTC
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
    
// Add or update these in the PhoenixTapeAudioProcessor class declaration
    bool isBusesLayoutSupported (const juce::AudioProcessor::BusesLayout& layouts) const override;
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    
    juce::AudioProcessorValueTreeState& getState() { return parameters; }

private:
    juce::AudioProcessorValueTreeState parameters;
    
    const juce::String INPUT_TRIM_ID = "input_trim";
    const juce::String PROCESS_ID = "process";
    const juce::String OUTPUT_TRIM_ID = "output_trim";
    const juce::String BRIGHTNESS_ID = "brightness";
    const juce::String TYPE_ID = "type";

    class PhoenixProcessor
    {
    public:
        PhoenixProcessor();
        void setSampleRate(double sampleRate);
        void reset();
        void setMode(float brightness, float type);
        void setProcessing(float amount);
        float sat(float x);
        float processSample(float x);

    private:
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
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    bool prepared = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhoenixTapeAudioProcessor)
};