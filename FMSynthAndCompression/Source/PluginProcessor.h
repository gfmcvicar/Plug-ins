/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class FMSynthCompressionAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    FMSynthCompressionAudioProcessor();
    ~FMSynthCompressionAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState parameters;
    //Declare float variables for modulatorPhase and modulatedCarrierPhase and initiliaze them in 0.
    
              // Phase of the modulator, range 0 to 1
       // Phase of the carrier, with modulation, range 0 to 1
    
    //Declare 2 float variablesmodulatorSignal and modulated CarrierSignal
    
    
    //Compression
    juce::dsp::ProcessSpec spec;
    juce::dsp::Compressor<float> compressor;

    float modulatorPhase = 0.0f;
    float carrierPhase = 0.0f;
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FMSynthCompressionAudioProcessor)
    
    std::atomic<float>* modulationDepthParam = nullptr;
    std::atomic<float>* modulatorFrequencyParam = nullptr;
    std::atomic<float>* carrierFrequencyParam = nullptr;

};
