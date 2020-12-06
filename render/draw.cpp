#include "draw.h"
#include "control.h"
#include "cull.h"
#include <ctime>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include "utility.h"

GLuint programID; //ID of shader program
GLFWwindow* window = NULL; //GLFW window of program

//shader program uniforms
GLuint u_LightPos, u_LightColor, u_AmbientLight, u_MvpMat, u_ModelMat, u_NormalMat;

//light parameters -- some of these are modified/overwritten by the render function
glm::vec3 lightPos(6.0f, 6.0f, 0.0f);
glm::vec3 lightColor(2.0f, 2.0f, 2.0f);
glm::vec3 ambientLight(0.5f, 0.5f, 0.5f);

//transformation matrices
glm::mat4 mvp;
glm::mat4 model;
glm::mat4 normal;
glm::mat4 view;
glm::mat4 project;

//objects in the current scene
std::vector<ModelCollection> sceneModels;

std::fstream statsFile; //file to record stats to
bool recordStats = false; //should stats be recorded?

//send matrices to GL shader program uniforms
void setMatrices() {
	glUniformMatrix4fv(u_MvpMat, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(u_ModelMat, 1, GL_FALSE, &model[0][0]);

	normal = glm::inverse(model);
	normal = glm::transpose(normal);
	glUniformMatrix4fv(u_NormalMat, 1, GL_FALSE, &normal[0][0]);
}

//send lights to GL shader program uniforms
void setLights() {
	glUniform3f(u_LightPos, lightPos[0], lightPos[1], lightPos[2]);
	glUniform3f(u_LightColor, lightColor[0], lightColor[1], lightColor[2]);
	glUniform3f(u_AmbientLight, ambientLight[0], ambientLight[1], ambientLight[2]);
}

//draw one model in GL
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

//draw a model for this object based on the current rendering mode (i.e. main meshes, occluders, bounding boxes, etc.)
void drawModelCollection(ModelCollection &m) {
	//recompute some matrices
	model = m.modelMatrix;
	mvp = project * view * model;

	if (drawModelType == OCCLUDER) { //just the occluder mesh
		drawModel(m.occluder);
	} else if (drawModelType == BOX) { //just the bounding box
		drawModel(m.box);
	} else if (drawModelType == MARKER) { //just the marker
		drawModel(m.marker);
	} else if (drawModelType == MARKER2) { //the marker and main mesh
		drawModel(m.marker);
		drawModel(m.main);
	} else { //just the main mesh
		drawModel(m.main);
	}
}

//make default scene
void makeDefaultScene() {
	cube = parseObj("models/cube.obj", 1.0f, 1.0f, 1.0f); //used to show where the light is
	ModelCollection orangeOffice = parseModelCollection("models/office/main.obj", 0.5f, 0.1f, 0.0f, "models/office/occluder2.obj", "models/office/box2.obj", "models/office/marker.obj");
	ModelCollection greenOffice = parseModelCollection("models/office/main.obj", 0.1f, 0.4f, 0.0f, "models/office/occluder2.obj", "models/office/box2.obj", "models/office/marker.obj");
	ModelCollection purpleOffice = parseModelCollection("models/office/main.obj", 0.4f, 0.0f, 0.7f, "models/office/occluder2.obj", "models/office/box2.obj", "models/office/marker.obj");
	ModelCollection blueOffice = parseModelCollection("models/office/main.obj", 0.0f, 0.0f, 0.4f, "models/office/occluder2.obj", "models/office/box2.obj", "models/office/marker.obj");
	ModelCollection redOffice = parseModelCollection("models/office/main.obj", 0.4f, 0.0f, 0.0f, "models/office/occluder2.obj", "models/office/box2.obj", "models/office/marker.obj");
	ModelCollection brownOffice = parseModelCollection("models/office/main.obj", 0.45f, 0.18f, 0.07f, "models/office/occluder2.obj", "models/office/box2.obj", "models/office/marker.obj");
	ModelCollection yellowOffice = parseModelCollection("models/office/main.obj", 0.4, 0.4f, 0.07f, "models/office/occluder2.obj", "models/office/box2.obj", "models/office/marker.obj");

	//Buildings outside of the "city block" of buildings
	brownOffice.modelMatrix = glm::mat4();
	brownOffice.modelMatrix = glm::translate(brownOffice.modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
	brownOffice.modelMatrix = glm::scale(brownOffice.modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	brownOffice.modelMatrix = glm::rotate(brownOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(brownOffice);

	brownOffice.modelMatrix = glm::mat4();
	brownOffice.modelMatrix = glm::translate(brownOffice.modelMatrix, glm::vec3(-40.0f, 0.0f, -36.0f));
	brownOffice.modelMatrix = glm::scale(brownOffice.modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	brownOffice.modelMatrix = glm::rotate(brownOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(brownOffice);

	//row 1
	redOffice.modelMatrix = glm::mat4();
	redOffice.modelMatrix = glm::translate(redOffice.modelMatrix, glm::vec3(-20.0f, 0.0f, -20.0f));
	redOffice.modelMatrix = glm::scale(redOffice.modelMatrix, glm::vec3(1.0f, 1.5f, 1.0f));
	redOffice.modelMatrix = glm::rotate(redOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(redOffice);

	orangeOffice.modelMatrix = glm::mat4();
	orangeOffice.modelMatrix = glm::translate(orangeOffice.modelMatrix, glm::vec3(-10.0f, 0.0f, -20.0f));
	orangeOffice.modelMatrix = glm::scale(orangeOffice.modelMatrix, glm::vec3(1.0f, 1.5f, 1.0f));
	orangeOffice.modelMatrix = glm::rotate(orangeOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(orangeOffice);

	yellowOffice.modelMatrix = glm::mat4();
	yellowOffice.modelMatrix = glm::translate(yellowOffice.modelMatrix, glm::vec3(0.0f, 0.0f, -20.0f));
	yellowOffice.modelMatrix = glm::scale(yellowOffice.modelMatrix, glm::vec3(1.0f, 1.5f, 1.0f));
	yellowOffice.modelMatrix = glm::rotate(yellowOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(yellowOffice);

	greenOffice.modelMatrix = glm::mat4();
	greenOffice.modelMatrix = glm::translate(greenOffice.modelMatrix, glm::vec3(10.0f, 0.0f, -20.0f));
	greenOffice.modelMatrix = glm::scale(greenOffice.modelMatrix, glm::vec3(1.0f, 1.5f, 1.0f));
	greenOffice.modelMatrix = glm::rotate(greenOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(greenOffice);

	blueOffice.modelMatrix = glm::mat4();
	blueOffice.modelMatrix = glm::translate(blueOffice.modelMatrix, glm::vec3(20.0f, 0.0f, -20.0f));
	blueOffice.modelMatrix = glm::scale(blueOffice.modelMatrix, glm::vec3(1.0f, 1.5f, 1.0f));
	blueOffice.modelMatrix = glm::rotate(blueOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(blueOffice);

	//row 2
	greenOffice.modelMatrix = glm::mat4();
	greenOffice.modelMatrix = glm::translate(greenOffice.modelMatrix, glm::vec3(-20.0f, 0.0f, -28.0f));
	greenOffice.modelMatrix = glm::scale(greenOffice.modelMatrix, glm::vec3(1.0f, 1.5f, 1.0f));
	greenOffice.modelMatrix = glm::rotate(greenOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(greenOffice);

	yellowOffice.modelMatrix = glm::mat4();
	yellowOffice.modelMatrix = glm::translate(yellowOffice.modelMatrix, glm::vec3(-10.0f, 0.0f, -28.0f));
	yellowOffice.modelMatrix = glm::scale(yellowOffice.modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	yellowOffice.modelMatrix = glm::rotate(yellowOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(yellowOffice);

	orangeOffice.modelMatrix = glm::mat4();
	orangeOffice.modelMatrix = glm::translate(orangeOffice.modelMatrix, glm::vec3(0.0f, 0.0f, -28.0f));
	orangeOffice.modelMatrix = glm::scale(orangeOffice.modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	orangeOffice.modelMatrix = glm::rotate(orangeOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(orangeOffice);

	redOffice.modelMatrix = glm::mat4();
	redOffice.modelMatrix = glm::translate(redOffice.modelMatrix, glm::vec3(10.0f, 0.0f, -28.0f));
	redOffice.modelMatrix = glm::scale(redOffice.modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	redOffice.modelMatrix = glm::rotate(redOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(redOffice);

	purpleOffice.modelMatrix = glm::mat4();
	purpleOffice.modelMatrix = glm::translate(purpleOffice.modelMatrix, glm::vec3(20.0f, 0.0f, -28.0f));
	purpleOffice.modelMatrix = glm::scale(purpleOffice.modelMatrix, glm::vec3(1.0f, 1.5f, 1.0f));
	purpleOffice.modelMatrix = glm::rotate(purpleOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(purpleOffice);

	//row 3
	blueOffice.modelMatrix = glm::mat4();
	blueOffice.modelMatrix = glm::translate(blueOffice.modelMatrix, glm::vec3(-20.0f, 0.0f, -36.0f));
	blueOffice.modelMatrix = glm::scale(blueOffice.modelMatrix, glm::vec3(1.0f, 1.5f, 1.0f));
	blueOffice.modelMatrix = glm::rotate(blueOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(blueOffice);

	purpleOffice.modelMatrix = glm::mat4();
	purpleOffice.modelMatrix = glm::translate(purpleOffice.modelMatrix, glm::vec3(-10.0f, 0.0f, -36.0f));
	purpleOffice.modelMatrix = glm::scale(purpleOffice.modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	purpleOffice.modelMatrix = glm::rotate(purpleOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(purpleOffice);

	redOffice.modelMatrix = glm::mat4();
	redOffice.modelMatrix = glm::translate(redOffice.modelMatrix, glm::vec3(0.0f, 0.0f, -36.0f));
	redOffice.modelMatrix = glm::scale(redOffice.modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	redOffice.modelMatrix = glm::rotate(redOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(redOffice);

	orangeOffice.modelMatrix = glm::mat4();
	orangeOffice.modelMatrix = glm::translate(orangeOffice.modelMatrix, glm::vec3(10.0f, 0.0f, -36.0f));
	orangeOffice.modelMatrix = glm::scale(orangeOffice.modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	orangeOffice.modelMatrix = glm::rotate(orangeOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(orangeOffice);

	yellowOffice.modelMatrix = glm::mat4();
	yellowOffice.modelMatrix = glm::translate(yellowOffice.modelMatrix, glm::vec3(20.0f, 0.0f, -36.0f));
	yellowOffice.modelMatrix = glm::scale(yellowOffice.modelMatrix, glm::vec3(1.0f, 1.5f, 1.0f));
	yellowOffice.modelMatrix = glm::rotate(yellowOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(yellowOffice);

	//row 4
	orangeOffice.modelMatrix = glm::mat4();
	orangeOffice.modelMatrix = glm::translate(orangeOffice.modelMatrix, glm::vec3(-20.0f, 0.0f, -44.0f));
	orangeOffice.modelMatrix = glm::scale(orangeOffice.modelMatrix, glm::vec3(1.0f, 1.5f, 1.0f));
	orangeOffice.modelMatrix = glm::rotate(orangeOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(orangeOffice);

	redOffice.modelMatrix = glm::mat4();
	redOffice.modelMatrix = glm::translate(redOffice.modelMatrix, glm::vec3(-10.0f, 0.0f, -44.0f));
	redOffice.modelMatrix = glm::scale(redOffice.modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	redOffice.modelMatrix = glm::rotate(redOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(redOffice);

	purpleOffice.modelMatrix = glm::mat4();
	purpleOffice.modelMatrix = glm::translate(purpleOffice.modelMatrix, glm::vec3(0.0f, 0.0f, -44.0f));
	purpleOffice.modelMatrix = glm::scale(purpleOffice.modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	purpleOffice.modelMatrix = glm::rotate(purpleOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(purpleOffice);

	blueOffice.modelMatrix = glm::mat4();
	blueOffice.modelMatrix = glm::translate(blueOffice.modelMatrix, glm::vec3(10.0f, 0.0f, -44.0f));
	blueOffice.modelMatrix = glm::scale(blueOffice.modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	blueOffice.modelMatrix = glm::rotate(blueOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(blueOffice);

	greenOffice.modelMatrix = glm::mat4();
	greenOffice.modelMatrix = glm::translate(greenOffice.modelMatrix, glm::vec3(20.0f, 0.0f, -44.0f));
	greenOffice.modelMatrix = glm::scale(greenOffice.modelMatrix, glm::vec3(1.0f, 1.5f, 1.0f));
	greenOffice.modelMatrix = glm::rotate(greenOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(greenOffice);

	//row 5
	yellowOffice.modelMatrix = glm::mat4();
	yellowOffice.modelMatrix = glm::translate(yellowOffice.modelMatrix, glm::vec3(-20.0f, 0.0f, -52.0f));
	yellowOffice.modelMatrix = glm::scale(yellowOffice.modelMatrix, glm::vec3(1.0f, 1.5f, 1.0f));
	yellowOffice.modelMatrix = glm::rotate(yellowOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(yellowOffice);

	greenOffice.modelMatrix = glm::mat4();
	greenOffice.modelMatrix = glm::translate(greenOffice.modelMatrix, glm::vec3(-10.0f, 0.0f, -52.0f));
	greenOffice.modelMatrix = glm::scale(greenOffice.modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	greenOffice.modelMatrix = glm::rotate(greenOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(greenOffice);

	blueOffice.modelMatrix = glm::mat4();
	blueOffice.modelMatrix = glm::translate(blueOffice.modelMatrix, glm::vec3(0.0f, 0.0f, -52.0f));
	blueOffice.modelMatrix = glm::scale(blueOffice.modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	blueOffice.modelMatrix = glm::rotate(blueOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(blueOffice);

	purpleOffice.modelMatrix = glm::mat4();
	purpleOffice.modelMatrix = glm::translate(purpleOffice.modelMatrix, glm::vec3(10.0f, 0.0f, -52.0f));
	purpleOffice.modelMatrix = glm::scale(purpleOffice.modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	purpleOffice.modelMatrix = glm::rotate(purpleOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(purpleOffice);

	redOffice.modelMatrix = glm::mat4();
	redOffice.modelMatrix = glm::translate(redOffice.modelMatrix, glm::vec3(20.0f, 0.0f, -52.0f));
	redOffice.modelMatrix = glm::scale(redOffice.modelMatrix, glm::vec3(1.0f, 1.5f, 1.0f));
	redOffice.modelMatrix = glm::rotate(redOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(redOffice);

	//row 6
	purpleOffice.modelMatrix = glm::mat4();
	purpleOffice.modelMatrix = glm::translate(purpleOffice.modelMatrix, glm::vec3(-20.0f, 0.0f, -60.0f));
	purpleOffice.modelMatrix = glm::scale(purpleOffice.modelMatrix, glm::vec3(1.0f, 1.5f, 1.0f));
	purpleOffice.modelMatrix = glm::rotate(purpleOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(purpleOffice);

	blueOffice.modelMatrix = glm::mat4();
	blueOffice.modelMatrix = glm::translate(blueOffice.modelMatrix, glm::vec3(-10.0f, 0.0f, -60.0f));
	blueOffice.modelMatrix = glm::scale(blueOffice.modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	blueOffice.modelMatrix = glm::rotate(blueOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(blueOffice);

	greenOffice.modelMatrix = glm::mat4();
	greenOffice.modelMatrix = glm::translate(greenOffice.modelMatrix, glm::vec3(0.0f, 0.0f, -60.0f));
	greenOffice.modelMatrix = glm::scale(greenOffice.modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	greenOffice.modelMatrix = glm::rotate(greenOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(greenOffice);

	yellowOffice.modelMatrix = glm::mat4();
	yellowOffice.modelMatrix = glm::translate(yellowOffice.modelMatrix, glm::vec3(10.0f, 0.0f, -60.0f));
	yellowOffice.modelMatrix = glm::scale(yellowOffice.modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	yellowOffice.modelMatrix = glm::rotate(yellowOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(yellowOffice);

	orangeOffice.modelMatrix = glm::mat4();
	orangeOffice.modelMatrix = glm::translate(orangeOffice.modelMatrix, glm::vec3(20.0f, 0.0f, -60.0f));
	orangeOffice.modelMatrix = glm::scale(orangeOffice.modelMatrix, glm::vec3(1.0f, 1.5f, 1.0f));
	orangeOffice.modelMatrix = glm::rotate(orangeOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(orangeOffice);

	//row 7
	redOffice.modelMatrix = glm::mat4();
	redOffice.modelMatrix = glm::translate(redOffice.modelMatrix, glm::vec3(-20.0f, 0.0f, -68.0f));
	redOffice.modelMatrix = glm::scale(redOffice.modelMatrix, glm::vec3(1.0f, 1.5f, 1.0f));
	redOffice.modelMatrix = glm::rotate(redOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(redOffice);

	orangeOffice.modelMatrix = glm::mat4();
	orangeOffice.modelMatrix = glm::translate(orangeOffice.modelMatrix, glm::vec3(-10.0f, 0.0f, -68.0f));
	orangeOffice.modelMatrix = glm::scale(orangeOffice.modelMatrix, glm::vec3(1.0f, 1.5f, 1.0f));
	orangeOffice.modelMatrix = glm::rotate(orangeOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(orangeOffice);

	yellowOffice.modelMatrix = glm::mat4();
	yellowOffice.modelMatrix = glm::translate(yellowOffice.modelMatrix, glm::vec3(0.0f, 0.0f, -68.0f));
	yellowOffice.modelMatrix = glm::scale(yellowOffice.modelMatrix, glm::vec3(1.0f, 1.5f, 1.0f));
	yellowOffice.modelMatrix = glm::rotate(yellowOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(yellowOffice);

	greenOffice.modelMatrix = glm::mat4();
	greenOffice.modelMatrix = glm::translate(greenOffice.modelMatrix, glm::vec3(10.0f, 0.0f, -68.0f));
	greenOffice.modelMatrix = glm::scale(greenOffice.modelMatrix, glm::vec3(1.0f, 1.5f, 1.0f));
	greenOffice.modelMatrix = glm::rotate(greenOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(greenOffice);

	blueOffice.modelMatrix = glm::mat4();
	blueOffice.modelMatrix = glm::translate(blueOffice.modelMatrix, glm::vec3(20.0f, 0.0f, -68.0f));
	blueOffice.modelMatrix = glm::scale(blueOffice.modelMatrix, glm::vec3(1.0f, 1.5f, 1.0f));
	blueOffice.modelMatrix = glm::rotate(blueOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(blueOffice);
}

//make the alternate scene
void makeAlternateScene() {
	cube = parseObj("models/cube.obj", 1.0f, 1.0f, 0.0f);
	ModelCollection orangeOffice = parseModelCollection("models/office/main.obj", 0.5f, 0.1f, 0.0f, "models/office/occluder2.obj", "models/office/box2.obj", "models/office/marker.obj");
	ModelCollection greenOffice = parseModelCollection("models/office/main.obj", 0.1f, 0.4f, 0.0f, "models/office/occluder2.obj", "models/office/box2.obj", "models/office/marker.obj");
	ModelCollection purpleOffice = parseModelCollection("models/office/main.obj", 0.4f, 0.0f, 0.7f, "models/office/occluder2.obj", "models/office/box2.obj", "models/office/marker.obj");
	ModelCollection blueOffice = parseModelCollection("models/office/main.obj", 0.0f, 0.0f, 0.4f, "models/office/occluder2.obj", "models/office/box2.obj", "models/office/marker.obj");
	ModelCollection redOffice = parseModelCollection("models/office/main.obj", 0.4f, 0.0f, 0.0f, "models/office/occluder2.obj", "models/office/box2.obj", "models/office/marker.obj");
	ModelCollection brownOffice = parseModelCollection("models/office/main.obj", 0.45f, 0.18f, 0.07f, "models/office/occluder2.obj", "models/office/box2.obj", "models/office/marker.obj");
	ModelCollection yellowOffice = parseModelCollection("models/office/main.obj", 0.3, 0.3f, 0.07f, "models/office/occluder2.obj", "models/office/box2.obj", "models/office/marker.obj");
	ModelCollection ground = parseModelCollection("models/cube.obj", 0.1f, 0.1f, 0.1f, "models/cube.obj", "models/cube.obj", "models/cube.obj");

	ground.modelMatrix = glm::mat4();
	ground.modelMatrix = glm::translate(ground.modelMatrix, glm::vec3(0.0f, -3.5f, 0.0f));
	ground.modelMatrix = glm::scale(ground.modelMatrix, glm::vec3(200.0f, 0.2f, 200.0f));
	ground.modelMatrix = glm::rotate(ground.modelMatrix, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(ground);

	//Big building
	brownOffice.modelMatrix = glm::mat4();
	brownOffice.modelMatrix = glm::translate(brownOffice.modelMatrix, glm::vec3(0.0f, 2.0f, -10.0f));
	brownOffice.modelMatrix = glm::scale(brownOffice.modelMatrix, glm::vec3(6.0f, 4.0f, 2.0f));
	brownOffice.modelMatrix = glm::rotate(brownOffice.modelMatrix, 0.0f, glm::vec3(0.0f, 2.0f, 0.0f));
	sceneModels.push_back(brownOffice);

	//Left row 1
	blueOffice.modelMatrix = glm::mat4();
	blueOffice.modelMatrix = glm::translate(blueOffice.modelMatrix, glm::vec3(-40.0f, 0.0f, -50.0f));
	blueOffice.modelMatrix = glm::scale(blueOffice.modelMatrix, glm::vec3(1.0f, 2.3f, 1.0f));
	blueOffice.modelMatrix = glm::rotate(blueOffice.modelMatrix, (GLfloat) PI / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(blueOffice);

	yellowOffice.modelMatrix = glm::mat4();
	yellowOffice.modelMatrix = glm::translate(yellowOffice.modelMatrix, glm::vec3(-35.0f, 0.0f, -50.0f));
	yellowOffice.modelMatrix = glm::scale(yellowOffice.modelMatrix, glm::vec3(-1.0f, 2.1f, 1.0f));
	yellowOffice.modelMatrix = glm::rotate(yellowOffice.modelMatrix, (GLfloat) PI / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(yellowOffice);

	blueOffice.modelMatrix = glm::mat4();
	blueOffice.modelMatrix = glm::translate(blueOffice.modelMatrix, glm::vec3(-28.0f, 0.0f, -50.0f));
	blueOffice.modelMatrix = glm::scale(blueOffice.modelMatrix, glm::vec3(-1.5f, 2.3f, 1.5f));
	blueOffice.modelMatrix = glm::rotate(blueOffice.modelMatrix, (GLfloat) PI / 4.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(blueOffice);

	//Left row 2
	yellowOffice.modelMatrix = glm::mat4();
	yellowOffice.modelMatrix = glm::translate(yellowOffice.modelMatrix, glm::vec3(-48.0f, 0.0f, -70.0f));
	yellowOffice.modelMatrix = glm::scale(yellowOffice.modelMatrix, glm::vec3(-1.0f, 2.4f, 1.0f));
	yellowOffice.modelMatrix = glm::rotate(yellowOffice.modelMatrix, (GLfloat) PI / 6.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(yellowOffice);

	greenOffice.modelMatrix = glm::mat4();
	greenOffice.modelMatrix = glm::translate(greenOffice.modelMatrix, glm::vec3(-36.0f, 0.0f, -70.0f));
	greenOffice.modelMatrix = glm::scale(greenOffice.modelMatrix, glm::vec3(-1.0f, 1.6f, 1.0f));
	greenOffice.modelMatrix = glm::rotate(greenOffice.modelMatrix, (GLfloat) -PI / 8.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(greenOffice);

	redOffice.modelMatrix = glm::mat4();
	redOffice.modelMatrix = glm::translate(redOffice.modelMatrix, glm::vec3(-20.0f, 0.0f, -70.0f));
	redOffice.modelMatrix = glm::scale(redOffice.modelMatrix, glm::vec3(-3.0f, 2.12f, -3.0f));
	redOffice.modelMatrix = glm::rotate(redOffice.modelMatrix, (GLfloat) PI / 3.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(redOffice);

	//Left row 3
	redOffice.modelMatrix = glm::mat4();
	redOffice.modelMatrix = glm::translate(redOffice.modelMatrix, glm::vec3(-45.0f, 0.0f, -90.0f));
	redOffice.modelMatrix = glm::scale(redOffice.modelMatrix, glm::vec3(1.0f, 2.0f, -1.0f));
	redOffice.modelMatrix = glm::rotate(redOffice.modelMatrix, (GLfloat) -PI / 3.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(redOffice);

	purpleOffice.modelMatrix = glm::mat4();
	purpleOffice.modelMatrix = glm::translate(purpleOffice.modelMatrix, glm::vec3(-27.0f, 0.0f, -90.0f));
	purpleOffice.modelMatrix = glm::scale(purpleOffice.modelMatrix, glm::vec3(2.0f, 2.6f, -2.0f));
	purpleOffice.modelMatrix = glm::rotate(purpleOffice.modelMatrix, (GLfloat) PI / 4.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(purpleOffice);

	orangeOffice.modelMatrix = glm::mat4();
	orangeOffice.modelMatrix = glm::translate(orangeOffice.modelMatrix, glm::vec3(-20.0f, 0.0f, -90.0f));
	orangeOffice.modelMatrix = glm::scale(orangeOffice.modelMatrix, glm::vec3(1.6f, 2.6f, -1.6f));
	orangeOffice.modelMatrix = glm::rotate(orangeOffice.modelMatrix, (GLfloat) -PI / 6.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(orangeOffice);

	//Right row 1
	greenOffice.modelMatrix = glm::mat4();
	greenOffice.modelMatrix = glm::translate(greenOffice.modelMatrix, glm::vec3(0.0f, 0.0f, -30.0f));
	greenOffice.modelMatrix = glm::scale(greenOffice.modelMatrix, glm::vec3(1.0f, 2.0f, 1.0f));
	greenOffice.modelMatrix = glm::rotate(greenOffice.modelMatrix, (GLfloat) PI / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(greenOffice);

	purpleOffice.modelMatrix = glm::mat4();
	purpleOffice.modelMatrix = glm::translate(purpleOffice.modelMatrix, glm::vec3(6.0f, 0.0f, -30.0f));
	purpleOffice.modelMatrix = glm::scale(purpleOffice.modelMatrix, glm::vec3(-1.0f, 2.1f, 1.0f));
	purpleOffice.modelMatrix = glm::rotate(purpleOffice.modelMatrix, (GLfloat) -PI / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(purpleOffice);

	orangeOffice.modelMatrix = glm::mat4();
	orangeOffice.modelMatrix = glm::translate(orangeOffice.modelMatrix, glm::vec3(14.0f, 0.0f, -30.0f));
	orangeOffice.modelMatrix = glm::scale(orangeOffice.modelMatrix, glm::vec3(-2.6f, 2.3f, 2.6f));
	orangeOffice.modelMatrix = glm::rotate(orangeOffice.modelMatrix, (GLfloat) PI / 6.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(orangeOffice);

	//Right row 2
	yellowOffice.modelMatrix = glm::mat4();
	yellowOffice.modelMatrix = glm::translate(yellowOffice.modelMatrix, glm::vec3(-2.0f, 0.0f, -45.0f));
	yellowOffice.modelMatrix = glm::scale(yellowOffice.modelMatrix, glm::vec3(1.0f, 2.0f, 1.0f));
	yellowOffice.modelMatrix = glm::rotate(yellowOffice.modelMatrix, -(GLfloat) PI / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(yellowOffice);

	blueOffice.modelMatrix = glm::mat4();
	blueOffice.modelMatrix = glm::translate(blueOffice.modelMatrix, glm::vec3(6.0f, 0.0f, -52.0f));
	blueOffice.modelMatrix = glm::scale(blueOffice.modelMatrix, glm::vec3(-1.0f, 2.1f, 1.0f));
	blueOffice.modelMatrix = glm::rotate(blueOffice.modelMatrix, (GLfloat) 2.0f * (GLfloat) PI, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(blueOffice);

	redOffice.modelMatrix = glm::mat4();
	redOffice.modelMatrix = glm::translate(redOffice.modelMatrix, glm::vec3(14.0f, 0.0f, -45.0f));
	redOffice.modelMatrix = glm::scale(redOffice.modelMatrix, glm::vec3(-2.6f, 2.3f, 2.6f));
	redOffice.modelMatrix = glm::rotate(redOffice.modelMatrix, (GLfloat) PI / 6.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(redOffice);

	purpleOffice.modelMatrix = glm::mat4();
	purpleOffice.modelMatrix = glm::translate(purpleOffice.modelMatrix, glm::vec3(0.0f, 0.0f, -70.0f));
	purpleOffice.modelMatrix = glm::scale(purpleOffice.modelMatrix, glm::vec3(-6.0f, 2.0f, 6.0f));
	purpleOffice.modelMatrix = glm::rotate(purpleOffice.modelMatrix, (GLfloat) 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	sceneModels.push_back(purpleOffice);
}

//Compare models -- used to sort objects in the scene
bool modelComparator(ModelCollection& m1, ModelCollection& m2) {
	return distSquaredToCamera(m1) < distSquaredToCamera(m2);
}

//render the current scene
void renderScene() {
	//update the light position (make the light move in a circle around the scene)
	double seconds = clock() / (double)CLOCKS_PER_SEC;
	lightPos = glm::vec3(50.0f * cos(seconds * 2.0), 8.0f, 50.0f * sin(seconds * 2.0) - 40.0f);

	glClearColor(0.3f, 0.3f, 0.3f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setLights();


	//sort the scene objects
	std::sort(sceneModels.begin(), sceneModels.end(), modelComparator);

	size_t drawn = 0; //objects drawn this frame
	dBuffer.reset();
	for (auto it = sceneModels.begin(); it != sceneModels.end(); it++) {
		if (shouldDraw(*it)) {
			drawModelCollection(*it);
			drawn++;
		}
	}


	//record stats: frame, scene models, models drawn
	if (recordStats) {
		statsFile << currentFrame << " " << sceneModels.size() << " " << drawn << std::endl;
	}

	//draw cube at light source
	model = glm::mat4();
	model = glm::translate(model, lightPos + glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
	mvp = project * view * model;
	drawModel(cube);
}


bool firstFrame = true; //use this for caching
double distSquaredToCamera(ModelCollection &m) {
	if (m.lastSorted == currentFrame) { //if already cached, return cached result
		return m.dist2ToCamera;
	}

	//otherwise compute result
	glm::vec4 transformed = view * m.modelMatrix * glm::vec4(m.boxCenter, 1.0f);
	glm::vec3 p = glm::vec3(transformed / transformed.a);
	m.dist2ToCamera = ((double)p.x * (double)p.x) + ((double)p.y * (double)p.y) + ((double)p.z * (double)p.z);

	return m.dist2ToCamera;
}
