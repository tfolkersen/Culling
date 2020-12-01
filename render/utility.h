#pragma once

#include <glm/glm.hpp>
#include <vector>

bool triComp(glm::vec2* p1, glm::vec2* p2);

void printVec(glm::vec2& v);

template<class T>
void printPair(std::pair<T, T>& p);

void printBits(uint32_t v);

template <class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v);

std::vector<std::string> split(std::string str, std::string del);

void fixTriangle(glm::vec2& t1, glm::vec2& t2, glm::vec2& t3);;;;

