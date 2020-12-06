#pragma once

/*	control file

	this handles user input and recording/playback of replays
	also defines parameters of movement (camera speed/mouse sensitivity)
*/

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <string>

//movements relative to the camera angle
#define KEY_FORWARD GLFW_KEY_W //Move forward
#define KEY_LEFT GLFW_KEY_A //Move left
#define KEY_BACK GLFW_KEY_S //Move backwards
#define KEY_RIGHT GLFW_KEY_D //Move right

//movements relative to the world
#define KEY_UP GLFW_KEY_SPACE //Move up
#define KEY_DOWN GLFW_KEY_LEFT_CONTROL //Move down

#define KEY_SWAP_MODELS GLFW_KEY_F //Toggle model rendering modes (normal mesh, bounding box, occluder, etc.)
#define KEY_SPEED GLFW_KEY_LEFT_SHIFT //Move faster while holding this

#define CAMERA_SPEED_DEFAULT 0.10f
#define CAMERA_SPEED_FAST 0.20f

#define MOUSE_SENSITIVITY 0.001 / 1.6 //How fast to rotate the camera with mouse movement

extern GLfloat cameraSpeed; //Camera movement per frame in any direction, this current frame

enum replayEnum {CONTROL, RECORD, PLAY}; //user control, user control + record, playback replay
extern int replayMode;

//main mesh, occluder mesh, bounding box, markers, markers and main mesh
enum modelTypeEnum{MAIN = 0, OCCLUDER = 1, BOX = 2, MARKER = 3, MARKER2 = 4};
extern int drawModelType;

extern std::string replayFileName;
extern std::fstream replayFile;

extern uint64_t currentFrame; //the current frame number (starts at 1)
extern uint64_t nextReplayFrame; //frame of next action in replay file being played
extern bool recordedFrameNumber; //in replay file, was current frame number already recorded?

extern GLfloat yaw; //left/right angle of camera
extern GLfloat pitch; //up/down angle of camera
extern GLfloat oldYaw; //previous frame's yaw
extern GLfloat oldPitch; //previous frame's pitch

bool keyPressed(int key); //true if key was pressed

void handlePlayback(); //get actions from replay file for this frame

////////// record functions won't do anything if replayMode isn't set to RECORD
void recordFrameNumber(); //record current frame number in replay file if it hasn't been already

void recordTranslate(const glm::vec3& v, bool beforeRotate); //record translation of camera into replay file

void recordYawPitch(GLfloat dYaw, GLfloat dPitch); //record movement of camera angle into replay file

void recordQuit(); //record quit into replay file

void handleGlobalInput(); //inputs allowed in all modes (quit/toggle model rendering mode)

void handleInput(); //handle inputs allowed in control/record mode (movements)
 
void cursorCallback(GLFWwindow* window, double xPos, double yPos); //capture mouse movement

