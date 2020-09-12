/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/**
 * @file resvg.h
 *
 * resvg C API
 */

#ifndef RESVG_H
#define RESVG_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define RESVG_MAJOR_VERSION 0
#define RESVG_MINOR_VERSION 11
#define RESVG_PATCH_VERSION 0
#define RESVG_VERSION "0.11.0"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief An SVG to #resvg_render_tree conversion options.
 *
 * Also, contains a fonts database used during text to path conversion.
 * The database is empty by default.
 */
typedef struct resvg_options resvg_options;

/**
 * @brief An opaque pointer to the rendering tree.
 */
typedef struct resvg_render_tree resvg_render_tree;

/**
 * @brief A raster image that contains rendering results.
 */
typedef struct resvg_image resvg_image;

/**
 * @brief List of possible errors.
 */
typedef enum resvg_error {
    /** Everything is ok. */
    RESVG_OK = 0,
    /** Only UTF-8 content are supported. */
    RESVG_ERROR_NOT_AN_UTF8_STR,
    /** Failed to open the provided file. */
    RESVG_ERROR_FILE_OPEN_FAILED,
    /** Only `svg` and `svgz` suffixes are supported. */
    RESVG_ERROR_INVALID_FILE_SUFFIX,
    /** Compressed SVG must use the GZip algorithm. */
    RESVG_ERROR_MALFORMED_GZIP,
    /**
     * SVG doesn't have a valid size.
     *
     * Occurs when width and/or height are <= 0.
     *
     * Also occurs if width, height and viewBox are not set.
     * This is against the SVG spec, but an automatic size detection is not supported yet.
     */
    RESVG_ERROR_INVALID_SIZE,
    /** Failed to parse an SVG data. */
    RESVG_ERROR_PARSING_FAILED,
} resvg_error;

/**
 * @brief An RGB color representation.
 */
typedef struct resvg_color {
    uint8_t r; /**< Red component. */
    uint8_t g; /**< Green component. */
    uint8_t b; /**< Blue component. */
} resvg_color;

/**
 * @brief A "fit to" type.
 *
 * All types produce proportional scaling.
 */
typedef enum resvg_fit_to_type {
    RESVG_FIT_TO_ORIGINAL, /**< Use an original image size. */
    RESVG_FIT_TO_WIDTH, /**< Fit an image to a specified width. */
    RESVG_FIT_TO_HEIGHT, /**< Fit an image to a specified height. */
    RESVG_FIT_TO_ZOOM, /**< Zoom an image using scaling factor */
} resvg_fit_to_type;

/**
 * @brief A "fit to" property.
 */
typedef struct resvg_fit_to {
    resvg_fit_to_type type; /**< Fit type. */

    /**
     * @brief Fit to value
     *
     * Not used by RESVG_FIT_TO_ORIGINAL.
     * Must be >= 1 for RESVG_FIT_TO_WIDTH and RESVG_FIT_TO_HEIGHT.
     * Must be > 0 for RESVG_FIT_TO_ZOOM.
     */
    float value;
} resvg_fit_to;

/**
 * @brief A shape rendering method.
 */
typedef enum resvg_shape_rendering {
    RESVG_SHAPE_RENDERING_OPTIMIZE_SPEED,
    RESVG_SHAPE_RENDERING_CRISP_EDGES,
    RESVG_SHAPE_RENDERING_GEOMETRIC_PRECISION,
} resvg_shape_rendering;

/**
 * @brief A text rendering method.
 */
typedef enum resvg_text_rendering {
    RESVG_TEXT_RENDERING_OPTIMIZE_SPEED,
    RESVG_TEXT_RENDERING_OPTIMIZE_LEGIBILITY,
    RESVG_TEXT_RENDERING_GEOMETRIC_PRECISION,
} resvg_text_rendering;

/**
 * @brief An image rendering method.
 */
typedef enum resvg_image_rendering {
    RESVG_IMAGE_RENDERING_OPTIMIZE_QUALITY,
    RESVG_IMAGE_RENDERING_OPTIMIZE_SPEED,
} resvg_image_rendering;

/**
 * @brief A rectangle representation.
 */
typedef struct resvg_rect {
    double x;
    double y;
    double width;
    double height;
} resvg_rect;

/**
 * @brief A size representation.
 */
typedef struct resvg_size {
    uint32_t width;
    uint32_t height;
} resvg_size;

/**
 * @brief A 2D transform representation.
 */
typedef struct resvg_transform {
    double a;
    double b;
    double c;
    double d;
    double e;
    double f;
} resvg_transform;

/**
 * @brief Initializes the library log.
 *
 * Use it if you want to see any warnings.
 *
 * Must be called only once.
 *
 * All warnings will be printed to the `stderr`.
 */
void resvg_init_log();

/**
 * @brief Creates a new #resvg_options object.
 *
 * Should be destroyed via #resvg_options_destroy.
 */
resvg_options* resvg_options_create();

