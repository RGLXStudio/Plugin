/*
  ==============================================================================

    Phoenix Saturation Plugin
    Created: 2025-01-08 12:54:16 UTC
    Author:  RGLXStudio

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class PhoenixSaturationAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit PhoenixSaturationAudioProcessorEditor(PhoenixSaturationAudioProcessor&);
    ~PhoenixSaturationAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // Reference to our processor
    PhoenixSaturationAudioProcessor& audioProcessor;

    // Sliders
    juce::Slider inputGainSlider;
    juce::Slider saturationSlider;
    juce::Slider outputGainSlider;

    // Labels
    juce::Label inputGainLabel;
    juce::Label saturationLabel;
    juce::Label outputGainLabel;
    juce::Label typeLabel;
    juce::Label brightnessLabel;

    // Combo boxes
    juce::ComboBox typeComboBox;
    juce::ComboBox brightnessComboBox;

    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> saturationAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> brightnessAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhoenixSaturationAudioProcessorEditor)
};
