#include "control.h"
#include "draw.h"
#include "utility.h"
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <fstream>
#include <algorithm>

GLfloat cameraSpeed = CAMERA_SPEED_DEFAULT; //Camera movement per frame in any direction for this frame
int replayMode = CONTROL;

int drawModelType = MAIN;

std::string replayFileName = "replay.txt";
std::fstream replayFile;

uint64_t currentFrame = 1;
uint64_t nextReplayFrame = 1;
bool recordedFrameNumber = false;

/*	every frame:
	WASD movements are applied relative to the camera angle
	the camera angle is un-rotated back to 0,0, first vertical angle and then horizontal angle
	up/down movement are applied (this is relative to the world)
	the camera angle is rotated to its new angle, first horizontal angle, then vertical angle

	unrotating and rotating in this order also ensures that the camera's roll stays at 0, as
	applying rotations in arbitrary orders will affect the roll which is not desired

*/
GLfloat yaw = 0.0f;
GLfloat pitch = 0.0f;
GLfloat oldYaw = yaw;
GLfloat oldPitch = pitch;

//true if key was pressed
bool keyPressed(int key) {
	return glfwGetKey(window, key) == GLFW_PRESS;
}

/*		for this frame, get actions from replay file
*/
void handlePlayback() {
	//no actions for this frame
	if (currentFrame != nextReplayFrame) {
		return;
	}

	std::string line;
	std::vector<std::string> tokens;

	std::vector<glm::vec3> translation1; //translations relative to camera angle (WASD)
	std::vector<glm::vec3> translation2; //translations relative to world (up/down)
	std::vector<std::pair<GLfloat, GLfloat>> yawPitch; //movements of yaw/pitch
	bool quit = false; //quit this frame?

	//Get all the actions before applying them
	while (true && !replayFile.eof()) {
		std::getline(replayFile, line);
		tokens = split(line, " ");

		if (tokens[0] == "f") { //number of next frame that has actions
			nextReplayFrame = std::stoull(tokens[1]);
			break; //no more actions for this frame
		} else if (tokens[0] == "t") { //translation1
			GLfloat dx = std::stof(tokens[1]);
			GLfloat dy = std::stof(tokens[2]);
			GLfloat dz = std::stof(tokens[3]);
			translation1.push_back(glm::vec3(dx, dy, dz));
		} else if (tokens[0] == "tt") { //translation2
			GLfloat dx = std::stof(tokens[1]);
			GLfloat dy = std::stof(tokens[2]);
			GLfloat dz = std::stof(tokens[3]);
			translation2.push_back(glm::vec3(dx, dy, dz));
		} else if (tokens[0] == "yp") { //yaw/pitch
			GLfloat dYaw = std::stof(tokens[1]);
			GLfloat dPitch = std::stof(tokens[2]);
			yawPitch.push_back(std::pair<GLfloat, GLfloat>(dYaw, dPitch));
		} else if (tokens[0] == "e") { //quit
			quit = true;
			break;
		}
	}

	//Perform all the actions
	if (quit) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	for (auto it = translation1.begin(); it != translation1.end(); it++) {
		view = glm::translate(glm::mat4(), *it) * view;
	}

	//Rotate the camera back to yaw/pitch of 0,0 to apply translation2 
	view = glm::rotate(glm::mat4(), -oldYaw, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(), -oldPitch, glm::vec3(1.0f, 0.0f, 0.0f)) * view;

	for (auto it = translation2.begin(); it != translation2.end(); it++) {
		view = glm::translate(glm::mat4(), *it) * view;
	}

	for (auto it = yawPitch.begin(); it != yawPitch.end(); it++) {
		yaw += it->first;
		pitch += it->second;
	}

	//prevent camera from vertically flipping by going past down/up directions
	pitch = std::min(((float) PI) / 2.0f, std::max(-((float) PI) / 2.0f, pitch));

	//apply new camera rotation
	view = glm::rotate(glm::mat4(), pitch, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(), yaw, glm::vec3(0.0f, 1.0f, 0.0f)) * view;

	oldYaw = yaw;
	oldPitch = pitch;
}

//record frame number of this frame into replay file if not already done
void recordFrameNumber() {
	if (recordedFrameNumber) {
		return;
	}
	recordedFrameNumber = true;
	replayFile << "f " << currentFrame << std::endl;
}

