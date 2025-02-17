#pragma once
#include <JuceHeader.h>

class SSLCompressor {
public:
    void prepare(const juce::dsp::ProcessSpec& spec) {
        sampleRate = spec.sampleRate;
        gainReductionSmoother.reset(sampleRate);
        updateFilters();
    }

    void process(juce::AudioBuffer<float>& buffer, float threshold, float ratio, 
                float attackMs, float releaseMs, float makeup, bool hpfEnabled,
                bool midSide, float drive, float stereoLink) {
        
        // Mid/Side processing
        if (midSide) processMidSide(buffer, true);

        // Compression
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        
        for (int sample = 0; sample < numSamples; ++sample) {
            float sum = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch) {
                sum += buffer.getSample(ch, sample);
            }
            float input = sum / numChannels;

            // Sidechain HPF
            float sidechain = hpfEnabled ? hpf.processSample(input) : input;
            
            // Gain reduction
            float levelDb = juce::Decibels::gainToDecibels(std::abs(sidechain));
            float gr = calculateGainReduction(levelDb, threshold, ratio);
            
            // Smoothing
            gr = gainReductionSmoother.process(gr, attackMs, releaseMs);
            lastGR = gr;

            // Apply gain
            float gain = juce::Decibels::decibelsToGain(gr + makeup);
            for (int ch = 0; ch < numChannels; ++ch) {
                buffer.setSample(ch, sample, buffer.getSample(ch, sample) * gain);
            }
        }

        // Saturation
        applySaturation(buffer, drive);

        // Mid/Side decode
        if (midSide) processMidSide(buffer, false);
    }

    float getLastGR() const { return lastGR; }

private:
    float sampleRate = 44100.0f;
    float lastGR = 0.0f;
    juce::dsp::IIR::Filter<float> hpf;

    struct BallisticsFilter {
        float env = 0.0f;
        float process(float input, float attackCoef, float releaseCoef) {
            if (input > env) env += attackCoef * (input - env);
            else env += releaseCoef * (input - env);
            return env;
        }
    } gainReductionSmoother;

    void updateFilters() {
        *hpf.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 150.0f);
    }

    float calculateGainReduction(float levelDb, float threshold, float ratio) {
        if (levelDb <= threshold) return 0.0f;
        return (threshold - levelDb) * (1.0f - 1.0f / ratio);
    }

    void applySaturation(juce::AudioBuffer<float>& buffer, float drive) {
        const float threshold = 0.8f;
        const float knee = 0.1f;
        
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            auto* data = buffer.getWritePointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i) {
                float x = data[i];
                if (x > threshold + knee) {
                    x = threshold + (x - threshold) / (1.0f + drive * (x - threshold));
                } else if (x < -threshold - knee) {
                    x = -threshold + (x + threshold) / (1.0f + drive * std::abs(x + threshold));
                }
                data[i] = x * (1.0f + drive * 0.3f);
            }
        }
    }

    void processMidSide(juce::AudioBuffer<float>& buffer, bool encode) {
        auto* left = buffer.getWritePointer(0);
        auto* right = buffer.getWritePointer(1);
        const float sqrt2 = 0.70710678f;

        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            float m = left[i];
            float s = right[i];
            
            if (encode) {
                m = (left[i] + right[i]) * sqrt2;
                s = (left[i] - right[i]) * sqrt2;
            } else {
                left[i] = (m + s) * sqrt2;
                right[i] = (m - s) * sqrt2;
            }
        }
    }
};