#include <iostream>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <common/shader.hpp>
#include <ctime>
//#include <immintrin.h>

#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

#define KEY_FORWARD GLFW_KEY_W
#define KEY_LEFT GLFW_KEY_A
#define KEY_BACK GLFW_KEY_S
#define KEY_RIGHT GLFW_KEY_D
#define KEY_UP GLFW_KEY_SPACE
#define KEY_DOWN GLFW_KEY_LEFT_SHIFT
#define FPS 60

#define KEY_R_UP GLFW_KEY_UP
#define KEY_R_DOWN GLFW_KEY_DOWN
#define KEY_R_LEFT GLFW_KEY_LEFT
#define KEY_R_RIGHT GLFW_KEY_RIGHT

#define PI 3.141592654

GLfloat cameraSpeed = 0.05f;
GLfloat rotationSpeed = 0.025;

#define MOUSE_SENSITIVITY 0.001 / 1.6

enum replayEnum {CONTROL, RECORD, PLAY};
int replayMode = CONTROL;

std::string replayFileName = "replay.txt";
std::fstream replayFile;

uint64_t currentFrame = 0;
uint64_t nextReplayFrame = 0;
bool recordedFrameNumber = false;

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

void test();

void fpsWait(double seconds) {
	if (seconds < 0) {
		return;
	}
	clock_t start = clock();
	while ((clock() - start) / (double) CLOCKS_PER_SEC < seconds) {
	}
}

void printBits(uint32_t v) {
	for (int i = 0; i < 32; i++) {
		//std::cout << ((v & (1 << (31 - i))) != 0) << " ";
		std::cout << ((v & (1 << (31 - i))) != 0);
	}
}

//1024x768
//#define BUFFER_WIDTH 32*32
//#define BUFFER_HEIGHT 32*24

//1600x1056
//#define BUFFER_WIDTH 32*50
//#define BUFFER_HEIGHT 32*33

//640x480
//#define BUFFER_WIDTH 32*20
//#define BUFFER_HEIGHT 32*15

#define BUFFER_WIDTH 32
#define BUFFER_HEIGHT 32

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

struct Model {
	GLuint varr;
	GLuint posBuff;
	GLuint colBuff;
	GLuint normBuff;
	int nVerts;
};

Model box;
Model plant;
Model cube;


#define BLOCK_HEIGHT 8

//block is 32 width and 8 height
struct Block {
	uint32_t bits[BLOCK_HEIGHT];

	void reset() {
		for (int i = 0; i < BLOCK_HEIGHT; i++) {
			bits[i] = 0;
		}
	}
};

struct DepthBuffer {
	Block* arr;
	uint32_t blockCount;

	uint32_t widthB;
	uint32_t heightB;

	DepthBuffer() {
		widthB = (BUFFER_WIDTH / 32);
		heightB = (BUFFER_HEIGHT / BLOCK_HEIGHT);

		blockCount = widthB * heightB;
		std::cout << "DepthBuffer making " << blockCount << " blocks" << std::endl;
		arr = new Block[blockCount];
	}

	~DepthBuffer() {
		delete[] arr;
	}

	void reset() {
		for (int i = 0; i < heightB; i++) {
			for (int j = 0; j < widthB; j++) {
				Block& b = getBlock(j, i);
				b.reset();
			}
		}
	}

	Block& getBlock(int x, int y) {
		int index = y * widthB + x;
		return arr[index];
	}

	void print() {
		for (int i = 0; i < heightB; i++) {
			for (int j = 0; j < BLOCK_HEIGHT; j++) {
				for (int k = 0; k < widthB; k++) {
					Block& b = getBlock(k, i);
					printBits(b.bits[j]);
				}
				std::cout << std::endl;
			}
		}
	}
};

DepthBuffer dBuffer;

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