/*		record translation of camera into replay file
	beforeRotate is true if the translation is relative to the camera angle (WASD)
	and false if it's relative to the world (up/down)
*/
void recordTranslate(const glm::vec3 &v, bool beforeRotate) {
	if (replayMode != RECORD) {
		return;
	}
	recordFrameNumber();

	std::string identifier = "t";
	if (!beforeRotate) {
		identifier = "tt";
	}

	replayFile << identifier << " " << v.x << " " << v.y << " " << v.z << std::endl;
}

//record camera angle movement into replay file
void recordYawPitch(GLfloat dYaw, GLfloat dPitch) {
	if (replayMode != RECORD) {
		return;
	}
	recordFrameNumber();

	replayFile << "yp " << dYaw << " " << dPitch << std::endl;
}

//record quit into replay file
void recordQuit() {
	if (replayMode != RECORD) {
		return;
	}
	recordFrameNumber();
	replayFile << "e" << std::endl;
}

/*		handle actions allowed in all playback modes (i.e. quit and toggling model rendering mode)
*/
void handleGlobalInput() {
	if (keyPressed(GLFW_KEY_ESCAPE)) {
		recordQuit();
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	static bool swappedLastFrame = false; //if swap key is held since last frame, don't swap again
	bool swapPress = keyPressed(KEY_SWAP_MODELS);
	if (swapPress && !swappedLastFrame) {
		swappedLastFrame = true;
		drawModelType = (drawModelType + 1) % 5;
	}

	if (!swapPress) {
		swappedLastFrame = false;
	}
}

/*		handle inputs allowed in control/record modes (camera movement)
*/
void handleInput() {
	//Fast movement
	if (keyPressed(KEY_SPEED)) {
		cameraSpeed = CAMERA_SPEED_FAST;
	} else {
		cameraSpeed = CAMERA_SPEED_DEFAULT;
	}

	//Movements relative to camera direction
	if (keyPressed(KEY_LEFT)) {
		glm::vec3 vec = glm::vec3(1.0f, 0.0f, 0.0f) * cameraSpeed;
		recordTranslate(vec, true);
		view = glm::translate(glm::mat4(), vec) * view;
	}

	if (keyPressed(KEY_RIGHT)) {
		glm::vec3 vec = glm::vec3(-1.0f, 0.0f, 0.0f) * cameraSpeed;
		recordTranslate(vec, true);
		view = glm::translate(glm::mat4(), vec) * view;
	}

	if (keyPressed(KEY_FORWARD)) {
		glm::vec3 vec = glm::vec3(0.0f, 0.0f, 1.0f) * cameraSpeed;
		recordTranslate(vec, true);
		view = glm::translate(glm::mat4(), vec) * view;
	}

	if (keyPressed(KEY_BACK)) {
		glm::vec3 vec = glm::vec3(0.0f, 0.0f, -1.0f) * cameraSpeed;
		recordTranslate(vec, true);
		view = glm::translate(glm::mat4(), vec) * view;
	}

	//unrotate camera to apply movements relative to world
	view = glm::rotate(glm::mat4(), -oldYaw, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(), -oldPitch, glm::vec3(1.0f, 0.0f, 0.0f)) * view;

	if (keyPressed(KEY_UP)) {
		glm::vec3 vec = glm::vec3(0.0f, -1.0f, 0.0f) * cameraSpeed;
		recordTranslate(vec, false);
		view = glm::translate(glm::mat4(), vec) * view;
	}

	if (keyPressed(KEY_DOWN)) {
		glm::vec3 vec = glm::vec3(0.0f, 1.0f, 0.0f) * cameraSpeed;
		recordTranslate(vec, false);
		view = glm::translate(glm::mat4(), vec) * view;
	}

	//limit camera's vertical angle to prevent it from flipping vertically
	pitch = std::min(((float) PI) / 2.0f, std::max(-((float) PI) / 2.0f, pitch));

	//apply new camera angle
	view = glm::rotate(glm::mat4(), pitch, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(), yaw, glm::vec3(0.0f, 1.0f, 0.0f)) * view;

	oldYaw = yaw;
	oldPitch = pitch;
}

/*	handle mouse movement if not in playback mode
*/
void cursorCallback(GLFWwindow* window, double xPos, double yPos) {
	if (replayMode == PLAY) {
		return;
	}
	double dx = xPos - (SCREEN_WIDTH / 2.0);
	double dy = yPos - (SCREEN_HEIGHT / 2.0);

	GLfloat dYaw = MOUSE_SENSITIVITY * dx;
	GLfloat dPitch = MOUSE_SENSITIVITY * dy;

	yaw += dYaw;
	pitch += dPitch;

	recordYawPitch(dYaw, dPitch);
}
