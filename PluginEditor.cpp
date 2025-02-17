#include "PluginEditor.h"

SSLCompressorAudioProcessorEditor::SSLCompressorAudioProcessorEditor(SSLCompressorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), grMeter(p) {
    
    // Slider setup
    auto makeSlider = [this](juce::Slider& slider, const juce::String& paramID) {
        addAndMakeVisible(slider);
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    };

    makeSlider(thresholdSlider, "threshold");
    makeSlider(ratioSlider, "ratio");
    makeSlider(attackSlider, "attack");
    makeSlider(releaseSlider, "release");
    makeSlider(makeupSlider, "makeup");
    makeSlider(mixSlider, "mix");
    makeSlider(driveSlider, "drive");

    // Button setup
    addAndMakeVisible(hpfButton);
    addAndMakeVisible(autoReleaseButton);
    addAndMakeVisible(midSideButton);
    hpfButton.setButtonText("HPF");
    autoReleaseButton.setButtonText("Auto Rel");
    midSideButton.setButtonText("M/S");

    // Meter
    addAndMakeVisible(grMeter);

    // Attachments
    thresholdAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "threshold", thresholdSlider);
    ratioAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "ratio", ratioSlider);
    // ... repeat for other parameters ...

    setSize(800, 400);
}

void SSLCompressorAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff303030));
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawText("SSL Bus Compressor", getLocalBounds().removeFromTop(30), juce::Justification::centred);
}

void SSLCompressorAudioProcessorEditor::resized() {
    auto area = getLocalBounds().reduced(10);
    grMeter.setBounds(area.removeFromRight(30));
    
    auto controls = area.removeFromTop(150);
    thresholdSlider.setBounds(controls.removeFromLeft(100));
    ratioSlider.setBounds(controls.removeFromLeft(100));
    attackSlider.setBounds(controls.removeFromLeft(100));
    releaseSlider.setBounds(controls.removeFromLeft(100));
    makeupSlider.setBounds(controls.removeFromLeft(100));
    
    auto bottomRow = area.removeFromTop(50);
    hpfButton.setBounds(bottomRow.removeFromLeft(80));
    autoReleaseButton.setBounds(bottomRow.removeFromLeft(80));
    midSideButton.setBounds(bottomRow.removeFromLeft(80));
    mixSlider.setBounds(bottomRow.removeFromLeft(100));
    driveSlider.setBounds(bottomRow.removeFromLeft(100));
}