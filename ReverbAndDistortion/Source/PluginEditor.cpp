/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ReverbDistortionAudioProcessorEditor::ReverbDistortionAudioProcessorEditor (ReverbDistortionAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    std::vector<juce::Slider*> sliders =
    {
        &GainDistortion,
        &roomSize,
        &damping,
        &wetLevel,
        &dryLevel,
        &width
    };

    std::vector<juce::Label*> labels =
    {
        &GainDistortionLabel,
        &roomSizeLabel,
        &dampingLabel,
        &wetLevelLabel,
        &dryLevelLabel,
        &widthLabel
    };

    juce::StringArray names =
    {
        "Gain", "Room Size", "Damping", "Wet", "Dry", "Width"
    };

    for (size_t i = 0; i < sliders.size(); ++i)
    {
        auto* slider = sliders[i];
        auto* label  = labels[i];

        addAndMakeVisible(slider);
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);

        addAndMakeVisible(label);
        label->setText(names[(int)i], juce::dontSendNotification);
        label->setJustificationType(juce::Justification::centred);
        label->attachToComponent(slider, false);
    }
    
    for (int i = 0; i < sliders.size(); ++i)
    {
        auto* slider = *(sliders.begin() + i);
        auto* label  = *(labels.begin() + i);
        
        addAndMakeVisible(slider);
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);

        addAndMakeVisible(label);
        label->setText(names[i], juce::dontSendNotification);
        label->setJustificationType(juce::Justification::centred);
        label->attachToComponent(slider, false);
    }
    
    addAndMakeVisible(DistortionSelection);
    DistortionSelection.addItemList(juce::StringArray {"Hard Clipping", "Soft Clipping", "Exponential Soft","Full Wave Rectifier", "Half Wave Rectifier"}, 1);
    DistortionSelection.setJustificationType(juce::Justification::centred);
        
            
    //Attachments
   GainDistortionAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "GainDistortion", GainDistortion);
    DistortionSelectionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.parameters, "DistortionSelection", DistortionSelection);
    
    roomSizeAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "roomSize", roomSize);
    dampingAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "damping",damping);
    wetLevelAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "wetLevel", wetLevel);
    dryLevelAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "dryLevel", dryLevel);
    widthAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "width", width);
    
    setSize (500, 500);
}

ReverbDistortionAudioProcessorEditor::~ReverbDistortionAudioProcessorEditor()
{
}

//==============================================================================
void ReverbDistortionAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (24.0f, juce::Font::bold));
    
    auto bounds = getLocalBounds();
    auto headerArea = bounds.removeFromTop(60);
    g.drawFittedText ("Reverb & Distortion", headerArea, juce::Justification::centred, 1);

    // Draw Section Outlines
    g.setColour (juce::Colours::grey.withAlpha(0.3f));

    auto mainArea = bounds.reduced(20);
    
    // Distortion Rect
    auto distRect = mainArea.removeFromTop(mainArea.getHeight() / 2);
    g.drawRoundedRectangle(distRect.toFloat(), 10.0f, 2.0f);
    g.drawText("Distortion", distRect.removeFromTop(20).reduced(10, 0), juce::Justification::left);

    // Reverb Rect
    mainArea.removeFromTop(20); // gap
    g.drawRoundedRectangle(mainArea.toFloat(), 10.0f, 2.0f);
    g.drawText("Reverb", mainArea.removeFromTop(20).reduced(10, 0), juce::Justification::left);
}

void ReverbDistortionAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(60);
    area.reduce(20, 20);

    auto distortionArea = area.removeFromTop(area.getHeight() / 2);
    DistortionSelection.setBounds(distortionArea.removeFromTop(30));
    distortionArea.removeFromTop(10);
    GainDistortion.setBounds(distortionArea.removeFromTop(100).withSizeKeepingCentre(100, 100));

    area.removeFromTop(20);
    auto reverbArea = area;
    
    auto sliderWidth = reverbArea.getWidth() / 5;
    auto reverbSliders = { &roomSize, &damping, &wetLevel, &dryLevel, &width };

    for (auto* slider : reverbSliders)
    {
        slider->setBounds(reverbArea.removeFromLeft(sliderWidth));
    }
}
