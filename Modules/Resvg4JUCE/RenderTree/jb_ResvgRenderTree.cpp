/*
This file is part of Resvg4JUCE.

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

*/

#include "jb_ResvgRenderTree.h"

#include <resvg.h>

namespace jb
{

namespace Resvg
{

// Helper function to cast our public enum classes into the resvg enum types
template <typename DstEnumType, typename SrcEnumClassType>
constexpr DstEnumType to (SrcEnumClassType src)
{
    using UT = typename std::underlying_type<SrcEnumClassType>::type;
    return static_cast<DstEnumType> (static_cast<UT> (src));
}

// Make sure that our public enum class constants match the resv constants
static_assert (RESVG_SHAPE_RENDERING_OPTIMIZE_SPEED      == to<resvg_shape_rendering> (ShapeRenderingMode::optimizeSpeed), "");
static_assert (RESVG_SHAPE_RENDERING_CRISP_EDGES         == to<resvg_shape_rendering> (ShapeRenderingMode::crispEdges), "");
static_assert (RESVG_SHAPE_RENDERING_GEOMETRIC_PRECISION == to<resvg_shape_rendering> (ShapeRenderingMode::geometricPrecision), "");

static_assert (RESVG_TEXT_RENDERING_OPTIMIZE_SPEED       == to<resvg_text_rendering> (TextRenderingMode::optimizeSpeed), "");
static_assert (RESVG_TEXT_RENDERING_GEOMETRIC_PRECISION  == to<resvg_text_rendering> (TextRenderingMode::geometricPrecision), "");
static_assert (RESVG_TEXT_RENDERING_OPTIMIZE_LEGIBILITY  == to<resvg_text_rendering> (TextRenderingMode::optimizeLegibility), "");

static_assert (RESVG_IMAGE_RENDERING_OPTIMIZE_SPEED      == to<resvg_image_rendering> (ImageRenderingMode::optimizeSpeed), "");
static_assert (RESVG_IMAGE_RENDERING_OPTIMIZE_QUALITY    == to<resvg_image_rendering> (ImageRenderingMode::optimizeQuality), "");

// Internal function to perform the actual rendering behind the various RenderTree::render functions
juce::Image renderTree (resvg_render_tree* tree, resvg_fit_to fit, juce::Colour backgroundColour)
{
    // Before rendering an SVG you need to have successfully loaded one into the tree
    jassert (tree != nullptr);

    resvg_color c { backgroundColour.getRed(), backgroundColour.getGreen(), backgroundColour.getBlue() };

    resvg_color* bgColour = backgroundColour.isTransparent() ? nullptr : &c;

    auto* resvgImage = resvg_render (tree, fit, bgColour);
    jassert (resvgImage != nullptr);

    auto h = resvg_image_get_height (resvgImage);
    auto w = resvg_image_get_width  (resvgImage);

    juce::Image image (juce::Image::PixelFormat::ARGB, static_cast<int> (w), static_cast<int> (h), false);
    juce::Image::BitmapData dstData (image, 0, 0, int (w), int (h), juce::Image::BitmapData::ReadWriteMode::writeOnly);

    // This is how resvg lays out the pixel data
    struct ResvgRGBA
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };

    size_t nBytes = sizeof (ResvgRGBA) * w * h;

    auto* srcData = reinterpret_cast<ResvgRGBA*> (const_cast<char*> (resvg_image_get_data (resvgImage, &nBytes)));

    // todo: is there a better way than this loop?
    for (int y = 0; y < static_cast<int> (h); ++y)
    {
        for (int x = 0; x < static_cast<int> (w); ++x)
        {
            auto px = *srcData++;
            dstData.setPixelColour (x, y, juce::Colour (px.r, px.g, px.b, px.a));
        }
    }

    resvg_image_destroy (resvgImage);

