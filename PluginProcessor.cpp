/*
  ==============================================================================

    Phoenix Saturation Plugin
    Created: 2025-01-14 06:30:56 UTC
    Author:  RGLXStudio

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

//==============================================================================
PhoenixSaturationAudioProcessor::PhoenixProcessor::PhoenixProcessor()
    : sat_type(0)
    , model_type(0)
    , processing(0.0f)
{
    reset();
}

void PhoenixSaturationAudioProcessor::PhoenixProcessor::setSampleRate(double sampleRate)
{
    // We don't need sample rate for hard clipping
}

void PhoenixSaturationAudioProcessor::PhoenixProcessor::reset()
{
    // Reset not needed for stateless processing
}

void PhoenixSaturationAudioProcessor::PhoenixProcessor::setMode(float brightness, float type)
{
    model_type = static_cast<int>(type);
    sat_type = static_cast<int>(brightness);
}

void PhoenixSaturationAudioProcessor::PhoenixProcessor::setProcessing(float amount)
{
    // Convert percentage to 0-1 range and scale to match JSFX
    processing = amount * 0.01f;
}

float PhoenixSaturationAudioProcessor::PhoenixProcessor::sat(float x)
{
    // Simple hard clipping like in JSFX
    if (x > 1.0f) return 1.0f;
    if (x < -1.0f) return -1.0f;
    return x;
}

float PhoenixSaturationAudioProcessor::PhoenixProcessor::processSample(float x)
{
    // Match JSFX drive scaling
    float drive = processing * 24.0f;
    
    // Apply drive with slight variations per type
    switch (sat_type) {
        case 0:  // Opal
            x *= std::pow(10.0f, drive * 0.05f);
            break;
            
        case 1:  // Gold
            x *= std::pow(10.0f, drive * 0.06f); // Slightly more aggressive
            break;
            
        case 2:  // Sapphire
            x *= std::pow(10.0f, drive * 0.045f); // Slightly gentler
            break;
            
        default:
            x *= std::pow(10.0f, drive * 0.05f);
            break;
    }
    
    // Apply saturation based on type
    switch (sat_type) {
        case 0:  // Opal - JSFX-like with slight smoothing
        {
            if (x > 1.0f) x = 1.0f;
            if (x < -1.0f) x = -1.0f;
            // Add very subtle harmonics
            float xx = x * x;
            x = x * (1.0f - xx * 0.1f);
            break;
        }
            
        case 1:  // Gold - JSFX-like with asymmetry
        {
            if (x > 1.0f) x = 1.0f;
            if (x < -1.0f) x = -0.95f; // Slight asymmetry
            break;
        }
            
        case 2:  // Sapphire - JSFX-like with slight compression
        {
            float sign = x > 0 ? 1.0f : -1.0f;
            x = sign * (1.0f - std::exp(-std::abs(x)));
            break;
        }
            
        default:  // Pure JSFX-style hard clipping
        {
            if (x > 1.0f) x = 1.0f;
            if (x < -1.0f) x = -1.0f;
            break;
        }
    }
    
    return x;
}

//==============================================================================
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
    parameters.addParameterListener(INPUT_TRIM_ID, this);
    parameters.addParameterListener(PROCESS_ID, this);
    parameters.addParameterListener(OUTPUT_TRIM_ID, this);
    parameters.addParameterListener(BRIGHTNESS_ID, this);
    parameters.addParameterListener(TYPE_ID, this);
}

PhoenixSaturationAudioProcessor::~PhoenixSaturationAudioProcessor()
{
    parameters.removeParameterListener(INPUT_TRIM_ID, this);
    parameters.removeParameterListener(PROCESS_ID, this);
    parameters.removeParameterListener(OUTPUT_TRIM_ID, this);
    parameters.removeParameterListener(BRIGHTNESS_ID, this);
    parameters.removeParameterListener(TYPE_ID, this);
}

void PhoenixSaturationAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
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
    
    leftChannel.reset();
    rightChannel.reset();
    
    leftChannel.setSampleRate(sampleRate);
    rightChannel.setSampleRate(sampleRate);
    
    const float brightness = parameters.getParameter(BRIGHTNESS_ID)->getValue() * 2.0f;
    const float type = parameters.getParameter(TYPE_ID)->getValue() * 4.0f;
    const float processAmount = parameters.getParameter(PROCESS_ID)->getValue();
    
    leftChannel.setMode(brightness, type);
    rightChannel.setMode(brightness, type);
    leftChannel.setProcessing(processAmount);
    rightChannel.setProcessing(processAmount);
    
    prepared = true;
}

void PhoenixSaturationAudioProcessor::releaseResources()
{
    prepared = false;
}

AudioProcessorEditor* PhoenixSaturationAudioProcessor::createEditor()
{
    return new PhoenixSaturationAudioProcessorEditor(*this);
}

void PhoenixSaturationAudioProcessor::parameterChanged(const String& parameterID, float newValue)
{
    if (parameterID == BRIGHTNESS_ID || parameterID == TYPE_ID)
    {
        const float brightness = parameters.getParameter(BRIGHTNESS_ID)->getValue() * 2.0f;
        const float type = parameters.getParameter(TYPE_ID)->getValue() * 4.0f;
        leftChannel.setMode(brightness, type);
        rightChannel.setMode(brightness, type);
    }
    else if (parameterID == PROCESS_ID)
    {
        leftChannel.setProcessing(newValue);
        rightChannel.setProcessing(newValue);
    }
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
        
        const float brightness = parameters.getParameter(BRIGHTNESS_ID)->getValue() * 2.0f;
        const float type = parameters.getParameter(TYPE_ID)->getValue() * 4.0f;
        const float processAmount = parameters.getParameter(PROCESS_ID)->getValue();
        
        leftChannel.setMode(brightness, type);
        rightChannel.setMode(brightness, type);
        leftChannel.setProcessing(processAmount);
        rightChannel.setProcessing(processAmount);
    }
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhoenixSaturationAudioProcessor();
}
