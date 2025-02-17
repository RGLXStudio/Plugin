#pragma once
#include <JuceHeader.h>
#include "SSLCompressor.h"

class SSLCompressorAudioProcessor : public juce::AudioProcessor {
public:
    SSLCompressorAudioProcessor();
    ~SSLCompressorAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState parameters;
    float getCurrentGR() const { return currentGR; }
    float getInputRMS() const { return inputRMS; }
    float getOutputRMS() const { return outputRMS; }

private:
    SSLCompressor compressor;
    float currentGR = 0.0f, inputRMS = 0.0f, outputRMS = 0.0f;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
    juce::AudioBuffer<float> rmsBuffer;
    int rmsWindowSize = 1024, writePos = 0;
    float dcOffset = 0.0f;
    juce::Random random;

    void encodeMidSide(juce::AudioBuffer<float>& buffer);
    void decodeMidSide(juce::AudioBuffer<float>& buffer);
    void applyMix(juce::AudioBuffer<float>& wet, const juce::AudioBuffer<float>& dry, float mix);
    void applyDCOffset(juce::AudioBuffer<float>& buffer);
    void updateMeters(const juce::AudioBuffer<float>& buffer);
    float calculateRMS(const juce::AudioBuffer<float>& buffer);

    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SSLCompressorAudioProcessor)
};