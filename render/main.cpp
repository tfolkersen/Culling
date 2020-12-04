#include <iostream>

//GL stuff
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "utility.h"
#include "cull.h"
#include "draw.h"
#include "control.h"
#include "models.h"


//To load vertex and fragment shaders
#include <common/shader.hpp>

#include <ctime>

#include <vector>
#include <algorithm>

//Stuff for file i/o
#include <fstream>
#include <sstream>
#include <string>



void init() {
//u_LightPos, u_LightColor, u_AmbientLight, u_MvpMat, u_ModelMat, u_NormalMat;
	u_LightPos = glGetUniformLocation(programID, "u_LightPos");
	u_LightColor = glGetUniformLocation(programID, "u_LightColor");
	u_AmbientLight = glGetUniformLocation(programID, "u_AmbientLight");
	u_MvpMat = glGetUniformLocation(programID, "u_MvpMat");
	u_ModelMat = glGetUniformLocation(programID, "u_ModelMat");
	u_NormalMat = glGetUniformLocation(programID, "u_NormalMat");

	std::cout << "Locations ";
	std::cout << u_LightPos << " " << u_LightColor << " " << u_AmbientLight << " " << u_MvpMat << " " << u_ModelMat << " " << u_NormalMat << std::endl;

	view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	project = glm::perspective(glm::radians(90.0f), (GLfloat) SCREEN_WIDTH / (GLfloat) SCREEN_HEIGHT, NEAR, 200.0f);

	makeScene3();

	if (replayMode == PLAY) {
		replayFile.open(replayFileName, std::fstream::in);
		if (replayFile.peek() == std::fstream::traits_type::eof()) {
			std::cout << "Empty replay file used in playback -- program will crash" << std::endl;
		}
	} else if (replayMode == RECORD) {
		replayFile.open(replayFileName, std::fstream::out | std::fstream::trunc);
	}

	std::cout << "Replay file status: " << replayFile.fail() << std::endl;

	if (replayMode == PLAY) {
		std::string line;
		std::vector<std::string> tokens;

		std::getline(replayFile, line);
		tokens = split(line, " ");
		nextReplayFrame = std::stoull(tokens[1]);
	}

	if (recordStats) {
		statsFile.open("stats.txt", std::fstream::out | std::fstream::trunc);
	}
}


int main(int argc, char **argv) {
	replayMode = CONTROL;
	for (int i = 1; i < argc; i++) {
		std::string token = argv[i];

		if (token == "-r") {
			replayMode = RECORD;
		}
		else if (token == "-p") {
			replayMode = PLAY;
		}
		else if (token == "-s") {
			recordStats = true;
		}
	}


	if (glfwInit() != GL_TRUE) {
		std::cerr << "Failed to init glfw" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "asd", NULL, NULL);

	if (window == NULL) {
		std::cerr << "Failed to make glfw window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to init GLEW" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	programID = LoadShaders("v.glsl", "f.glsl");
	glUseProgram(programID);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	glfwSetCursorPosCallback(window, cursorCallback);

	init();


	do {
		recordedFrameNumber = false;

		clock_t start = clock();
		render2();
		glfwSwapBuffers(window);

		glfwPollEvents();

		handleGlobalInput();
		if (replayMode == PLAY) {
			handlePlayback();
		} else {
			handleInput();
		}

		glfwSetCursorPos(window, SCREEN_WIDTH / 2.0, SCREEN_HEIGHT / 2.0);
		clock_t end = clock();

		double elapsed = (end - start) / (double)CLOCKS_PER_SEC;
		fpsWait(1 / (double)FPS - elapsed);
		currentFrame++;
	} while (!glfwWindowShouldClose(window));

	replayFile.close();
	statsFile.close();
	std::cout << "Ending on frame " << currentFrame << std::endl;
	return 0;
}



//rasterizeSilent(glm::vec2(0.5, -0.4), glm::vec2(-0.6, -0.3), glm::vec2(-0.04, -0.9)); //benchmark reference
