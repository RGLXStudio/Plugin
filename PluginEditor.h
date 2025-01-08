/*
  ==============================================================================

    Phoenix Tape Plugin
    Created: 2025-01-08 06:32:36 UTC
    Author:  RGLXStudio

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class PhoenixTapeAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    PhoenixTapeAudioProcessorEditor (PhoenixTapeAudioProcessor&);
    ~PhoenixTapeAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    PhoenixTapeAudioProcessor& audioProcessor;
    
    juce::Slider inputTrimSlider;
    juce::Slider processSlider;
    juce::Slider outputTrimSlider;
    juce::ComboBox brightnessBox;
    juce::ComboBox typeBox;
    
    juce::Label inputTrimLabel;
    juce::Label processLabel;
    juce::Label outputTrimLabel;
    juce::Label brightnessLabel;
    juce::Label typeLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputTrimAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> processAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputTrimAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> brightnessAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhoenixTapeAudioProcessorEditor)
};