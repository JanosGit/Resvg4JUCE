/*
This file is part of Resvg4JUCE.

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

*/

#include "jb_ResvgRenderTree.h"

#if JUCE_INTEL
#include "immintrin.h"
#endif

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

// Internal function to swap the red and blue component of a colour since resvg expects them swapped in comparison to juce
juce::Colour swapRB (juce::Colour c)
{
    auto* asUint8 = reinterpret_cast<uint8_t*> (&c);

    std::swap (asUint8[0], asUint8[2]);

    return c;
}

constexpr int64_t bytesPerPixel = 4;

// Swaps the red and blue compoment of an rgba image with a certain number of pixels via a for loop over all pixels
void swapRBnonSIMD (uint8_t* data, int64_t numPixel)
{
    const auto* end = data + (bytesPerPixel * numPixel);

    for (; data != end; data += bytesPerPixel)
        std::swap (data[0], data[2]);
}

#if JUCE_INTEL
// SSE optimized implementation of swapRB
void swapRBSSE (uint8_t* data, int64_t numPixel)
{
    const auto* end = data + (bytesPerPixel * numPixel);
    int64_t numBytes = end - data;

    auto* simdAlignedData = juce::snapPointerToAlignment (data, sizeof (__m128i));

    const int64_t numBytesPreSIMD = simdAlignedData - data;

    if (numBytesPreSIMD % bytesPerPixel != 0)
    {
        swapRBnonSIMD (data, numPixel);
        return;
    }

    swapRBnonSIMD (data, numBytesPreSIMD / bytesPerPixel);
    numBytes -= numBytesPreSIMD;

    const int64_t numBytesPostSIMD= numBytes % int (sizeof (__m128i));
    const auto* endSIMD = simdAlignedData + (numBytes - numBytesPostSIMD);

    __m128i shuffleMask = _mm_set_epi8 (15, 12, 13, 14,
                                        11, 8,  9, 10,
                                        7,  4,  5,  6,
                                        3,  0,  1,  2);

    for (; simdAlignedData != endSIMD; simdAlignedData += sizeof (__m128i))
    {
        auto in = _mm_load_si128 (reinterpret_cast<__m128i*> (simdAlignedData));
        auto out = _mm_shuffle_epi8 (in, shuffleMask);
        _mm_store_si128 (reinterpret_cast<__m128i*> (simdAlignedData), out);
    }

    swapRBnonSIMD (simdAlignedData, numBytesPostSIMD / bytesPerPixel);
}

void swapRB (uint8_t* data, int64_t numBytes)
{
    swapRBSSE (data, numBytes);
}

#else
void swapRB (uint8_t* data, int64_t numBytes)
{
    swapRBnonSIMD (data, numBytes);
}
#endif

// Internal function to perform the actual rendering behind the various RenderTree::render functions
juce::Image renderTree (resvg_render_tree* tree, resvg_fit_to fit, juce::Colour backgroundColour, juce::Rectangle<int>&& imageBounds)
{
    // Before rendering an SVG you need to have successfully loaded one into the tree
    jassert (tree != nullptr);

    const auto h = imageBounds.getHeight();
    const auto w = imageBounds.getWidth();

    juce::Image image (juce::Image::PixelFormat::ARGB, w, h, backgroundColour.isTransparent());

    image.clear (imageBounds, swapRB (backgroundColour));

    juce::Image::BitmapData dstData (image, 0, 0, w, h, juce::Image::BitmapData::ReadWriteMode::writeOnly);

    resvg_render (tree, fit,
                  static_cast<uint32_t> (w),
                  static_cast<uint32_t> (h),
                  reinterpret_cast<char*> (dstData.data));

    // Red and blue components have to be swapped since resvg orders them differently compared to juce
    swapRB (dstData.data, static_cast<int64_t> (w) * static_cast<int64_t> (h));

    return image;
}

juce::Image renderTree (resvg_render_tree* tree, resvg_fit_to fit, juce::Colour backgroundColour)
{
    // Before rendering an SVG you need to have successfully loaded one into the tree
    jassert (tree != nullptr);

    auto imageSize = resvg_get_image_size (tree);

    if (fit.type == RESVG_FIT_TO_ZOOM)
    {
        imageSize.width *= fit.value;
        imageSize.height *= fit.value;
    }

    return renderTree (tree, fit, backgroundColour, juce::Rectangle<double> (imageSize.width, imageSize.height).toNearestIntEdges());
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

    return static_cast<float> (size.width / size.height);
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

juce::Image RenderTree::render (juce::Rectangle<float> dstSize, juce::Colour backgroundColour)
{
    auto dstAspectRatio = dstSize.getAspectRatio();
    auto srcAspectRatio = getAspectRatio();

    resvg_fit_to fit;
    if (srcAspectRatio < dstAspectRatio)
    {
        // The source image is wider than the destination image --> fit to height
        fit.type = RESVG_FIT_TO_HEIGHT;
        fit.value = dstSize.getHeight();

        dstSize.setWidth (fit.value * srcAspectRatio);
    }
    else
    {
        fit.type = RESVG_FIT_TO_WIDTH;
        fit.value = dstSize.getWidth();

        dstSize.setHeight (fit.value / srcAspectRatio);
    }

    jassert (fit.value >= 1.0f);

    return renderTree ((resvg_render_tree*) tree, fit, backgroundColour, dstSize.toNearestIntEdges());
}
}
}
