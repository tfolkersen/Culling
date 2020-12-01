#pragma once


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "models.h"

#define FPS 60 //Max frames per second

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768


extern GLuint programID;
extern GLFWwindow* window;

extern GLuint u_LightPos, u_LightColor, u_AmbientLight, u_MvpMat, u_ModelMat, u_NormalMat;

extern glm::vec3 lightPos;
extern glm::vec3 lightColor;
extern glm::vec3 ambientLight;

extern glm::mat4 mvp;
extern glm::mat4 model;
extern glm::mat4 normal;
extern glm::mat4 view;
extern glm::mat4 project;

extern std::vector<Model3> sceneModels;



void setMatrices();

void setLights();

void drawModel(Model& m);

void drawModel3(Model3& m);

void makeModels();

void render();


void makeScene2();
bool modelComparator(const Model3& m1, const Model3& m2);

void render2();

double distSquaredToCamera(const Model3& m);
