#pragma once

/*
		models file

		responsible for loading models from obj files and organizing
		collections of meshes for objects
*/

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>


//Model reprenting one mesh (one obj file)
struct Model {
	GLuint varr; //GL vertex array ID
	GLuint posBuff; //GL array_buffer id of position data
	GLuint colBuff; //GL array_buffer id of color data
	GLuint normBuff; //GL array_buffer ID of normal data
	int nVerts; //number of vertices in model

	Model();

	Model(const Model& m); //copy constructor
};

extern Model cube; //used to show where the light source is

/*		collection of meshes for one object

	contains models for each rendering mode
*/
struct ModelCollection {
	Model main; //main mesh to render in GL

	Model occluder; //occluder model to draw in GL
	std::vector<GLfloat> occluderData; //raw occluder data (for rendering into depth buffer)
	
	Model box; //bounding box mesh to render in GL
	std::vector<GLfloat> boxData; //raw box data (for depth test in depth buffer)
	glm::vec3 boxCenter; //center point of bounding box (used for sorting scene objects)

	Model marker; //marker model to render in GL

	glm::mat4 modelMatrix; //model matrix for this object


	uint64_t lastSorted; //frame that this object's center was last transformed on
	glm::vec3 transformedCenter; //center transformed for this frame
	double dist2ToCamera; //squared distance to camera this frame

	ModelCollection();

	ModelCollection(const ModelCollection& m); //copy constructor
};


/*		parse and obj file and return a model

	fileName -- name of obj file
	r, g, b are red, green, blue values to use for this model
	posData is an optional argument that gets filled with the raw data (for getting raw occluder mesh/bounding box data)
		this is also the buffer sent to the GL buffer
*/
Model parseObj(std::string fileName, GLfloat r, GLfloat g, GLfloat b, std::vector<GLfloat>& posData = std::vector<GLfloat>());

/*		parse multiple obj files and store the models in a ModelCollection represnting one scene object


	mainFileName -- file name of main mesh (the one actually seen in the scene)
	r, g, b -- red, green, and blue values to use for the main mesh
	occluderFileName -- file name of the occlusion mesh
	boxFileName -- file name of the bounding box of the main mesh
	markerFileName -- file name of the marker for the main mesh (this should be a very tall, thin pillar positioned somewhere near the center of the main mesh)


*/
ModelCollection parseModelCollection(std::string mainFileName, GLfloat r, GLfloat g, GLfloat b, std::string occluderFileName, std::string boxFileName, std::string markerFileName);

/*	given coordinate data for a model, return the center point
*/
glm::vec3 modelDataCenter(const std::vector<GLfloat>& data);
