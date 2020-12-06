#pragma once

/*		draw file

	this file is responsible for rendering scenes

	this header defines some parameters for rendering (screen dimensions)

*/

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "models.h"
#include <fstream>

#define FPS 60 //Max frames per second

//screen dimensions -- should probably match the depth buffer dimensions but this isn't necessary
#define SCREEN_WIDTH 1440
#define SCREEN_HEIGHT 1024

//distances of near and far planes for perspective projection
#define NEAR 0.01f
#define FAR 200.0f

enum sceneIDEnum { SCENE_DEFAULT = 0, SCENE_ALTERNATE = 1 }; //which scene to use
extern int sceneID;

extern GLuint programID; //GL program ID of shader program
extern GLFWwindow* window; //GLFW window of main window

//uniform locations in shader program
extern GLuint u_LightPos, u_LightColor, u_AmbientLight, u_MvpMat, u_ModelMat, u_NormalMat;

//light parameters -- these are updated in the scene's render function
extern glm::vec3 lightPos;
extern glm::vec3 lightColor;
extern glm::vec3 ambientLight;

extern glm::mat4 mvp; //mvp matrix
extern glm::mat4 model; //current model matrix
extern glm::mat4 normal; //normal matrix -- set by drawModel
extern glm::mat4 view; //view matrix
extern glm::mat4 project; //projection matrix (perspective)

extern std::fstream statsFile; //file to record stats to
extern bool recordStats; //should stats be recorded?

//models in the current scene
extern std::vector<ModelCollection> sceneModels;

//send matrices to GL program
void setMatrices();

//send light parameters to GL program
void setLights();

//render one model in GL
void drawModel(Model& m);

//render a model from an object in GL according to the current rendering mode
void drawModelCollection(ModelCollection& m);

//load models for default scene
void makeAlternateScene();

//load models for alternate scene
void makeDefaultScene();

//comparator for models based on depth -- use to sort scene by depth
bool modelComparator(ModelCollection& m1, ModelCollection& m2);

//render scene
void renderScene();

//distance of an object to the camera based on current view of the scene
double distSquaredToCamera(ModelCollection& m);