Model parseObj(std::string fileName, GLfloat r, GLfloat g, GLfloat b) {
	std::string line;
	std::vector<std::string> tokens;
	std::vector<std::vector<std::string>> subtokens;

	std::vector<GLfloat> vertices;
	std::vector<GLfloat> normals;

	std::vector<GLfloat> posData;
	std::vector<GLfloat> normalData;
	std::vector<GLfloat> colorData;

	std::fstream f(fileName);
	while (std::getline(f, line)) {
		tokens = split(line, " ");
		if (tokens.empty()) {
			continue;
		}

		if (tokens[0] == "v") {
			GLfloat x = std::stof(tokens[1]);
			GLfloat y = std::stof(tokens[2]);
			GLfloat z = std::stof(tokens[3]);
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
		} else if (tokens[0] == "vn") {
			GLfloat x = std::stof(tokens[1]);
			GLfloat y = std::stof(tokens[2]);
			GLfloat z = std::stof(tokens[3]);
			normals.push_back(x);
			normals.push_back(y);
			normals.push_back(z);
		}
	}
	f.close();


	f = std::fstream(fileName);
	while (std::getline(f, line)) {
		tokens = split(line, " ");
		if (tokens.empty()) {
			continue;
		}

		if (tokens[0] == "f") {
			subtokens.clear();

			for (size_t j = 1; j < tokens.size(); j++) {
				subtokens.push_back(split(tokens[j], "/"));
			}

			for (size_t j = 0; j < subtokens.size() - 2; j++) {
				for (size_t k = 0; k < 3; k++) {
					int vi;
					int vni;

					if (k == 0) {
						vi = std::stoi(subtokens[0][0]) - 1;
						vni = std::stoi(subtokens[0][2]) - 1;
					} else {
						vi = std::stoi(subtokens[j + k][0]) - 1;
						vni = std::stoi(subtokens[j + k][2]) - 1;
					}

					posData.push_back(vertices[vi * 3 + 0]);
					posData.push_back(vertices[vi * 3 + 1]);
					posData.push_back(vertices[vi * 3 + 2]);
					normalData.push_back(normals[vni * 3 + 0]);
					normalData.push_back(normals[vni * 3 + 1]);
					normalData.push_back(normals[vni * 3 + 2]);
					colorData.push_back(r);
					colorData.push_back(g);
					colorData.push_back(b);
				}
			}
		}
	}
	f.close();

	Model m;
	glGenVertexArrays(1, &m.varr);
	glBindVertexArray(m.varr);

	glGenBuffers(1, &m.posBuff);
	glBindBuffer(GL_ARRAY_BUFFER, m.posBuff);
	glBufferData(GL_ARRAY_BUFFER, posData.size() * sizeof(GLfloat), posData.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &m.colBuff);
	glBindBuffer(GL_ARRAY_BUFFER, m.colBuff);
	glBufferData(GL_ARRAY_BUFFER, colorData.size() * sizeof(GLfloat), colorData.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &m.normBuff);
	glBindBuffer(GL_ARRAY_BUFFER, m.normBuff);
	glBufferData(GL_ARRAY_BUFFER, normalData.size() * sizeof(GLfloat), normalData.data(), GL_STATIC_DRAW);

	m.nVerts = posData.size() / 3;
	return m;

}


std::pair<int, int> convert(const glm::vec2 &v) {
	int W = BUFFER_WIDTH - 1;
	int H = BUFFER_HEIGHT - 1;
	GLfloat Wf = (GLfloat)W;
	GLfloat Hf = (GLfloat)H;
	int x = round((Wf / 2.0) * v.x + (Wf / 2.0));
	int y = round(-(Hf / 2.0) * v.y + (Hf / 2.0));

	//int x = (Wf / 2.0) * v.x + (Wf / 2.0);
	//int y = -(Hf / 2.0) * v.y + (Hf / 2.0);

	return std::pair<int, int>(x, y);
}

void convertVec(glm::vec2 &v) {
	int W = BUFFER_WIDTH - 1;
	int H = BUFFER_HEIGHT - 1;
	GLfloat Wf = (GLfloat)W;
	GLfloat Hf = (GLfloat)H;
	v.x = (Wf / 2.0) * v.x + (Wf / 2.0);
	v.y = -(Hf / 2.0) * v.y + (Hf / 2.0);
}

