/*==============================================================================
    Phoenix Saturation Plugin
    Created: 2025-01-14 07:39:55 UTC
    Author:  RGLXStudio
==============================================================================*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

PhoenixSaturationAudioProcessor::PhoenixProcessor::PhoenixProcessor()
    : processing(0.0f)
    , sat_type(0)
    , model_type(0)
    , sr_scale(1.0f)
    , s(0.0f)
    , prev_x(0.0f)
    , a3(1.0f)
    , f1(0.5f)
    , p20(0.25f)
    , p24(0.1f)
    , auto_gain_a1(-0.5f)
    , auto_gain_a2(0.1f)
    , hpf_k(0.0f)
    , lpf_k(0.0f)
    , auto_gain(1.0f)
{
}

void PhoenixSaturationAudioProcessor::PhoenixProcessor::setSampleRate(double sampleRate)
{
    // Original Phoenix has fixed scaling depending on sample rate: {1.0, 0.5, 0.25}
    this->sr_scale = 1.0 / std::ceil(sampleRate / 44100.0);
}

void PhoenixSaturationAudioProcessor::PhoenixProcessor::reset()
{
    this->s = 0.0f;
    this->prev_x = 0.0f;
}

void PhoenixSaturationAudioProcessor::PhoenixProcessor::setMode(float brightness, float type)
{
    model_type = static_cast<int>(type);

    switch (model_type) {
        case 0: // Luminescent
            if (brightness == 0) { this->hpf_k = 0.625f; this->lpf_k = 0.1875f; } // Opal
            if (brightness == 1) { this->hpf_k = 0.4375f; this->lpf_k = 0.3125f; } // Gold
            if (brightness == 2) { this->hpf_k = 0.1875f; this->lpf_k = 0.375f; }  // Sapphire
            this->a3 = 0.25f;
            this->f1 = 0.75f;
            this->p20 = 0.3125f;
            this->p24 = 0.0625f;
            this->auto_gain_a1 = -0.416f;
            this->auto_gain_a2 = 0.092f;
            break;
        case 1: // Iridescent
            if (brightness == 0) { this->hpf_k = 0.625f; this->lpf_k = 0.1875f; } // Opal
            if (brightness == 1) { this->hpf_k = 0.375f; this->lpf_k = 0.3125f; } // Gold
            if (brightness == 2) { this->hpf_k = 0.3125f; this->lpf_k = 0.5f; }   // Sapphire
            this->a3 = 0.25f;
            this->f1 = 0.875f;
            this->p20 = 0.3125f;
            this->p24 = 0.0625f;
            this->auto_gain_a1 = -0.393f;
            this->auto_gain_a2 = 0.082f;
            break;
        case 2: // Radiant
            if (brightness == 0) { this->hpf_k = 0.75f; this->lpf_k = 0.125f; } // Opal
            if (brightness == 1) { this->hpf_k = 0.45629901f; this->lpf_k = 0.375f; } // Gold
            if (brightness == 2) { this->hpf_k = 0.375f; this->lpf_k = 0.5f; }  // Sapphire
            this->a3 = 0.375f;
            this->f1 = 0.75f;
            this->p20 = 0.1875f;
            this->p24 = 0.0125f;
            this->auto_gain_a1 = -0.441f;
            this->auto_gain_a2 = 0.103f;
            break;
        case 3: // Luster
            if (brightness == 0) { this->hpf_k = 0.75f; this->lpf_k = 0.125f; } // Opal
            if (brightness == 1) { this->hpf_k = 0.45629901f; this->lpf_k = 0.375f; } // Gold
            if (brightness == 2) { this->hpf_k = 0.375f; this->lpf_k = 0.5625f; }  // Sapphire
            this->a3 = 1.0f;
            this->f1 = 0.6875f;
            this->p20 = 0.27343899f;
            this->p24 = 0.1171875f;
            this->auto_gain_a1 = -0.712f;
            this->auto_gain_a2 = 0.172f;
            break;
        case 4: // Dark Essence
            if (brightness == 0) { this->hpf_k = 0.75f; this->lpf_k = 0.125f; } // Opal
            if (brightness == 1) { this->hpf_k = 0.45629901f; this->lpf_k = 0.375f; } // Gold
            if (brightness == 2) { this->hpf_k = 0.375f; this->lpf_k = 0.5625f; }  // Sapphire
            this->a3 = 0.375f;
            this->f1 = 0.75f;
            this->p20 = 0.5625f;
            this->p24 = 0.0125f;
            this->auto_gain_a1 = -0.636f;
            this->auto_gain_a2 = 0.17f;
            break;
    }
}

void PhoenixSaturationAudioProcessor::PhoenixProcessor::setProcessing(float amount)
{
    // Apply a scaling factor to reduce overall attenuation
    float scalingFactor = 0.25f;
    processing = (amount / 100.0f) * scalingFactor;
    this->auto_gain = 1.0f + processing * this->auto_gain_a1 + processing * processing * this->auto_gain_a2;
}

float PhoenixSaturationAudioProcessor::PhoenixProcessor::sat(float x)
{
    float y = 0.0f;
    float xx = x * x;

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

    return y;
}

float PhoenixSaturationAudioProcessor::PhoenixProcessor::processSample(float x)
{
    if (processing <= 0.0f)
        return x;

    x *= std::pow(10.0f, processing * this->a3 * 0.05f) * this->auto_gain;

    if (x > 1.0f) x = 1.0f;
    if (x < -1.0f) x = -1.0f;

    float y = this->sat(x);

    // Apply curve smoothing using interpolation
    float s = this->s + (y - this->s) * this->lpf_k;
    float output = processing * (s - x * this->p24);

    this->s = s;
    this->prev_x = x;

    return (output + x) * this->auto_gain;
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

void PhoenixSaturationAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    leftChannel.setSampleRate(sampleRate);
    rightChannel.setSampleRate(sampleRate);
    leftChannel.reset();
    rightChannel.reset();

    const float processAmount = parameters.getParameter(PROCESS_ID)->getValue();
    const float brightness = parameters.getParameter(BRIGHTNESS_ID)->getValue();
    const float type = parameters.getParameter(TYPE_ID)->getValue();

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

void PhoenixSaturationAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;

    if (!prepared)
        return;

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    const float inputGain = std::pow(10.0f, parameters.getRawParameterValue(INPUT_TRIM_ID)->load() / 20.0f);
    const float outputGain = std::pow(10.0f, parameters.getRawParameterValue(OUTPUT_TRIM_ID)->load() / 20.0f);

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < numSamples; ++sample)
        {
            if (channel == 0)
                channelData[sample] = leftChannel.processSample(channelData[sample] * inputGain) * outputGain;
            else if (channel == 1)
                channelData[sample] = rightChannel.processSample(channelData[sample] * inputGain) * outputGain;
        }
    }
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
        const float brightness = parameters.getParameter(BRIGHTNESS_ID)->getValue();
        const float type = parameters.getParameter(TYPE_ID)->getValue();
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
        const float brightness = parameters.getParameter(BRIGHTNESS_ID)->getValue();
        const float type = parameters.getParameter(TYPE_ID)->getValue();

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
