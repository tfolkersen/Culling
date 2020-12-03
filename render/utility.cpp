#include "utility.h"
#include "cull.h"
#include <iostream>

bool triComp(glm::vec2* p1, glm::vec2* p2) {
	return p1->y > p2->y;
}

void printVec(glm::vec2& v) {
	std::cout << "[" << v.x << " " << v.y << "]";
}

void printVec(glm::vec3& v) {
	std::cout << "[" << v.x << " " << v.y << " " << v.z << "]";
}

void printVec(glm::vec4& v) {
	std::cout << "[" << v.x << " " << v.y <<  " " << v.z << " " << v.a << "]";
}

template<class T, class S>
void printPair(std::pair<T, S>& p) {
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
	//TODO this causes lockups
	GLfloat epsilon = 0.001;
	bool problem = true;
	int iters = 0;
	return;

	while (problem) {
		iters++;
		if (iters > 1) {
			std::cout << "FixTriangle: " << iters << std::endl;

			std::cout << epsilon << " " << epsilon * 0.9 << " " << epsilon * 0.95 << std::endl;

		}
		problem = false;
		if (t1.y == t2.y || t1.y == t3.y || t2.y == t3.y) {
			problem = true;
			t1.y += epsilon;
			t2.y += epsilon * 0.9;
			t3.y += epsilon * 0.95;
		}

	}
}

void convertVec(glm::vec2 &v) {
	int W = BUFFER_WIDTH - 1;
	int H = BUFFER_HEIGHT - 1;
	GLfloat Wf = (GLfloat)W;
	GLfloat Hf = (GLfloat)H;
	v.x = (Wf / 2.0) * v.x + (Wf / 2.0);
	v.y = -(Hf / 2.0) * v.y + (Hf / 2.0);
}

void fpsWait(double seconds) {
	if (seconds < 0) {
		return;
	}
	clock_t start = clock();
	while ((clock() - start) / (double) CLOCKS_PER_SEC < seconds) {
	}
}


