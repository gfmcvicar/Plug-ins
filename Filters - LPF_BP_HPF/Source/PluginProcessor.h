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
class LPF_BP_HPFAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    LPF_BP_HPFAudioProcessor();
    ~LPF_BP_HPFAudioProcessor() override;

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
        
    //Loading a file
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    void loadFile(const juce::File& file);

    //DSP
    using Filter = juce::dsp::IIR::Filter<float>; //Avoid writing again juce::Dsp::filter
    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    using FilterPtr = std::shared_ptr<Coefficients>;


    struct Band
    {
        juce::dsp::ProcessorDuplicator<Filter, Coefficients> filter;
        juce::dsp::Gain<float> gain;
        
    };
    
    Band lowBand;
    Band midLowBand;
    Band midHighBand;
    Band highBand;

    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LPF_BP_HPFAudioProcessor)
};
