#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <string>



#define KEY_FORWARD GLFW_KEY_W //Move forward
#define KEY_LEFT GLFW_KEY_A //Move left
#define KEY_BACK GLFW_KEY_S //Move backwards
#define KEY_RIGHT GLFW_KEY_D //Move right
#define KEY_UP GLFW_KEY_SPACE //Move up
#define KEY_DOWN GLFW_KEY_LEFT_CONTROL //Move down
#define KEY_SWAP_MODELS GLFW_KEY_F //Toggle model rendering modes (normal/occluder mesh/bounding box)
#define KEY_SPEED GLFW_KEY_LEFT_SHIFT //Move fast

#define CAMERA_SPEED_DEFAULT 0.10f
#define CAMERA_SPEED_FAST 0.20f;



#define MOUSE_SENSITIVITY 0.001 / 1.6 //How fast to rotate the camera with mouse movement

extern GLfloat cameraSpeed; //Camera movement per frame in any direction

enum replayEnum {CONTROL, RECORD, PLAY};
extern int replayMode;

enum modelTypeEnum{MAIN = 0, OCCLUDER = 1, BOX = 2};
extern int drawModelType;

extern std::string replayFileName;
extern std::fstream replayFile;

extern uint64_t currentFrame;
extern uint64_t nextReplayFrame;
extern bool recordedFrameNumber;

extern GLfloat yaw;
extern GLfloat pitch;
extern GLfloat oldYaw;
extern GLfloat oldPitch;

bool keyPressed(int key);

void handlePlayback();

void recordFrameNumber();

void recordTranslate(const glm::vec3& v, bool beforeRotate);

void recordYawPitch(GLfloat dYaw, GLfloat dPitch);

void recordQuit();

void handleGlobalInput();

void handleInput();

void cursorCallback(GLFWwindow* window, double xPos, double yPos);

