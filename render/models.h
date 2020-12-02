#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>



struct Model {
	GLuint varr;
	GLuint posBuff;
	GLuint colBuff;
	GLuint normBuff;
	int nVerts;

	Model();

	Model(const Model& m);
};

struct ModelCollection {
	Model main;

	Model occluder;
	std::vector<GLfloat> occluderData;

	Model box;
	std::vector<GLfloat> boxData;
	glm::vec3 boxCenter;

	Model marker;

	glm::mat4 modelMatrix;

	ModelCollection();

	ModelCollection(const ModelCollection& m);
};

extern Model box;
extern Model plant;
extern Model cube;
extern ModelCollection office;


Model parseObj(std::string fileName, GLfloat r, GLfloat g, GLfloat b, std::vector<GLfloat>& posData = std::vector<GLfloat>());

ModelCollection parseModelCollection(std::string mainFileName, GLfloat r, GLfloat g, GLfloat b, std::string occluderFileName, std::string boxFileName, std::string markerFileName);

glm::vec3 modelDataCenter(const std::vector<GLfloat>& data);
