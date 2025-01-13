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
    : sr_scale(1.0f), s(0.0f), prev_x(0.0f),
      hpf_k(0.0f), lpf_k(0.0f), a3(0.0f), f1(0.0f),
      p20(0.0f), p24(0.0f), g0(true), sat_type(0),
      model_type(0), processing(0.0f), 
      auto_gain_a1(0.0f), auto_gain_a2(0.0f), auto_gain(1.0f)
{
    reset();
}

void PhoenixSaturationAudioProcessor::PhoenixProcessor::setSampleRate(double sampleRate)
{
    sr_scale = 1.0f / std::ceil(sampleRate / 44100.0f);
}

void PhoenixSaturationAudioProcessor::PhoenixProcessor::reset()
{
    s = 0.0f;
    prev_x = 0.0f;
}

// ... [keep other code the same until setMode function]

void PhoenixSaturationAudioProcessor::PhoenixProcessor::setMode(float brightness, float type)
{
    model_type = static_cast<int>(type);
    sat_type = static_cast<int>(brightness);

    // Smoother filter coefficients
    hpf_k = 0.045f * sr_scale;  // Return to original value
    lpf_k = 0.052f * sr_scale;  // Return to original value
    
    // More balanced character settings
    f1 = 0.55f;      // Return to original
    p20 = 0.25f;     // Return to original
    p24 = 0.28f;     // Return to original
    a3 = 0.35f;      // Return to original
    g0 = true;

    // Balanced model-specific adjustments
    switch (model_type) {
        case 1:  // Iridescent
            f1 = 0.48f;
            p20 = 0.29f;
            a3 = 0.37f;
            break;
        case 2:  // Radiant
            f1 = 0.44f;
            p24 = 0.33f;
            a3 = 0.39f;
            break;
        case 3:  // Luster
            a3 = 0.76f;     // Increased from 0.36f
            p20 = 0.61f;    // Increased from 0.31f
            g0 = false;
            break;
        case 4:  // Dark Essence
            f1 = 0.58f;     // Increased from 0.38f
            p20 = 0.63f;    // Increased from 0.33f
            p24 = 0.55f;    // Increased from 0.35f
            a3 = 0.72f;     // Increased from 0.42f
            break;
    }
}

void PhoenixSaturationAudioProcessor::PhoenixProcessor::setProcessing(float amount)
{
    processing = amount;
    // More balanced gain staging
    auto_gain_a1 = 1.0f + processing * 0.18f;    // Return to original
    auto_gain_a2 = 1.0f + processing * 0.12f;    // Return to original
    auto_gain = 1.0f / (auto_gain_a1 * auto_gain_a2);
    
    // Gentler boost
    if (processing > 0.7f) {  // Return to original threshold
        auto_gain *= 1.0f + (processing - 0.7f) * 0.3f;  // Return to original
    }
}

float PhoenixSaturationAudioProcessor::PhoenixProcessor::sat(float x)
{
    switch (sat_type) {
        case 0:  // Opal - Smooth, warm harmonics
            {
                // Update envelope follower
                envelope = std::max(std::abs(x), envelope * envFollowCoeff);
                float warmth = 1.0f + 0.2f * envelope;
                
                // Soft saturation with warm harmonics
                float y = std::tanh(x * 1.5f * warmth);
                float y2 = y * y;
                float y3 = y2 * y;
                float y5 = y3 * y2;
                
                return y + 0.1f * y3 - 0.05f * y5;
            }
            
        case 1:  // Gold - Balanced asymmetric distortion
            {
                // Dynamic response
                envelope = std::max(std::abs(x), envelope * envFollowCoeff);
                float dynamicDrive = 1.0f + 0.3f * envelope;
                
                // Asymmetric processing
                float pos = x > 0 ? x : x * 0.95f;
                float drive = 1.8f * dynamicDrive;
                float base = pos / (1.0f + std::abs(pos * drive));
                
                // Harmonics generation
                float base2 = base * base;
                float base3 = base2 * base;
                float base5 = base3 * base2;
                
                // Combine harmonics with careful balance
                return base + 0.15f * base2 + 0.08f * base3 - 0.02f * base5;
            }
            
        case 2:  // Sapphire - Cleaner, focused harmonics
            {
                // Dynamic adaptation
                envelope = std::max(std::abs(x), envelope * envFollowCoeff);
                float clarity = 1.0f + 0.15f * envelope;
                
                // Main saturation with focus on clarity
                float y = std::atan(x * 1.85f * clarity) / 1.57f;
                float y2 = y * y;
                float y3 = y2 * y;
                float y5 = y3 * y2;
                
                // Balanced harmonic mix
                return y + 0.12f * y3 - 0.03f * y5;
            }
            
        default:  // Transparent - Linear passthrough with subtle enhancement
            {
                // Soft limiting
                float clip = x > 1.0f ? 1.0f : (x < -1.0f ? -1.0f : x);
                float clip2 = clip * clip;
                float clip3 = clip2 * clip;
                
                // Very subtle harmonics
                return clip + 0.02f * clip3;
            }
    }
}

float PhoenixSaturationAudioProcessor::PhoenixProcessor::processSample(float x)
{
    const float proc = processing * a3;
    
    // Gentler high-pass filter
    const float x1 = hpf_k * x + 0.9985f * (x - prev_x);  // Changed from 0.9965f
    
    // More balanced pre-saturation
    const float x2 = x1 * (f1 + 0.25f * proc) + x1 * (1.0f + 0.08f * proc * proc);
    
    const float x3 = (!g0) ? x : x2;
    
    // Gentler saturation stages
    float x4;
    if (model_type == 3) {  // Luster
        x4 = sat(x2 * proc * 1.35f);  // Return to original value
    } else {
        x4 = sat(x2 + 0.025f * proc * x2 * x2);  // Return to original value
    }
    
    // Add smoothing to prevent alternating samples
    const float smooth_amount = 0.1f;  // Add this
    s = (1.0f - smooth_amount) * s + smooth_amount * x4;  // Smooth the signal
    
    const float x5 = sat(s * proc * p20 + x3);

    prev_x = x;
    s += (x5 - s) * lpf_k;
    float y = proc * (s - x * p24);

    // Remove the aggressive enhancement
    y = y + 0.05f * y * y * y;  // Reduced from 0.15f

    return (y + x) * auto_gain;
}

// [Rest of your existing PluginProcessor.cpp implementation remains unchanged]

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

AudioProcessorEditor* PhoenixSaturationAudioProcessor::createEditor()
{
    return new PhoenixSaturationAudioProcessorEditor(*this);
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
