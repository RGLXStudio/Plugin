/*
  ==============================================================================

    Phoenix Saturation Plugin
    Created: 2025-01-14 06:52:24 UTC
    Author:  RGLXStudio

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

float PhoenixSaturationAudioProcessor::PhoenixProcessor::processSample(float x)
{
    // Base drive calculation (JSFX-style)
    float drive = processing * 24.0f; // Scale 0-1 to 0-24 range
    
    // Modify drive based on model type
    switch (model_type) {
        case 0: // Luminescent - standard
            break;
        case 1: // Iridescent - slightly less aggressive
            drive *= 0.92f;
            break;
        case 2: // Radiant - slightly more aggressive
            drive *= 1.08f;
            break;
        case 3: // Luster - much gentler
            drive *= 0.75f;
            break;
        case 4: // Dark Essence - most aggressive
            drive *= 1.25f;
            break;
    }
    
    // Apply drive with JSFX-style gain calculation
    x *= std::pow(10.0f, drive * 0.05f);
    
    // Apply character variations while maintaining JSFX-like clipping
    switch (sat_type) {
        case 0: // Opal - standard hard clip with slight harmonics
        {
            // Clip at ±1.0
            if (x > 1.0f) return 1.0f;
            if (x < -1.0f) return -1.0f;
            // Add subtle harmonics without changing the basic sound
            float xx = x * x;
            return x * (1.0f - xx * 0.05f);
        }
        
        case 1: // Gold - asymmetric clip
        {
            // Different positive/negative thresholds
            if (x > 1.0f) return 1.0f;
            if (x < -1.0f) return -0.98f;
            return x;
        }
        
        case 2: // Sapphire - smoother clip
        {
            // Soft clip transition near ±1.0
            float abs_x = std::abs(x);
            if (abs_x > 1.0f) {
                return x > 0.0f ? 1.0f : -1.0f;
            }
            if (abs_x > 0.9f) {
                float t = (abs_x - 0.9f) * 10.0f; // 0 to 1 transition
                return x > 0.0f ? 
                    x * (1.0f - t) + t : 
                    x * (1.0f - t) - t;
            }
            return x;
        }
        
        default: // Pure JSFX hard clip
            if (x > 1.0f) return 1.0f;
            if (x < -1.0f) return -1.0f;
            return x;
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
    parameters.addParameterListener(INPUT_TRIM_ID, this);
    parameters.addParameterListener(OUTPUT_TRIM_ID, this);
}

PhoenixSaturationAudioProcessor::~PhoenixSaturationAudioProcessor()
{
    parameters.removeParameterListener(PROCESS_ID, this);
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
    
    leftChannel.setProcessing(processAmount);
    rightChannel.setProcessing(processAmount);
    
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
        leftChannel.setProcessing(processAmount);
        rightChannel.setProcessing(processAmount);
    }
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhoenixSaturationAudioProcessor();
}
