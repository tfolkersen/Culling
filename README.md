# Occlusion Culling Project

OpenGL code, CMake files, and libraries based on tutorial from:
http://www.opengl-tutorial.org/

Forked from
https://github.com/opengl-tutorials/ogl

Office building model from here:
https://www.cgtrader.com/free-3d-models/exterior/office/free-office-building
by user "lukass12"/"Lukas carnota"

Implements "masked occlusion culling" as proposed in the paper "Masked Software Occlusion Culling" by J. Hasselgren, M. Andersson, and T. Akenine-Möller. However, I don't use AVX or any other kind of SIMD instructions, so my solution is not as performant as theirs.

I only have the first version of AVX, which only supports floating point operations, and the rasterization step uses bit shifts. Attempts to speed up the culling logic using only AVX floating point instructions seemed to slow down the solution, as there's some overhead involved.

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
* Make sure the solution configuration in Visual Studio is set to "Release" and not "Debug", or else the code will be too slow (this option should be near the top of the window)
* Build the solution named "render"
* root/render/render.exe is the executable to run
There are solutions other than "render" that are an artifact of the tutorial code, but you shouldn't need to build these.

## Usage
Open render.exe from the command-line

Flags go in any order and are:

* -r — (r)ecord a replay into replay.txt
* -p — (p)lay the replay file, replay.txt
* -s — output (s)tatistics to stats.txt
* -a — use the (a)lternate scene instead of the default one

-p and -s are mutually exclusive

Example: 
```./render.exe -p -a -s```
Will play the replay file in the alternate scene and will output statistics.

With no flags, the user is free to move around the default scene without outputting statistics or recording a replay

### Controls
Key bindings are specified in controls.h

These movements are relative to the camera's facing direction:
* W — move forward
* A — move backward
* S — move left
* D — move right

These movements are relative to the world instead of the camera's facing direction:
* Spacebar — move up
* Left control — move down

Other controls:
* Left shift — hold this to move faster
* F — this swaps the rendering mode (more info further down the README)
* Escape — close the program

### Render modes
Cycle through these modes in this order by pressing F. These modes only affect what's displayed in the window, and don't affect the culling logic at all.

* Main/default — show the meshes that should be rendered in the scene.
* Occluders — (low poly, red meshes) show the occlusion meshes that get rasterized into the depth buffer by the culling logic.
* Bounding boxes — (yellow boxes) show the bounding boxes that are used for visibility tests by the culling logic.
* Markers only — (narrow blue pillars) show markers near the centers of objects, used to visualize culling effect better.
* Markers and main — (main meshes and narrow blue pillars) show the main meshes and the blue pillars together. Another way of visualizing culling effect.

### Replays
The replay file is stored in root/render/replay.txt. You must record before you can playback. With the -r flag, all actions are recorded except for toggling the render mode. Replays store movements instead of keys pressed, so you can modify things like the camera movement speed in control.h and the replay should still work.

Playback is done with the -p flag, and the program will close if there is no replay.txt file. Replays don't know which scene they were recorded in, so if you use the flags ```-r -a``` and then use only the flag ```-p``` it will playback in the wrong scene.

Replays are a way to gather statistics across different versions of the program, under the same scene and actions. For example, you can modify the culling logic and then generate new statistics in the same environment using replays. 
