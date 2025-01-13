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
    // Plugin Title Label
    addAndMakeVisible(titleLabel);
    titleLabel.setText("Phoenix Saturation", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    // Input Gain
    addAndMakeVisible(inputGainSlider);
    inputGainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    inputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    inputGainSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    inputGainSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(255, 154, 1));
    inputGainLabel.setText("Input", juce::dontSendNotification);
    inputGainLabel.setJustificationType(juce::Justification::centred);
    inputGainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(inputGainLabel);

    // Saturation
    addAndMakeVisible(saturationSlider);
    saturationSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    saturationSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    saturationSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    saturationSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(255, 154, 1));
    saturationLabel.setText("Drive", juce::dontSendNotification);
    saturationLabel.setJustificationType(juce::Justification::centred);
    saturationLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(saturationLabel);

    // Output Gain
    addAndMakeVisible(outputGainSlider);
    outputGainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    outputGainSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    outputGainSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(255, 154, 1));
    outputGainLabel.setText("Output", juce::dontSendNotification);
    outputGainLabel.setJustificationType(juce::Justification::centred);
    outputGainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(outputGainLabel);

    // Type ComboBox
    addAndMakeVisible(typeComboBox);
    typeComboBox.addItem("Luminescent", 1);
    typeComboBox.addItem("Iridescent", 2);
    typeComboBox.addItem("Radiant", 3);
    typeComboBox.addItem("Luster", 4);
    typeComboBox.addItem("Dark Essence", 5);
    typeComboBox.setJustificationType(juce::Justification::centred);
    typeComboBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(45, 45, 45));
    typeComboBox.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    typeLabel.setText("Type", juce::dontSendNotification);
    typeLabel.setJustificationType(juce::Justification::centred);
    typeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(typeLabel);

    // Brightness ComboBox
    addAndMakeVisible(brightnessComboBox);
    brightnessComboBox.addItem("Opal", 1);
    brightnessComboBox.addItem("Gold", 2);
    brightnessComboBox.addItem("Sapphire", 3);
    brightnessComboBox.setJustificationType(juce::Justification::centred);
    brightnessComboBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(45, 45, 45));
    brightnessComboBox.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    brightnessLabel.setText("Character", juce::dontSendNotification);
    brightnessLabel.setJustificationType(juce::Justification::centred);
    brightnessLabel.setColour(juce::Label::textColourId, juce::Colours::white);
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

    setSize(400, 400);
}

PhoenixSaturationAudioProcessorEditor::~PhoenixSaturationAudioProcessorEditor()
{
}

void PhoenixSaturationAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Fill background
    g.fillAll(juce::Colour(30, 30, 30));

    // Add a subtle gradient
    juce::ColourGradient gradient(
        juce::Colour(40, 40, 40), 0.0f, 0.0f,
        juce::Colour(25, 25, 25), 0.0f, static_cast<float>(getHeight()),
        false);
    g.setGradientFill(gradient);
    g.fillRect(getLocalBounds());

    // Add border
    g.setColour(juce::Colour(60, 60, 60));
    g.drawRect(getLocalBounds(), 1);
}

void PhoenixSaturationAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    
    // Title row
    auto titleRow = area.removeFromTop(50);
    titleLabel.setBounds(titleRow);
    
    area.removeFromTop(10); // Spacing

    // Input row (3 knobs)
    auto inputRow = area.removeFromTop(100);
    auto sliderWidth = inputRow.getWidth() / 3;
    
    auto inputArea = inputRow.removeFromLeft(sliderWidth);
    inputGainLabel.setBounds(inputArea.removeFromTop(20));
    inputGainSlider.setBounds(inputArea);
    
    inputRow.removeFromLeft(10); // Spacing
    
    auto saturationArea = inputRow.removeFromLeft(sliderWidth);
    saturationLabel.setBounds(saturationArea.removeFromTop(20));
    saturationSlider.setBounds(saturationArea);
    
    inputRow.removeFromLeft(10); // Spacing
    
    auto outputArea = inputRow.removeFromLeft(sliderWidth);
    outputGainLabel.setBounds(outputArea.removeFromTop(20));
    outputGainSlider.setBounds(outputArea);
    
    area.removeFromTop(20); // Spacing

    // ComboBox row
    auto comboRow = area.removeFromTop(60);
    auto comboWidth = (comboRow.getWidth() - 10) / 2;
    
    auto typeArea = comboRow.removeFromLeft(comboWidth);
    typeLabel.setBounds(typeArea.removeFromTop(20));
    typeComboBox.setBounds(typeArea);
    
    comboRow.removeFromLeft(10); // Spacing
    
    auto brightnessArea = comboRow.removeFromLeft(comboWidth);
    brightnessLabel.setBounds(brightnessArea.removeFromTop(20));
    brightnessComboBox.setBounds(brightnessArea);
}
