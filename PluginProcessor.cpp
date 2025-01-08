/*
  ==============================================================================

    Phoenix Saturation Plugin
    Created: 2025-01-08 11:04:32 UTC
    Author:  RGLXStudio

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PhoenixTapeAudioProcessorEditor::VUMeter::VUMeter()
{
    startTimerHz(30); // 30 fps update rate
}

void PhoenixTapeAudioProcessorEditor::VUMeter::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const float meterHeight = bounds.getHeight() * (0.1f + 0.9f * displayLevel);
    
    // Background
    g.setColour(juce::Colour(0xFF2D3436));
    g.fillRect(bounds);

    // Meter gradient
    juce::ColourGradient gradient(
        juce::Colour(0xFF27AE60),  // Green
        bounds.getBottomLeft(),
        juce::Colour(0xFFF27121),  // Orange
        bounds.getTopLeft(),
        false);
    gradient.addColour(0.7, juce::Colour(0xFFE74C3C)); // Red
    
    g.setGradientFill(gradient);
    g.fillRect(bounds.withHeight(meterHeight).withBottomY(bounds.getBottom()));
}

void PhoenixTapeAudioProcessorEditor::VUMeter::resized()
{
}

void PhoenixTapeAudioProcessorEditor::VUMeter::timerCallback()
{
    const float decay = 0.75f;
    displayLevel = displayLevel * decay + level * (1.0f - decay);
    repaint();
}

void PhoenixTapeAudioProcessorEditor::VUMeter::setLevel(float newLevel)
{
    level = newLevel;
}

//==============================================================================
PhoenixTapeAudioProcessorEditor::PhoenixTapeAudioProcessorEditor(PhoenixTapeAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Set modern look and feel
    setLookAndFeel(&modernLookAndFeel);
    
    // Input Trim
    addAndMakeVisible(inputTrimSlider);
    inputTrimSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    inputTrimSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    inputTrimSlider.setTextValueSuffix(" dB");
    
    addAndMakeVisible(inputTrimLabel);
    inputTrimLabel.setText("Input Trim", juce::dontSendNotification);
    inputTrimLabel.setJustificationType(juce::Justification::centred);
    
    // Process
    addAndMakeVisible(processSlider);
    processSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    processSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    processSlider.setTextValueSuffix("%");
    
    addAndMakeVisible(processLabel);
    processLabel.setText("Process", juce::dontSendNotification);
    processLabel.setJustificationType(juce::Justification::centred);
    
    // Output Trim
    addAndMakeVisible(outputTrimSlider);
    outputTrimSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputTrimSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    outputTrimSlider.setTextValueSuffix(" dB");
    
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
    typeBox.addItem("Luminescent", 1);
    typeBox.addItem("Iridescent", 2);
    typeBox.addItem("Radiant", 3);
    typeBox.addItem("Luster", 4);
    typeBox.addItem("Dark Essence", 5);
    
    addAndMakeVisible(typeLabel);
    typeLabel.setText("Type", juce::dontSendNotification);
    typeLabel.setJustificationType(juce::Justification::centred);
    
    // VU Meters
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);
    
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

    setSize(500, 400);
}

PhoenixTapeAudioProcessorEditor::~PhoenixTapeAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void PhoenixTapeAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background fill
    g.fillAll(juce::Colour(0xFF1E272E));

    // Logo/title
    g.setColour(juce::Colour(0xFFF27121));
    g.setFont(juce::Font(24.0f, juce::Font::bold));
    g.drawText("Phoenix Saturation", getLocalBounds().reduced(10),
               juce::Justification::top, true);
}

void PhoenixTapeAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    const int margin = 10;
    const int topMargin = 40; // Space for title
    const int controlHeight = 100;
    const int labelHeight = 20;
    const int meterWidth = 30;
    const int comboBoxWidth = 120;
    const int comboBoxHeight = 24;
    
    bounds.removeFromTop(topMargin);
    
    // Meters
    auto meterBounds = bounds.removeFromRight(meterWidth);
    outputMeter.setBounds(meterBounds);
    bounds.removeFromRight(margin);
    meterBounds = bounds.removeFromLeft(meterWidth);
    inputMeter.setBounds(meterBounds);
    bounds.removeFromLeft(margin);
    
    // Top row - Input Trim, Process, Output Trim
    auto topRow = bounds.removeFromTop(controlHeight + labelHeight).reduced(margin);
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
    bounds.removeFromTop(margin * 2);
    auto bottomRow = bounds.removeFromTop(labelHeight + comboBoxHeight).reduced(margin);
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

