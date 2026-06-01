#include "PluginEditor.h"

// ==============================================================================
// LOOK AND FEEL IMPLEMENTATION
// ==============================================================================

CrystalLookAndFeel::CrystalLookAndFeel()
{
    setColour (juce::Slider::thumbColourId,            juce::Colours::white);
    setColour (juce::Slider::rotarySliderFillColourId, juce::Colours::cyan.withAlpha (0.5f));
    setColour (juce::Slider::textBoxTextColourId,      juce::Colours::white.withAlpha (0.9f));
    setColour (juce::Slider::textBoxBackgroundColourId,juce::Colours::black.withAlpha (0.2f));
    setColour (juce::Slider::textBoxOutlineColourId,   juce::Colours::cyan.withAlpha  (0.2f));
}

void CrystalLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& label)
{
    if (dynamic_cast<juce::Slider*> (label.getParentComponent()))
    {
        g.setColour (label.findColour (juce::Label::backgroundColourId));
        g.fillRoundedRectangle (label.getLocalBounds().toFloat(), 3.0f);
        g.setColour (label.findColour (juce::Label::outlineColourId));
        g.drawRoundedRectangle (label.getLocalBounds().toFloat(), 3.0f, 1.0f);
    }
    LookAndFeel_V4::drawLabel (g, label);
}

void CrystalLookAndFeel::drawRotarySlider (juce::Graphics& g,
    int x, int y, int width, int height,
    float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle,
    juce::Slider&)
{
    auto radius  = (float)juce::jmin (width / 2, height / 2) - 10.0f;
    auto centreX = (float)x + (float)width  * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto angle   = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Hexagonal body
    juce::Path p;
    for (int i = 0; i < 6; ++i)
    {
        float ang = juce::MathConstants<float>::twoPi * i / 6.0f;
        auto  px  = centreX + radius * std::cos (ang);
        auto  py  = centreY + radius * std::sin (ang);
        if (i == 0) p.startNewSubPath (px, py); else p.lineTo (px, py);
    }
    p.closeSubPath();

    g.setColour (juce::Colours::darkgrey.darker().withAlpha (0.8f));
    g.fillPath   (p);
    g.setColour (juce::Colours::cyan.withAlpha (0.6f));
    g.strokePath (p, juce::PathStrokeType (1.5f));

    // Pointer
    g.setColour (juce::Colours::white);
    juce::Path r;
    r.addRectangle (-1.0f, -radius, 2.0f, radius * 0.4f);
    g.fillPath (r, juce::AffineTransform::rotation (angle).translated (centreX, centreY));
}

// ==============================================================================
// EDITOR IMPLEMENTATION
// ==============================================================================

CrystalliserAudioProcessorEditor::CrystalliserAudioProcessorEditor (CrystalliserAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&crystalLookAndFeel);

    // Row 1: Core
    setupControl (lengthSlider,      lengthLabel,      lengthAttach,      "length",        "LENGTH");
    setupControl (mixSlider,         mixLabel,         mixAttach,         "mix",           "MIX");
    setupControl (persistenceSlider, persistenceLabel, persistenceAttach, "persistence",   "PERSISTENCE");

    // Row 2: Modulation
    setupControl (lfoRateSlider,  lfoRateLabel,  lfoRateAttach,  "lfoRate",  "MOD RATE");
    setupControl (lfoDepthSlider, lfoDepthLabel, lfoDepthAttach, "lfoDepth", "MOD DEPTH");

    // Row 3: Space
    setupControl (roomSlider,   roomLabel,   roomAttach,   "reverbRoom",     "ROOM");
    setupControl (dampSlider,   dampLabel,   dampAttach,   "reverbDamping",  "DAMP");
    setupControl (widthSlider,  widthLabel,  widthAttach,  "reverbWidth",    "WIDTH");
    setupControl (revWetSlider, revWetLabel, revWetAttach, "reverbWet",      "REV WET");

    setSize (550, 520);
}

CrystalliserAudioProcessorEditor::~CrystalliserAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void CrystalliserAudioProcessorEditor::setupControl (
    juce::Slider& s, juce::Label& l, std::unique_ptr<Attachment>& a,
    const juce::String& paramID, const juce::String& title)
{
    addAndMakeVisible (s);
    s.setSliderStyle  (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 65, 16);
    a = std::make_unique<Attachment> (processor.parameters, paramID, s);

    addAndMakeVisible (l);
    l.setText              (title, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setColour            (juce::Label::textColourId, juce::Colours::cyan.withAlpha (0.7f));
    l.setFont              (juce::FontOptions (11.0f));
}

void CrystalliserAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (
        juce::Colour (0xFF0A0F14), 0, 0,
        juce::Colour (0xFF1B262F), 0, (float)getHeight(), false));
    g.fillAll();

    g.setColour (juce::Colours::white.withAlpha (0.8f));
    g.setFont   (juce::FontOptions (22.0f));
    g.drawText  ("The Crystalliser", 0, 15, getWidth(), 40, juce::Justification::centred);
}

void CrystalliserAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (20);
    area.removeFromTop (50);

    auto placeRow = [](juce::Rectangle<int> rowArea, int count,
                       juce::Slider** sliders, juce::Label** labels)
    {
        int w = rowArea.getWidth() / count;
        for (int i = 0; i < count; ++i)
        {
            auto col = rowArea.removeFromLeft (w).reduced (5);
            labels[i]->setBounds  (col.removeFromTop (18));
            sliders[i]->setBounds (col);
        }
    };

    // Row 1: Length, Mix, Persistence
    juce::Slider* r1s[] = { &lengthSlider, &mixSlider, &persistenceSlider };
    juce::Label*  r1l[] = { &lengthLabel,  &mixLabel,  &persistenceLabel  };
    placeRow (area.removeFromTop (130), 3, r1s, r1l);

    // Row 2: Modulation
    juce::Slider* r2s[] = { &lfoRateSlider, &lfoDepthSlider };
    juce::Label*  r2l[] = { &lfoRateLabel,  &lfoDepthLabel  };
    placeRow (area.removeFromTop (130), 2, r2s, r2l);

    // Row 3: Reverb
    juce::Slider* r3s[] = { &roomSlider, &dampSlider, &widthSlider, &revWetSlider };
    juce::Label*  r3l[] = { &roomLabel,  &dampLabel,  &widthLabel,  &revWetLabel  };
    placeRow (area.removeFromTop (110), 4, r3s, r3l);
}
