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

    SVGComponent (const juce::File& svgFile)
    {
        svg.loadFromFile (svgFile);
    }

    SVGComponent (const char* svgData, size_t svgSize)
    {
        svg.loadFromBinaryData (svgData, svgSize);
    }

    static std::unique_ptr<SVGComponent> make (const juce::File& svgFile)
    {
        std::unique_ptr<SVGComponent> c (new SVGComponent());

        if (c->svg.loadFromFile (svgFile))
            return c;

        return nullptr;
    }

    void setImagePlacement (juce::RectanglePlacement placement)
    {
        imagePlacement = placement;
    }

    juce::RectanglePlacement getImagePlacement()
    {
        return imagePlacement;
    }

    void resized() override
    {
        auto newImageBounds = getLocalBounds().toFloat() * getDesktopScaleFactor();

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