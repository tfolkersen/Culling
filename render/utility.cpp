#include "utility.h"
#include "cull.h"
#include <iostream>

//triangle comparator -- for sorting triangles according to their y coordinates
//sort by decreasing y coordinate
bool triComp(glm::vec2* p1, glm::vec2* p2) {
	return p1->y > p2->y;
}

//Print vec2 to stdout
void printVec(glm::vec2& v) {
	std::cout << "[" << v.x << " " << v.y << "]";
}

//Print vec3 to stdout
void printVec(glm::vec3& v) {
	std::cout << "[" << v.x << " " << v.y << " " << v.z << "]";
}

//Print vec4 to stdout
void printVec(glm::vec4& v) {
	std::cout << "[" << v.x << " " << v.y <<  " " << v.z << " " << v.a << "]";
}

//Print pair to stdout
template<class T, class S>
void printPair(std::pair<T, S>& p) {
	std::cout << "<" << p.first << " " << p.second << ">";
}

//print bits of a 32 bit register -- least significant bit is on the far right
void printBits(uint32_t v) {
	for (int i = 0; i < 32; i++) {
		std::cout << ((v & (1 << (31 - i))) != 0);
	}
}

//operator to print vector to ostream
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

//split a string by a delimiter -- see header file for details
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

//add epsilons to y coordinates until no points have the same y coordinate -- see header
void fixTriangle(glm::vec2& t1, glm::vec2& t2, glm::vec2& t3) {
	//TODO this might cause lockups but in practice it doesn't seem to
	GLfloat epsilon = 0.001;
	bool problem = true;
	int iters = 0;

	while (problem) {
		iters++;
		if (iters > 5) {
			std::cout << "fixTriangle in utility.cpp used " << iters << " iterations -- this should be fixed" << std::endl;
		}
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
			t3.y += epsilon * 0.8f;
		}
	}
}

/*
	convert vector from NDC space to pixel space (pixel space of depth buffer)

	this allows depth buffers of dimensions different from the screen resolution
	to be used
*/
void convertVec(glm::vec2 &v) {
	int W = BUFFER_WIDTH - 1;
	int H = BUFFER_HEIGHT - 1;
	GLfloat Wf = (GLfloat)W;
	GLfloat Hf = (GLfloat)H;
	v.x = (Wf / 2.0) * v.x + (Wf / 2.0);
	v.y = -(Hf / 2.0) * v.y + (Hf / 2.0);
}

//wait for this amount of time -- busy wait because windows function didn't work for some reason
void fpsWait(double seconds) {
	if (seconds < 0) {
		return;
	}
	clock_t start = clock();
	while ((clock() - start) / (double) CLOCKS_PER_SEC < seconds) {
	}
}
