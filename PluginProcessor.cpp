/*
  ==============================================================================

    Phoenix Saturation Plugin
    Created: 2025-01-09 06:42:21 UTC
    Author:  RGLXStudio

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

//==============================================================================
PhoenixSaturationAudioProcessor::PhoenixProcessor::PhoenixProcessor()
    : sr_scale(1.0f)
    , s(0.0f)
    , prev_x(0.0f)
    , envelope(0.0f)
    , envFollowCoeff(0.99f)
    , hpf_k(0.042f)
    , lpf_k(0.058f)
    , a3(0.45f)
    , f1(0.65f)
    , p20(0.35f)
    , p24(0.15f)
    , g0(true)
    , sat_type(0)
    , model_type(0)
    , processing(0.0f)
{
    reset();
}

void PhoenixSaturationAudioProcessor::PhoenixProcessor::setSampleRate(double sampleRate)
{
    sr_scale = 1.0f / std::ceil(sampleRate / 44100.0f);
    hpf_k = 0.042f * sr_scale;
    lpf_k = 0.058f * sr_scale;
}

void PhoenixSaturationAudioProcessor::PhoenixProcessor::reset()
{
    s = 0.0f;
    prev_x = 0.0f;
    envelope = 0.0f;
}

// Would you like me to continue with the rest of the implementation? I'll share the rest of PluginProcessor.cpp and PluginEditor files.
void PhoenixSaturationAudioProcessor::PhoenixProcessor::setMode(float brightness, float type)
{
    model_type = static_cast<int>(type);
    sat_type = static_cast<int>(brightness);

    // More aggressive base parameters
    hpf_k = 0.042f * sr_scale;
    lpf_k = 0.058f * sr_scale;
    f1 = 0.65f;      
    p20 = 0.35f;     
    p24 = 0.15f;     
    a3 = 0.45f;      
    g0 = true;

    if (model_type == 0) { // Luminescent
        if (sat_type == 1) { // Gold
            hpf_k = 0.038f * sr_scale;
            lpf_k = 0.048f * sr_scale;
            f1 = 0.58f;
            p20 = 0.42f;    
            p24 = 0.12f;    
            a3 = 0.52f;     
        }
    }

    switch (model_type) {
        case 1:  // Iridescent
            f1 = 0.55f;
            p20 = 0.38f;
            a3 = 0.48f;
            break;
        case 2:  // Radiant
            f1 = 0.52f;
            p24 = 0.18f;
            a3 = 0.55f;
            break;
        case 3:  // Luster
            a3 = 0.85f;     
            p20 = 0.72f;    
            g0 = false;
            break;
        case 4:  // Dark Essence
            f1 = 0.68f;
            p20 = 0.75f;    
            p24 = 0.22f;
            a3 = 0.82f;     
            break;
    }
}

void PhoenixSaturationAudioProcessor::PhoenixProcessor::setProcessing(float amount)
{
    // Direct processing amount without auto-gain compensation
    processing = amount;
    
    if (sat_type == 1) { // Gold
        processing *= 1.8f; // More aggressive drive
    }
    else {
        processing *= 1.5f;
    }
}

float PhoenixSaturationAudioProcessor::PhoenixProcessor::sat(float x)
{
    switch (sat_type) {
        case 0:  // Opal - Hard clipping with smooth transition
        {
            float drive = 2.0f;
            x *= drive;
            // Hard clip at ±1.0 with smooth transition
            if (x > 1.0f) return 1.0f;
            if (x < -1.0f) return -1.0f;
            // Smooth polynomial near the clipping points
            float xx = x * x;
            return x * (1.0f - xx * 0.3f);
        }
            
        case 1:  // Gold - Heavy asymmetric distortion
        {
            float drive = 2.2f;
            float pos = x > 0 ? x : x * 0.92f;
            
            // More aggressive clipping for positive values
            if (pos > 1.0f) return 1.0f;
            if (pos < -1.0f) return -0.92f;
            
            float base = pos / (1.0f + std::abs(pos * drive));
            float base2 = base * base;
            float base3 = base2 * base;
            
            return (base + 0.18f * base2 + 0.12f * base3) * 2.5f;
        }
            
        case 2:  // Sapphire - Hard clipping with harmonic enhancement
        {
            float drive = 2.5f;
            x *= drive;
            
            // Hard clip with slight softening
            if (x > 1.0f) return 1.0f;
            if (x < -1.0f) return -1.0f;
            
            float y = x * (1.0f - std::abs(x) * 0.2f);
            float y2 = y * y;
            float y3 = y2 * y;
            
            return y + 0.15f * y3;
        }
            
        default:  // Transparent - Pure hard clipping
        {
            if (x > 1.0f) return 1.0f;
            if (x < -1.0f) return -1.0f;
            return x;
        }
    }
}

float PhoenixSaturationAudioProcessor::PhoenixProcessor::processSample(float x)
{
    // Input stage with high-pass filter
    const float x1 = hpf_k * x + (x - prev_x);
    
    // More aggressive pre-saturation
    const float proc = processing * a3 * 1.5f;
    const float x2 = x1 * (f1 + 0.5f * proc) + x1 * (1.0f + 0.15f * proc * proc);
    
    // Apply saturation
    float saturated = sat(g0 ? x2 : x);
    
    // Gold-specific output boost
    if (sat_type == 1) {
        saturated *= 1.4f;
    }
    
    // Smoothing with less dampening
    const float smooth_amount = 0.12f;
    s = (1.0f - smooth_amount) * s + smooth_amount * saturated;
    
    // Post-processing and output
    prev_x = x;
    float y = proc * (s - x * (p24 * 0.5f));
    
    // Return processed signal without auto-gain compensation
    return y + x;
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

// Would you like me to continue with the rest of the implementation, including the remaining processor methods and editor files?

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
    const float processAmount = parameters.getParameter(PROCESS_ID)->getValue() / 100.0f;
    
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
        const float processAmount = newValue / 100.0f;
        leftChannel.setProcessing(processAmount);
        rightChannel.setProcessing(processAmount);
    }
}

bool PhoenixSaturationAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Support mono and stereo configurations
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // Input and output layout must match
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
        const float processAmount = parameters.getParameter(PROCESS_ID)->getValue() / 100.0f;
        
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
