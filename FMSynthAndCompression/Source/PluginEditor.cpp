/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FMSynthCompressionAudioProcessorEditor::FMSynthCompressionAudioProcessorEditor (FMSynthCompressionAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    auto sliders = { &ModulationDepth, &ModulatorFrequency, &CarrierFrequency, &Threshold,&Ratio,&AttackTime,&ReleaseTime,&MakeUpGain };
    auto labels  = { &ModulationDepthLabel, &ModulatorFrequencyLabel,&CarrierFrequencyLabel,&ThresholdLabel,&RatioLabel,&AttackTimeLabel,&ReleaseTimeLabel,&MakeUpGainLabel };
    for (auto* slider : sliders)
               {
                   addAndMakeVisible (slider);
                   slider->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
                   slider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
               }
           for (auto* label : labels)
               {
                   addAndMakeVisible (label);
                   label->setJustificationType (juce::Justification::centred);
                   label->setBorderSize(juce::BorderSize<int>(0));
               }
           
         ModulationDepthLabel.setText ("Modulation Depth", juce::dontSendNotification);
        ModulationDepthLabel.setJustificationType(juce::Justification::centred);
        ModulationDepthLabel.attachToComponent (&ModulationDepth, false);
    
        ModulatorFrequencyLabel.setText ("Modulator Frequency", juce::dontSendNotification);
        ModulatorFrequencyLabel.setJustificationType(juce::Justification::centred);
        ModulatorFrequencyLabel.attachToComponent (&ModulatorFrequency, false);
    
        CarrierFrequencyLabel.setText ("Carrier Frequency", juce::dontSendNotification);
        CarrierFrequencyLabel.setJustificationType(juce::Justification::centred);
        CarrierFrequencyLabel.attachToComponent (&CarrierFrequency, false);
    
    //Compression
    ThresholdLabel.setText ("Threshold", juce::dontSendNotification);
   ThresholdLabel.setJustificationType(juce::Justification::centred);
   ThresholdLabel.attachToComponent (&Threshold, false);
    
    RatioLabel.setText ("Ratio", juce::dontSendNotification);
   RatioLabel.setJustificationType(juce::Justification::centred);
   RatioLabel.attachToComponent (&Ratio, false);
    
    AttackTimeLabel.setText ("AttackTime", juce::dontSendNotification);
   AttackTimeLabel.setJustificationType(juce::Justification::centred);
   AttackTimeLabel.attachToComponent (&AttackTime, false);
    
    ReleaseTimeLabel.setText ("ReleaseTime", juce::dontSendNotification);
   ReleaseTimeLabel.setJustificationType(juce::Justification::centred);
   ReleaseTimeLabel.attachToComponent (&ReleaseTime, false);
    
   MakeUpGainLabel.setText ("MakeUpGain", juce::dontSendNotification);
   MakeUpGainLabel.setJustificationType(juce::Justification::centred);
   MakeUpGainLabel.attachToComponent (&MakeUpGain, false);
    
    
    //Attachments
    ModulationDepthAttachment= std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "ModulationDepth", ModulationDepth);

    ModulatorFrequencyAttachment= std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "ModulatorFrequency",ModulatorFrequency);

    CarrierFrequencyAttachment= std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "CarrierFrequency", CarrierFrequency);
    
    //Compression
    ThresholdAttachment= std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "Threshold", Threshold);
    RatioAttachment= std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "Ratio", Ratio);
    AttackTimeAttachment= std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "AttackTime", AttackTime);
    ReleaseTimeAttachment= std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "ReleaseTime", ReleaseTime);
    MakeUpGainAttachment= std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "MakeUpGain", MakeUpGain);
           
    setSize (500, 500);
}

FMSynthCompressionAudioProcessorEditor::~FMSynthCompressionAudioProcessorEditor()
{
}

//==============================================================================
void FMSynthCompressionAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    auto bounds = getLocalBounds().reduced(10).withTrimmedTop(40);
    auto fmArea   = bounds.removeFromTop(bounds.getHeight() / 2).reduced(2);
    auto compArea = bounds.reduced(2);

    // FM box
    g.setColour (juce::Colours::grey.withAlpha(0.2f));
    g.fillRoundedRectangle(fmArea.toFloat(), 10.0f);
    g.setColour (juce::Colours::white.withAlpha(0.5f));
    g.drawRoundedRectangle(fmArea.toFloat(), 10.0f, 2.0f);

    // Compression box
    g.setColour (juce::Colours::grey.withAlpha(0.2f));
    g.fillRoundedRectangle(compArea.toFloat(), 10.0f);
    g.setColour (juce::Colours::white.withAlpha(0.5f));
    g.drawRoundedRectangle(compArea.toFloat(), 10.0f, 2.0f);

    // Headers
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    g.drawText ("FM MODULATION", getLocalBounds().removeFromTop(40), juce::Justification::centred);
}

void FMSynthCompressionAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    area.removeFromTop(40); // title

    auto fmArea   = area.removeFromTop(area.getHeight() / 2).reduced(5);
    auto compArea = area.reduced(5);

    // FM sliders
    auto sliderWidth = fmArea.getWidth() / 3;
    ModulationDepth.setBounds    (fmArea.removeFromLeft(sliderWidth).reduced(10));
    ModulatorFrequency.setBounds (fmArea.removeFromLeft(sliderWidth).reduced(10));
    CarrierFrequency.setBounds   (fmArea.removeFromLeft(sliderWidth).reduced(10));

    // Compression sliders
    sliderWidth = compArea.getWidth() / 5;
    Threshold.setBounds   (compArea.removeFromLeft(sliderWidth).reduced(10));
    Ratio.setBounds       (compArea.removeFromLeft(sliderWidth).reduced(10));
    AttackTime.setBounds  (compArea.removeFromLeft(sliderWidth).reduced(10));
    ReleaseTime.setBounds (compArea.removeFromLeft(sliderWidth).reduced(10));
    MakeUpGain.setBounds  (compArea.removeFromLeft(sliderWidth).reduced(10));
}