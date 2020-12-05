#pragma once


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "models.h"
#include <fstream>

#define FPS 60 //Max frames per second

#define SCREEN_WIDTH 1440
#define SCREEN_HEIGHT 1024

#define NEAR 0.01f


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

extern std::fstream statsFile;
extern bool recordStats;

extern std::vector<ModelCollection> sceneModels;



void setMatrices();

void setLights();

void drawModel(Model& m);

void drawModelCollection(ModelCollection& m);

void makeModels();

void render();


void makeScene2();
void makeScene3();
bool modelComparator(const ModelCollection& m1, const ModelCollection& m2);

void render2();

double distSquaredToCamera(const ModelCollection& m);
