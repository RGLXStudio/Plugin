#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class SSLCompressorAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    SSLCompressorAudioProcessorEditor(SSLCompressorAudioProcessor&);
    ~SSLCompressorAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SSLCompressorAudioProcessor& audioProcessor;

    // Controls
    juce::Slider thresholdSlider, ratioSlider, attackSlider, releaseSlider, makeupSlider, mixSlider, driveSlider;
    juce::ToggleButton hpfButton, autoReleaseButton, midSideButton;
    
    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttach, ratioAttach, 
        attackAttach, releaseAttach, makeupAttach, mixAttach, driveAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> hpfAttach, autoReleaseAttach, midSideAttach;

    // Meters
    struct GRMeter : public juce::Component, public juce::Timer {
        GRMeter(SSLCompressorAudioProcessor& p) : processor(p) { startTimerHz(30); }
        void paint(juce::Graphics& g) override {
            g.setColour(juce::Colours::red);
            g.fillRect(getLocalBounds().removeFromLeft(juce::jmap(processor.getCurrentGR(), -24.0f, 0.0f, 0.0f, (float)getWidth())));
        }
        void timerCallback() override { repaint(); }
        SSLCompressorAudioProcessor& processor;
    } grMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SSLCompressorAudioProcessorEditor)
};