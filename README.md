# Occlusion Culling Project

OpenGL code, CMake files, and libraries based on tutorial from:
http://www.opengl-tutorial.org/

Forked from
https://github.com/opengl-tutorials/ogl

Office building model from here:
https://www.cgtrader.com/free-3d-models/exterior/office/free-office-building
by user "lukass12"

Implements "masked occlusion culling" as proposed in the paper "Masked Software Occlusion Culling" by J. Hasselgren, M. Andersson, and T. Akenine-Möller. However, I don't use AVX or any other kind of SIMD instructions, so my solution is not as performant as theirs.

The original AVX implementation only supports floating point operations, and the rasterization step uses bit shifts. Attempts to speed up the culling logic using only AVX floating point instructions seemed to slow down the solution, as there's some overhead involved.

## Building
Because this is a fork of the opengl-tutorial code, these instructions are based on the ones from here http://www.opengl-tutorial.org/beginners-tutorials/tutorial-1-opening-a-window/


* Clone this repository
* Use CMake in Windows and point the source code directory to the root of this repository, and the build directory to root/build
* Click configure and select Visual Studio 16 2019 (other versions will probably work but I used this one)
* Leave other fields blank and check "Use default native compilers"
* Click Finish
* After configuration finishes, there might be some warnings and things highlighted in red, click configure again and these should go away
* Click generate
* Open Tutorials.sln in root/build
* Make sure the solution configuration in Visual Studio is set to "Release" and not "Debug", or the code will be too slow
* Build the solution named "render"
* root/render/render.exe is the executable to run

