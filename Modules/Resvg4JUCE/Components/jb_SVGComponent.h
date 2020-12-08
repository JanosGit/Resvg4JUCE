/*
This file is part of Resvg4JUCE.

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

*/

namespace jb
{

/**
 * A component that owns an Resvg::RenderTree. On each resize, it renders an Image according to the Components size
 * and displays it according to the placement set through setImagePlacement (default is centred)
 */
class SVGComponent : public juce::Component
{
public:

    /**
     * Creates an SVGComponent from an svg file. You have to make sure that this is a valid svg, otherwise
     * behaviour is undefined
     */
    SVGComponent (const juce::File& svgFile)
    {
        auto successLoading = svg.loadFromFile (svgFile);

        jassert (successLoading);
        juce::ignoreUnused (successLoading);
    }

    /**
     * Creates an SVGComponent from an binary data. You have to make sure that this is a valid svg, otherwise
     * behaviour is undefined
     */
    SVGComponent (const char* svgData, int svgSize)
    {
        auto successLoading = svg.loadFromBinaryData (svgData, svgSize);

        jassert (successLoading);
        juce::ignoreUnused (successLoading);
    }

    /** Creates an SVGComponent from a pre-generated svgRenderTree */
    SVGComponent (Resvg::RenderTree&& svgRenderTree) : svg (std::move (svgRenderTree))
    {
        jassert (svg.isValid());
    }

    /** Sets how the image generated from the SVG is placed on the components surface */
    void setImagePlacement (juce::RectanglePlacement placement)
    {
        imagePlacement = placement;
    }

    /** Returns how the image generated from the SVG is placed on the components surface */
    juce::RectanglePlacement getImagePlacement()
    {
        return imagePlacement;
    }

    void resized() override
    {
        auto scale = juce::Desktop::getInstance().getDisplays().findDisplayForPoint (getBounds().getCentre()).scale;

        auto newImageBounds = getLocalBounds().toFloat() * scale;

        if (newImageBounds == cachedImageBounds)
            return;

        cachedImage = svg.render (newImageBounds);
        cachedImageBounds = newImageBounds;
    }

    void paint (juce::Graphics& g) override
    {
        g.drawImage (cachedImage, getLocalBounds().toFloat(), imagePlacement);
    }

private:
    SVGComponent() {}

    Resvg::RenderTree svg;

    juce::Image cachedImage;
    juce::Rectangle<float> cachedImageBounds;

    juce::RectanglePlacement imagePlacement = juce::RectanglePlacement::centred;
};

}
