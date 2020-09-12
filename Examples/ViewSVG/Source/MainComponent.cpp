#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize (600, 800);
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillCheckerBoard (getLocalBounds().toFloat(), 20, 20, juce::Colours::white, juce::Colours::lightgrey);

    if (svg == nullptr)
    {
        g.setFont (juce::Font (20.0f));
        g.setColour (juce::Colours::black);
        g.drawText ("Drag & Drop your SVG file into this window", getLocalBounds(), juce::Justification::centred, true);
    }
}

void MainComponent::resized()
{
    if (svg != nullptr)
        svg->setBounds (getLocalBounds());
}

bool MainComponent::isInterestedInFileDrag (const juce::StringArray& files)
{
    if (files.size() != 1)
        return false;

    juce::File possibleSVG (files[0]);

    if (possibleSVG.hasFileExtension ("svg") || possibleSVG.hasFileExtension ("svgz"))
        return true;

    return false;
}

void MainComponent::filesDropped (const juce::StringArray& files, int, int)
{
    juce::File newSVGFile (files[0]);

    auto newSVGComponent = jb::SVGComponent::make (newSVGFile);

    if (newSVGComponent != nullptr)
    {
        if (svg != nullptr)
            removeChildComponent (svg.get());

        svg = std::move (newSVGComponent);

        addAndMakeVisible (*svg);

        resized();
        repaint();
    }
}