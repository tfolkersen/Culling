#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <iostream>
#include <vector>



#define PI 3.141592654
#define PRINT(x) \
	std::cout << x << std::endl
#define SIGN(x) \
	(x > 0)

#define WHITE() std::cout << "\033[1;37m"
#define RED() std::cout << "\033[1;31m"


bool triComp(glm::vec2* p1, glm::vec2* p2);

void printVec(glm::vec2& v);
void printVec(glm::vec3& v);
void printVec(glm::vec4& v);

template<class T>
void printPair(std::pair<T, T>& p);

void printBits(uint32_t v);

template <class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v);

std::vector<std::string> split(std::string str, std::string del);

void fixTriangle(glm::vec2& t1, glm::vec2& t2, glm::vec2& t3);;;;


void convertVec(glm::vec2& v);

void fpsWait(double seconds);