uint32_t line(uint32_t e0, uint32_t e1, uint32_t e2, uint32_t o0, uint32_t o1, uint32_t o2) {
	uint32_t m0 = ~0;
	m0 = e0 >= 32 ? 0 : (m0 >> e0);
	m0 ^= o0;

	uint32_t m1 = ~0;
	m1 = e1 >= 32 ? 0 : (m1 >> e1);
	m1 ^= o1;

	uint32_t m2 = ~0;
	m2 = e2 >= 32 ? 0 : (m2 >> e2);
	m2 ^= o2;


	return m0 & m1 & m2;
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

/*
void rasterizeSilentAVX(glm::vec2 t1, glm::vec2 t2, glm::vec2 t3) {
	fixTriangle(t1, t2, t3);

	GLfloat cx = (t1.x + t2.x + t3.x) / 3.0f;
	GLfloat cy = (t1.y + t2.y + t3.y) / 3.0f;
	glm::vec2 center(cx, cy);

	std::vector<glm::vec2*> ps;
	ps.push_back(&t1);
	ps.push_back(&t2);
	ps.push_back(&t3);
	std::sort(ps.begin(), ps.end(), triComp);

	glm::vec2& p1 = *ps[0];
	glm::vec2& p2 = *ps[1];
	glm::vec2& p3 = *ps[2];

	glm::vec2 l1 = p2 - p1;
	glm::vec2 l2 = p3 - p1;
	glm::vec2 l3 = p3 - p2;

	glm::vec2 n1(l1.y, -l1.x);
	glm::vec2 n2(l2.y, -l2.x);
	glm::vec2 n3(l3.y, -l3.x);

	//true if left is outside
	bool o1 = glm::dot(n1, (center - p1)) < 0;
	bool o2 = glm::dot(n2, (center - p1)) < 0;
	bool o3 = glm::dot(n3, (center - p2)) < 0;
	uint32_t mask1 = o1 ? 0 : ~0;
	uint32_t mask2 = o2 ? 0 : ~0;
	uint32_t mask3 = o3 ? 0 : ~0;

//	GLfloat s1 = l1.x / l1.y;
//	GLfloat s2 = l2.x / l2.y;
//	GLfloat s3 = l3.x / l3.y;
//
//	GLfloat slopeFactor = (GLfloat)HEIGHT / 2;
//	s1 /= slopeFactor;
//	s2 /= slopeFactor;
//	s3 /= slopeFactor;

	glm::vec2 f1 = p1 + ((1.0f - p1.y) / l1.y) * l1;
	glm::vec2 f2 = p1 + ((1.0f - p1.y) / l2.y) * l2;
	glm::vec2 f3 = p2 + ((1.0f - p2.y) / l3.y) * l3;

	convertVec(f1);
	convertVec(f2);
	convertVec(f3);
	convertVec(p1);
	convertVec(p2);
	convertVec(p3);
	l1 = p2 - p1;
	l2 = p3 - p1;
	l3 = p3 - p2;
	GLfloat s1 = l1.x / l1.y;
	GLfloat s2 = l2.x / l2.y;
	GLfloat s3 = l3.x / l3.y;

	//std::cout << "Slope: " << s1 << " " << s2 << " " << s3 << std::endl;
	GLfloat minY = std::min(p1.y, std::min(p2.y, p3.y));
	GLfloat maxY = std::max(p1.y, std::max(p2.y, p3.y));
	GLfloat minX = std::min(p1.x, std::min(p2.x, p3.x));
	GLfloat maxX = std::max(p1.x, std::max(p2.x, p3.x));

	int iStart = std::max(((int)minY) / BLOCK_HEIGHT, 0);
	int iEnd = std::min(((int)maxY) / BLOCK_HEIGHT, (int) dBuffer.heightB - 1);

	int jStart = std::max(((int)minX) / 32, 0);
	int jEnd = std::min(((int)maxX) / 32, (int) dBuffer.widthB - 1);
	
	__m256 v_f1x = _mm256_set_ps(f1.x, f1.x, f1.x, f1.x, f1.x, f1.x, f1.x, f1.x);
	__m256 v_f2x = _mm256_set_ps(f2.x, f2.x, f2.x, f2.x, f2.x, f2.x, f2.x, f2.x);
	__m256 v_f3x = _mm256_set_ps(f3.x, f3.x, f3.x, f3.x, f3.x, f3.x, f3.x, f3.x);

	__m256 v_s1 = _mm256_set_ps(s1, s1, s1, s1, s1, s1, s1, s1);
	__m256 v_s2 = _mm256_set_ps(s2, s2, s2, s2, s2, s2, s2, s2);
	__m256 v_s3 = _mm256_set_ps(s3, s3, s3, s3, s3, s3, s3, s3);

	__m256 v_scanOffset = _mm256_set_ps(7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f);

	GLfloat e1f[BLOCK_HEIGHT];
	GLfloat e2f[BLOCK_HEIGHT];
	GLfloat e3f[BLOCK_HEIGHT];

	for (int i = iStart; i <= iEnd ; i++) {
		GLfloat scanBase = (GLfloat) i * BLOCK_HEIGHT;

		__m256 v_scanBase = _mm256_set_ps(scanBase, scanBase, scanBase, scanBase, scanBase, scanBase, scanBase, scanBase);
		__m256 v_scanLine = _mm256_add_ps(v_scanBase, v_scanOffset);

		__m256 v_e1f = v_f1x;
		__m256 v_mul1 = _mm256_mul_ps(v_scanLine, v_s1);
		v_e1f = _mm256_add_ps(v_e1f, v_mul1);
		_mm256_storeu_ps(e1f, v_e1f);


		__m256 v_e2f = v_f2x;
		__m256 v_mul2 = _mm256_mul_ps(v_scanLine, v_s2);
		v_e2f = _mm256_add_ps(v_e2f, v_mul2);
		_mm256_storeu_ps(e2f, v_e2f);

		__m256 v_e3f = v_f3x;
		__m256 v_mul3 = _mm256_mul_ps(v_scanLine, v_s3);
		v_e3f = _mm256_add_ps(v_e3f, v_mul3);
		_mm256_storeu_ps(e3f, v_e3f);

		//GLfloat e1f[BLOCK_HEIGHT];
		//GLfloat e2f[BLOCK_HEIGHT];
		//GLfloat e3f[BLOCK_HEIGHT];

		//e1f[0] = f1.x + (GLfloat)scanBase * s1;
		//e2f[0] = f2.x + (GLfloat)scanBase * s2;
		//e3f[0] = f3.x + (GLfloat)scanBase * s3;

		//for (int r = 1; r < BLOCK_HEIGHT; r++) {
		//	e1f[r] = e1f[r - 1] + s1;
		//	e2f[r] = e2f[r - 1] + s2;
		//	e3f[r] = e3f[r - 1 ] + s3;
		//}

		for (int j = jStart; j <= jEnd; j++) {
			Block& b = dBuffer.getBlock(j, i);

			for (int k = 0; k < BLOCK_HEIGHT; k++) {
				//These lines are important
				uint32_t e1 = std::max(0.0f, e1f[k] - j * 32.0f);
				uint32_t e2 = std::max(0.0f, e2f[k] - j * 32.0f);
				uint32_t e3 = std::max(0.0f, e3f[k] - j * 32.0f);

				uint32_t result = line(e1, e2, e3, mask1, mask2, mask3);
				b.bits[k] = result;
			}
		}
	}
}
*/

void rasterizeSilent(glm::vec2 t1, glm::vec2 t2, glm::vec2 t3) {
	fixTriangle(t1, t2, t3);

	GLfloat cx = (t1.x + t2.x + t3.x) / 3.0f;
	GLfloat cy = (t1.y + t2.y + t3.y) / 3.0f;
	glm::vec2 center(cx, cy);

	std::vector<glm::vec2*> ps;
	ps.push_back(&t1);
	ps.push_back(&t2);
	ps.push_back(&t3);
	std::sort(ps.begin(), ps.end(), triComp);

	glm::vec2& p1 = *ps[0];
	glm::vec2& p2 = *ps[1];
	glm::vec2& p3 = *ps[2];

	glm::vec2 l1 = p2 - p1;
	glm::vec2 l2 = p3 - p1;
	glm::vec2 l3 = p3 - p2;

	glm::vec2 n1(l1.y, -l1.x);
	glm::vec2 n2(l2.y, -l2.x);
	glm::vec2 n3(l3.y, -l3.x);

	//true if left is outside
	bool o1 = glm::dot(n1, (center - p1)) < 0;
	bool o2 = glm::dot(n2, (center - p1)) < 0;
	bool o3 = glm::dot(n3, (center - p2)) < 0;
	uint32_t mask1 = o1 ? 0 : ~0;
	uint32_t mask2 = o2 ? 0 : ~0;
	uint32_t mask3 = o3 ? 0 : ~0;

	//	GLfloat s1 = l1.x / l1.y;
	//	GLfloat s2 = l2.x / l2.y;
	//	GLfloat s3 = l3.x / l3.y;
	//
	//	GLfloat slopeFactor = (GLfloat)HEIGHT / 2;
	//	s1 /= slopeFactor;
	//	s2 /= slopeFactor;
	//	s3 /= slopeFactor;

	glm::vec2 f1 = p1 + ((1.0f - p1.y) / l1.y) * l1;
	glm::vec2 f2 = p1 + ((1.0f - p1.y) / l2.y) * l2;
	glm::vec2 f3 = p2 + ((1.0f - p2.y) / l3.y) * l3;

	convertVec(f1);
	convertVec(f2);
	convertVec(f3);
	convertVec(p1);
	convertVec(p2);
	convertVec(p3);
	l1 = p2 - p1;
	l2 = p3 - p1;
	l3 = p3 - p2;
	GLfloat s1 = l1.x / l1.y;
	GLfloat s2 = l2.x / l2.y;
	GLfloat s3 = l3.x / l3.y;


	//std::cout << "Slope: " << s1 << " " << s2 << " " << s3 << std::endl;
	GLfloat minY = std::min(p1.y, std::min(p2.y, p3.y));
	GLfloat maxY = std::max(p1.y, std::max(p2.y, p3.y));
	GLfloat minX = std::min(p1.x, std::min(p2.x, p3.x));
	GLfloat maxX = std::max(p1.x, std::max(p2.x, p3.x));

	int iStart = std::max(((int)minY) / BLOCK_HEIGHT, 0);
	int iEnd = std::min(((int)maxY) / BLOCK_HEIGHT, (int)dBuffer.heightB - 1);

	int jStart = std::max(((int)minX) / 32, 0);
	int jEnd = std::min(((int)maxX) / 32, (int)dBuffer.widthB - 1);


	for (int i = iStart; i <= iEnd; i++) {
		int scanBase = i * BLOCK_HEIGHT;

		GLfloat e1f[BLOCK_HEIGHT];
		GLfloat e2f[BLOCK_HEIGHT];
		GLfloat e3f[BLOCK_HEIGHT];

		e1f[0] = f1.x + (GLfloat)scanBase * s1;
		e2f[0] = f2.x + (GLfloat)scanBase * s2;
		e3f[0] = f3.x + (GLfloat)scanBase * s3;

		for (int r = 1; r < BLOCK_HEIGHT; r++) {
			e1f[r] = e1f[r - 1] + s1;
			e2f[r] = e2f[r - 1] + s2;
			e3f[r] = e3f[r - 1] + s3;
		}

		for (int j = jStart; j <= jEnd; j++) {
			Block& b = dBuffer.getBlock(j, i);

			for (int k = 0; k < BLOCK_HEIGHT; k++) {
				//These lines are important
				uint32_t e1 = std::max(0.0f, e1f[k] - j * 32.0f);
				uint32_t e2 = std::max(0.0f, e2f[k] - j * 32.0f);
				uint32_t e3 = std::max(0.0f, e3f[k] - j * 32.0f);

				uint32_t result = line(e1, e2, e3, mask1, mask2, mask3);
				//b.bits[k] = result;
				printBits(result);
				std::cout << " events " << e1 << " " << e2 << " " << e3 << std::endl;
				b.bits[k] |= result;
			}
		}
	}
}





void rasterize(glm::vec2 t1, glm::vec2 t2, glm::vec2 t3) {
	fixTriangle(t1, t2, t3);

	GLfloat cx = (t1.x + t2.x + t3.x) / 3.0f;
	GLfloat cy = (t1.y + t2.y + t3.y) / 3.0f;
	glm::vec2 center(cx, cy);

	std::vector<glm::vec2*> ps;
	ps.push_back(&t1);
	ps.push_back(&t2);
	ps.push_back(&t3);
	std::sort(ps.begin(), ps.end(), triComp);

	//std::cout << "Sorted: ";
	//for (auto it = ps.begin(); it != ps.end(); it++) {
	//	std::cout << (**it).y << " ";
	//}
	//std::cout << std::endl;

	glm::vec2& p1 = *ps[0];
	glm::vec2& p2 = *ps[1];
	glm::vec2& p3 = *ps[2];

	glm::vec2 l1 = p2 - p1;
	glm::vec2 l2 = p3 - p1;
	glm::vec2 l3 = p3 - p2;

	glm::vec2 n1(l1.y, -l1.x);
	glm::vec2 n2(l2.y, -l2.x);
	glm::vec2 n3(l3.y, -l3.x);
	n1 = glm::normalize(n1);
	n2 = glm::normalize(n2);
	n3 = glm::normalize(n3);

	//true if left is outside
	bool o1 = glm::dot(n1, (center - p1)) < 0;
	bool o2 = glm::dot(n2, (center - p1)) < 0;
	bool o3 = glm::dot(n3, (center - p2)) < 0;
	uint32_t mask1 = o1 ? 0 : ~0;
	uint32_t mask2 = o2 ? 0 : ~0;
	uint32_t mask3 = o3 ? 0 : ~0;

	//printVec(p1);
	//std::cout << std::endl;
	//printVec(p2);
	//std::cout << std::endl;
	//printVec(p3);
	//std::cout << std::endl;
	//std::cout << o1 << " " << o2 << " " << o3 << std::endl;

	GLfloat s1 = l1.x / l1.y;
	GLfloat s2 = l2.x / l2.y;
	GLfloat s3 = l3.x / l3.y;

	GLfloat slopeFactor = (GLfloat)BUFFER_HEIGHT / 2;
	s1 /= slopeFactor;
	s2 /= slopeFactor;
	s3 /= slopeFactor;

	//std::cout << "Slope: " << s1 << " " << s2 << " " << s3 << std::endl;

	auto pair1 = convert(p1);
	auto pair2 = convert(p2);
	auto pair3 = convert(p3);

	//printPair(pair1);
	//std::cout << std::endl;
	//printPair(pair2);
	//std::cout << std::endl;
	//printPair(pair3);
	//std::cout << std::endl;

	glm::vec2 f1 = p1 + ((1.0f - p1.y) / l1.y) * l1;
	glm::vec2 f2 = p1 + ((1.0f - p1.y) / l2.y) * l2;
	glm::vec2 f3 = p2 + ((1.0f - p2.y) / l3.y) * l3;
	//printVec(f1);
	//printVec(f2);
	//printVec(f3);
	//std::cout << std::endl;


	for (int i = 0; i < dBuffer.heightB; i++) {
		for (int j = 0; j < dBuffer.widthB; j++) {
			Block& b = dBuffer.getBlock(j, i);

			for (int k = 0; k < BLOCK_HEIGHT; k++) {
				int scanLine = i * BLOCK_HEIGHT + k;
				std::pair<int, int> conv1 = convert(f1 - glm::vec2(1, 0) * (GLfloat) scanLine * s1);
				std::pair<int, int> conv2 = convert(f2 - glm::vec2(1, 0) * (GLfloat) scanLine * s2);
				std::pair<int, int> conv3 = convert(f3 - glm::vec2(1, 0) * (GLfloat) scanLine * s3);

				uint32_t e1 = std::max(0, conv1.first - j * 32);
				uint32_t e2 = std::max(0, conv2.first - j * 32);
				uint32_t e3 = std::max(0, conv3.first - j * 32);

				uint32_t result = line(e1, e2, e3, mask1, mask2, mask3);
				b.bits[k] = result;

				//printBits(result);
				//std::cout << "{ " << e1 << " " << e2 << " " << e3 << " }" << std::endl;

			}
		}
	}


	/*
	for (int i = 0; i < 32; i++) {
		std::pair<int, int> conv1 = convert(f1);
		std::pair<int, int> conv2 = convert(f2);
		std::pair<int, int> conv3 = convert(f3);
		uint32_t e1 = std::max(0, conv1.first);
		uint32_t e2 = std::max(0, conv2.first);
		uint32_t e3 = std::max(0, conv3.first);

		uint32_t result = line(e1, e2, e3, mask1, mask2, mask3);
		printBits(result);
		std::cout << "{" << conv1.first << " " << conv2.first << " " << conv3.first << "} ";
		std::cout << f1.x << " - " << f2.x << " - " << f3.x;
		std::cout << std::endl;

		f1.x -= s1 / 16.0;
		f2.x -= s2 / 16.0;
		f3.x -= s3 / 16.0;
	}
	*/
}

void jank() { //120-125x improvement
	return;
	//meme();
	//exit(0);

	//in 1600x1056
	//rasterize takes about 40 seconds to render 4000
	//rasterizeSilent takes about 0.317 seconds to render 4000

	//in 1024x768
	//rasterize takes about 18.5 seconds to render 4000
	//rasterizeSilent takes about 0.153 seconds to render 4000

	//rasterize(glm::vec2(-0.5f, 0.5f), glm::vec2(0.0f, 1.0f), glm::vec2(0.5f, 0.3f));
	//rasterize(glm::vec2(), glm::vec2(), glm::vec2());

	dBuffer.reset();
	clock_t start = clock();

	std::cout << "Starting rasterize" << std::endl;
	for (int i = 0; i < 1; i++) {
		rasterizeSilent(glm::vec2(0.5, -0.4), glm::vec2(-0.6, -0.3), glm::vec2(-0.04, -0.9)); //benchmark reference
		//rasterizeSilent(glm::vec2(-0.5f, 0.0f), glm::vec2(0.0f, 0.5f), glm::vec2(0.5f, 0.0f));
	}
	//rasterize(glm::vec2(-0.5f, 0.0f), glm::vec2(0.0f, 0.5f), glm::vec2(0.5f, 0.0f));
	std::cout << "Done rasterize" << std::endl;
	clock_t end = clock();
	std::cout << "Time " << (end - start) / (double) CLOCKS_PER_SEC << std::endl;

	//std::cout << "Printing buffer" << std::endl;
	//dBuffer.print();
}


GLuint programID;
GLFWwindow* window = NULL;

GLuint u_LightPos, u_LightColor, u_AmbientLight, u_MvpMat, u_ModelMat, u_NormalMat;

glm::vec3 lightPos(6.0f, 6.0f, 0.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
glm::vec3 ambientLight(0.2f, 0.2f, 0.2f);

glm::mat4 mvp;
glm::mat4 model;
glm::mat4 normal;
glm::mat4 view;
glm::mat4 project;

GLfloat yaw = 0.0f;
GLfloat pitch = 0.0f;
GLfloat oldYaw = yaw;
GLfloat oldPitch = pitch;



void setMatrices() {
	glUniformMatrix4fv(u_MvpMat, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(u_ModelMat, 1, GL_FALSE, &model[0][0]);

	normal = glm::inverse(model);
	normal = glm::transpose(normal);
	glUniformMatrix4fv(u_NormalMat, 1, GL_FALSE, &normal[0][0]);
}

void setLights() {
	glUniform3f(u_LightPos, lightPos[0], lightPos[1], lightPos[2]);
	glUniform3f(u_LightColor, lightColor[0], lightColor[1], lightColor[2]);
	glUniform3f(u_AmbientLight, ambientLight[0], ambientLight[1], ambientLight[2]);
}


void drawModel(Model &m) {
	glBindVertexArray(m.varr);

	glBindBuffer(GL_ARRAY_BUFFER, m.posBuff);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glBindBuffer(GL_ARRAY_BUFFER, m.colBuff);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glBindBuffer(GL_ARRAY_BUFFER, m.normBuff);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	setMatrices();

	glDrawArrays(GL_TRIANGLES, 0, m.nVerts);
}

void makeModels() {
	glGenVertexArrays(1, &box.varr);
	glBindVertexArray(box.varr);

	glGenBuffers(1, &box.posBuff);
	glBindBuffer(GL_ARRAY_BUFFER, box.posBuff);
	const static GLfloat boxPos[] = {
	//1.0f, 1.0f, 0.0f, //9
	//-1.0f, 1.0f, 0.0f, //7
	//-1.0f, 0.0f, 0.0f, //1

	-1.0f, 0.0f, 0.0f, //1
	1.0f, 0.0f, 0.0f, //3
	1.0f, 1.0f, 0.0f //9
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxPos), boxPos, GL_STATIC_DRAW);

	glGenBuffers(1, &box.colBuff);
	glBindBuffer(GL_ARRAY_BUFFER, box.colBuff);
	const static GLfloat boxCol[] = {
		//0.0f, 0.0f, 1.0f, //9
		//0.0f, 1.0f, 0.0f, //7
		//1.0f, 0.0f, 0.0f, //1

		1.0f, 0.0f, 0.0f, //1
		1.0f, 0.0f, 1.0f, //3
		0.0f, 0.0f, 1.0f //9
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxCol), boxCol, GL_STATIC_DRAW);

	glGenBuffers(1, &box.normBuff);
	glBindBuffer(GL_ARRAY_BUFFER, box.normBuff);
	const static GLfloat boxNorm[] = {
	//	0.0f, 0.0f, 1.0f,
		//0.0f, 0.0f, 1.0f,
		//0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxNorm), boxNorm, GL_STATIC_DRAW);

	box.nVerts = 3;

	plant = parseObj("models/banana_plant.obj", 0.3f, 1.0f, 0.0f);
	cube = parseObj("models/cube.obj", 1.0f, 1.0f, 0.0f);
}

void init() {
//u_LightPos, u_LightColor, u_AmbientLight, u_MvpMat, u_ModelMat, u_NormalMat;
	u_LightPos = glGetUniformLocation(programID, "u_LightPos");
	u_LightColor = glGetUniformLocation(programID, "u_LightColor");
	u_AmbientLight = glGetUniformLocation(programID, "u_AmbientLight");
	u_MvpMat = glGetUniformLocation(programID, "u_MvpMat");
	u_ModelMat = glGetUniformLocation(programID, "u_ModelMat");
	u_NormalMat = glGetUniformLocation(programID, "u_NormalMat");

	std::cout << "Locations ";
	std::cout << u_LightPos << " " << u_LightColor << " " << u_AmbientLight << " " << u_MvpMat << " " << u_ModelMat << " " << u_NormalMat << std::endl;

	view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	project = glm::perspective(glm::radians(90.0f), (GLfloat) SCREEN_WIDTH / (GLfloat) SCREEN_HEIGHT, 0.01f, 100.0f);

	makeModels();

	if (replayMode == PLAY) {
		replayFile.open(replayFileName, std::fstream::in);
		if (replayFile.peek() == std::fstream::traits_type::eof()) {
			std::cout << "Empty replay file used in playback -- program will crash" << std::endl;
		}
	} else if (replayMode == RECORD) {
		replayFile.open(replayFileName, std::fstream::out | std::fstream::trunc);
	}

	std::cout << "Replay file status: " << replayFile.fail() << std::endl;

	if (replayMode == PLAY) {
		std::string line;
		std::vector<std::string> tokens;

		std::getline(replayFile, line);
		tokens = split(line, " ");
		nextReplayFrame = std::stoull(tokens[1]);
	}

}

void render() {
	double seconds = clock() / (double)CLOCKS_PER_SEC;
	lightPos = glm::vec3(4.0f * cos(seconds), 0.0f, 4.0f * sin(seconds));

	glClearColor(0.3f, 0.3f, 0.3f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setLights();


	GLfloat rad = seconds / 1.0f;
	rad = 0.0f;

	model = glm::rotate(glm::mat4(), rad, glm::vec3(0.0f, 1.0f, 0.0f));

	mvp = project * view * model;
	test();
	drawModel(box);

	model = glm::mat4();
	model = glm::translate(model, glm::vec3(0.0f, -1.0f, 1.0f));
	model = glm::rotate(model, rad, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
	mvp = project * view * model;
	drawModel(plant);

	model = glm::mat4();
	model = glm::translate(model, lightPos + glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
	mvp = project * view * model;
	drawModel(cube);
}

void handlePlayback() {
	if (currentFrame != nextReplayFrame) {
		return;
	}

	std::string line;
	std::vector<std::string> tokens;

	std::vector<glm::vec3> translation1;
	std::vector<glm::vec3> translation2;
	std::vector<std::pair<GLfloat, GLfloat>> yawPitch;
	bool quit = false;

	//Get all the actions
	while (true && !replayFile.eof()) {
		std::getline(replayFile, line);
		tokens = split(line, " ");

		if (tokens[0] == "f") {
			nextReplayFrame = std::stoull(tokens[1]);
			break;
		} else if (tokens[0] == "t") {
			GLfloat dx = std::stof(tokens[1]);
			GLfloat dy = std::stof(tokens[2]);
			GLfloat dz = std::stof(tokens[3]);
			translation1.push_back(glm::vec3(dx, dy, dz));
		} else if (tokens[0] == "tt") {
			GLfloat dx = std::stof(tokens[1]);
			GLfloat dy = std::stof(tokens[2]);
			GLfloat dz = std::stof(tokens[3]);
			translation2.push_back(glm::vec3(dx, dy, dz));
		} else if (tokens[0] == "yp") {
			GLfloat dYaw = std::stof(tokens[1]);
			GLfloat dPitch = std::stof(tokens[2]);
			yawPitch.push_back(std::pair<GLfloat, GLfloat>(dYaw, dPitch));
		} else if (tokens[0] == "e") {
			quit = true;
			break;
		}
	}

	//Perform all the actions

	if (quit) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	for (auto it = translation1.begin(); it != translation1.end(); it++) {
		view = glm::translate(glm::mat4(), *it) * view;
	}

	view = glm::rotate(glm::mat4(), -oldYaw, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(), -oldPitch, glm::vec3(1.0f, 0.0f, 0.0f)) * view;

	for (auto it = translation2.begin(); it != translation2.end(); it++) {
		view = glm::translate(glm::mat4(), *it) * view;
	}

	for (auto it = yawPitch.begin(); it != yawPitch.end(); it++) {
		yaw += it->first;
		pitch += it->second;
	}

	pitch = std::min(((float) PI) / 2.0f, std::max(-((float) PI) / 2.0f, pitch));

	view = glm::rotate(glm::mat4(), pitch, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(), yaw, glm::vec3(0.0f, 1.0f, 0.0f)) * view;

	oldYaw = yaw;
	oldPitch = pitch;
}

bool keyPressed(int key) {
	return glfwGetKey(window, key) == GLFW_PRESS;
}

void recordFrameNumber() {
	if (recordedFrameNumber) {
		return;
	}
	recordedFrameNumber = true;
	replayFile << "f " << currentFrame << std::endl;
}


void recordTranslate(const glm::vec3 &v, bool beforeRotate) {
	if (replayMode != RECORD) {
		return;
	}
	recordFrameNumber();

	std::string identifier = "t";
	if (!beforeRotate) {
		identifier = "tt";
	}

	replayFile << identifier << " " << v.x << " " << v.y << " " << v.z << std::endl;
}

void recordYawPitch(GLfloat dYaw, GLfloat dPitch) {
	if (replayMode != RECORD) {
		return;
	}
	recordFrameNumber();

	replayFile << "yp " << dYaw << " " << dPitch << std::endl;
}

void recordQuit() {
	if (replayMode != RECORD) {
		return;
	}
	recordFrameNumber();
	replayFile << "e" << std::endl;
}

void handleInput() {
	if (keyPressed(GLFW_KEY_ESCAPE)) {
		recordQuit();
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (keyPressed(KEY_LEFT)) {
		glm::vec3 vec = glm::vec3(1.0f, 0.0f, 0.0f) * cameraSpeed;
		recordTranslate(vec, true);
		view = glm::translate(glm::mat4(), vec) * view;
	}

	if (keyPressed(KEY_RIGHT)) {
		glm::vec3 vec = glm::vec3(-1.0f, 0.0f, 0.0f) * cameraSpeed;
		recordTranslate(vec, true);
		view = glm::translate(glm::mat4(), vec) * view;
	}

	if (keyPressed(KEY_FORWARD)) {
		glm::vec3 vec = glm::vec3(0.0f, 0.0f, 1.0f) * cameraSpeed;
		recordTranslate(vec, true);
		view = glm::translate(glm::mat4(), vec) * view;
	}

	if (keyPressed(KEY_BACK)) {
		glm::vec3 vec = glm::vec3(0.0f, 0.0f, -1.0f) * cameraSpeed;
		recordTranslate(vec, true);
		view = glm::translate(glm::mat4(), vec) * view;
	}

	view = glm::rotate(glm::mat4(), -oldYaw, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(), -oldPitch, glm::vec3(1.0f, 0.0f, 0.0f)) * view;

	if (keyPressed(KEY_UP)) {
		glm::vec3 vec = glm::vec3(0.0f, -1.0f, 0.0f) * cameraSpeed;
		recordTranslate(vec, false);
		view = glm::translate(glm::mat4(), vec) * view;
	}

	if (keyPressed(KEY_DOWN)) {
		glm::vec3 vec = glm::vec3(0.0f, 1.0f, 0.0f) * cameraSpeed;
		recordTranslate(vec, false);
		view = glm::translate(glm::mat4(), vec) * view;
	}

	pitch = std::min(((float) PI) / 2.0f, std::max(-((float) PI) / 2.0f, pitch));


	view = glm::rotate(glm::mat4(), pitch, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(), yaw, glm::vec3(0.0f, 1.0f, 0.0f)) * view;

	oldYaw = yaw;
	oldPitch = pitch;
}

void cursorCallback(GLFWwindow* window, double xPos, double yPos) {
	double dx = xPos - (SCREEN_WIDTH / 2.0);
	double dy = yPos - (SCREEN_HEIGHT / 2.0);

	GLfloat dYaw = MOUSE_SENSITIVITY * dx;
	GLfloat dPitch = MOUSE_SENSITIVITY * dy;

	yaw += dYaw;
	pitch += dPitch;

	recordYawPitch(dYaw, dPitch);
}


int main() {

	jank();
	if (glfwInit() != GL_TRUE) {
		std::cerr << "Failed to init glfw" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "asd", NULL, NULL);

	if (window == NULL) {
		std::cerr << "Failed to make glfw window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to init GLEW" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	programID = LoadShaders("v.glsl", "f.glsl");
	glUseProgram(programID);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glfwSetCursorPosCallback(window, cursorCallback);

	init();


	do {
		recordedFrameNumber = false;

		clock_t start = clock();
		render();
		glfwSwapBuffers(window);

		glfwPollEvents();

		if (replayMode == PLAY) {
			handlePlayback();
		} else {
			handleInput();
		}

		glfwSetCursorPos(window, SCREEN_WIDTH / 2.0, SCREEN_HEIGHT / 2.0);
		clock_t end = clock();

		double elapsed = (end - start) / (double)CLOCKS_PER_SEC;
		fpsWait(1 / (double)FPS - elapsed);
		currentFrame++;
	} while (!glfwWindowShouldClose(window));

	replayFile.close();
	std::cout << "Ending on frame " << currentFrame << std::endl;
	return 0;
}


void maskVec4s(glm::vec4 p1, glm::vec4 p2, glm::vec4 p3) {
	p1 = mvp * p1;
	p2 = mvp * p2;
	p3 = mvp * p3;
	p1 /= p1.a;
	p2 /= p2.a;
	p3 /= p3.a;

	glm::vec2 t1(p1.x, p1.y);
	glm::vec2 t2(p2.x, p2.y);
	glm::vec2 t3(p3.x, p3.y);

	std::cout << "depths (" << p1.z << " " << p2.z << " " << p3.z << ")" << std::endl;
	printVec(t1);
	printVec(t2);
	printVec(t3);
	std::cout << std::endl;


	if (abs(p1.z) >= 1.0f || abs(p2.z) >= 1.0f || abs(p3.z) >= 1.0f) {
		std::cout << "\033[1;31m";
	} else {
		std::cout << "\033[1;37m";
	}

	if (abs(p1.z) >= 1.0f && abs(p2.z) >= 1.0f && abs(p3.z) >= 1.0f) {
		return;
	}

	rasterizeSilent(t1, t2, t3);
}


void test() {
	glm::vec4 p1(1.0f, 1.0f, 0.0f, 1.0f);
	glm::vec4 p2(-1.0f, 1.0f, 0.0f, 1.0f);
	glm::vec4 p3(-1.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 p4(-1.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 p5(1.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 p6(1.0f, 1.0f, 0.0f, 1.0f);
	
	dBuffer.reset();
	//maskVec4s(p1, p2, p3);
	maskVec4s(p4, p5, p6);
	//dBuffer.print();




}
