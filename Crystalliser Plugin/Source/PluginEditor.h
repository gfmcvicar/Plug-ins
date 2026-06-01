#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// ==============================================================================
// LOOK AND FEEL
// ==============================================================================

class CrystalLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CrystalLookAndFeel();

    void drawLabel        (juce::Graphics&, juce::Label&) override;
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;
};

// ==============================================================================
// EDITOR
// ==============================================================================

class CrystalliserAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit CrystalliserAudioProcessorEditor (CrystalliserAudioProcessor&);
    ~CrystalliserAudioProcessorEditor() override;

    void paint   (juce::Graphics&) override;
    void resized () override;

private:
    CrystalliserAudioProcessor& processor;
    CrystalLookAndFeel crystalLookAndFeel;

    // Sliders
    juce::Slider lengthSlider, mixSlider, persistenceSlider;
    juce::Slider lfoRateSlider, lfoDepthSlider;
    juce::Slider roomSlider, dampSlider, widthSlider, revWetSlider;

    // Labels
    juce::Label lengthLabel, mixLabel, persistenceLabel;
    juce::Label lfoRateLabel, lfoDepthLabel;
    juce::Label roomLabel, dampLabel, widthLabel, revWetLabel;

    // Attachments
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> lengthAttach, mixAttach, persistenceAttach;
    std::unique_ptr<Attachment> lfoRateAttach, lfoDepthAttach;
    std::unique_ptr<Attachment> roomAttach, dampAttach, widthAttach, revWetAttach;

    void setupControl (juce::Slider&, juce::Label&, std::unique_ptr<Attachment>&,
                       const juce::String& paramID, const juce::String& title);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CrystalliserAudioProcessorEditor)
};
