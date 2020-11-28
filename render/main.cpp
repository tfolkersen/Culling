#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <common/shader.hpp>
#include <ctime>

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

struct Model {
	GLuint varr;
	GLuint posBuff;
	GLuint colBuff;
	GLuint normBuff;
	int nVerts;
};

Model box;

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

void makeModels() {
	glGenVertexArrays(1, &box.varr);
	glBindVertexArray(box.varr);


	glGenBuffers(1, &box.posBuff);
	glBindBuffer(GL_ARRAY_BUFFER, box.posBuff);
	const static GLfloat boxPos[] = {
	-1.0f, 0.0f, 0.0f, //1
	-1.0f, 1.0f, 0.0f, //7
	1.0f, 1.0f, 0.0f, //9

	1.0f, 1.0f, 0.0f, //9
	1.0f, 0.0f, 0.0f, //3
	-1.0f, 0.0f, 0.0f //1
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxPos), boxPos, GL_STATIC_DRAW);

	glGenBuffers(1, &box.colBuff);
	glBindBuffer(GL_ARRAY_BUFFER, box.colBuff);
	const static GLfloat boxCol[] = {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f
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

	makeModels();
}

void render() {
	glClearColor(0.3f, 0.3f, 0.3f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT);

	setLights();

	view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	project = glm::perspective(glm::radians(45.0f), 1024.0f / 768.0f, 0.01f, 100.0f);


	time_t t = time(NULL);
	GLfloat rad = ((GLfloat)t) / 1.0;
	model = glm::rotate(model, 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));

	mvp = project * view * model;
	drawModel(box);


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

	window = glfwCreateWindow(1024, 768, "asd", NULL, NULL);

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

	programID = LoadShaders("v.glsl", "f.glsl");
	glUseProgram(programID);

	init();

	do {
		render();
		glfwSwapBuffers(window);

		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}


	} while (!glfwWindowShouldClose(window));



	return 0;
}