/**
 * @brief Sets an SVG image path.
 *
 * Used to resolve relative image paths.
 *
 * Must be UTF-8. Can be set to NULL.
 *
 * Default: NULL
 */
void resvg_options_set_file_path(resvg_options *opt, const char *path);

/**
 * @brief Sets the target DPI.
 *
 * Impact units conversion.
 *
 * Default: 96
 */
void resvg_options_set_dpi(resvg_options *opt, double dpi);

/**
 * @brief Sets the default font family.
 *
 * Will be used when no `font-family` attribute is set in the SVG.
 *
 * Must be UTF-8. NULL is not allowed.
 *
 * Default: Times New Roman
 */
void resvg_options_set_font_family(resvg_options *opt, const char *family);

/**
 * @brief Sets the default font size.
 *
 * Will be used when no `font-size` attribute is set in the SVG.
 *
 * Default: 12
 */
void resvg_options_set_font_size(resvg_options *opt, double size);

/**
 * @brief Sets the `serif` font family.
 *
 * Must be UTF-8. NULL is not allowed.
 *
 * Default: Times New Roman
 */
void resvg_options_set_serif_family(resvg_options *opt, const char *family);

/**
 * @brief Sets the `sans-serif` font family.
 *
 * Must be UTF-8. NULL is not allowed.
 *
 * Default: Arial
 */
void resvg_options_set_sans_serif_family(resvg_options *opt, const char *family);

/**
 * @brief Sets the `cursive` font family.
 *
 * Must be UTF-8. NULL is not allowed.
 *
 * Default: Comic Sans MS
 */
void resvg_options_set_cursive_family(resvg_options *opt, const char *family);

/**
 * @brief Sets the `fantasy` font family.
 *
 * Must be UTF-8. NULL is not allowed.
 *
 * Default: Papyrus on macOS, Impact on other OS'es
 */
void resvg_options_set_fantasy_family(resvg_options *opt, const char *family);

/**
 * @brief Sets the `monospace` font family.
 *
 * Must be UTF-8. NULL is not allowed.
 *
 * Default: Courier New
 */
void resvg_options_set_monospace_family(resvg_options *opt, const char *family);

/**
 * @brief Sets a comma-separated list of languages.
 *
 * Will be used to resolve a `systemLanguage` conditional attribute.
 *
 * Example: en,en-US.
 *
 * Must be UTF-8. Can be NULL.
 *
 * Default: en
 */
void resvg_options_set_languages(resvg_options *opt, const char *languages);

/**
 * @brief Sets the default shape rendering method.
 *
 * Will be used when an SVG element's `shape-rendering` property is set to `auto`.
 *
 * Default: `RESVG_SHAPE_RENDERING_GEOMETRIC_PRECISION`
 */
void resvg_options_set_shape_rendering_mode(resvg_options *opt, resvg_shape_rendering mode);

/**
 * @brief Sets the default text rendering method.
 *
 * Will be used when an SVG element's `text-rendering` property is set to `auto`.
 *
 * Default: `RESVG_TEXT_RENDERING_OPTIMIZE_LEGIBILITY`
 */
void resvg_options_set_text_rendering_mode(resvg_options *opt, resvg_text_rendering mode);

/**
 * @brief Sets the default image rendering method.
 *
 * Will be used when an SVG element's `image-rendering` property is set to `auto`.
 *
 * Default: `RESVG_IMAGE_RENDERING_OPTIMIZE_QUALITY`
 */
void resvg_options_set_image_rendering_mode(resvg_options *opt, resvg_image_rendering mode);

/**
 * @brief Keep named groups.
 *
 * If set to `true`, all non-empty groups with `id` attribute will not be removed.
 *
 * Default: false
 */
void resvg_options_set_keep_named_groups(resvg_options *opt, bool keep);

/**
 * @brief Loads a font data into the internal fonts database.
 *
 * Prints a warning into the log when the data is not a valid TrueType font.
 */
void resvg_options_load_font_data(resvg_options *opt, const char *data, size_t len);

/**
 * @brief Loads a font file into the internal fonts database.
 *
 * Prints a warning into the log when the data is not a valid TrueType font.
 *
 * @return #resvg_error with RESVG_OK, RESVG_ERROR_NOT_AN_UTF8_STR or RESVG_ERROR_FILE_OPEN_FAILED
 */
int resvg_options_load_font_file(resvg_options *opt, const char *file_path);

/**
 * @brief Loads system fonts into the internal fonts database.
 *
 * This method is very IO intensive.
 *
 * This method should be executed only once per #resvg_options.
 *
 * The system scanning is not perfect, so some fonts may be omitted.
 * Please send a bug report in this case.
 *
 * Prints warnings into the log.
 */
void resvg_options_load_system_fonts(resvg_options *opt);

/**
 * @brief Destroys the #resvg_options.
 */
void resvg_options_destroy(resvg_options *opt);

/**
 * @brief Creates #resvg_render_tree from file.
 *
 * .svg and .svgz files are supported.
 *
 * See #resvg_is_image_empty for details.
 *
 * @param file_path UTF-8 file path.
 * @param opt Rendering options.
 * @param tree Parsed render tree. Should be destroyed via #resvg_tree_destroy.
 * @return #resvg_error
 */
