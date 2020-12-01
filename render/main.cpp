//rasterizeSilent(glm::vec2(0.5, -0.4), glm::vec2(-0.6, -0.3), glm::vec2(-0.04, -0.9)); //benchmark reference
#include <iostream>
#include "utility.h"
#include "cull.h"

//GL stuff
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//To load vertex and fragment shaders
#include <common/shader.hpp>

#include <ctime>

#include <vector>
#include <algorithm>

//Stuff for file i/o
#include <fstream>
#include <sstream>
#include <string>

//////////////////// Controls
//Keys
#define KEY_FORWARD GLFW_KEY_W //Move forward
#define KEY_LEFT GLFW_KEY_A //Move left
#define KEY_BACK GLFW_KEY_S //Move backwards
#define KEY_RIGHT GLFW_KEY_D //Move right
#define KEY_UP GLFW_KEY_SPACE //Move up
#define KEY_DOWN GLFW_KEY_LEFT_SHIFT //Move down
#define KEY_SWAP_MODELS GLFW_KEY_F //Toggle model rendering modes (normal/occluder mesh/bounding box)

GLfloat cameraSpeed = 0.05f; //Camera movement per frame in any direction
#define MOUSE_SENSITIVITY 0.001 / 1.6 //How fast to rotate the camera with mouse movement


#define PI 3.141592654
#define FPS 60 //Max frames per second


enum replayEnum {CONTROL, RECORD, PLAY};
int replayMode = CONTROL;

enum modelTypeEnum{MAIN = 0, OCCLUDER = 1, BOX = 2};
int drawModelType = MAIN;

std::string replayFileName = "replay.txt";
std::fstream replayFile;

uint64_t currentFrame = 0;
uint64_t nextReplayFrame = 0;
bool recordedFrameNumber = false;


bool keyPressed(int key);



void fpsWait(double seconds) {
	if (seconds < 0) {
		return;
	}
	clock_t start = clock();
	while ((clock() - start) / (double) CLOCKS_PER_SEC < seconds) {
	}
}



#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

struct Model {
	GLuint varr;
	GLuint posBuff;
	GLuint colBuff;
	GLuint normBuff;
	int nVerts;

	Model() {
	}

	Model(const Model &m) {
		this->varr = m.varr;
		this->posBuff = m.posBuff;
		this->colBuff = m.colBuff;
		this->nVerts = m.nVerts;
	}
};

struct Model3 {
	Model main;
	Model occluder;
	Model box;

	std::vector<GLfloat> occluderData;
	std::vector<GLfloat> boxData;
	glm::mat4 modelMatrix;

	Model3() {
	}

	Model3(const Model3 &m) {
		this->main = m.main;
		this->occluder = m.occluder;
		this->box = m.box;
		this->occluderData = m.occluderData;
		this->boxData = m.boxData;
		this->modelMatrix = m.modelMatrix;
	}
};

Model box;
Model plant;
Model cube;
Model3 office;





Model parseObj(std::string fileName, GLfloat r, GLfloat g, GLfloat b, std::vector<GLfloat>& posData = std::vector<GLfloat>());

Model3 parseModel3(std::string mainFileName, GLfloat r, GLfloat g, GLfloat b, std::string occluderFileName, std::string boxFileName) {
	Model3 m;

	m.main = parseObj(mainFileName, r, g, b);
	m.occluder = parseObj(occluderFileName, 1.0f, 0.0f, 0.0f, m.occluderData);
	m.box = parseObj(boxFileName, 1.0f, 1.0f, 0.0f, m.boxData);

	return m;
}

