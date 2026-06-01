#pragma once
#include <JuceHeader.h>

class CrystalliserAudioProcessor  : public juce::AudioProcessor
{
public:
    CrystalliserAudioProcessor();
    ~CrystalliserAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    
    // ===== Parameters =====
    
    juce::AudioProcessorValueTreeState parameters;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:


    // ===== Crystalliser Engine =====
    enum State { Recording, Frozen };
    State state = Recording;

    juce::AudioBuffer<float> CrystalliserBuffer;
    int writeIndex = 0;
    float playbackIndex = 0.0f;
    int loopStart = 0;
    float currentLoopLength = 0;
    int targetLoopLength = 0;
    float envelope = 0.0f;
    float fade = 1.0f;
    float lfoPhase = 0.0f;
    float compEnvelope = 0.0f;
    double sr = 44100.0;
    int loopCounter = 0;
    float CrystalliserRMS = 0.0f;

    // ===== Reverb =====
    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;
    
    juce::dsp::Limiter<float> masterLimiter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CrystalliserAudioProcessor)
};
