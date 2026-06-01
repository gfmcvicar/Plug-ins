/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SynthADRAudioProcessorEditor::SynthADRAudioProcessorEditor (SynthADRAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), myKeyboard(audioProcessor.keyboardState, juce::MidiKeyboardComponent::Orientation::horizontalKeyboard)
{
    //Sliders
    addAndMakeVisible(attackSlider);
    addAndMakeVisible(decaySlider);
    addAndMakeVisible(sustainSlider);
    addAndMakeVisible(releaseSlider);
        
    //ComboBox
    addAndMakeVisible(oscSelection);
    oscSelection.addItemList(juce::StringArray {"Sine", "SawTooth", "Square"}, 1);
    oscSelection.setJustificationType(juce::Justification::centred);
    
    //Styling sliders:
    attackSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    attackLabel.setText("Attack", juce::dontSendNotification);
    attackLabel.setJustificationType(juce::Justification::centred);
    attackLabel.attachToComponent(&attackSlider, false);

    decaySlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    decaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    decayLabel.setText("Decay", juce::dontSendNotification);
    decayLabel.setJustificationType(juce::Justification::centred);
    decayLabel.attachToComponent(&decaySlider, false);
    
    sustainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    sustainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    sustainLabel.setText("Sustain", juce::dontSendNotification);
    sustainLabel.setJustificationType(juce::Justification::centred);
    sustainLabel.attachToComponent(&sustainSlider, false);
    
    releaseSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    releaseLabel.setText("Release", juce::dontSendNotification);
    releaseLabel.setJustificationType(juce::Justification::centred);
    releaseLabel.attachToComponent(&releaseSlider, false);
    
    //Attachments
    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "attack", attackSlider);
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "decay", decaySlider);
    sustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "sustain", sustainSlider);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "release", releaseSlider);
    
    // ComboBox attachment - THIS WAS MISSING!
    oscSelectionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.parameters, "osc", oscSelection);
            
    setSize (600, 450);
    addAndMakeVisible(&myKeyboard);
}

SynthADRAudioProcessorEditor::~SynthADRAudioProcessorEditor()
{
}

//==============================================================================
void SynthADRAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (24.0f, juce::Font::bold));

    auto headerArea = getLocalBounds().removeFromTop(50);
        
    g.drawFittedText ("Synth ADR", headerArea, juce::Justification::centred, 1);
}

void SynthADRAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    
    // Remove the same 50 pixels we used for the title in paint()
    // so the rest of the UI starts below the title
    area.removeFromTop(50);

    auto topRow = area.removeFromTop(100);
         
    attackSlider.setBounds(topRow.removeFromLeft(100));
    decaySlider.setBounds(topRow.removeFromLeft(100));
    sustainSlider.setBounds(topRow.removeFromLeft(100));
    releaseSlider.setBounds(topRow.removeFromLeft(100));
           
    oscSelection.setBounds(area.removeFromTop(40).withSizeKeepingCentre(150, 30));
       
    myKeyboard.setBounds(area.removeFromBottom(140));
}