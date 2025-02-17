#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SSLCompressorAudioProcessor::SSLCompressorAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "SSLParameters", createParameters())
{
    // Initialize parameter listeners
    parameters.addParameterListener("threshold", this);
    parameters.addParameterListener("ratio", this);
    // Add listeners for other parameters as needed...
}

SSLCompressorAudioProcessor::~SSLCompressorAudioProcessor()
{
}

//==============================================================================
void SSLCompressorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // DSP Setup
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    compressor.prepare(spec);
    dcOffset = 0.0f;

    // Oversampling
    oversampling.reset(new juce::dsp::Oversampling<float>(
        spec.numChannels, 
        2, // 4x oversampling
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true
    ));
    oversampling->initProcessing(samplesPerBlock);

    // RMS
    rmsBuffer.setSize(spec.numChannels, rmsWindowSize);
    rmsBuffer.clear();
    writePos = 0;
}

void SSLCompressorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    // Parameter fetching
    auto threshold = parameters.getRawParameterValue("threshold")->load();
    auto ratio = parameters.getRawParameterValue("ratio")->load();
    auto attack = parameters.getRawParameterValue("attack")->load();
    auto release = parameters.getRawParameterValue("release")->load();
    auto makeup = parameters.getRawParameterValue("makeup")->load();
    auto hpfOn = parameters.getRawParameterValue("hpf")->load() > 0.5f;
    auto midSide = parameters.getRawParameterValue("midside")->load() > 0.5f;
    auto mix = parameters.getRawParameterValue("mix")->load();
    auto drive = parameters.getRawParameterValue("drive")->load();
    auto stereoLink = parameters.getRawParameterValue("stereolink")->load();
    
    // Create dry buffer for mix control
    juce::AudioBuffer<float> dryBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    dryBuffer.makeCopyOf(buffer);

    // Oversampling
    auto osBuffer = oversampling->processSamplesUp(buffer);
    
    // Mid/Side processing
    if(midSide) encodeMidSide(osBuffer);
    
    // Compression processing
    compressor.process(osBuffer, threshold, ratio, attack, release, makeup, 
                      hpfOn, midSide, drive, stereoLink);
    
    // Saturation
    applySaturation(osBuffer, drive);
    
    // Convert back from Mid/Side
    if(midSide) decodeMidSide(osBuffer);
    
    // Downsampling
    oversampling->processSamplesDown(buffer);
    
    // Mix control
    applyMix(buffer, dryBuffer, mix);
    
    // DC offset emulation
    applyDCOffset(buffer);
    
    // Update metering
    updateMeters(buffer);
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout SSLCompressorAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "threshold", "Threshold", 
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f), -10.0f, "dB"
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "ratio", "Ratio", 
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f), 4.0f
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "attack", "Attack", 
        juce::NormalisableRange<float>(0.1f, 100.0f, 0.1f), 10.0f, "ms"
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "release", "Release", 
        juce::NormalisableRange<float>(50.0f, 3000.0f, 1.0f), 300.0f, "ms"
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "makeup", "Makeup Gain", 
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f, "dB"
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "hpf", "Sidechain HPF", false
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "midside", "Mid/Side Mode", false
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "mix", "Mix", 
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "drive", "Saturation", 
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "autorelease", "Auto Release", true
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "stereolink", "Stereo Link", 
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
void SSLCompressorAudioProcessor::encodeMidSide(juce::AudioBuffer<float>& buffer)
{
    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    
    for(int i = 0; i < buffer.getNumSamples(); ++i) {
        const float mid = (left[i] + right[i]) * 0.7071f;
        const float side = (left[i] - right[i]) * 0.7071f;
        left[i] = mid;
        right[i] = side;
    }
}

void SSLCompressorAudioProcessor::decodeMidSide(juce::AudioBuffer<float>& buffer)
{
    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    
    for(int i = 0; i < buffer.getNumSamples(); ++i) {
        const float mid = left[i];
        const float side = right[i];
        left[i] = (mid + side) * 0.7071f;
        right[i] = (mid - side) * 0.7071f;
    }
}

void SSLCompressorAudioProcessor::applyMix(juce::AudioBuffer<float>& wet, 
                                         const juce::AudioBuffer<float>& dry, 
                                         float mix)
{
    for(int ch = 0; ch < wet.getNumChannels(); ++ch) {
        wet.addFrom(ch, 0, dry, ch, 0, wet.getNumSamples(), 1.0f - mix);
        wet.applyGain(ch, 0, wet.getNumSamples(), mix);
    }
}

void SSLCompressorAudioProcessor::applyDCOffset(juce::AudioBuffer<float>& buffer)
{
    dcOffset = 0.0001f * random.nextFloat() * 
              std::sin(juce::MathConstants<float>::twoPi * random.nextFloat());
    
    for(int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        buffer.addFrom(ch, 0, dcOffset, buffer.getNumSamples());
    }
}

void SSLCompressorAudioProcessor::updateMeters(const juce::AudioBuffer<float>& buffer)
{
    // Update gain reduction meter
    currentGR = compressor.getLastGR();
    
    // Update RMS meters
    inputRMS = calculateRMS(buffer);
    outputRMS = calculateRMS(buffer);
}

float SSLCompressorAudioProcessor::calculateRMS(const juce::AudioBuffer<float>& buffer)
{
    float sum = 0.0f;
    const int numSamples = buffer.getNumSamples();
    
    for(int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        const float* channelData = buffer.getReadPointer(ch);
        for(int i = 0; i < numSamples; ++i) {
            sum += channelData[i] * channelData[i];
        }
    }
    
    return std::sqrt(sum / (numSamples * buffer.getNumChannels()));
}

//==============================================================================
// JUCE Boilerplate
const juce::String SSLCompressorAudioProcessor::getName() const { return "SSL Bus Compressor"; }
bool SSLCompressorAudioProcessor::acceptsMidi() const { return false; }
bool SSLCompressorAudioProcessor::producesMidi() const { return false; }
double SSLCompressorAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int SSLCompressorAudioProcessor::getNumPrograms() { return 1; }
int SSLCompressorAudioProcessor::getCurrentProgram() { return 0; }
void SSLCompressorAudioProcessor::setCurrentProgram(int) { }
const juce::String SSLCompressorAudioProcessor::getProgramName(int) { return {}; }
void SSLCompressorAudioProcessor::changeProgramName(int, const juce::String&) { }

//==============================================================================
// Save/Load state
void SSLCompressorAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SSLCompressorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if(xmlState.get() != nullptr)
        if(xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SSLCompressorAudioProcessor();
}