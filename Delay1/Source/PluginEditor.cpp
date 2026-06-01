/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayAudioProcessorEditor::DelayAudioProcessorEditor (DelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    //addAndMakeVisible your sliders
    addAndMakeVisible(delayTimeSlider);
   
    
    //Set the styles  and attach them to the components
   delayTimeSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
   delayTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
   delayTimeLabel.setText("Delay Time", juce::dontSendNotification);
   delayTimeLabel.setJustificationType(juce::Justification::centred);
    delayTimeLabel.attachToComponent(&delayTimeSlider, false);
    
    addAndMakeVisible(feedbackSlider);
   
    
    //Set the styles  and attach them to the components
   feedbackSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
   feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
   feedbackLabel.setText("Feedback", juce::dontSendNotification);
   feedbackLabel.setJustificationType(juce::Justification::centred);
    feedbackLabel.attachToComponent(&feedbackSlider, false);
   
    addAndMakeVisible(mixSlider);
   
    //Set the styles  and attach them to the components
   mixSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
   mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
   mixLabel.setText("Mix", juce::dontSendNotification);
   mixLabel.setJustificationType(juce::Justification::centred);
    mixLabel.attachToComponent(&mixSlider, false);
    
    //Attach them to the processor
    delayTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "delayTime", delayTimeSlider);
    
    feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "feedback", feedbackSlider);
     
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "mix", feedbackSlider);
    
    setSize (400, 300);
}

DelayAudioProcessorEditor::~DelayAudioProcessorEditor()
{
}

//==============================================================================
void DelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

        g.setColour (juce::Colours::white); // A nice color for the title
        g.setFont (juce::FontOptions (24.0f, juce::Font::bold)); // Make it bigger and bold

        auto headerArea = getLocalBounds().removeFromTop(50);
            
        g.drawFittedText ("Delay", headerArea, juce::Justification::centred, 1);
}

void DelayAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
        
        area.removeFromTop(50);

        auto topRow = area.removeFromTop(100);
             
        delayTimeSlider.setBounds(topRow.removeFromLeft(100));
        feedbackSlider.setBounds(topRow.removeFromLeft(100));
        mixSlider.setBounds(topRow.removeFromLeft(100));
       
}

