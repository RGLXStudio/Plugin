/*==============================================================================
    Phoenix Saturation Plugin
    Created: 2025-01-13 15:30:59 UTC
    Author:  RGLXStudio
==============================================================================*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

PhoenixSaturationAudioProcessorEditor::PhoenixSaturationAudioProcessorEditor(PhoenixSaturationAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Plugin Title Label
    addAndMakeVisible(titleLabel);
    titleLabel.setText("Phoenix Saturation", juce::dontSendNotification);
    titleLabel.setFont(juce::Font("Avenir", 26.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    // Input Gain
    addAndMakeVisible(inputGainSlider);
    inputGainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    inputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    inputGainSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    inputGainSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffF57C00)); // Orange color
    inputGainSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
    inputGainLabel.setText("Input", juce::dontSendNotification);
    inputGainLabel.setJustificationType(juce::Justification::centred);
    inputGainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(inputGainLabel);

    // Saturation
    addAndMakeVisible(saturationSlider);
    saturationSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    saturationSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    saturationSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    saturationSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffF57C00)); // Orange color
    saturationSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
    saturationLabel.setText("Drive", juce::dontSendNotification);
    saturationLabel.setJustificationType(juce::Justification::centred);
    saturationLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(saturationLabel);

    // Output Gain
    addAndMakeVisible(outputGainSlider);
    outputGainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    outputGainSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    outputGainSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffF57C00)); // Orange color
    outputGainSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
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
    typeComboBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2E2E2E));
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
    brightnessComboBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2E2E2E));
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

    setSize(420, 420);
}

PhoenixSaturationAudioProcessorEditor::~PhoenixSaturationAudioProcessorEditor()
{
}

void PhoenixSaturationAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Fill background with a gradient
    juce::ColourGradient backgroundGradient(juce::Colour(0xff1e1e1e), 0.0f, 0.0f,
                                            juce::Colour(0xff121212), 0.0f, static_cast<float>(getHeight()), false);
    g.setGradientFill(backgroundGradient);
    g.fillRect(getLocalBounds());

    // Add a subtle gradient
    juce::ColourGradient gradient(
        juce::Colour(0xff2e2e2e), 0.0f, 0.0f,
        juce::Colour(0xff1e1e1e), 0.0f, static_cast<float>(getHeight()),
        false);
    g.setGradientFill(gradient);
    g.fillRect(getLocalBounds());

    // Add border with shadow effect
    g.setColour(juce::Colour(0xff3e3e3e));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), 10.0f, 2.0f);
}

void PhoenixSaturationAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    
    // Title row
    auto titleRow = area.removeFromTop(60);
    titleLabel.setBounds(titleRow);
    
    area.removeFromTop(10); // Spacing

    // Input row (3 knobs)
    auto inputRow = area.removeFromTop(120);
    auto sliderWidth = inputRow.getWidth() / 3;
    
    auto inputArea = inputRow.removeFromLeft(sliderWidth);
    inputGainLabel.setBounds(inputArea.removeFromTop(20));
    inputGainSlider.setBounds(inputArea.reduced(10));
    
    inputRow.removeFromLeft(10); // Spacing
    
    auto saturationArea = inputRow.removeFromLeft(sliderWidth);
    saturationLabel.setBounds(saturationArea.removeFromTop(20));
    saturationSlider.setBounds(saturationArea.reduced(10));
    
    inputRow.removeFromLeft(10); // Spacing
    
    auto outputArea = inputRow.removeFromLeft(sliderWidth);
    outputGainLabel.setBounds(outputArea.removeFromTop(20));
    outputGainSlider.setBounds(outputArea.reduced(10));
    
    area.removeFromTop(20); // Spacing

    // ComboBox row
    auto comboRow = area.removeFromTop(80);
    auto comboWidth = (comboRow.getWidth() - 20) / 2;
    
    auto typeArea = comboRow.removeFromLeft(comboWidth);
    typeLabel.setBounds(typeArea.removeFromTop(20));
    typeComboBox.setBounds(typeArea.reduced(10));
    
    comboRow.removeFromLeft(20); // Spacing
    
    auto brightnessArea = comboRow.removeFromLeft(comboWidth);
    brightnessLabel.setBounds(brightnessArea.removeFromTop(20));
    brightnessComboBox.setBounds(brightnessArea.reduced(10));
}
