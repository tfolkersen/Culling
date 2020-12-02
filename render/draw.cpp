#include "draw.h"
#include "control.h"
#include "cull.h"
#include <ctime>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

GLuint programID;
GLFWwindow* window = NULL;

GLuint u_LightPos, u_LightColor, u_AmbientLight, u_MvpMat, u_ModelMat, u_NormalMat;

glm::vec3 lightPos(6.0f, 6.0f, 0.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
glm::vec3 ambientLight(0.3f, 0.3f, 0.3f);

glm::mat4 mvp;
glm::mat4 model;
glm::mat4 normal;
glm::mat4 view;
glm::mat4 project;

std::vector<ModelCollection> sceneModels;


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

void drawModelCollection(ModelCollection &m) {
	model = m.modelMatrix;
	mvp = project * view * model;
	if (drawModelType == OCCLUDER) {
		drawModel(m.occluder);
	} else if (drawModelType == BOX) {
		drawModel(m.box);
	} else if (drawModelType == MARKER) {
		drawModel(m.marker);
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
	office = parseModelCollection("models/office/main.obj", 0.41f, 0.2f, 0.0f, "models/office/occluder.obj", "models/office/box.obj", "models/office/marker.obj");
}


void render() {

	double seconds = clock() / (double)CLOCKS_PER_SEC;
	lightPos = glm::vec3(4.0f * cos(seconds), 0.0f, 4.0f * sin(seconds));


	glm::vec4 center = office.modelMatrix * glm::vec4(office.boxCenter, 1);
	center /= center.a;
	lightPos = glm::vec3(center);

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
	drawModelCollection(office);

	model = glm::mat4();
	model = glm::translate(model, lightPos + glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
	mvp = project * view * model;
	drawModel(cube);
}


void makeScene2() {
	cube = parseObj("models/cube.obj", 1.0f, 1.0f, 0.0f);

	
	office = parseModelCollection("models/office/main.obj", 0.41f, 0.2f, 0.0f, "models/office/occluder.obj", "models/office/box.obj", "models/office/marker.obj");
	office.modelMatrix = glm::mat4();
	office.modelMatrix = glm::translate(office.modelMatrix, glm::vec3(-1.0f, 0.5f, -7.0f));
	office.modelMatrix = glm::scale(office.modelMatrix, glm::vec3(2.0f, 2.0f, 0.5f));
	sceneModels.push_back(office);


	office = parseModelCollection("models/office/main.obj", 0.5f, 0.2f, 1.0f, "models/office/occluder.obj", "models/office/box.obj", "models/office/marker.obj");
	office.modelMatrix = glm::mat4();
	office.modelMatrix = glm::translate(office.modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
	office.modelMatrix = glm::scale(office.modelMatrix, glm::vec3(1.0f, 1.4f, 1.0f));
	sceneModels.push_back(office);

}

bool modelComparator(const ModelCollection& m1, const ModelCollection& m2) {
	return distSquaredToCamera(m1) < distSquaredToCamera(m2);
}

void render2() {
	double seconds = clock() / (double)CLOCKS_PER_SEC;
	lightPos = glm::vec3(6.0f * cos(seconds), 4.0f, 6.0f * sin(seconds) - 2.0f);

	glClearColor(0.3f, 0.3f, 0.3f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setLights();


	std::sort(sceneModels.begin(), sceneModels.end(), modelComparator);

	dBuffer.reset();
	for (auto it = sceneModels.begin(); it != sceneModels.end(); it++) {
		if (shouldDraw(*it)) {
			drawModelCollection(*it);
		} else {
			//std::cout << "skipped " << currentFrame << std::endl;
		}
	}

	//sceneModels[0].modelMatrix = glm::rotate(sceneModels[0].modelMatrix, 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));

	model = glm::mat4();
	model = glm::translate(model, lightPos + glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
	mvp = project * view * model;
	drawModel(cube);
}

double distSquaredToCamera(const ModelCollection &m) {
	glm::vec4 transformed = view * m.modelMatrix * glm::vec4(m.boxCenter, 1.0f);
	glm::vec3 p = glm::vec3(transformed / transformed.a);
	return ((double)p.x * p.x) + ((double)p.y * p.y) + ((double)p.z * p.z);
}



