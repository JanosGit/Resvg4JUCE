/*
This file is part of Resvg4JUCE.

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

*/

namespace jb
{

namespace Resvg
{

enum class ShapeRenderingMode
{
    optimizeSpeed,
    crispEdges,
    geometricPrecision,
};

enum class TextRenderingMode
{
    optimizeSpeed,
    optimizeLegibility,
    geometricPrecision,
};

enum class ImageRenderingMode
{
    optimizeQuality,
    optimizeSpeed,
};

struct Options
{
    double dpi = 96;

    ShapeRenderingMode shapeRendering = ShapeRenderingMode::geometricPrecision;

    TextRenderingMode textRendering = TextRenderingMode::optimizeLegibility;

    ImageRenderingMode imageRendering = ImageRenderingMode::optimizeQuality;
};

/*
 * Initializes the resvg library log.
 *
 * Use it if you want to see any internal resvg warnings, they will be printed to stderr
 * Must be called only once.
*/
void initLog();

/**
 * This class encapsulates an resvg_render_tree and gives you functions to load svg files into it and render them
 * to a juce::Image. Once you have loaded an svg into the tree, you can call render multiple times without re-loading
 * the svg.
 */
class RenderTree
{
public:

    /** Creates a render tree with default options */
    RenderTree();

    /** Creates a render tree with a custom DPI preference */
    RenderTree (double dpi);

    /** Creates a render tree with custom options */
    RenderTree (const Options& renderingOptions);

    /** Moves constructor */
    RenderTree (RenderTree&& other);

    ~RenderTree();

    /** Parses an SVG file into this tree. Returns true on success, false otherwise */
    bool loadFromFile (const juce::File& svgFile);

    /** Parses an SVG file from binary data into this tree. Returns true on success, false otherwise */
    bool loadFromBinaryData (const char* data, size_t size);

    /** Returns true if a file has been successfully loaded into this tree */
    bool isValid();

    /** Returns the size that is stored in the SVG. Returns an empty rectangle if no SVG has been loaded yet */
    juce::Rectangle<int> getSize();

    /** Returns the aspect ratio (width over height) of the SVG. Returns -1.0 if no SVG has been loaded yet*/
    float getAspectRatio();

    /**
     * Renders the SVG to an image of the size matching the size stored in the SVG. The background can be either fully
     * transparent or a fully solid colour.
     */
    juce::Image render (juce::Colour backgroundColour = juce::Colours::transparentBlack);

    /**
     * Renders the SVG to an image of the size matching the size stored in the SVG adusted by the zoom factor passed in.
     * The background can be either fully transparent or a fully solid colour.
     */
    juce::Image render (float zoomFactor, juce::Colour backgroundColour = juce::Colours::transparentBlack);

    /**
     * Renders the SVG to an image that fits the given destination rectangle. The image returned might be smaller than
     * the destination rectangle passed, as this call always preserves the apsect ratio of the original SVG.
     * The background can be either fully transparent or a fully solid colour.
     */
    juce::Image render (const juce::Rectangle<float>& dstSize, juce::Colour backgroundColour = juce::Colours::transparentBlack);


private:

    // Using void pointers to not expose resvg types to the public interface
    void* options = nullptr;
    void *tree = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RenderTree)
};

}

}

