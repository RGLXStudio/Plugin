/*
  ==============================================================================

    Phoenix Saturation Plugin
    Created: 2025-01-08 13:05:15 UTC
    Author:  RGLXStudio

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

//==============================================================================
PhoenixSaturationAudioProcessorEditor::PhoenixSaturationAudioProcessorEditor(PhoenixSaturationAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Set window size
    setSize(400, 300);

    // Initialize sliders
    addAndMakeVisible(inputGainSlider);
    inputGainSlider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    inputGainSlider.setRange(-12.0, 12.0, 0.1);
    inputGainSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 90, 20);
    inputGainSlider.setValue(0.0);

    addAndMakeVisible(saturationSlider);
    saturationSlider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    saturationSlider.setRange(0.0, 100.0, 0.1);
    saturationSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 90, 20);
    saturationSlider.setValue(0.0);

    addAndMakeVisible(outputGainSlider);
    outputGainSlider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    outputGainSlider.setRange(-12.0, 12.0, 0.1);
    outputGainSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 90, 20);
    outputGainSlider.setValue(0.0);

    // Initialize labels with modern font
    addAndMakeVisible(inputGainLabel);
    inputGainLabel.setText("Input Gain", dontSendNotification);
    inputGainLabel.setJustificationType(Justification::centred);
    inputGainLabel.setFont(Font(16.0f));

    addAndMakeVisible(saturationLabel);
    saturationLabel.setText("Saturation", dontSendNotification);
    saturationLabel.setJustificationType(Justification::centred);
    saturationLabel.setFont(Font(16.0f));

    addAndMakeVisible(outputGainLabel);
    outputGainLabel.setText("Output Gain", dontSendNotification);
    outputGainLabel.setJustificationType(Justification::centred);
    outputGainLabel.setFont(Font(16.0f));

    // Initialize ComboBoxes
    addAndMakeVisible(typeComboBox);
    typeComboBox.addItem("Luminescent", 1);
    typeComboBox.addItem("Iridescent", 2);
    typeComboBox.addItem("Radiant", 3);
    typeComboBox.addItem("Luster", 4);
    typeComboBox.addItem("Dark Essence", 5);
    typeComboBox.setSelectedItemIndex(0);

    addAndMakeVisible(brightnessComboBox);
    brightnessComboBox.addItem("Opal", 1);
    brightnessComboBox.addItem("Gold", 2);
    brightnessComboBox.addItem("Sapphire", 3);
    brightnessComboBox.setSelectedItemIndex(0);

    // Initialize ComboBox labels
    addAndMakeVisible(typeLabel);
    typeLabel.setText("Type", dontSendNotification);
    typeLabel.setJustificationType(Justification::centred);
    typeLabel.setFont(Font(16.0f));

    addAndMakeVisible(brightnessLabel);
    brightnessLabel.setText("Brightness", dontSendNotification);
    brightnessLabel.setJustificationType(Justification::centred);
    brightnessLabel.setFont(Font(16.0f));

    // Set up parameter attachments
    inputGainAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getState(), "input_trim", inputGainSlider);
    saturationAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getState(), "process", saturationSlider);
    outputGainAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getState(), "output_trim", outputGainSlider);
    typeAttachment = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getState(), "type", typeComboBox);
    brightnessAttachment = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getState(), "brightness", brightnessComboBox);
}

PhoenixSaturationAudioProcessorEditor::~PhoenixSaturationAudioProcessorEditor()
{
}

void PhoenixSaturationAudioProcessorEditor::paint(Graphics& g)
{
    // Fill background
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

    // Draw title
    g.setColour(Colours::white);
    g.setFont(Font(24.0f).boldened());
    g.drawText("Phoenix Saturation", getLocalBounds().removeFromTop(40),
               Justification::centred, true);
}

void PhoenixSaturationAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    const int margin = 10;
    const int labelHeight = 20;
    const int sliderSize = 100;
    const int comboBoxWidth = 120;
    const int comboBoxHeight = 24;

    // Reserve space for title
    bounds.removeFromTop(40);

    // Layout for sliders
    auto sliderArea = bounds.removeFromTop(sliderSize + labelHeight);
    auto w = (sliderArea.getWidth() - 2 * margin) / 3;

    // Input Gain
    auto inputArea = sliderArea.removeFromLeft(w);
    inputGainLabel.setBounds(inputArea.removeFromTop(labelHeight));
    inputGainSlider.setBounds(inputArea);

    sliderArea.removeFromLeft(margin);

    // Saturation
    auto saturationArea = sliderArea.removeFromLeft(w);
    saturationLabel.setBounds(saturationArea.removeFromTop(labelHeight));
    saturationSlider.setBounds(saturationArea);

    sliderArea.removeFromLeft(margin);

    // Output Gain
    auto outputArea = sliderArea;
    outputGainLabel.setBounds(outputArea.removeFromTop(labelHeight));
    outputGainSlider.setBounds(outputArea);

    // Layout for combo boxes
    bounds.removeFromTop(margin * 2);
    auto comboArea = bounds.removeFromTop(labelHeight + comboBoxHeight);
    auto comboWidth = (comboArea.getWidth() - margin) / 2;

    // Type
    auto typeArea = comboArea.removeFromLeft(comboWidth);
    typeLabel.setBounds(typeArea.removeFromTop(labelHeight));
    typeComboBox.setBounds(typeArea.withSizeKeepingCentre(comboBoxWidth, comboBoxHeight));

    comboArea.removeFromLeft(margin);

    // Brightness
    auto brightnessArea = comboArea;
    brightnessLabel.setBounds(brightnessArea.removeFromTop(labelHeight));
    brightnessComboBox.setBounds(brightnessArea.withSizeKeepingCentre(comboBoxWidth, comboBoxHeight));
}
