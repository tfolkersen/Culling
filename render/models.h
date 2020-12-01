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

struct Model3 {
	Model main;
	Model occluder;
	Model box;

	std::vector<GLfloat> occluderData;
	std::vector<GLfloat> boxData;
	glm::mat4 modelMatrix;

	Model3();

	Model3(const Model3& m);
};

extern Model box;
extern Model plant;
extern Model cube;
extern Model3 office;


Model parseObj(std::string fileName, GLfloat r, GLfloat g, GLfloat b, std::vector<GLfloat>& posData = std::vector<GLfloat>());

Model3 parseModel3(std::string mainFileName, GLfloat r, GLfloat g, GLfloat b, std::string occluderFileName, std::string boxFileName);


