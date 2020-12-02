#include "cull.h"
#include "utility.h"
#include <vector>
#include <algorithm>
#include "draw.h"
#include <cmath>

DepthBuffer dBuffer;

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
				b.bits[k] |= result;
			}
		}
	}
}

void Block::reset() {
	for (int i = 0; i < BLOCK_HEIGHT; i++) {
		bits[i] = 0;
	}
	reference = 1.0f;
	working = 1.0f;

}

DepthBuffer::DepthBuffer() {
	widthB = (BUFFER_WIDTH / 32);
	heightB = (BUFFER_HEIGHT / BLOCK_HEIGHT);

	blockCount = widthB * heightB;
	std::cout << "DepthBuffer making " << blockCount << " blocks" << std::endl;
	arr = new Block[blockCount];
}

DepthBuffer::~DepthBuffer() {
	delete[] arr;
}

void DepthBuffer::reset() {
	for (int i = 0; i < heightB; i++) {
		for (int j = 0; j < widthB; j++) {
			Block& b = getBlock(j, i);
			b.reset();
		}
	}
}

Block& DepthBuffer::getBlock(int x, int y) {
	int index = y * widthB + x;
	return arr[index];
}

void DepthBuffer::print() {
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

////////////////////////////////////////////////////////////

bool windowClip(const glm::vec3 &p1, const glm::vec3 &p2) {
	//LRDUBF
	#define CLIP_CODE(p) \
		((p.x <= -1.0f) << 5) | ((p.x >= 1.0f) << 4) | ((p.y <= -1.0f) << 3) | ((p.y >= 1.0f) << 2) | ((p.z <= -1.0f) << 1) | ((p.z >= 1.0f))

	int code1 = CLIP_CODE(p1);
	int code2 = CLIP_CODE(p2);

	if (code1 != 0 && code2 != 0 && (code1 & code2) != 0) {
		return true;
	}
}

bool triangleOutsideWindow(const glm::vec4 &p1, const glm::vec4 &p2, const glm::vec4 &p3) { //might not really work
	glm::vec3 t1(p1);
	glm::vec3 t2(p2);
	glm::vec3 t3(p3);

	return windowClip(t1, t2) && windowClip(t1, t3) && windowClip(t2, t3);
}

void transformBoundingBox(const Model3 &m, GLfloat &minX, GLfloat &maxX, GLfloat &minY, GLfloat &maxY, GLfloat &minZ, GLfloat &maxZ, bool &badPoints, bool &allBadPoints) {
	minX = std::numeric_limits<GLfloat>::max();
	maxX = std::numeric_limits<GLfloat>::min();

	minY = std::numeric_limits<GLfloat>::max();
	maxY = std::numeric_limits<GLfloat>::min();

	minZ = std::numeric_limits<GLfloat>::max();
	maxZ = std::numeric_limits<GLfloat>::min();

	size_t pointCount = 0;
	size_t badPointCount = 0;
	for (auto it = m.boxData.begin(); it != m.boxData.end();) {
		pointCount++;
		glm::vec4 p1(*it++, *it++, *it++, 1.0f);

		glm::vec4 p2 = view * m.modelMatrix * p1;
		p2 /= p2.w;
		std::vector<int> sign1;
		sign1.push_back(SIGN(p2.x));
		sign1.push_back(SIGN(p2.y));

		glm::vec4 p3 = project * view * m.modelMatrix * p1;
		p3 /= p3.w;
		std::vector<int> sign2;
		sign2.push_back(SIGN(p3.x));
		sign2.push_back(SIGN(p3.y));

		if (sign1 != sign2) {
			badPointCount++;
		}

		minX = std::min(minX, p3.x);
		maxX = std::max(maxX, p3.x);

		minY = std::min(minY, p3.y);
		maxY = std::max(maxY, p3.y);

		minZ = std::min(minZ, p3.z);
		maxZ = std::max(maxZ, p3.z);
		
	}

	minX = std::max(minX, -1.0f);
	maxX = std::min(maxX, 1.0f);

	minY = std::max(minY, -1.0f);
	maxY = std::min(maxY, 1.0f);

	minZ = std::max(minZ, -1.0f);
	maxZ = std::min(maxZ, 1.0f);

	badPoints = badPointCount > 0;
	allBadPoints = (badPointCount == pointCount);
}

bool depthTest(GLfloat minX, GLfloat maxX, GLfloat minY, GLfloat maxY, GLfloat minZ, GLfloat maxZ) {
	std::cout << "coords " << minX << " " << maxX << " " << minY << " " << maxY << std::endl;


	glm::vec2 minP(minX, minY);
	glm::vec2 maxP(maxX, maxY);

	convertVec(minP);
	convertVec(maxP);

	minX = minP.x;
	maxX = maxP.x;

	minY = maxP.y;
	maxY = minP.y;

	int iStart = std::max(((int)minY) / BLOCK_HEIGHT, 0);
	int iEnd = std::min((int)ceil(maxY / (GLfloat) BLOCK_HEIGHT), (int)dBuffer.heightB - 1);

	int jStart = std::max(((int)minX) / 32, 0); 
	int jEnd = std::min((int)ceil(maxX / 32.0f), (int)dBuffer.widthB - 1);

	std::cout << "indices " << iStart << " " << iEnd << " " << jStart << " " << jEnd << std::endl;

	for (int i = iStart; i <= iEnd; i++) {
		for (int j = jStart; j <= jEnd; j++) {
			Block& b = dBuffer.getBlock(j, i);

			if (b.reference >= minZ) {
				return true;
			}

			//for (int k = 0; k < BLOCK_HEIGHT; k++) {
			//	uint32_t result = ~0;
			//	b.bits[k] = result;
			//}
		}
	}
	return false;
}


bool shouldDraw(const Model3& m) {
	bool reject = false;

	//Transform bounding box into bounding square
	GLfloat minX, maxX, minY, maxY, minZ, maxZ;
	bool badPoints, allBadPoints;
	transformBoundingBox(m, minX, maxX, minY, maxY, minZ, maxZ, badPoints, allBadPoints);
	std::cout << minZ;

	if (allBadPoints) {
		return false;
	}

	if (badPoints) {
		return true;
	}

	//Depth test
	bool visible = depthTest(minX, maxX, minY, maxY, minZ, maxZ);



	return visible;

	//Print box
	/*
	dBuffer.reset();
	for (auto it = box.begin(); it != box.end();) {
		rasterize(*it++, *it++, *it++);
	}
	dBuffer.print();
	std::cout << std::endl;
	*/
}
