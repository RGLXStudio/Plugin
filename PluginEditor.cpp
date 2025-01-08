/*
  ==============================================================================

    Phoenix Saturation Plugin
    Created: 2025-01-08 09:00:49 UTC
    Author:  RGLXStudio

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

PhoenixSaturationAudioProcessor::PhoenixProcessor::PhoenixProcessor()
    : sr_scale(1.0f), s(0.0f), prev_x(0.0f),
      hpf_k(0.0f), lpf_k(0.0f), a3(0.0f), f1(0.0f),
      p20(0.0f), p24(0.0f), g0(true), sat_type(0),
      model_type(0), processing(0.0f), 
      auto_gain_a1(0.0f), auto_gain_a2(0.0f), auto_gain(1.0f)
{
    reset();
}

void PhoenixTapeAudioProcessor::PhoenixProcessor::setSampleRate(double sampleRate)
{
    sr_scale = 1.0f / std::ceil(sampleRate / 44100.0f);
}

void PhoenixTapeAudioProcessor::PhoenixProcessor::reset()
{
    s = 0.0f;
    prev_x = 0.0f;
}

void PhoenixTapeAudioProcessor::PhoenixProcessor::setMode(float brightness, float type)
{
    model_type = static_cast<int>(type);
    sat_type = static_cast<int>(brightness);

    // Enhanced filter coefficients for better frequency response
    hpf_k = 0.045f * sr_scale;
    lpf_k = 0.052f * sr_scale;
    
    // Default character settings
    f1 = 0.55f;
    p20 = 0.25f;
    p24 = 0.28f;
    a3 = 0.35f;
    g0 = true;

    // Enhanced saturation characteristics per model
    switch (model_type) {
        case 1:  // Iridescent - Warmer harmonics
            f1 = 0.48f;
            p20 = 0.29f;
            a3 = 0.37f;
            break;
        case 2:  // Radiant - Brighter character
            f1 = 0.44f;
            p24 = 0.33f;
            a3 = 0.39f;
            break;
        case 3:  // Luster - Smooth compression
            a3 = 0.36f;
            p20 = 0.31f;
            g0 = false;
            break;
        case 4:  // Dark Essence - Rich harmonics
            f1 = 0.38f;
            p20 = 0.33f;
            p24 = 0.35f;
            a3 = 0.42f;
            break;
    }
}

void PhoenixTapeAudioProcessor::PhoenixProcessor::setProcessing(float amount)
{
    processing = amount;
    // Enhanced auto-gain compensation
    auto_gain_a1 = 1.0f + processing * 0.28f;
    auto_gain_a2 = 1.0f + processing * 0.18f;
    auto_gain = 1.0f / (auto_gain_a1 * auto_gain_a2);
}

float PhoenixTapeAudioProcessor::PhoenixProcessor::sat(float x)
{
    // Enhanced saturation algorithms with improved harmonics
    switch (sat_type) {
        case 0:  // Opal - Smooth, transparent
            return std::tanh(x * 1.25f) / 1.25f;
        case 1:  // Gold - Warm, vintage character
            return x / (1.0f + std::abs(x * 1.15f));
        case 2:  // Sapphire - Clear, precise
            {
                float y = std::atan(x * 1.35f) / 1.35f;
                return y + 0.025f * y * y * y; // Enhanced harmonic content
            }
        default:
            return x;
    }
}

float PhoenixTapeAudioProcessor::PhoenixProcessor::processSample(float x)
{
    const float proc = processing * a3;
    
    // Enhanced high-pass filter stage
    const float x1 = hpf_k * x + 0.997f * (x - prev_x);
    
    // Improved pre-saturation stage
    const float x2 = x1 * (f1 + 0.15f * proc) + x1;
    
    // Mode-dependent processing path
    const float x3 = (!g0) ? x : x2;
    
    // Enhanced saturation stage with model-specific processing
    float x4;
    if (model_type == 3) {
        x4 = sat(x2 * proc * 1.25f);
    } else {
        x4 = sat(x2 + 0.02f * proc * x2 * x2);
    }
    
    // Final saturation stage with feedback
    const float x5 = sat(x4 * proc * p20 + x3);

    // State update and filtering
    prev_x = x;
    s += (x5 - s) * lpf_k;
    float y = proc * (s - x * p24);

    // Model-specific output processing
    if (model_type == 3) {
        y *= 0.65f; // Adjusted compensation for Luster model
    }
    
    if (model_type == 4) {
        y = sat(y * 1.15f); // Enhanced Dark Essence character
    }

    // Final output with auto-gain compensation
    return (y + x) * auto_gain;
}

PhoenixTapeAudioProcessor::PhoenixTapeAudioProcessor()
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

PhoenixTapeAudioProcessor::~PhoenixTapeAudioProcessor()
{
    parameters.removeParameterListener(INPUT_TRIM_ID, this);
    parameters.removeParameterListener(PROCESS_ID, this);
    parameters.removeParameterListener(OUTPUT_TRIM_ID, this);
    parameters.removeParameterListener(BRIGHTNESS_ID, this);
    parameters.removeParameterListener(TYPE_ID, this);
}

void PhoenixTapeAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    
    if (!prepared)
        return;

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    // Get parameter values
    const float inputGain = std::pow(10.0f, 
        parameters.getRawParameterValue(INPUT_TRIM_ID)->load() / 20.0f);
    const float outputGain = std::pow(10.0f, 
        parameters.getRawParameterValue(OUTPUT_TRIM_ID)->load() / 20.0f);

    // Process channels
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

void PhoenixTapeAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
    
    // Reset and prepare processors
    leftChannel.reset();
    rightChannel.reset();
    
    leftChannel.setSampleRate(sampleRate);
    rightChannel.setSampleRate(sampleRate);
    
    // Initialize parameters
    const float brightness = parameters.getParameter(BRIGHTNESS_ID)->getValue() * 2.0f;
    const float type = parameters.getParameter(TYPE_ID)->getValue() * 4.0f;
    const float processAmount = parameters.getParameter(PROCESS_ID)->getValue() / 100.0f;
    
    leftChannel.setMode(brightness, type);
    rightChannel.setMode(brightness, type);
    leftChannel.setProcessing(processAmount);
    rightChannel.setProcessing(processAmount);
    
    prepared = true;
}

void PhoenixTapeAudioProcessor::releaseResources()
{
    prepared = false;
}

void PhoenixTapeAudioProcessor::parameterChanged(const String& parameterID, float newValue)
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

bool PhoenixTapeAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Support mono and stereo configurations
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // Input and output layout must match
    return (layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet());
}

void PhoenixTapeAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PhoenixTapeAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
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

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhoenixTapeAudioProcessor();
}
