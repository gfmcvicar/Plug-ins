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
class ReverbDistortionAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ReverbDistortionAudioProcessorEditor (ReverbDistortionAudioProcessor&);
    ~ReverbDistortionAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ReverbDistortionAudioProcessor& audioProcessor;
    //Distortion
    juce::Slider GainDistortion;
    juce::ComboBox DistortionSelection;
    juce::Label GainDistortionLabel,DistortionSelectionLabel;
    //Reverb
    juce::Slider roomSize,damping,wetLevel,dryLevel,width;
    juce::Label roomSizeLabel, dampingLabel,wetLevelLabel,dryLevelLabel,widthLabel;
    //Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> DistortionSelectionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> GainDistortionAttachment,roomSizeAttachment,dampingAttachment,wetLevelAttachment,dryLevelAttachment,widthAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbDistortionAudioProcessorEditor)
};
