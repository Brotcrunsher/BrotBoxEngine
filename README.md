# Brot Box Engine
![Build Status](https://travis-ci.org/Brotcrunsher/BrotBoxEngine.svg?branch=master)

A C++ prototyping engine that is striving for an easy to use API, hiding all the dirty details so that you don't have to care.

#### Used technology

* [C++](https://isocpp.org/)
* [CMake](https://cmake.org/)
* [Vulkan](https://vulkan.lunarg.com/)
* [GLFW](https://www.glfw.org/)
* [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h)
* [stb_truetype](https://github.com/nothings/stb/blob/master/stb_truetype.h)
* [Google Test](https://github.com/google/googletest)
* [Travis CI](https://travis-ci.com/)
* [Box2D](https://box2d.org/)

## Quick Start

There are a lot of examples in the repository. Have a look at [ExampleSnake](https://github.com/Brotcrunsher/BrotBoxEngine/tree/master/ExampleSnake) for a first start. To create build files for your local machine you have to download and install [CMake](https://cmake.org/) and the [Vulkan SDK](https://vulkan.lunarg.com/). Then, while being in the main directory of the repository, execute the following commands in a terminal:

    mkdir Build
    cd Build
    cmake ..

After that, build files like Solution Files (on Windows) are created. To build the projects you can either use your favorite IDE or directly interact with CMake using the following command while you are in the Build directory:

    cmake --build . --target ExampleSnake

This will build an executable that you may run.


## FAQ

### Would you recommend using the Brot Box Engine to create a game right now?
*No.* The Brot Box Engine is still in a very, very early state. The API is changing rapidly, without warning, without changelogs and without backwards compatibility. 

### Can I contribute to the Brot Box Engine?
*Yes.* Feel free to open merge requests! They are always welcome, even if you are a beginner. In such a case you get information about what can be improved, and you can learn from it. Don't be shy, just go for it!

### Which compilers are supported?
The project is regularly build with Microsoft Visual Studio Version 16.5.4 (with its default MSVC) as well as GCC Version 8.4.0. Other versions of said compilers probably also work as long as they have full C++17 support. Completely different compilers like clang are untested, though it's quite possible that they work as well. Feel free to try and let me know.

## Other Resources

[This](https://github.com/Brotcrunsher/BrotBoxEngineExampleExternal) shows you how you can set up an external project that uses the Brot Box Engine. The repository itself is tiny, but it's CMake code downloads and integrates the Brot Box Engine in an easy to use way.