    return image;
}

void initLog()
{
    resvg_init_log();
}

RenderTree::RenderTree ()
{
    options = resvg_options_create();
    jassert (options != nullptr);
}

RenderTree::RenderTree (double dpi) : RenderTree()
{
    resvg_options_set_dpi ((resvg_options*) options, dpi);
}

RenderTree::RenderTree (const Options& renderingOptions) : RenderTree()
{
    resvg_options_set_dpi                  ((resvg_options*) options, renderingOptions.dpi);
    resvg_options_set_shape_rendering_mode ((resvg_options*) options, to<resvg_shape_rendering> (renderingOptions.shapeRendering));
    resvg_options_set_text_rendering_mode  ((resvg_options*) options, to<resvg_text_rendering>  (renderingOptions.textRendering));
    resvg_options_set_image_rendering_mode ((resvg_options*) options, to<resvg_image_rendering> (renderingOptions.imageRendering));
}

RenderTree::RenderTree (RenderTree&& other)
  : options (other.options),
    tree    (other.tree)
{
    other.options = nullptr;
    other.tree = nullptr;
}

RenderTree::~RenderTree ()
{
    if (options != nullptr)
        resvg_options_destroy ((resvg_options*) options);

    if (tree != nullptr)
        resvg_tree_destroy ((resvg_render_tree*) tree);
}

bool RenderTree::loadFromFile (const juce::File& svgFile)
{
    jassert (svgFile.existsAsFile());
    auto fullPath = svgFile.getFullPathName();

    if (tree != nullptr)
        resvg_tree_destroy ((resvg_render_tree*) tree);

    auto result = resvg_parse_tree_from_file (fullPath.toRawUTF8(), (resvg_options*) options, (resvg_render_tree**) &tree);

    if (result != RESVG_OK || tree == nullptr)
    {
        tree = nullptr;
        return false;
    }

    return true;
}

bool RenderTree::loadFromBinaryData (const char* data, int size)
{
    if (tree != nullptr)
        resvg_tree_destroy ((resvg_render_tree*) tree);

    auto result = resvg_parse_tree_from_data (data, static_cast<size_t>(size), (resvg_options*) options, (resvg_render_tree**) &tree);

    if (result != RESVG_OK || tree == nullptr)
    {
        tree = nullptr;
        return false;
    }

    return true;
}

bool RenderTree::isValid()
{
    return tree != nullptr;
}

juce::Rectangle<int> RenderTree::getSize()
{
    if (tree == nullptr)
        return {};

    auto size = resvg_get_image_size ((resvg_render_tree*) tree);

    return { static_cast<int> (size.width), static_cast<int> (size.height) };
}

float RenderTree::getAspectRatio()
{
    if (tree == nullptr)
        return -1.0f;

    auto size = resvg_get_image_size ((resvg_render_tree*) tree);

    return size.width / static_cast<float> (size.height);
}

juce::Image RenderTree::render (juce::Colour backgroundColour)
{
    resvg_fit_to fit {resvg_fit_to_type::RESVG_FIT_TO_ORIGINAL};
    return renderTree ((resvg_render_tree*) tree, fit, backgroundColour);
}

juce::Image RenderTree::render (float zoomFactor, juce::Colour backgroundColour)
{
    resvg_fit_to fit {resvg_fit_to_type::RESVG_FIT_TO_ZOOM, zoomFactor};
    return renderTree ((resvg_render_tree*) tree, fit, backgroundColour);
}

juce::Image RenderTree::render (const juce::Rectangle<float>& dstSize, juce::Colour backgroundColour)
{
    auto dstAspectRatio = dstSize.getAspectRatio();
    auto srcAspectRatio = getAspectRatio();

    resvg_fit_to fit;
    if (srcAspectRatio > dstAspectRatio)
    {
        // The source image is wider than the destination image --> fit to height
        fit.type = RESVG_FIT_TO_HEIGHT;
        fit.value = dstSize.getHeight();
    }
    else
    {
        fit.type = RESVG_FIT_TO_WIDTH;
        fit.value = dstSize.getWidth();
    }

    return renderTree ((resvg_render_tree*) tree, fit, backgroundColour);
}
}
}
