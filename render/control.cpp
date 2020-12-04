#include "control.h"
#include "draw.h"
#include "utility.h"
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <fstream>
#include <algorithm>


GLfloat cameraSpeed = CAMERA_SPEED_DEFAULT; //Camera movement per frame in any direction
int replayMode = CONTROL;

int drawModelType = MAIN;

std::string replayFileName = "replay.txt";
std::fstream replayFile;

uint64_t currentFrame = 0;
uint64_t nextReplayFrame = 0;
bool recordedFrameNumber = false;

GLfloat yaw = 0.0f;
GLfloat pitch = 0.0f;
GLfloat oldYaw = yaw;
GLfloat oldPitch = pitch;



bool keyPressed(int key) {
	return glfwGetKey(window, key) == GLFW_PRESS;
}

void handlePlayback() {
	if (currentFrame != nextReplayFrame) {
		return;
	}

	std::string line;
	std::vector<std::string> tokens;

	std::vector<glm::vec3> translation1;
	std::vector<glm::vec3> translation2;
	std::vector<std::pair<GLfloat, GLfloat>> yawPitch;
	bool quit = false;

	//Get all the actions
	while (true && !replayFile.eof()) {
		std::getline(replayFile, line);
		tokens = split(line, " ");

		if (tokens[0] == "f") {
			nextReplayFrame = std::stoull(tokens[1]);
			break;
		} else if (tokens[0] == "t") {
			GLfloat dx = std::stof(tokens[1]);
			GLfloat dy = std::stof(tokens[2]);
			GLfloat dz = std::stof(tokens[3]);
			translation1.push_back(glm::vec3(dx, dy, dz));
		} else if (tokens[0] == "tt") {
			GLfloat dx = std::stof(tokens[1]);
			GLfloat dy = std::stof(tokens[2]);
			GLfloat dz = std::stof(tokens[3]);
			translation2.push_back(glm::vec3(dx, dy, dz));
		} else if (tokens[0] == "yp") {
			GLfloat dYaw = std::stof(tokens[1]);
			GLfloat dPitch = std::stof(tokens[2]);
			yawPitch.push_back(std::pair<GLfloat, GLfloat>(dYaw, dPitch));
		} else if (tokens[0] == "e") {
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

	view = glm::rotate(glm::mat4(), -oldYaw, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(), -oldPitch, glm::vec3(1.0f, 0.0f, 0.0f)) * view;

	for (auto it = translation2.begin(); it != translation2.end(); it++) {
		view = glm::translate(glm::mat4(), *it) * view;
	}

	for (auto it = yawPitch.begin(); it != yawPitch.end(); it++) {
		yaw += it->first;
		pitch += it->second;
	}

	pitch = std::min(((float) PI) / 2.0f, std::max(-((float) PI) / 2.0f, pitch));

	view = glm::rotate(glm::mat4(), pitch, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(), yaw, glm::vec3(0.0f, 1.0f, 0.0f)) * view;

	oldYaw = yaw;
	oldPitch = pitch;
}


void recordFrameNumber() {
	if (recordedFrameNumber) {
		return;
	}
	recordedFrameNumber = true;
	replayFile << "f " << currentFrame << std::endl;
}


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

void recordYawPitch(GLfloat dYaw, GLfloat dPitch) {
	if (replayMode != RECORD) {
		return;
	}
	recordFrameNumber();

	replayFile << "yp " << dYaw << " " << dPitch << std::endl;
}

void recordQuit() {
	if (replayMode != RECORD) {
		return;
	}
	recordFrameNumber();
	replayFile << "e" << std::endl;
}

void handleGlobalInput() {
	if (keyPressed(GLFW_KEY_ESCAPE)) {
		recordQuit();
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	static bool swappedLastFrame = false;
	bool swapPress = keyPressed(KEY_SWAP_MODELS);
	if (swapPress && !swappedLastFrame) {
		swappedLastFrame = true;
		drawModelType = (drawModelType + 1) % 5;
	}

	if (!swapPress) {
		swappedLastFrame = false;
	}
}



void handleInput() {
	if (keyPressed(KEY_SPEED)) {
		cameraSpeed = CAMERA_SPEED_FAST;
	} else {
		cameraSpeed = CAMERA_SPEED_DEFAULT;
	}

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

	pitch = std::min(((float) PI) / 2.0f, std::max(-((float) PI) / 2.0f, pitch));


	view = glm::rotate(glm::mat4(), pitch, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(), yaw, glm::vec3(0.0f, 1.0f, 0.0f)) * view;

	oldYaw = yaw;
	oldPitch = pitch;
}

void cursorCallback(GLFWwindow* window, double xPos, double yPos) {
	double dx = xPos - (SCREEN_WIDTH / 2.0);
	double dy = yPos - (SCREEN_HEIGHT / 2.0);

	GLfloat dYaw = MOUSE_SENSITIVITY * dx;
	GLfloat dPitch = MOUSE_SENSITIVITY * dy;

	yaw += dYaw;
	pitch += dPitch;

	recordYawPitch(dYaw, dPitch);
}


