#pragma once
#include <glm/glm.hpp>
#include <iostream>

#define BUFFER_WIDTH 32
#define BUFFER_HEIGHT 32
#define BLOCK_HEIGHT 8

uint32_t line(uint32_t e0, uint32_t e1, uint32_t e2, uint32_t o0, uint32_t o1, uint32_t o2);

void rasterizeSilent(glm::vec2 t1, glm::vec2 t2, glm::vec2 t3);

struct Block {
	uint32_t bits[BLOCK_HEIGHT];

	void reset();
};

struct DepthBuffer {
	Block* arr;
	uint32_t blockCount;

	uint32_t widthB;
	uint32_t heightB;

	DepthBuffer();

	~DepthBuffer();

	void reset();

	Block& getBlock(int x, int y);

	void print();
};

extern DepthBuffer dBuffer;

