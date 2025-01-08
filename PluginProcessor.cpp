/*
  ==============================================================================

    Phoenix Tape Plugin
    Created: 2025-01-08 06:48:30 UTC
    Author:  RGLXStudio

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

PhoenixTapeAudioProcessor::PhoenixProcessor::PhoenixProcessor() 
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
    sr_scale = 1.0f / ceil(sampleRate / 44100.0f);
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

    hpf_k = 0.042f * sr_scale;
    lpf_k = 0.047f * sr_scale;
    
    f1 = 0.55f;
    p20 = 0.25f;
    p24 = 0.28f;
    a3 = 0.35f;
    g0 = true;

    switch (model_type) {
        case 1:  // Iridescent
            f1 = 0.48f;
            p20 = 0.27f;
            break;
        case 2:  // Radiant
            f1 = 0.42f;
            p24 = 0.32f;
            a3 = 0.38f;
            break;
        case 3:  // Luster
            a3 = 0.36f;
            p20 = 0.29f;
            g0 = false;
            break;
        case 4:  // Dark Essence
            f1 = 0.38f;
            p20 = 0.31f;
            p24 = 0.34f;
            a3 = 0.40f;
            break;
    }
}

void PhoenixTapeAudioProcessor::PhoenixProcessor::setProcessing(float amount)
{
    processing = amount;
    auto_gain_a1 = 1.0f + processing * 0.25f;
    auto_gain_a2 = 1.0f + processing * 0.15f;
    auto_gain = 1.0f / (auto_gain_a1 * auto_gain_a2);
}

float PhoenixTapeAudioProcessor::PhoenixProcessor::sat(float x)
{
    switch (sat_type) {
        case 0:  // Opal
            return std::tanh(x * 1.2f) / 1.2f;
        case 1:  // Gold
            return x / (1.0f + std::abs(x * 1.1f));
        case 2:  // Sapphire
            return std::atan(x * 1.3f) / 1.3f;
        default:
            return x;
    }
}

float PhoenixTapeAudioProcessor::PhoenixProcessor::processSample(float x)
{
    float proc = processing * a3;
    float x1 = hpf_k * x + 0.995f * (x - prev_x);
    float x2 = x1 * f1 + x1;
    float x3 = (!g0) ? x : x2;
    float x4 = (model_type == 3) ? sat(x2 * proc * 1.2f) : sat(x2);
    float x5 = sat(x4 * proc * p20 + x3);

    prev_x = x;
    s += (x5 - s) * lpf_k;
    float y = proc * (s - x * p24);

    if (model_type == 3) {
        y *= 0.6f;
    }
    
    if (model_type == 4) {
        y = sat(y * 1.1f);
    }

    return (y + x) * auto_gain;
}

PhoenixTapeAudioProcessor::PhoenixTapeAudioProcessor()
    : AudioProcessor(BusesProperties()
                    .withInput("Input", AudioChannelSet::stereo(), true)
                    .withOutput("Output", AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", {
          std::make_unique<AudioParameterFloat>(INPUT_TRIM_ID, "Input Trim",
                                              NormalisableRange<float>(-6.0f, 6.0f, 0.01f),
                                              0.0f),
          std::make_unique<AudioParameterFloat>(PROCESS_ID, "Process",
                                              NormalisableRange<float>(0.0f, 100.0f, 0.01f),
                                              0.0f),
          std::make_unique<AudioParameterFloat>(OUTPUT_TRIM_ID, "Output Trim",
                                              NormalisableRange<float>(-6.0f, 6.0f, 0.01f),
                                              0.0f),
          std::make_unique<AudioParameterChoice>(BRIGHTNESS_ID, "Brightness",
                                               StringArray{"Opal", "Gold", "Sapphire"},
                                               0),
          std::make_unique<AudioParameterChoice>(TYPE_ID, "Type",
                                               StringArray{"Luminiscent", "Iridescent", "Radiant", "Luster", "Dark Essence"},
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

void PhoenixTapeAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == BRIGHTNESS_ID || parameterID == TYPE_ID)
    {
        auto brightness = parameters.getParameter(BRIGHTNESS_ID)->getValue() * 2;
        auto type = parameters.getParameter(TYPE_ID)->getValue() * 4;
        leftChannel.setMode(brightness, type);
        rightChannel.setMode(brightness, type);
    }
    else if (parameterID == PROCESS_ID)
    {
        float processAmount = newValue / 100.0f;
        leftChannel.setProcessing(processAmount);
        rightChannel.setProcessing(processAmount);
    }
}

const juce::String PhoenixTapeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PhoenixTapeAudioProcessor::acceptsMidi() const
{
    return false;
}

bool PhoenixTapeAudioProcessor::producesMidi() const
{
    return false;
}

bool PhoenixTapeAudioProcessor::isMidiEffect() const
{
    return false;
}

double PhoenixTapeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PhoenixTapeAudioProcessor::getNumPrograms()
{
    return 1;
}

int PhoenixTapeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PhoenixTapeAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String PhoenixTapeAudioProcessor::getProgramName(int index)
{
    return {};
}

void PhoenixTapeAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

void PhoenixTapeAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
    
    // Reset the processors
    leftChannel.reset();
    rightChannel.reset();
    
    leftChannel.setSampleRate(sampleRate);
    rightChannel.setSampleRate(sampleRate);
    
    auto brightness = parameters.getParameter(BRIGHTNESS_ID)->getValue() * 2;
    auto type = parameters.getParameter(TYPE_ID)->getValue() * 4;
    auto processAmount = parameters.getParameter(PROCESS_ID)->getValue() / 100.0f;
    
    leftChannel.setMode(brightness, type);
    rightChannel.setMode(brightness, type);
    leftChannel.setProcessing(processAmount);
    rightChannel.setProcessing(processAmount);
    
    prepared = true;
}

bool PhoenixTapeAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Accept mono and stereo configurations
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Input and output layout must match
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void PhoenixTapeAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == BRIGHTNESS_ID || parameterID == TYPE_ID)
    {
        auto brightness = parameters.getParameter(BRIGHTNESS_ID)->getValue() * 2;
        auto type = parameters.getParameter(TYPE_ID)->getValue() * 4;
        leftChannel.setMode(brightness, type);
        rightChannel.setMode(brightness, type);
    }
    else if (parameterID == PROCESS_ID)
    {
        float processAmount = newValue / 100.0f;
        leftChannel.setProcessing(processAmount);
        rightChannel.setProcessing(processAmount);
    }
}

void PhoenixTapeAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PhoenixTapeAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
            
            auto brightness = parameters.getParameter(BRIGHTNESS_ID)->getValue() * 2;
            auto type = parameters.getParameter(TYPE_ID)->getValue() * 4;
            auto processAmount = parameters.getParameter(PROCESS_ID)->getValue() / 100.0f;
            
            leftChannel.setMode(brightness, type);
            rightChannel.setMode(brightness, type);
            leftChannel.setProcessing(processAmount);
            rightChannel.setProcessing(processAmount);
        }
    }
}

bool PhoenixTapeAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* PhoenixTapeAudioProcessor::createEditor()
{
    return new PhoenixTapeAudioProcessorEditor(*this);
}

const juce::String PhoenixTapeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PhoenixTapeAudioProcessor::acceptsMidi() const
{
    return false;
}

bool PhoenixTapeAudioProcessor::producesMidi() const
{
    return false;
}

bool PhoenixTapeAudioProcessor::isMidiEffect() const
{
    return false;
}

double PhoenixTapeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PhoenixTapeAudioProcessor::getNumPrograms()
{
    return 1;
}

int PhoenixTapeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PhoenixTapeAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String PhoenixTapeAudioProcessor::getProgramName(int index)
{
    return {};
}

void PhoenixTapeAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhoenixTapeAudioProcessor();
}



