#include <GL/glew.h>
#include <iostream>
#include "utility.h"

bool triComp(glm::vec2* p1, glm::vec2* p2) {
	return p1->y > p2->y;
}

void printVec(glm::vec2& v) {
	std::cout << "[" << v.x << " " << v.y << "]";
}

template<class T>
void printPair(std::pair<T, T>& p) {
	std::cout << "<" << p.first << " " << p.second << ">";
}

void printBits(uint32_t v) {
	for (int i = 0; i < 32; i++) {
		//std::cout << ((v & (1 << (31 - i))) != 0) << " ";
		std::cout << ((v & (1 << (31 - i))) != 0);
	}
}

template <class T>
std::ostream& operator<<(std::ostream &os, const std::vector<T> &v) {
	os << "<";
	for (auto it = v.begin();;) {
		os << *it;
		if (++it == v.end()) {
			break;
		}
		os << " ";
	}
	os << ">";
	return os;
}

std::vector<std::string> split(std::string str, std::string del) {
	std::vector<std::string> list;

	size_t i = str.find(del);
	while (i != -1 && str.length() != 0) {
		std::string left = str.substr(0, i);
		if (left.length() != 0) {
			list.push_back(left);
		}
		if (i == str.length() - 1) {
			str = "";
			break;
		}
		str = str.substr(i + 1);
		i = str.find(del);
	}
	if (str.length() != 0) {
		list.push_back(str);
	}

	return list;
}

void fixTriangle(glm::vec2 &t1, glm::vec2 &t2, glm::vec2 &t3) {
	GLfloat epsilon = 0.0001;
	bool problem = true;

	while (problem) {
		problem = false;
		if (t1.y == t2.y) {
			problem = true;
			t1.y += epsilon;
		}

		if (t1.y == t3.y) {
			problem = true;
			t3.y += epsilon * 0.9f;
		}

		if (t2.y == t3.y) {
			problem = true;
			t2.y += epsilon * 0.95f;
		}
	}
}
