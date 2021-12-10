# Resvg4JUCE
A JUCE module that wraps the resvg SVG rendering library in a JUCE compatible interface.  The project is currently based on resvg 0.19.0.

This is an attempt to use resvg to overcome problems with the JUCE integrated SVG rendering facility. It comes with two important classes:
`jb::Resvg::RenderTree`, which encapsulates a subset of the original resvg interface in a convenient C++ class. It allows rendering SVG files to a `juce::Image`. `jb::SVGComponent` is a `juce::Component` that owns a render tree and automatically displays the rendered image on the components surface.

While being built for JUCE, this module is not desgined to work with the Projucer but only with the newer CMake based build of JUCE. Usage is pretty simple:

```
# Add JUCE in one of the recomended ways before this line

# Make your projects CMakeList.txt find Resvg4JUCE
add_subdirectory (Path/To/Resvg4JUCE)

# Create a plugin or standalone juce target
juce_add_xy (Foo ...)

# Link Resvg4JUCE to that target
target_link_libraries (Foo jb::Resvg4JUCE)
```

As resvg is a rust library, your build machine needs all the rust build tools. The rust sources of the library will be compiled into a static library that will be  linked to your target. Note for Windows users: Rust libraries are always compiled against the dynamic linked Visual C++ runtime. Please make sure that you'll link your whole project against the dynamic runtime and not the static one, other combinations are likely to fail with compiling at all.
