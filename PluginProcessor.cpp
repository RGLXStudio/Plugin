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
    , hpf_k(0.045f)
    , lpf_k(0.052f)
    , a3(0.35f)
    , f1(0.55f)
    , p20(0.25f)
    , p24(0.28f)
    , g0(true)
    , sat_type(0)
    , model_type(0)
    , processing(0.0f)
    , auto_gain_a1(1.0f)
    , auto_gain_a2(1.0f)
    , auto_gain(1.0f)
{
    reset();
}

void PhoenixSaturationAudioProcessor::PhoenixProcessor::setSampleRate(double sampleRate)
{
    sr_scale = 1.0f / std::ceil(sampleRate / 44100.0f);
    hpf_k = 0.045f * sr_scale;
    lpf_k = 0.052f * sr_scale;
}

void PhoenixSaturationAudioProcessor::PhoenixProcessor::reset()
{
    s = 0.0f;
    prev_x = 0.0f;
    envelope = 0.0f;
}

// ... [keep other code the same until setMode function]

void PhoenixSaturationAudioProcessor::PhoenixProcessor::setMode(float brightness, float type)
{
    model_type = static_cast<int>(type);
    sat_type = static_cast<int>(brightness);

    // Smoother filter coefficients
    hpf_k = 0.045f * sr_scale;  // Return to original value
    lpf_k = 0.052f * sr_scale;  // Return to original value
   
  if (model_type == 0) { // Luminescent
        if (sat_type == 1) { // Gold
            hpf_k = 0.041f * sr_scale;  // Slightly reduced high-pass
            lpf_k = 0.047f * sr_scale;  // Slightly reduced low-pass
            
            // Adjusted character parameters
            f1 = 0.51f;      // Fine-tuned presence
            p20 = 0.27f;     // Refined harmonics balance
            p24 = 0.29f;     // Adjusted output stage
            a3 = 0.36f;      // Refined drive scaling
        }
    }
    
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
    
    if (sat_type == 1) { // Gold
        // Refined gain staging for Gold mode
        auto_gain_a1 = 1.0f + processing * 0.15f;    // Reduced from 0.16f
        auto_gain_a2 = 1.0f + processing * 0.11f;    // Reduced from 0.14f
        auto_gain = 1.0f / (auto_gain_a1 * auto_gain_a2);
        
        // Additional compensation at high processing values
        if (processing > 0.65f) {
            auto_gain *= 1.0f + (processing - 0.65f) * 0.2f;
        }
    }
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
            // Update envelope follower for dynamic control
            envelope = std::max(std::abs(x), envelope * 0.995f);  // Slightly slower envelope
            
            // Dynamic drive adjustment
            float dynamicDrive = 1.0f + 0.25f * envelope;
            
            // Asymmetric processing with refined curve
            float pos = x > 0 ? x : x * 0.97f;  // Less asymmetric than before
            float drive = 1.65f * dynamicDrive;  // Reduced from 1.8f
            
            // Main waveshaping
            float base = pos / (1.0f + std::abs(pos * drive));
            
            // Harmonic generation with careful balance
            float base2 = base * base;
            float base3 = base2 * base;
            float base5 = base3 * base2;
            
            // Combine harmonics with adjusted coefficients
            float shaped = base + 0.12f * base2 + 0.06f * base3 - 0.015f * base5;
            
            // Output level adjustment to match reference
            return shaped * 0.965f;  // Added scaling factor
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
    // Input stage with high-pass filter
    const float x1 = hpf_k * x + (1.0f - hpf_k) * (x - prev_x);
    
    // Pre-saturation
    const float proc = processing * a3;
    const float x2 = x1 * (f1 + 0.25f * proc) + x1 * (1.0f + 0.08f * proc * proc);
    
    // Apply saturation
    const float saturated = sat(g0 ? x2 : x);
    
    // Smoothing
    const float smooth_amount = 0.08f;
    s = (1.0f - smooth_amount) * s + smooth_amount * saturated;
    
    // Post-processing and output
    prev_x = x;
    float y = proc * (s - x * p24);
    
    // Final output with auto-gain
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
