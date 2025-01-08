/*
  ==============================================================================

    Phoenix Saturation Plugin
    Created: 2025-01-08 11:02:36 UTC
    Author:  RGLXStudio

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class PhoenixTapeAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit PhoenixTapeAudioProcessorEditor (PhoenixTapeAudioProcessor&);
    ~PhoenixTapeAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // Custom LookAndFeel class for modern styling
    class ModernLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        ModernLookAndFeel()
        {
            setColour(juce::Slider::thumbColourId, juce::Colour(0xFFF27121));
            setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFFF27121));
            setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xFF2D3436));
            setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF2D3436));
            setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFFF27121));
            setColour(juce::ComboBox::buttonColourId, juce::Colour(0xFFF27121));
        }
    };

    // Reference to the processor
    PhoenixTapeAudioProcessor& audioProcessor;

    // Modern look and feel instance
    ModernLookAndFeel modernLookAndFeel;

    // Sliders
    juce::Slider inputTrimSlider;
    juce::Slider processSlider;
    juce::Slider outputTrimSlider;

    // Labels
    juce::Label inputTrimLabel;
    juce::Label processLabel;
    juce::Label outputTrimLabel;
    juce::Label brightnessLabel;
    juce::Label typeLabel;

    // Combo boxes
    juce::ComboBox brightnessBox;
    juce::ComboBox typeBox;

    // Value tree attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputTrimAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> processAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputTrimAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> brightnessAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment;

    // VU Meter component
    class VUMeter : public juce::Component,
                   public juce::Timer
    {
    public:
        VUMeter();
        void paint(juce::Graphics& g) override;
        void resized() override;
        void timerCallback() override;
        void setLevel(float newLevel);
    
    private:
        float level = 0.0f;
        float displayLevel = 0.0f;
    };

    VUMeter inputMeter;
    VUMeter outputMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhoenixTapeAudioProcessorEditor)
};
