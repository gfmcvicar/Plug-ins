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
class LPF_BP_HPFAudioProcessorEditor  : public juce::AudioProcessorEditor
{
    public:
        LPF_BP_HPFAudioProcessorEditor (LPF_BP_HPFAudioProcessor&);
        ~LPF_BP_HPFAudioProcessorEditor() override;

        //==============================================================================
        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        LPF_BP_HPFAudioProcessor& audioProcessor;
        juce::Slider lowGain,midlowGain,midhighGain,highGain,lowQ,midlowQ,midhighQ,highQ,lowFrequency,midlowFrequency,midhighFrequency,highFrequency;
        juce::Label lowLabel,midlowLabel,midhighLabel,highLabel,lowQLabel,midlowQLabel,midhighQLabel,highQLabel,lowFrequencyLabel,midlowFrequencyLabel,midhighFrequencyLabel,highFrequencyLabel;
        juce::TextButton file,play,stop;
        std::unique_ptr<juce::FileChooser> chooser;
        
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowGainAttachment,midlowGainAttachment,midhighGainAttachment, highGainAttachment,lowQattachment, midlowQAttachment,midhighQAttachment,highQAttachment,lowFrequencyAttachment,midlowFrequencyAttachment,midhighFrequencyAttachment,highFrequencyAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> fileAttachment,playAttachment,stopAttachment;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LPF_BP_HPFAudioProcessorEditor)
    };
