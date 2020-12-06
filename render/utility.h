#pragma once

/*		utility file

	this file is responsible for miscellaneous functions
*/

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <iostream>
#include <vector>

#define PI 3.141592654 //definition of pi (mainly for rotations)

//print something to stdout
#define PRINT(x) \
	std::cout << x << std::endl

//1 if x is positive, 0 otherwise
#define SIGN(x) \
	(x > 0)

//change colour of stdout (for debugging)
#define WHITE() std::cout << "\033[1;37m"
#define RED() std::cout << "\033[1;31m"

//triangle comparator -- for sorting triangles according to their y coordinates
bool triComp(glm::vec2* p1, glm::vec2* p2);

//Print vectors to stdout
void printVec(glm::vec2& v);
void printVec(glm::vec3& v);
void printVec(glm::vec4& v);

//Print pair to stdout
template<class T>
void printPair(std::pair<T, T>& p);

//print bits of a 32 bit register -- least significant bit is on the far right
void printBits(uint32_t v);

//operator to print vector to ostream
template <class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v);

/*
	split a string by the delimitter and return a vector of strings

	example:
	split("abc, abc", ",") returns a vector containing "abc" and " abc"
	
	empty strings aren't produced in the case of consecutive delimiters, i.e.
	split("abc,,,def", ",") returns a vector containing "abc" and "def"
*/
std::vector<std::string> split(std::string str, std::string del);

/*	given points of a triangle, add epsilons to the y coordinates until no points have the same
	y coordinate

	if this causes lockups it might need to be modified
*/
void fixTriangle(glm::vec2& t1, glm::vec2& t2, glm::vec2& t3);

/*
	convert vector from NDC space to pixel space

	pixel space dimensions used are those of the depth buffer
*/
void convertVec(glm::vec2& v);

//wait for this amount of time
void fpsWait(double seconds);
