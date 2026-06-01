
//==============================================================================
/*
  ==============================================================================
    This file contains the basic framework code for a JUCE plugin editor.
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PanningAudioProcessorEditor::PanningAudioProcessorEditor (PanningAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // --- SLIDERS ---
    auto sliders = { &panning, &distance };
    for (auto* slider : sliders)
    {
        addAndMakeVisible(slider);
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    }

    // --- LABELS ---
    auto labels = { &panningLabel, &distanceLabel };
    for (auto* label : labels)
    {
        addAndMakeVisible(label);
        label->setJustificationType(juce::Justification::centred);
        label->setBorderSize(juce::BorderSize<int>(0));
    }

    // Label text + attachment
    panningLabel.setText("Panning", juce::dontSendNotification);
    panningLabel.attachToComponent(&panning, false);

    distanceLabel.setText("Distance", juce::dontSendNotification);
    distanceLabel.attachToComponent(&distance, false);

    // --- ATTACHMENTS ---
    panningAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "panning", panning);

    distanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "distance", distance);

    setSize(400, 300);
}

PanningAudioProcessorEditor::~PanningAudioProcessorEditor() {}

//==============================================================================
void PanningAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    auto headerArea = getLocalBounds().removeFromTop(50);
    g.drawFittedText("Panning", headerArea, juce::Justification::centred, 1);
}

void PanningAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    area.removeFromTop(40);
    auto leftColumn  = area.removeFromLeft(area.getWidth() / 2);
    auto rightColumn = area;
    distance.setBounds(leftColumn.reduced(25));
    panning.setBounds(rightColumn.reduced(25));
}
