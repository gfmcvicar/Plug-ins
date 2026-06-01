/*
  ==============================================================================

    Signals.h
    Created: 8 Feb 2026 8:18:19pm
    Author:  Nelly Victoria Alexandra Garcia Sihuay

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

// Synthesiser Sound class - required for JUCE synthesiser
class MySignal : public juce::SynthesiserSound {
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

// Base class for all voice types
class VoiceBase : public juce::SynthesiserVoice
{
public:
    VoiceBase() {}
    
    bool canPlaySound(juce::SynthesiserSound* sound) override {
        return dynamic_cast<MySignal*>(sound) != nullptr;
    }
    
    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}
    
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int) override {
        // Calculate frequency from MIDI note number
        auto cyclesPerSecond = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        auto cyclesPerSample = cyclesPerSecond / getSampleRate();
        phaseDelta = cyclesPerSample * 2.0f * juce::MathConstants<float>::pi;
        
        // Set level based on velocity
        level = velocity * 0.15f;
        
        // Reset phase
        phase = 0.0f;
        
        // Start ADSR envelope
        adsr.noteOn();
    }

    void stopNote(float, bool allowTailOff) override {
        adsr.noteOff();
        if (!allowTailOff || !adsr.isActive())
            clearCurrentNote();
    }

    virtual float getNextSample() = 0;

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
        while (--numSamples >= 0) {
            auto rawSample = getNextSample();
            auto env = adsr.getNextSample();
            auto finalSample = rawSample * env * level;

            for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                outputBuffer.addSample(i, startSample, finalSample);

            startSample++;
            
            if (!adsr.isActive())
                clearCurrentNote();
        }
    }
    
    void setSampleRate(double newSampleRate) {
        adsr.setSampleRate(newSampleRate);
    }

    void updateAdsr(float a, float d, float s, float r, double sr, int oscChoice) {
        oscSelection = oscChoice;
        adsr.setSampleRate(sr);
        adsrParams.attack = a;
        adsrParams.decay = d;
        adsrParams.sustain = s;
        adsrParams.release = r;
        adsr.setParameters(adsrParams);
    }

protected:
    float phase = 0.0f;
    float phaseDelta = 0.0f;
    float level = 0.0f;
    int oscSelection = 0;
    
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
};

// Sine wave oscillator
class SineVoice : public VoiceBase {
public:
    bool canPlaySound(juce::SynthesiserSound* sound) override {
        return oscSelection == 0 && dynamic_cast<MySignal*>(sound) != nullptr;
    }
    
    float getNextSample() override {
        // Generate sine wave
        auto sample = std::sin(phase);
        
        // Increment phase
        phase += phaseDelta;
        
        // Wrap phase to prevent overflow
        if (phase > 2.0f * juce::MathConstants<float>::pi)
            phase -= 2.0f * juce::MathConstants<float>::pi;
        
        return sample;
    }
};

// Sawtooth wave oscillator
class SawVoice : public VoiceBase {
public:
    bool canPlaySound(juce::SynthesiserSound* sound) override {
        return oscSelection == 1 && dynamic_cast<MySignal*>(sound) != nullptr;
    }
    
    float getNextSample() override {
        // Generate sawtooth wave (ramps from -1 to 1)
        auto sample = (phase / juce::MathConstants<float>::pi) - 1.0f;
        
        // Increment phase
        phase += phaseDelta;
        
        // Wrap phase to prevent overflow
        if (phase > 2.0f * juce::MathConstants<float>::pi)
            phase -= 2.0f * juce::MathConstants<float>::pi;
        
        return sample;
    }
};

// Square wave oscillator
class SquareVoice : public VoiceBase {
public:
    bool canPlaySound(juce::SynthesiserSound* sound) override {
        return oscSelection == 2 && dynamic_cast<MySignal*>(sound) != nullptr;
    }
    
    float getNextSample() override {
        // Generate square wave (switches between -1 and 1)
        auto sample = phase < juce::MathConstants<float>::pi ? 1.0f : -1.0f;
        
        // Increment phase
        phase += phaseDelta;
        
        // Wrap phase to prevent overflow
        if (phase > 2.0f * juce::MathConstants<float>::pi)
            phase -= 2.0f * juce::MathConstants<float>::pi;
        
        return sample;
    }
};

