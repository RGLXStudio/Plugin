/*==============================================================================
    Phoenix Saturation Plugin
    Created: 2025-01-14 06:33:38 UTC
    Author: RGLXStudio
==============================================================================*/

#pragma once
#include <JuceHeader.h>

// Parameter IDs
#define INPUT_TRIM_ID "input_trim"
#define PROCESS_ID "process"
#define OUTPUT_TRIM_ID "output_trim"
#define BRIGHTNESS_ID "brightness"
#define TYPE_ID "type"

class PhoenixSaturationAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit PhoenixSaturationAudioProcessorEditor(PhoenixSaturationAudioProcessor&);
    ~PhoenixSaturationAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    PhoenixSaturationAudioProcessor& audioProcessor;

    juce::Label titleLabel;
    juce::Slider inputGainSlider, saturationSlider, outputGainSlider;
    juce::Label inputGainLabel, saturationLabel, outputGainLabel;
    juce::ComboBox typeComboBox, brightnessComboBox;
    juce::Label typeLabel, brightnessLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> saturationAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> brightnessAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhoenixSaturationAudioProcessorEditor)
};
