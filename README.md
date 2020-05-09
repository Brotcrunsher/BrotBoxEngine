# Brotbox Engine
![Build Status](https://travis-ci.org/Brotcrunsher/BrotboxEngine.svg?branch=master)

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

## Quick Start

There are a lot of examples in the repository. Have a look at [ExampleSnake](https://github.com/Brotcrunsher/BrotboxEngine/tree/master/ExampleSnake) for a first start. To create build files for your local machine you have to download and install [CMake](https://cmake.org/) and the [Vulkan SDK](https://vulkan.lunarg.com/). Then, while being in the main directory of the repository, execute the following commands in a terminal:

    mkdir Build
    cd Build
    cmake ..

After that, build files like Solution Files (on Windows) are created. Build and launch ExampleSnake and have a look at its source code.


## FAQ

### Would you recommend using the Brotbox Engine to create a game right now?
*No.* The Brotbox Engine is still in a very, very early state. The API is changing rapidly, without warning, without changelogs and without backwards compatibility. 

### Can I contribute to the Brotbox Engine?
*Yes.* Feel free to open merge requests! They are always welcome, even if you are a beginner. In such a case you get information about what can be improved, and you can learn from it. Don't be shy, just go for it!