int resvg_parse_tree_from_file(const char *file_path,
                               const resvg_options *opt,
                               resvg_render_tree **tree);

/**
 * @brief Creates #resvg_render_tree from data.
 *
 * See #resvg_is_image_empty for details.
 *
 * @param data SVG data. Can contain SVG string or gzip compressed data.
 * @param len Data length.
 * @param opt Rendering options.
 * @param tree Parsed render tree. Should be destroyed via #resvg_tree_destroy.
 * @return #resvg_error
 */
int resvg_parse_tree_from_data(const char *data,
                               const size_t len,
                               const resvg_options *opt,
                               resvg_render_tree **tree);

/**
 * @brief Checks that tree has any nodes.
 *
 * @param tree Render tree.
 * @return Returns `true` if tree has any nodes.
 */
bool resvg_is_image_empty(const resvg_render_tree *tree);

/**
 * @brief Returns an image size.
 *
 * The size of a canvas that required to render this SVG.
 *
 * @param tree Render tree.
 * @return Image size.
 */
resvg_size resvg_get_image_size(const resvg_render_tree *tree);

/**
 * @brief Returns an image viewbox.
 *
 * @param tree Render tree.
 * @return Image viewbox.
 */
resvg_rect resvg_get_image_viewbox(const resvg_render_tree *tree);

/**
 * @brief Returns an image bounding box.
 *
 * Can be smaller or bigger than a `viewbox`.
 *
 * @param tree Render tree.
 * @param bbox Image's bounding box.
 * @return `false` if an image has no elements.
 */
bool resvg_get_image_bbox(const resvg_render_tree *tree,
                          resvg_rect *bbox);

/**
 * @brief Returns `true` if a renderable node with such an ID exists.
 *
 * @param tree Render tree.
 * @param id Node's ID. UTF-8 string.
 * @return `true` if a node exists.
 * @return `false` if a node doesn't exist or ID isn't a UTF-8 string.
 * @return `false` if a node exists, but not renderable.
 */
bool resvg_node_exists(const resvg_render_tree *tree,
                       const char *id);

/**
 * @brief Returns node's transform by ID.
 *
 * @param tree Render tree.
 * @param id Node's ID. UTF-8 string.
 * @param ts Node's transform.
 * @return `true` if a node exists.
 * @return `false` if a node doesn't exist or ID isn't a UTF-8 string.
 * @return `false` if a node exists, but not renderable.
 */
bool resvg_get_node_transform(const resvg_render_tree *tree,
                              const char *id,
                              resvg_transform *ts);

/**
 * @brief Returns node's bounding box by ID.
 *
 * @param tree Render tree.
 * @param id Node's ID.
 * @param bbox Node's bounding box.
 * @return `false` if a node with such an ID does not exist
 * @return `false` if ID isn't a UTF-8 string.
 * @return `false` if ID is an empty string
 */
bool resvg_get_node_bbox(const resvg_render_tree *tree,
                         const char *id,
                         resvg_rect *bbox);

/**
 * @brief Returns image's unpremultiplied RGBA data.
 *
 * Must not be modified.
 */
const char* resvg_image_get_data(resvg_image *image, size_t *len);

/**
 * @brief Returns image's width.
 */
uint32_t resvg_image_get_width(resvg_image *image);

/**
 * @brief Returns image's height.
 */
uint32_t resvg_image_get_height(resvg_image *image);

/**
 * @brief Destroys the #resvg_image.
 */
void resvg_image_destroy(resvg_image *image);

/**
 * @brief Destroys the #resvg_render_tree.
 */
void resvg_tree_destroy(resvg_render_tree *tree);

/**
 * @brief Renders the #resvg_render_tree onto the image.
 *
 * @param tree A render tree.
 * @param fit_to Specifies into which region the image should be fit.
 * @param background An optional background color.
 * @return A pointer to image on success.
 *         The pointer must be deallocated via #resvg_image_destroy
 * @return `NULL` when image allocation fails.
 */
resvg_image* resvg_render(const resvg_render_tree *tree,
                          resvg_fit_to fit_to,
                          resvg_color *background);

/**
 * @brief Renders a Node by ID onto the image.
 *
 * @param tree A render tree.
 * @param id Node's ID.
 * @param fit_to Specifies into which region the image should be fit.
 * @param background An optional background color.
 * @return A pointer to image on success.
 *         The pointer must be deallocated via #resvg_image_destroy
 * @return `NULL` when image allocation fails.
 * @return `NULL` when `id` is not a non-empty UTF-8 string.
 * @return `NULL` when the selected `id` is not present.
 * @return `NULL` when an element has a zero bbox.
 */
resvg_image* resvg_render_node(const resvg_render_tree *tree,
                               const char *id,
                               resvg_fit_to fit_to,
                               resvg_color *background);

#ifdef __cplusplus
}
#endif

#endif /* RESVG_H */
