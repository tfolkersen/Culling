# Occlusion Culling Project

OpenGL code, CMake files, and libraries based on tutorial from:
http://www.opengl-tutorial.org/

Build instructions from the original tutorial are here:
http://www.opengl-tutorial.org/beginners-tutorials/tutorial-1-opening-a-window/
Use CMake in Windows to generate the project. I used Visual Studio 2019. Instead of using the code from the tutorial's repository, you use this repository's code.

Forked from
https://github.com/opengl-tutorials/ogl

Office building model from here:
https://www.cgtrader.com/free-3d-models/exterior/office/free-office-building
by user "lukass12"

Implements "masked occlusion culling" as proposed in the paper "Masked Software Occlusion Culling" by J. Hasselgren, M. Andersson, and T. Akenine-Möller. However, I don't use AVX or any other kind of SIMD instructions, so my solution is not as performant as theirs.

The original AVX implementation only supports floating point operations, and the rasterization step uses bit shifts. Attempts to speed up the culling logic using only AVX floating point instructions seemed to slow down the solution, as there's some overhead involved.


