/*
  ==============================================================================

    Phoenix Saturation Plugin
    Created: 2025-01-14 07:39:55 UTC
    Author:  RGLXStudio

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

PhoenixSaturationAudioProcessor::PhoenixProcessor::PhoenixProcessor()
    : processing(0.0f)
    , sat_type(0)
    , model_type(0)
    , a3(1.0f)
    , f1(0.5f)
    , p20(0.25f)
    , p24(0.1f)
    , auto_gain_a1(-0.5f)
    , auto_gain_a2(0.1f)
{
}

void PhoenixSaturationAudioProcessor::PhoenixProcessor::setProcessing(float amount)
{
    // Use exponential curve for more natural drive control
    // and scale it to match JSFX behavior
    processing = std::pow(amount * 0.01f, 1.5f) * 0.01f;
}

float PhoenixSaturationAudioProcessor::PhoenixProcessor::processSample(float x)
{
    // If processing is 0, bypass the effect completely
    if (processing <= 0.0f)
        return x;
        
    // Apply drive with auto-gain compensation
    // Scale the processing for auto-gain calculation
    float proc_scaled = processing * 0.5f;
    float auto_gain = 1.0f + proc_scaled * auto_gain_a1 + proc_scaled * proc_scaled * auto_gain_a2;
    
    // Back to original drive scaling but with better curve
    float drive = processing * 24.0f * a3;
    
    x *= std::pow(10.0f, drive * 0.05f) * auto_gain;
    
    // Hard clip
    if (x > 1.0f) x = 1.0f;
    if (x < -1.0f) x = -1.0f;
    
    float xx = x * x;
    float y = 0.0f;
    
    // Apply the exact polynomial coefficients from JSFX
    switch (sat_type) {
        case 0: // Opal
            y = x * (2.86008989f + xx * (-4.5530714f + xx * (1.45923194f + xx * -0.21303786f)));
            break;
        case 1: // Gold
            y = x * (1.42392761f + xx * (1.56719233f + xx * (-10.98489801f + xx * (11.45169548f + -3.45056185f * xx))));
            break;
        case 2: // Sapphire
            y = x * (1.95790007f + xx * (2.15489826f + xx * (-15.10425859f + xx * (15.74610246f + -4.74452895f * xx))));
            break;
    }
    
    // Apply presence, harmonics balance and output stage
    return y * f1 * (1.0f + p20 * xx) * (1.0f + p24 * xx);
}

void PhoenixSaturationAudioProcessor::PhoenixProcessor::setMode(float brightness, float type)
{
    sat_type = static_cast<int>(brightness);
    model_type = static_cast<int>(type);
    
    // Set parameters based on type exactly like JSFX
    switch (model_type) {
        case 0: // Luminescent
            a3 = 1.0f;
            f1 = 0.5f;
            p20 = 0.25f;
            p24 = 0.1f;
            auto_gain_a1 = -0.5f;
            auto_gain_a2 = 0.1f;
            break;
            
        case 1: // Iridescent
            a3 = 0.75f;
            f1 = 0.625f;
            p20 = 0.375f;
            p24 = 0.08f;
            auto_gain_a1 = -0.6f;
            auto_gain_a2 = 0.15f;
            break;
            
        case 2: // Radiant
            a3 = 1.5f;
            f1 = 0.75f;
            p20 = 0.5f;
            p24 = 0.15f;
            auto_gain_a1 = -0.55f;
            auto_gain_a2 = 0.125f;
            break;
            
        case 3: // Luster
            a3 = 1.0f;
            f1 = 0.6875f;
            p20 = 0.27343899f;
            p24 = 0.1171875f;
            auto_gain_a1 = -0.712f;
            auto_gain_a2 = 0.172f;
            break;
            
        case 4: // Dark Essence
            a3 = 0.375f;
            f1 = 0.75f;
            p20 = 0.5625f;
            p24 = 0.0125f;
            auto_gain_a1 = -0.636f;
            auto_gain_a2 = 0.17f;
            break;
    }
}

PhoenixSaturationAudioProcessor::PhoenixSaturationAudioProcessor()
    : AudioProcessor(BusesProperties()
                    .withInput("Input", AudioChannelSet::stereo(), true)
                    .withOutput("Output", AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", {
          std::make_unique<AudioParameterFloat>(INPUT_TRIM_ID, "Input Trim",
                                              NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
                                              0.0f),
          std::make_unique<AudioParameterFloat>(PROCESS_ID, "Process",
                                              NormalisableRange<float>(0.0f, 100.0f, 0.1f),
                                              0.0f),
          std::make_unique<AudioParameterFloat>(OUTPUT_TRIM_ID, "Output Trim",
                                              NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
                                              0.0f),
          std::make_unique<AudioParameterChoice>(BRIGHTNESS_ID, "Brightness",
                                               StringArray{"Opal", "Gold", "Sapphire"},
                                               0),
          std::make_unique<AudioParameterChoice>(TYPE_ID, "Type",
                                               StringArray{"Luminescent", "Iridescent", "Radiant", "Luster", "Dark Essence"},
                                               0)
      })
{
    parameters.addParameterListener(PROCESS_ID, this);
    parameters.addParameterListener(BRIGHTNESS_ID, this);
    parameters.addParameterListener(TYPE_ID, this);
  parameters.addParameterListener(INPUT_TRIM_ID, this);
parameters.addParameterListener(OUTPUT_TRIM_ID, this);
}

PhoenixSaturationAudioProcessor::~PhoenixSaturationAudioProcessor()
{
    parameters.removeParameterListener(PROCESS_ID, this);
    parameters.removeParameterListener(BRIGHTNESS_ID, this);
    parameters.removeParameterListener(TYPE_ID, this);
  parameters.removeParameterListener(INPUT_TRIM_ID, this);
parameters.removeParameterListener(OUTPUT_TRIM_ID, this);
}

void PhoenixSaturationAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;
    
    if (!prepared)
        return;

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    const float inputGain = std::pow(10.0f, 
        parameters.getRawParameterValue(INPUT_TRIM_ID)->load() / 20.0f);
    const float outputGain = std::pow(10.0f, 
        parameters.getRawParameterValue(OUTPUT_TRIM_ID)->load() / 20.0f);

    if (numChannels >= 1)
    {
        auto* channelData = buffer.getWritePointer(0);
        for (auto sample = 0; sample < numSamples; ++sample)
        {
            channelData[sample] = leftChannel.processSample(channelData[sample] * inputGain) * outputGain;
        }
    }
    
    if (numChannels >= 2)
    {
        auto* channelData = buffer.getWritePointer(1);
        for (auto sample = 0; sample < numSamples; ++sample)
        {
            channelData[sample] = rightChannel.processSample(channelData[sample] * inputGain) * outputGain;
        }
    }
}

void PhoenixSaturationAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
    
    const float processAmount = parameters.getParameter(PROCESS_ID)->getValue();
    const float brightness = parameters.getParameter(BRIGHTNESS_ID)->getValue() * 2.0f;
    const float type = parameters.getParameter(TYPE_ID)->getValue() * 4.0f;
    
    leftChannel.setProcessing(processAmount);
    rightChannel.setProcessing(processAmount);
    leftChannel.setMode(brightness, type);
    rightChannel.setMode(brightness, type);
    
    prepared = true;
}

void PhoenixSaturationAudioProcessor::releaseResources()
{
    prepared = false;
}

void PhoenixSaturationAudioProcessor::parameterChanged(const String& parameterID, float newValue)
{
    if (parameterID == PROCESS_ID)
    {
        leftChannel.setProcessing(newValue);
        rightChannel.setProcessing(newValue);
    }
    else if (parameterID == BRIGHTNESS_ID || parameterID == TYPE_ID)
    {
        const float brightness = parameters.getParameter(BRIGHTNESS_ID)->getValue() * 2.0f;
        const float type = parameters.getParameter(TYPE_ID)->getValue() * 4.0f;
        leftChannel.setMode(brightness, type);
        rightChannel.setMode(brightness, type);
    }
    // Add these cases
    else if (parameterID == INPUT_TRIM_ID || parameterID == OUTPUT_TRIM_ID)
    {
        // The trim values are handled directly in processBlock, 
        // so we don't need to do anything here
    }
}

AudioProcessorEditor* PhoenixSaturationAudioProcessor::createEditor()
{
    return new PhoenixSaturationAudioProcessorEditor(*this);
}

bool PhoenixSaturationAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    return (layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet());
}

void PhoenixSaturationAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PhoenixSaturationAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
    {
        parameters.replaceState(ValueTree::fromXml(*xmlState));
        
        const float processAmount = parameters.getParameter(PROCESS_ID)->getValue();
        const float brightness = parameters.getParameter(BRIGHTNESS_ID)->getValue() * 2.0f;
        const float type = parameters.getParameter(TYPE_ID)->getValue() * 4.0f;
        
        leftChannel.setProcessing(processAmount);
        rightChannel.setProcessing(processAmount);
        leftChannel.setMode(brightness, type);
        rightChannel.setMode(brightness, type);
    }
}



AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhoenixSaturationAudioProcessor();
}