Model parseObj(std::string fileName, GLfloat r, GLfloat g, GLfloat b, std::vector<GLfloat> &posData){
	posData.clear();

	std::string line;
	std::vector<std::string> tokens;
	std::vector<std::vector<std::string>> subtokens;

	std::vector<GLfloat> vertices;
	std::vector<GLfloat> normals;

	std::vector<GLfloat> normalData;
	std::vector<GLfloat> colorData;

	std::fstream f(fileName);
	while (std::getline(f, line)) {
		tokens = split(line, " ");
		if (tokens.empty()) {
			continue;
		}

		if (tokens[0] == "v") {
			GLfloat x = std::stof(tokens[1]);
			GLfloat y = std::stof(tokens[2]);
			GLfloat z = std::stof(tokens[3]);
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
		} else if (tokens[0] == "vn") {
			GLfloat x = std::stof(tokens[1]);
			GLfloat y = std::stof(tokens[2]);
			GLfloat z = std::stof(tokens[3]);
			normals.push_back(x);
			normals.push_back(y);
			normals.push_back(z);
		}
	}
	f.close();

	f = std::fstream(fileName);
	while (std::getline(f, line)) {
		tokens = split(line, " ");
		if (tokens.empty()) {
			continue;
		}

		if (tokens[0] == "f") {
			subtokens.clear();

			for (size_t j = 1; j < tokens.size(); j++) {
				subtokens.push_back(split(tokens[j], "/"));
			}

			for (size_t j = 0; j < subtokens.size() - 2; j++) {
				for (size_t k = 0; k < 3; k++) {
					int vi;
					int vni;

					if (k == 0) {
						vi = std::stoi(subtokens[0][0]) - 1;
						vni = std::stoi(subtokens[0][2]) - 1;
					} else {
						vi = std::stoi(subtokens[j + k][0]) - 1;
						vni = std::stoi(subtokens[j + k][2]) - 1;
					}

					posData.push_back(vertices[vi * 3 + 0]);
					posData.push_back(vertices[vi * 3 + 1]);
					posData.push_back(vertices[vi * 3 + 2]);
					normalData.push_back(normals[vni * 3 + 0]);
					normalData.push_back(normals[vni * 3 + 1]);
					normalData.push_back(normals[vni * 3 + 2]);
					colorData.push_back(r);
					colorData.push_back(g);
					colorData.push_back(b);
				}
			}
		}
	}
	f.close();

	Model m;
	glGenVertexArrays(1, &m.varr);
	glBindVertexArray(m.varr);

	glGenBuffers(1, &m.posBuff);
	glBindBuffer(GL_ARRAY_BUFFER, m.posBuff);
	glBufferData(GL_ARRAY_BUFFER, posData.size() * sizeof(GLfloat), posData.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &m.colBuff);
	glBindBuffer(GL_ARRAY_BUFFER, m.colBuff);
	glBufferData(GL_ARRAY_BUFFER, colorData.size() * sizeof(GLfloat), colorData.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &m.normBuff);
	glBindBuffer(GL_ARRAY_BUFFER, m.normBuff);
	glBufferData(GL_ARRAY_BUFFER, normalData.size() * sizeof(GLfloat), normalData.data(), GL_STATIC_DRAW);

	m.nVerts = posData.size() / 3;
	return m;

}







GLuint programID;
GLFWwindow* window = NULL;

GLuint u_LightPos, u_LightColor, u_AmbientLight, u_MvpMat, u_ModelMat, u_NormalMat;

