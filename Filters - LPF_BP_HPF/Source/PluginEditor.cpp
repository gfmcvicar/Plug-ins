/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LPF_BP_HPFAudioProcessorEditor::LPF_BP_HPFAudioProcessorEditor (LPF_BP_HPFAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    //BUTTONS:
        auto buttons={&file, &play, &stop};
         for (auto* button:buttons)
         {
             addAndMakeVisible(button);
             if (button != &file)
                     button->setClickingTogglesState(true);
         }
        file.setButtonText("Load File");
        fileAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "file", file);
        play.setButtonText("Play");
        playAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "play", play);
        stop.setButtonText("Stop");
        stopAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "stop", stop);
        
        auto sliders = { &lowGain, &midlowGain, &midhighGain, &highGain, &lowQ, &midlowQ, &midhighQ, &highQ, &lowFrequency, &midlowFrequency, &midhighFrequency, &highFrequency }; // Creates slider controls for all EQ parameters
        auto labels  = { &lowLabel, &midlowLabel, &midhighLabel, &highLabel, &lowQLabel, &midlowQLabel, &midhighQLabel, &highQLabel, &lowFrequencyLabel, &midlowFrequencyLabel, &midhighFrequencyLabel, &highFrequencyLabel };
        //instead of writing add andmake visible to every single slider we create foor loops :)
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
        lowFrequencyLabel.setText ("Low Frequency", juce::dontSendNotification);
        lowFrequencyLabel.setJustificationType(juce::Justification::centred);
        lowFrequencyLabel.attachToComponent (&lowFrequency, false);
    
        midlowFrequencyLabel.setText ("Mid Low Freq.", juce::dontSendNotification);
        midlowFrequencyLabel.setJustificationType(juce::Justification::centred);
        midlowFrequencyLabel.attachToComponent (&midlowFrequency, false);
    
        midhighFrequencyLabel.setText ("Mid High Freq.", juce::dontSendNotification);
        midhighFrequencyLabel.setJustificationType(juce::Justification::centred);
        midhighFrequencyLabel.attachToComponent (&midhighFrequency, false);
    
        highFrequencyLabel.setText ("High Frequency", juce::dontSendNotification);
        highFrequencyLabel.setJustificationType(juce::Justification::centred);
        highFrequencyLabel.attachToComponent (&highFrequency, false);
    
        lowLabel.setText ("Low Gain", juce::dontSendNotification);
        lowLabel.setJustificationType(juce::Justification::centred);
        lowLabel.attachToComponent (&lowGain, false);
    
        midlowLabel.setText ("Mid Low Gain", juce::dontSendNotification);
        midlowLabel.setJustificationType(juce::Justification::centred);
        midlowLabel.attachToComponent (&midlowGain, false);
    
        midhighLabel.setText ("Mid High Gain", juce::dontSendNotification);
        midhighLabel.setJustificationType(juce::Justification::centred);
        midhighLabel.attachToComponent (&midhighGain, false);
        
        highLabel.setText ("High Gain", juce::dontSendNotification);
        highLabel.setJustificationType(juce::Justification::centred);
        highLabel.attachToComponent (&highGain, false);
    
        lowQLabel.setText ("Low Q", juce::dontSendNotification);
        lowQLabel.setJustificationType(juce::Justification::centred);
        lowQLabel.attachToComponent (&lowQ, false);
    
        midlowQLabel.setText ("Mid Low Q", juce::dontSendNotification);
        midlowQLabel.setJustificationType(juce::Justification::centred);
        midlowQLabel.attachToComponent (&midlowQ, false);
    
        midhighQLabel.setText ("Mid High Q", juce::dontSendNotification);
        midhighQLabel.setJustificationType(juce::Justification::centred);
        midhighQLabel.attachToComponent (&midhighQ, false);
        
        highQLabel.setText ("High Q", juce::dontSendNotification);
        highQLabel.setJustificationType(juce::Justification::centred);
        highQLabel.attachToComponent (&highQ, false);
        
        lowFrequencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters,"lowFrequency", lowFrequency);
        midlowFrequencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters,"midlowFrequency", midlowFrequency);
        midhighFrequencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters,"midhighFrequency", midhighFrequency);
        highFrequencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters,"highFrequency", highFrequency);
        lowGainAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "lowGain", lowGain);
        midlowGainAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "midlowGain", midlowGain);
        midhighGainAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "midhighGain", midhighGain);
        highGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters,"highGain", highGain);
        lowQattachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "lowQ", lowQ);
        midlowQAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "midlowQ", midlowQ);
        midhighQAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "midhighQ", midhighQ);
        highQAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters,"highQ", highQ);
    
    
        //Load the file
        file.onClick = [this]()
        {
            chooser = std::make_unique<juce::FileChooser> ("Select an audio file...",juce::File{},"*.wav;*.mp3");

            auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

            chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)
            {
                auto file = fc.getResult();

                if (file.exists())
                {
                    audioProcessor.loadFile (file);
                }
            });
        };
        setSize (750, 500);
}

LPF_BP_HPFAudioProcessorEditor::~LPF_BP_HPFAudioProcessorEditor()
{
}

//==============================================================================
void LPF_BP_HPFAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

        g.setColour (juce::Colours::white);
        g.setFont (juce::FontOptions (24.0f, juce::Font::bold));

        auto headerArea = getLocalBounds().removeFromTop(50);
                    
        g.drawFittedText ("Parallel EQ", headerArea, juce::Justification::centred, 1);
}

void LPF_BP_HPFAudioProcessorEditor::resized() // Layout organised into four vertical columns
{
    auto area = getLocalBounds().reduced(20);
    area.removeFromTop(40); // title

    auto buttonArea = area.removeFromTop(40);

    int buttonWidth = buttonArea.getWidth() / 3;
    file.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(10));
    play.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(10));
    stop.setBounds(buttonArea.reduced(10));

    // ---------- 4 COLUMNS ----------
    int columnWidth = area.getWidth() / 4;

    auto lowArea      = area.removeFromLeft(columnWidth).reduced(20,0);
    auto midLowArea   = area.removeFromLeft(columnWidth).reduced(20,0);
    auto midHighArea  = area.removeFromLeft(columnWidth).reduced(20,0);
    auto highArea     = area.reduced(10,0);

    // ---------- ROW HEIGHT ----------
    int rowHeight = lowArea.getHeight() / 3;

    // LOW
    lowFrequency.setBounds(lowArea.removeFromTop(rowHeight).reduced(10));
    lowGain.setBounds(lowArea.removeFromTop(rowHeight).reduced(10));
    lowQ.setBounds(lowArea.removeFromTop(rowHeight).reduced(10));

    // MID LOW
    midlowFrequency.setBounds(midLowArea.removeFromTop(rowHeight).reduced(10));
    midlowGain.setBounds(midLowArea.removeFromTop(rowHeight).reduced(10));
    midlowQ.setBounds(midLowArea.removeFromTop(rowHeight).reduced(10));

    // MID HIGH
    midhighFrequency.setBounds(midHighArea.removeFromTop(rowHeight).reduced(10));
    midhighGain.setBounds(midHighArea.removeFromTop(rowHeight).reduced(10));
    midhighQ.setBounds(midHighArea.removeFromTop(rowHeight).reduced(10));

    // HIGH
    highFrequency.setBounds(highArea.removeFromTop(rowHeight).reduced(10));
    highGain.setBounds(highArea.removeFromTop(rowHeight).reduced(10));
    highQ.setBounds(highArea.removeFromTop(rowHeight).reduced(10));
}
