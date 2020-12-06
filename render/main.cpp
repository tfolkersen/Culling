/*	main file -- this is where execution starts
*/


#include <iostream>

//GL stuff
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//Other project files
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

/*
		Initializes variables, files, and the scene
*/
void init() {
	//Get uniform locations
	//u_LightPos, u_LightColor, u_AmbientLight, u_MvpMat, u_ModelMat, u_NormalMat;
	u_LightPos = glGetUniformLocation(programID, "u_LightPos");
	u_LightColor = glGetUniformLocation(programID, "u_LightColor");
	u_AmbientLight = glGetUniformLocation(programID, "u_AmbientLight");
	u_MvpMat = glGetUniformLocation(programID, "u_MvpMat");
	u_ModelMat = glGetUniformLocation(programID, "u_ModelMat");
	u_NormalMat = glGetUniformLocation(programID, "u_NormalMat");

	//initialize matrices
	view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	project = glm::perspective(glm::radians(90.0f), (GLfloat) SCREEN_WIDTH / (GLfloat) SCREEN_HEIGHT, NEAR, FAR);

	//Initialize the scene
	makeScene3();

	//Initialize replay and stats files
	if (replayMode == PLAY) {
		replayFile.open(replayFileName, std::fstream::in);
		if (replayFile.peek() == std::fstream::traits_type::eof()) {
			std::cout << "Empty replay file used in playback mode -- program will now stop" << std::endl;
			exit(-1);
		}
	} else if (replayMode == RECORD) {
		replayFile.open(replayFileName, std::fstream::out | std::fstream::trunc);
	}

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


//Main function...
int main(int argc, char **argv) {
	
	//Parse command line arguments
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


	//Initialize GLFW and make window
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

	//Initialize glew and enable some GL features
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to init GLEW" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //prevent cursor from being seen in window

	programID = LoadShaders("v.glsl", "f.glsl");
	glUseProgram(programID);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	glfwSetCursorPosCallback(window, cursorCallback); //used to capture mouse movement

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

		glfwSetCursorPos(window, SCREEN_WIDTH / 2.0, SCREEN_HEIGHT / 2.0); //reset cursor to center of window
		clock_t end = clock();

		double elapsed = (end - start) / (double)CLOCKS_PER_SEC; //how long this frame took
		fpsWait(1 / (double)FPS - elapsed); //limit FPS
		currentFrame++;
	} while (!glfwWindowShouldClose(window));

	replayFile.close();
	statsFile.close();
	std::cout << "Ending on frame " << currentFrame << std::endl;
	return 0;
}
