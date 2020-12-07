#pragma once

/*		culling file


	this implements the occlusion culling logic:
	rasterization of occluders, depth test, updating of depth buffer, and
	data structures for these things

	also defines parameters of the culling logic

	the tiles of the depth buffer are called "Blocks" in this code
*/

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <iostream>

#include "models.h"
#include <vector>


//these should match screen resolution defined in draw.h, but they don't need to
#define BUFFER_WIDTH 1440 //width in pixels of depth buffer -- must be a multiple of 32 (width of uint32_t)
#define BUFFER_HEIGHT 1024 //height in pixels of depth buffer -- must be a multiple of BLOCK_HEIGHT

//height in pixels of a block, 8 is used because in the original paper's implementation, AVX instructions
//are used to operate on 8 uint32_ts at a time, making it so an entire block can be dealt with using
//a few AVX instructions
#define BLOCK_HEIGHT 8

/*	

	this is the function coverageSIMD() from the Hasselgren et al. paper

	get bit mask for one scanline

	e0, e1, e2 are x coordinates of each triangle edge on this scanline,
	relative to the block's coordinates in pixel space

	o0, o1, o2 are masks that are used to xor the result of the bit shift
	these should be 0 for left-facing triangle edges, and ~0 (which is all 1s) for right-facing
	triangle edges
*/
uint32_t line(uint32_t e0, uint32_t e1, uint32_t e2, uint32_t o0, uint32_t o1, uint32_t o2);

/*		render triangle specified by these points in NDC coordinates,
		into the depth buffer. assumes clipping with near plane has already been done

		this isn't used by any of the culling logic -- it's here to be able to visualize
		the rasterization of a triangle into the depth buffer

*/
void rasterize(glm::vec2 t1, glm::vec2 t2, glm::vec2 t3);


/*
	Tile as specified by the Hasselgren et al. paper
*/
//One block or "tile" of the depth buffer
//a pixel belongs to the working depth if its bit is set, otherwise it belongs to the reference depth
struct Block {
	uint32_t bits[BLOCK_HEIGHT]; //the bit mask of the block
	GLfloat reference; //reference depth (zMax0 in the paper)
	GLfloat working; //working depth (zMax1 in the paper)

	void reset(); //reset this block (use this every frame)
};

//Depth buffer containing all blocks
struct DepthBuffer {
	Block* arr; //array of blocks composing the depth buffer
	uint32_t blockCount; //how many blocks exist in this buffer

	uint32_t widthB; //width of buffer in blocks
	uint32_t heightB; //height of buffer in blocks

	DepthBuffer();

	~DepthBuffer();

	void reset(); //clear all blocks

	Block& getBlock(int x, int y); //get block reference -- coordinates refer to blocks, (0,0) is top left

	void print(); //print depth buffer's masks to stdout -- used to debug/visualize depth buffer
};

extern DepthBuffer dBuffer; //global depth buffer

//true if object should be drawn according to depth buffer -- also updates depth buffer
//this is the function used in the renderer
bool shouldDraw(const ModelCollection& m);