glm::vec3 lightPos(6.0f, 6.0f, 0.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
glm::vec3 ambientLight(0.2f, 0.2f, 0.2f);

glm::mat4 mvp;
glm::mat4 model;
glm::mat4 normal;
glm::mat4 view;
glm::mat4 project;

GLfloat yaw = 0.0f;
GLfloat pitch = 0.0f;
GLfloat oldYaw = yaw;
GLfloat oldPitch = pitch;

void setMatrices() {
	glUniformMatrix4fv(u_MvpMat, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(u_ModelMat, 1, GL_FALSE, &model[0][0]);

	normal = glm::inverse(model);
	normal = glm::transpose(normal);
	glUniformMatrix4fv(u_NormalMat, 1, GL_FALSE, &normal[0][0]);
}

void setLights() {
	glUniform3f(u_LightPos, lightPos[0], lightPos[1], lightPos[2]);
	glUniform3f(u_LightColor, lightColor[0], lightColor[1], lightColor[2]);
	glUniform3f(u_AmbientLight, ambientLight[0], ambientLight[1], ambientLight[2]);
}

void drawModel(Model &m) {
	glBindVertexArray(m.varr);

	glBindBuffer(GL_ARRAY_BUFFER, m.posBuff);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glBindBuffer(GL_ARRAY_BUFFER, m.colBuff);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glBindBuffer(GL_ARRAY_BUFFER, m.normBuff);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	setMatrices();

	glDrawArrays(GL_TRIANGLES, 0, m.nVerts);
}

void drawModel3(Model3 &m) {
	model = m.modelMatrix;
	mvp = project * view * model;
	if (drawModelType == OCCLUDER) {
		drawModel(m.occluder);
	} else if (drawModelType == BOX) {
		drawModel(m.box);
	} else {
		drawModel(m.main);
	}
}

void makeModels() {
	glGenVertexArrays(1, &box.varr);
	glBindVertexArray(box.varr);

	glGenBuffers(1, &box.posBuff);
	glBindBuffer(GL_ARRAY_BUFFER, box.posBuff);
	const static GLfloat boxPos[] = {
	1.0f, 1.0f, 0.0f, //9
	-1.0f, 1.0f, 0.0f, //7
	-1.0f, 0.0f, 0.0f, //1

	-1.0f, 0.0f, 0.0f, //1
	1.0f, 0.0f, 0.0f, //3
	1.0f, 1.0f, 0.0f //9
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxPos), boxPos, GL_STATIC_DRAW);

	glGenBuffers(1, &box.colBuff);
	glBindBuffer(GL_ARRAY_BUFFER, box.colBuff);
	const static GLfloat boxCol[] = {
		0.0f, 0.0f, 1.0f, //9
		0.0f, 1.0f, 0.0f, //7
		1.0f, 0.0f, 0.0f, //1

		1.0f, 0.0f, 0.0f, //1
		1.0f, 0.0f, 1.0f, //3
		0.0f, 0.0f, 1.0f //9
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxCol), boxCol, GL_STATIC_DRAW);

	glGenBuffers(1, &box.normBuff);
	glBindBuffer(GL_ARRAY_BUFFER, box.normBuff);
	const static GLfloat boxNorm[] = {
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxNorm), boxNorm, GL_STATIC_DRAW);

	box.nVerts = 6;

	plant = parseObj("models/banana_plant.obj", 0.3f, 1.0f, 0.0f);
	cube = parseObj("models/cube.obj", 1.0f, 1.0f, 0.0f);
	office = parseModel3("models/office/main.obj", 0.41f, 0.2f, 0.0f, "models/office/occluder.obj", "models/office/box.obj");
}

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
	project = glm::perspective(glm::radians(90.0f), (GLfloat) SCREEN_WIDTH / (GLfloat) SCREEN_HEIGHT, 0.01f, 100.0f);

	makeModels();

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

}

void render() {

	double seconds = clock() / (double)CLOCKS_PER_SEC;
	lightPos = glm::vec3(4.0f * cos(seconds), 0.0f, 4.0f * sin(seconds));

	glClearColor(0.3f, 0.3f, 0.3f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setLights();


	GLfloat rad = seconds / 1.0f;
	rad = 0.0f;

	model = glm::rotate(glm::mat4(), rad, glm::vec3(0.0f, 1.0f, 0.0f));

	mvp = project * view * model;
	drawModel(box);

	model = glm::mat4();
	model = glm::translate(model, glm::vec3(0.0f, -1.0f, 1.0f));
	model = glm::rotate(model, rad, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
	mvp = project * view * model;
	drawModel(plant);

	model = glm::mat4();
	mvp = project * view * model;
	drawModel3(office);

	model = glm::mat4();
	model = glm::translate(model, lightPos + glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
	mvp = project * view * model;
	drawModel(cube);
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
		drawModelType = (drawModelType + 1) % 3;
	}

	if (!swapPress) {
		swappedLastFrame = false;
	}
}



void handleInput() {

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


int main() {
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

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glfwSetCursorPosCallback(window, cursorCallback);

	init();


	do {
		recordedFrameNumber = false;

		clock_t start = clock();
		render();
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
	std::cout << "Ending on frame " << currentFrame << std::endl;
	return 0;
}

bool keyPressed(int key) {
	return glfwGetKey(window, key) == GLFW_PRESS;
}

