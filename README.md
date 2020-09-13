# Resvg4JUCE
A JUCE module that wraps the resvg SVG rendering library in a JUCE compatible interface

This is an attempt to use resvg to overcome problems with the JUCE integrated SVG rendering facility. It comes with two important classes:
`jb::Resvg::RenderTree`, which encapsulates a subset of the original resvg interface in a convenient C++ class. It allows rendering SVG files to a `juce::Image`. `jb::SVGComponent` is a `juce::Component` that owns a render tree and automatically displays the rendered image on the components surface.

As resvg is a rust library, we need to include it as a precompiled library. For simplicity, this repository already contains a precompiled static library for MacOS, for Windows there is a script included to build it yourself and copy it to the desired place. In order to build the lib yourself, you have to clone the resvg submodule, which points to my own fork, that is only modified in such a way that it outputs a staic library rather than a dynamic one. You'll obviously need rust build tools and (not so obviously) need clang-cl on Windows, as resvg is based on a striped-down version of skia, which can only be compiled with clang.

Due to my lack of rust know-how I failed to get another build configuration than /MD working on Windows at the moment. All hints regarding what can be done about this are welcome!
