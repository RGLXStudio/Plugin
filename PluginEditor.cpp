/*
  ==============================================================================

    Phoenix Tape Plugin
    Created: 2025-01-08 06:32:36 UTC
    Author:  RGLXStudio

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

PhoenixTapeAudioProcessorEditor::PhoenixTapeAudioProcessorEditor (PhoenixTapeAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Input Trim
    addAndMakeVisible(inputTrimSlider);
    inputTrimSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    inputTrimSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 20);
    
    addAndMakeVisible(inputTrimLabel);
    inputTrimLabel.setText("Input Trim", juce::dontSendNotification);
    inputTrimLabel.setJustificationType(juce::Justification::centred);
    
    // Process
    addAndMakeVisible(processSlider);
    processSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    processSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 20);
    
    addAndMakeVisible(processLabel);
    processLabel.setText("Process", juce::dontSendNotification);
    processLabel.setJustificationType(juce::Justification::centred);
    
    // Output Trim
    addAndMakeVisible(outputTrimSlider);
    outputTrimSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputTrimSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 20);
    
    addAndMakeVisible(outputTrimLabel);
    outputTrimLabel.setText("Output Trim", juce::dontSendNotification);
    outputTrimLabel.setJustificationType(juce::Justification::centred);
    
    // Brightness
    addAndMakeVisible(brightnessBox);
    brightnessBox.addItem("Opal", 1);
    brightnessBox.addItem("Gold", 2);
    brightnessBox.addItem("Sapphire", 3);
    
    addAndMakeVisible(brightnessLabel);
    brightnessLabel.setText("Brightness", juce::dontSendNotification);
    brightnessLabel.setJustificationType(juce::Justification::centred);
    
    // Type
    addAndMakeVisible(typeBox);
    typeBox.addItem("Luminiscent", 1);
    typeBox.addItem("Iridescent", 2);
    typeBox.addItem("Radiant", 3);
    typeBox.addItem("Luster", 4);
    typeBox.addItem("Dark Essence", 5);
    
    addAndMakeVisible(typeLabel);
    typeLabel.setText("Type", juce::dontSendNotification);
    typeLabel.setJustificationType(juce::Justification::centred);
    
    // Attachments
    inputTrimAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getState(), "input_trim", inputTrimSlider);
    processAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getState(), "process", processSlider);
    outputTrimAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getState(), "output_trim", outputTrimSlider);
    brightnessAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getState(), "brightness", brightnessBox);
    typeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getState(), "type", typeBox);

    setSize (400, 300);
}

PhoenixTapeAudioProcessorEditor::~PhoenixTapeAudioProcessorEditor()
{
}

void PhoenixTapeAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void PhoenixTapeAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    const int margin = 10;
    const int controlHeight = 100;
    const int labelHeight = 20;
    const int comboBoxWidth = 120;
    const int comboBoxHeight = 24;
    
    // Top row - Input Trim, Process, Output Trim
    auto topRow = area.removeFromTop(controlHeight + labelHeight).reduced(margin);
    auto sliderWidth = (topRow.getWidth() - margin * 2) / 3;
    
    // Input Trim
    auto inputTrimArea = topRow.removeFromLeft(sliderWidth);
    inputTrimLabel.setBounds(inputTrimArea.removeFromTop(labelHeight));
    inputTrimSlider.setBounds(inputTrimArea);
    
    topRow.removeFromLeft(margin);
    
    // Process
    auto processArea = topRow.removeFromLeft(sliderWidth);
    processLabel.setBounds(processArea.removeFromTop(labelHeight));
    processSlider.setBounds(processArea);
    
    topRow.removeFromLeft(margin);
    
    // Output Trim
    auto outputTrimArea = topRow;
    outputTrimLabel.setBounds(outputTrimArea.removeFromTop(labelHeight));
    outputTrimSlider.setBounds(outputTrimArea);
    
    // Bottom row - Brightness and Type
    area.removeFromTop(margin);
    auto bottomRow = area.removeFromTop(labelHeight + comboBoxHeight).reduced(margin);
    auto comboArea = (bottomRow.getWidth() - margin) / 2;
    
    // Brightness
    auto brightnessArea = bottomRow.removeFromLeft(comboArea);
    brightnessLabel.setBounds(brightnessArea.removeFromTop(labelHeight));
    brightnessBox.setBounds(brightnessArea.withSizeKeepingCentre(comboBoxWidth, comboBoxHeight));
    
    bottomRow.removeFromLeft(margin);
    
    // Type
    auto typeArea = bottomRow;
    typeLabel.setBounds(typeArea.removeFromTop(labelHeight));
    typeBox.setBounds(typeArea.withSizeKeepingCentre(comboBoxWidth, comboBoxHeight));
}