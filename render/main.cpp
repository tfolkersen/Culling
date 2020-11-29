#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <common/shader.hpp>
#include <ctime>
//#include <immintrin.h>

#include <vector>
#include <algorithm>



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

#define WIDTH 32*32
#define HEIGHT 32*24
//#define WIDTH 32*50
//#define HEIGHT 32*33
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
		widthB = (WIDTH / 32);
		heightB = (HEIGHT / BLOCK_HEIGHT);

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

std::pair<int, int> convert(const glm::vec2 &v) {
	int W = WIDTH - 1;
	int H = HEIGHT - 1;
	GLfloat Wf = (GLfloat)W;
	GLfloat Hf = (GLfloat)H;
	int x = round((Wf / 2.0) * v.x + (Wf / 2.0));
	int y = round(-(Hf / 2.0) * v.y + (Hf / 2.0));

	//int x = (Wf / 2.0) * v.x + (Wf / 2.0);
	//int y = -(Hf / 2.0) * v.y + (Hf / 2.0);

	return std::pair<int, int>(x, y);
}

void convertVec(glm::vec2 &v) {
	int W = WIDTH - 1;
	int H = HEIGHT - 1;
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
	int iEnd = std::min(((int)maxY) / BLOCK_HEIGHT, (int) dBuffer.heightB - 1);

	int jStart = std::max(((int)minX) / 32, 0);
	int jEnd = std::min(((int)maxX) / 32, (int) dBuffer.widthB - 1);
	

	for (int i = iStart; i <= iEnd ; i++) {
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
			e3f[r] = e3f[r - 1 ] + s3;
		}

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

	GLfloat slopeFactor = (GLfloat)HEIGHT / 2;
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
	for (int i = 0; i < 4000; i++) {
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

struct Model {
	GLuint varr;
	GLuint posBuff;
	GLuint colBuff;
	GLuint normBuff;
	int nVerts;
};

Model box;

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
	-1.0f, 0.0f, 0.0f, //1
	-1.0f, 1.0f, 0.0f, //7
	1.0f, 1.0f, 0.0f, //9

	1.0f, 1.0f, 0.0f, //9
	1.0f, 0.0f, 0.0f, //3
	-1.0f, 0.0f, 0.0f //1
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxPos), boxPos, GL_STATIC_DRAW);

	glGenBuffers(1, &box.colBuff);
	glBindBuffer(GL_ARRAY_BUFFER, box.colBuff);
	const static GLfloat boxCol[] = {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxCol), boxCol, GL_STATIC_DRAW);

	glGenBuffers(1, &box.normBuff);
	glBindBuffer(GL_ARRAY_BUFFER, box.normBuff);
	const static GLfloat boxNorm[] = {
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxNorm), boxNorm, GL_STATIC_DRAW);

	box.nVerts = 6;
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

	makeModels();
}

void render() {
	glClearColor(0.3f, 0.3f, 0.3f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT);

	setLights();

	view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	project = glm::perspective(glm::radians(45.0f), 1024.0f / 768.0f, 0.01f, 100.0f);


	time_t t = time(NULL);
	GLfloat rad = ((GLfloat)t) / 1.0;
	model = glm::rotate(model, 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));

	mvp = project * view * model;
	drawModel(box);


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

	window = glfwCreateWindow(1024, 768, "asd", NULL, NULL);

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

	programID = LoadShaders("v.glsl", "f.glsl");
	glUseProgram(programID);

	init();

	do {
		render();
		glfwSwapBuffers(window);

		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}


	} while (!glfwWindowShouldClose(window));



	return 0;
}

//https://www.codeproject.com/Articles/874396/Crunching-Numbers-with-AVX-and-AVX
/*
	__m256 evens = _mm256_set_ps(2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0);
	__m256 odds = _mm256_set_ps(1.0, 3.0, 5.0, 7.0, 9.0, 11.0, 13.0, 15.0);
	__m256 result = _mm256_add_ps(evens, odds);
	for (int i = 7; i >= 0; i--) {
		std::cout << ((float*)&result)[i] << " ";
	}
	std::cout << std::endl;


*/