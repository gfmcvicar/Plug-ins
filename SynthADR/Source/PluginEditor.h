/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Signals.h"

//==============================================================================
/**
*/
class SynthADRAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SynthADRAudioProcessorEditor (SynthADRAudioProcessor&);
    ~SynthADRAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SynthADRAudioProcessor& audioProcessor;
    juce::Slider attackSlider,sustainSlider, releaseSlider,decaySlider;
    juce::ComboBox oscSelection;
    //labels
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel;
    //Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> oscSelectionAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    
    
        
    juce::MidiKeyboardComponent myKeyboard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthADRAudioProcessorEditor)
};