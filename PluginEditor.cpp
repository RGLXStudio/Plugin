/*
  ==============================================================================

    Phoenix Saturation Plugin
    Created: 2025-01-13 15:30:59 UTC
    Author:  RGLXStudio

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

PhoenixSaturationAudioProcessorEditor::PhoenixSaturationAudioProcessorEditor(PhoenixSaturationAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Input Gain
    addAndMakeVisible(inputGainSlider);
    inputGainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    inputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    inputGainLabel.setText("Input", juce::dontSendNotification);
    inputGainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(inputGainLabel);

    // Saturation
    addAndMakeVisible(saturationSlider);
    saturationSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    saturationSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    saturationLabel.setText("Drive", juce::dontSendNotification);
    saturationLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(saturationLabel);

    // Output Gain
    addAndMakeVisible(outputGainSlider);
    outputGainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    outputGainLabel.setText("Output", juce::dontSendNotification);
    outputGainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(outputGainLabel);

    // Type ComboBox
    addAndMakeVisible(typeComboBox);
    typeLabel.setText("Type", juce::dontSendNotification);
    typeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(typeLabel);

    // Brightness ComboBox
    addAndMakeVisible(brightnessComboBox);
    brightnessLabel.setText("Brightness", juce::dontSendNotification);
    brightnessLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(brightnessLabel);

    // Attachments
    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getState(), INPUT_TRIM_ID, inputGainSlider);
    
    saturationAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getState(), PROCESS_ID, saturationSlider);
    
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getState(), OUTPUT_TRIM_ID, outputGainSlider);
    
    typeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getState(), TYPE_ID, typeComboBox);
    
    brightnessAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getState(), BRIGHTNESS_ID, brightnessComboBox);

    setSize(400, 300);
}

PhoenixSaturationAudioProcessorEditor::~PhoenixSaturationAudioProcessorEditor()
{
}

void PhoenixSaturationAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void PhoenixSaturationAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    
    // Top row - ComboBoxes
    auto topRow = area.removeFromTop(60);
    auto comboBoxWidth = topRow.getWidth() / 2;
    
    typeLabel.setBounds(topRow.removeFromLeft(comboBoxWidth).removeFromTop(20));
    typeComboBox.setBounds(topRow.removeFromLeft(comboBoxWidth).removeFromBottom(30));
    
    brightnessLabel.setBounds(topRow.removeFromLeft(comboBoxWidth).removeFromTop(20));
    brightnessComboBox.setBounds(topRow.removeFromLeft(comboBoxWidth).removeFromBottom(30));

    // Main controls row
    auto controlsRow = area.removeFromTop(180);
    auto sliderWidth = controlsRow.getWidth() / 3;
    
    // Input Gain
    auto inputArea = controlsRow.removeFromLeft(sliderWidth);
    inputGainLabel.setBounds(inputArea.removeFromTop(20));
    inputGainSlider.setBounds(inputArea);
    
    // Saturation
    auto saturationArea = controlsRow.removeFromLeft(sliderWidth);
    saturationLabel.setBounds(saturationArea.removeFromTop(20));
    saturationSlider.setBounds(saturationArea);
    
    // Output Gain
    auto outputArea = controlsRow.removeFromLeft(sliderWidth);
    outputGainLabel.setBounds(outputArea.removeFromTop(20));
    outputGainSlider.setBounds(outputArea);
}
