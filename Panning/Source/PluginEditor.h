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
class PanningAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    PanningAudioProcessorEditor (PanningAudioProcessor&);
    ~PanningAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
private:
        PanningAudioProcessor& audioProcessor;

        juce::Slider panning;
        juce::Slider distance;

        juce::Label panningLabel;
        juce::Label distanceLabel;

        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panningAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> distanceAttachment;
    
};
