/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class FMSynthCompressionAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    FMSynthCompressionAudioProcessorEditor (FMSynthCompressionAudioProcessor&);
    ~FMSynthCompressionAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    juce::Slider ModulationDepth, ModulatorFrequency, CarrierFrequency, Threshold,Ratio,AttackTime,ReleaseTime, MakeUpGain;
    juce::Label ModulationDepthLabel, ModulatorFrequencyLabel,CarrierFrequencyLabel,ThresholdLabel,RatioLabel,AttackTimeLabel,ReleaseTimeLabel, MakeUpGainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ModulationDepthAttachment,ModulatorFrequencyAttachment, CarrierFrequencyAttachment,ThresholdAttachment, RatioAttachment,ReleaseTimeAttachment,MakeUpGainAttachment,AttackTimeAttachment;
  
private:

    FMSynthCompressionAudioProcessor& audioProcessor;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FMSynthCompressionAudioProcessorEditor)
};