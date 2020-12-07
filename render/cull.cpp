#include "cull.h"
#include "utility.h"
#include <vector>
#include <algorithm>
#include "draw.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "control.h"

DepthBuffer dBuffer;

/*
	pixel space is like canvas coordinates in WebGL -- top left is 0,0 and right/down is positive

*/

/*
	This code is from the Hasselgren et al. paper
*/
//Get bit mask for one scanline -- see header file for details
uint32_t line(uint32_t e0, uint32_t e1, uint32_t e2, uint32_t o0, uint32_t o1, uint32_t o2) {
	
	//First edge's mask
	uint32_t m0 = ~0; //load register with 1s
	m0 = e0 >= 32 ? 0 : (m0 >> e0); //shift result to x coordinate of edge -- don't shift past 32 (undefined behaviour)
	m0 ^= o0; //flip if necessary

	//Second edge's mask
	uint32_t m1 = ~0;
	m1 = e1 >= 32 ? 0 : (m1 >> e1);
	m1 ^= o1;

	//Third edge's mask
	uint32_t m2 = ~0;
	m2 = e2 >= 32 ? 0 : (m2 >> e2);
	m2 ^= o2;

	return m0 & m1 & m2;
}

/*	render bit mask of triangle into depth buffer -- see header for details

	only for debugging and visualizing the rasterization -- not part of culling logic
*/
void rasterize(glm::vec2 t1, glm::vec2 t2, glm::vec2 t3) {
	fixTriangle(t1, t2, t3); //ensure y coords are all different

	//centroid
	GLfloat cx = (t1.x + t2.x + t3.x) / 3.0f;
	GLfloat cy = (t1.y + t2.y + t3.y) / 3.0f;
	glm::vec2 center(cx, cy);

	//sort points by decreasing height
	std::vector<glm::vec2*> ps;
	ps.push_back(&t1);
	ps.push_back(&t2);
	ps.push_back(&t3);
	std::sort(ps.begin(), ps.end(), triComp);

	glm::vec2& p1 = *ps[0];
	glm::vec2& p2 = *ps[1];
	glm::vec2& p3 = *ps[2];

	//downward-pointing edges
	glm::vec2 l1 = p2 - p1;
	glm::vec2 l2 = p3 - p1;
	glm::vec2 l3 = p3 - p2;

	//left-facing normals (because edges are downward facing)
	glm::vec2 n1(l1.y, -l1.x);
	glm::vec2 n2(l2.y, -l2.x);
	glm::vec2 n3(l3.y, -l3.x);

	//true if left is outside of the triangle
	bool o1 = glm::dot(n1, (center - p1)) < 0; //point in triangle test for one edge -- but with left-facing normal
	bool o2 = glm::dot(n2, (center - p1)) < 0;
	bool o3 = glm::dot(n3, (center - p2)) < 0;
	uint32_t mask1 = o1 ? 0 : ~0;
	uint32_t mask2 = o2 ? 0 : ~0;
	uint32_t mask3 = o3 ? 0 : ~0;

	//move along line to the top of the screen (y = 1.0)
	glm::vec2 f1 = p1 + ((1.0f - p1.y) / l1.y) * l1;
	glm::vec2 f2 = p1 + ((1.0f - p1.y) / l2.y) * l2;
	glm::vec2 f3 = p2 + ((1.0f - p2.y) / l3.y) * l3;

	//convert points at top of screen, and points, into pixel space
	convertVec(f1);
	convertVec(f2);
	convertVec(f3);
	convertVec(p1);
	convertVec(p2);
	convertVec(p3);

	//remake line vectors in pixel space
	l1 = p2 - p1;
	l2 = p3 - p1;
	l3 = p3 - p2;
	// dx/dy slopes of each edge
	GLfloat s1 = l1.x / l1.y;
	GLfloat s2 = l2.x / l2.y;
	GLfloat s3 = l3.x / l3.y;

	//bounding square of triangle in pixel space
	GLfloat minY = std::min(p1.y, std::min(p2.y, p3.y));
	GLfloat maxY = std::max(p1.y, std::max(p2.y, p3.y));
	GLfloat minX = std::min(p1.x, std::min(p2.x, p3.x));
	GLfloat maxX = std::max(p1.x, std::max(p2.x, p3.x));

	//coordinate ranges of blocks possibly overlapping triangle
	int iStart = std::max(((int)minY) / BLOCK_HEIGHT, 0);
	int iEnd = std::min(((int)maxY) / BLOCK_HEIGHT, (int)dBuffer.heightB - 1);

	int jStart = std::max(((int)minX) / 32, 0);
	int jEnd = std::min(((int)maxX) / 32, (int)dBuffer.widthB - 1);

	for (int i = iStart; i <= iEnd; i++) { //iterate over height
		int scanBase = i * BLOCK_HEIGHT; //height of top scanline (pixel space)

		//compute events for each scanline in pixel space
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

		for (int j = jStart; j <= jEnd; j++) { //iterate over width
			Block& b = dBuffer.getBlock(j, i);

			for (int k = 0; k < BLOCK_HEIGHT; k++) { //rasterize into entire block
				//actual events relative to this block
				uint32_t e1 = std::max(0.0f, e1f[k] - j * 32.0f);
				uint32_t e2 = std::max(0.0f, e2f[k] - j * 32.0f);
				uint32_t e3 = std::max(0.0f, e3f[k] - j * 32.0f);

				uint32_t result = line(e1, e2, e3, mask1, mask2, mask3);
				b.bits[k] |= result;
			}
		}
	}
}

//clear block by settings its depths to 1.0 (the maximum depth) and clearing mask
void Block::reset() {
	for (int i = 0; i < BLOCK_HEIGHT; i++) {
		bits[i] = 0;
	}
	reference = 1.0f;
	working = 0.0f;

}

//construct depth buffer and allocate blocks
DepthBuffer::DepthBuffer() {
	widthB = (BUFFER_WIDTH / 32);
	heightB = (BUFFER_HEIGHT / BLOCK_HEIGHT);

	blockCount = widthB * heightB;
	std::cout << "DepthBuffer making " << blockCount << " blocks" << std::endl;
	arr = new Block[blockCount];
}

//delete blocks
DepthBuffer::~DepthBuffer() {
	delete[] arr;
}

//reset all blocks in buffer
void DepthBuffer::reset() {
	for (int i = 0; i < heightB; i++) {
		for (int j = 0; j < widthB; j++) {
			Block& b = getBlock(j, i);
			b.reset();
		}
	}
}

/*	get block of depth buffer

	coordinates refer to blocks

	top left block is 0,0
	block to the right of that is 1,0
	bottom right block is (widthB - 1, heightB - 1)
*/
Block& DepthBuffer::getBlock(int x, int y) {
	int index = y * widthB + x;
	return arr[index];
}

//print depth buffer masks to stdout (for visualization and debugging of depth buffer)
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

//True if point is inside the near plane -- (false if point is behind camera)
#define INSIDE(p) \
	(p.z <= -NEAR)

/*
	transform raw GLfloat triangle data with respect to the model/view matrix, then
	perform clipping with respect to the near plane, and write resultant triangles into vector of vec3s
	after applying perspective transformation

	clipping of a triangle may produce 0, 1, or 2 triangles

	data -- GLfloat data of triangles in format: x, y, z, x, y, z, x, y, z (3 points specify triangle, and those points are contiguous in this buffer)
	tris -- where resultant triangles are written to (3 points specify triangle)
	model -- model matrix to use for transformations

*/
void transformPoints(const std::vector<GLfloat> &data, std::vector<glm::vec3> &tris, const glm::mat4 &model) {
	/*
		I have no idea why, but this scaling and rotation is needed, otherwise the rasterization shows a different view of the object than the GL view...

		in testing, GL and the rasterizer show the same result, except for when the
		data comes from a Model
	*/
	glm::mat4 rot = glm::rotate(glm::mat4(), (GLfloat)-PI / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 scale = glm::scale(glm::mat4(), glm::vec3(1.0f, 1.0f, -1.0f));

	for (auto it = data.begin(); it != data.end();) {
		//get one triangle from the buffer
		glm::vec4 t1(*it++, *it++, *it++, 1.0f);
		glm::vec4 t2(*it++, *it++, *it++, 1.0f);
		glm::vec4 t3(*it++, *it++, *it++, 1.0f);
		
		//transform it to camera space
		t1 = view * model * rot * scale * t1;
		t1 /= t1.w;
		t2 = view * model * rot * scale * t2;
		t2 /= t2.w;
		t3 = view * model * rot * scale * t3;
		t3 /= t3.w;

		//now perform clipping with the near plane
		std::vector<glm::vec4*> points;
		std::vector<glm::vec4> face; //face resultant from clipping (can be empty, or be a triangle, or a square)
		points.push_back(&t1);
		points.push_back(&t2);
		points.push_back(&t3);

		std::vector<std::pair<glm::vec4*, glm::vec4*>> pairs; //pairs representing triangle edges
		pairs.push_back(std::pair<glm::vec4*, glm::vec4*>(points[0], points[1]));
		pairs.push_back(std::pair<glm::vec4*, glm::vec4*>(points[1], points[2]));
		pairs.push_back(std::pair<glm::vec4*, glm::vec4*>(points[2], points[0]));

		//for each line
		for (auto it = pairs.begin(); it != pairs.end(); it++) {
			glm::vec4& s = (*it->first);
			glm::vec4& p = (*it->second);

			//This is the Sutherland-Hodgman clipping algorithm
			if (INSIDE(s) && INSIDE(p)) {
				//both inside -- output p
				face.push_back(p);
			}
			else if (INSIDE(s) && !INSIDE(p)) {
				//p outside -- clip p with respect to near plane and output result
				GLfloat a = (-NEAR - s.z) / (p.z - s.z);
				glm::vec4 i = s + (p - s) * a;
				face.push_back(i);
			}
			else if (!INSIDE(s) && !INSIDE(p)) {
				//both outside -- reject both
				continue;
			}
			else if (!INSIDE(s) && INSIDE(p)) {
				//outside to inside -- clip s and output i and p
				GLfloat a = (-NEAR - s.z) / (p.z - s.z);
				glm::vec4 i = s + (p - s) * a;
				face.push_back(i);
				face.push_back(p);
			}
		}

		//project resulting face's points
		for (auto it = face.begin(); it != face.end(); it++) {
			(*it) = project * (*it);
			(*it) /= it->w;
		}

		//turn face into triangles
		for (int i = 1; i + 1 < face.size(); i += 1) {
			glm::vec4& p1 = face[0];
			glm::vec4& p2 = face[i];
			glm::vec4& p3 = face[i + 1];

			tris.push_back(glm::vec3(p1));
			tris.push_back(glm::vec3(p2));
			tris.push_back(glm::vec3(p3));
		}
	}
}

/*		given an object, find its bounding box in NDC coordinates (after projection transformation)

*/
void transformBoundingBox(const ModelCollection &m, GLfloat &minX, GLfloat &maxX, GLfloat &minY, GLfloat &maxY, GLfloat &minZ, GLfloat &maxZ) {
	//transform and clip bounding box with respect to near plane
	std::vector<glm::vec3> tris;
	transformPoints(m.boxData, tris, m.modelMatrix);

	//Defaults
	minX = std::numeric_limits<GLfloat>::max();
	maxX = std::numeric_limits<GLfloat>::min();

	minY = std::numeric_limits<GLfloat>::max();
	maxY = std::numeric_limits<GLfloat>::min();

	minZ = std::numeric_limits<GLfloat>::max();
	maxZ = std::numeric_limits<GLfloat>::min();

	//get min/max for each point
	for (auto it = tris.begin(); it != tris.end(); it++) {
		glm::vec3& p = *it;

		minX = std::min(minX, p.x);
		maxX = std::max(maxX, p.x);

		minY = std::min(minY, p.y);
		maxY = std::max(maxY, p.y);

		minZ = std::min(minZ, p.z);
		maxZ = std::max(maxZ, p.z);
	}

	/* don't clamp -- allow invalid ranges
	//clamp these results to be within [-1,1]
	minX = std::max(minX, -1.0f);
	maxX = std::min(maxX, 1.0f);

	minY = std::max(minY, -1.0f);
	maxY = std::min(maxY, 1.0f);

	minZ = std::max(minZ, -1.0f);
	maxZ = std::min(maxZ, 1.0f);
	*/
}


//implements depth test as described by the Hasselgren et al. paper
//given bounding box of object in NDC space (after applying transformBoundingBox), return true if box is visible according to depth buffer
bool depthTest(GLfloat minX, GLfloat maxX, GLfloat minY, GLfloat maxY, GLfloat minZ, GLfloat maxZ) {
	//bounding rectangle points
	glm::vec2 minP(minX, minY);
	glm::vec2 maxP(maxX, maxY);

	//convert from NDC into pixel space
	convertVec(minP);
	convertVec(maxP);

	minX = minP.x;
	maxX = maxP.x;

	//these values need to be swapped as the axes differ between NDC/pixel space (sign of y axis directions)
	minY = maxP.y;
	maxY = minP.y;

	//block coordinates of blocks possibly overlapping the bounding box
	int iStart = std::max(((int)minY) / BLOCK_HEIGHT, 0);
	int iEnd = std::min((int)ceil(maxY / (GLfloat) BLOCK_HEIGHT), (int)dBuffer.heightB - 1);

	int jStart = std::max(((int)minX) / 32, 0); 
	int jEnd = std::min((int)ceil(maxX / 32.0f), (int)dBuffer.widthB - 1);

	for (int i = iStart; i <= iEnd; i++) { //iterate over height
		for (int j = jStart; j <= jEnd; j++) { //iterate over width
			Block& b = dBuffer.getBlock(j, i);

			if (b.reference >= minZ) {
				return true; //bounding box might be visible in this block -- so object is considered visible
			}
		}
	}
	return false;
}

/*	given triangle points in NDC space, render triangle into depth buffer and update depths as needed

	maxZ is z of triangle
*/
void renderIntoDepthBuffer(glm::vec2 t1, glm::vec2 t2, glm::vec2 t3, GLfloat maxZ) {
	fixTriangle(t1, t2, t3); //ensure points have different heights (to ensure downward facing edges can be made, and slopes are not infinite)

	//centroid
	GLfloat cx = (t1.x + t2.x + t3.x) / 3.0f;
	GLfloat cy = (t1.y + t2.y + t3.y) / 3.0f;
	glm::vec2 center(cx, cy);

	//sort points in order of decreasing height
	std::vector<glm::vec2*> ps;
	ps.push_back(&t1);
	ps.push_back(&t2);
	ps.push_back(&t3);
	std::sort(ps.begin(), ps.end(), triComp);

	glm::vec2& p1 = *ps[0];
	glm::vec2& p2 = *ps[1];
	glm::vec2& p3 = *ps[2];

	//downward-facing edge vectors
	glm::vec2 l1 = p2 - p1;
	glm::vec2 l2 = p3 - p1;
	glm::vec2 l3 = p3 - p2;

	//left-facing normals (because of the downward facing edges)
	glm::vec2 n1(l1.y, -l1.x);
	glm::vec2 n2(l2.y, -l2.x);
	glm::vec2 n3(l3.y, -l3.x);

	//true if left is outside
	bool o1 = glm::dot(n1, (center - p1)) < 0; //this is the point in triangle test, but instead of inward facing normals we use left facing normals
	bool o2 = glm::dot(n2, (center - p1)) < 0;
	bool o3 = glm::dot(n3, (center - p2)) < 0;
	uint32_t mask1 = o1 ? 0 : ~0;
	uint32_t mask2 = o2 ? 0 : ~0;
	uint32_t mask3 = o3 ? 0 : ~0;

	//extrapolate edges to top of the screen in NDC space (y = 1.0)
	glm::vec2 f1 = p1 + ((1.0f - p1.y) / l1.y) * l1;
	glm::vec2 f2 = p1 + ((1.0f - p1.y) / l2.y) * l2;
	glm::vec2 f3 = p2 + ((1.0f - p2.y) / l3.y) * l3;

	//convert extrapolated points, and triangle points, into pixel space
	convertVec(f1);
	convertVec(f2);
	convertVec(f3);
	convertVec(p1);
	convertVec(p2);
	convertVec(p3);

	//recompute lines in pixel space
	l1 = p2 - p1;
	l2 = p3 - p1;
	l3 = p3 - p2;
	// dx/dy slopes of triangle edges
	GLfloat s1 = l1.x / l1.y;
	GLfloat s2 = l2.x / l2.y;
	GLfloat s3 = l3.x / l3.y;


	//Find coordinates of blocks possibly overlapping triangle
	GLfloat minY = std::min(p1.y, std::min(p2.y, p3.y));
	GLfloat maxY = std::max(p1.y, std::max(p2.y, p3.y));
	GLfloat minX = std::min(p1.x, std::min(p2.x, p3.x));
	GLfloat maxX = std::max(p1.x, std::max(p2.x, p3.x));

	int iStart = std::max(((int)minY) / BLOCK_HEIGHT, 0);
	int iEnd = std::min(((int)maxY) / BLOCK_HEIGHT, (int)dBuffer.heightB - 1);

	int jStart = std::max(((int)minX) / 32, 0);
	int jEnd = std::min(((int)maxX) / 32, (int)dBuffer.widthB - 1);


	for (int i = iStart; i <= iEnd; i++) { //iterate over height
		int scanBase = i * BLOCK_HEIGHT; //height of top scanline within this block in pixel space

		//x coordinates of events for each scanline/triangle edge in pixel space
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

		for (int j = jStart; j <= jEnd; j++) { //iterate over width
			Block& b = dBuffer.getBlock(j, i);

			////////////////////////////This section is the depth buffer update from the Hasselgren et al. paper
			//zMax is the tri.maxZ in the paper, tile.zMax0 is b.reference, tile.zMax1 is b.working

			//heuristic to throw away working layer -- this is used in the paper to help prevent objects
			//in the background from leaking into the foreground
			GLfloat dist1t = b.working - maxZ;
			GLfloat dist01 = b.reference - b.working;
			if (dist1t > dist01) {
				b.working = 0.0f;
				for (int i = 0; i < BLOCK_HEIGHT; i++) {
					b.bits[i] = 0;
				}
			}

			//merge triangle into working layer
			b.working = std::max(b.working, maxZ); //this might move the working layer deeper -- and is why the heuristic above is used
			for (int k = 0; k < BLOCK_HEIGHT; k++) {
				//x coordinates of events relative to this block and scanline
				uint32_t e1 = std::max(0.0f, e1f[k] - j * 32.0f);
				uint32_t e2 = std::max(0.0f, e2f[k] - j * 32.0f);
				uint32_t e3 = std::max(0.0f, e3f[k] - j * 32.0f);

				uint32_t result = line(e1, e2, e3, mask1, mask2, mask3);
				b.bits[k] |= result;
			}

			//update reference layer if mask is full
			bool full = true;
			for (int i = 0; i < BLOCK_HEIGHT; i++) {
				full = (full && b.bits[i] == ~0);
			}
			if (full) {
				b.reference = std::min(b.reference, b.working); //I use the min instead of just assigning b.reference as in the paper -- this produces slightly better results
				b.working = 0.0f;
				for (int i = 0; i < BLOCK_HEIGHT; i++) {
					b.bits[i] = 0;
				}
			}
			/////////////////////////////////////
		}
	}
}

/* update depth buffer based on object m
*/
void updateDepthBuffer(const ModelCollection &m) {

	//transform and clip triangles with respect to the near plane
	std::vector<glm::vec3> transformed;
	transformPoints(m.occluderData, transformed, m.modelMatrix);

	for (auto it = transformed.begin(); it != transformed.end();) { //while there are still triangles in the buffer
		//get one triangle
		glm::vec3& p1 = *it++;
		glm::vec3& p2 = *it++;
		glm::vec3& p3 = *it++;

		//max depth of triangle
		GLfloat maxZ = std::max(p1.z, std::max(p2.z, p3.z));

		glm::vec2 t1(p1);
		glm::vec2 t2(p2);
		glm::vec2 t3(p3);

		//do rasterization/updates in depth buffer
		renderIntoDepthBuffer(t1, t2, t3, maxZ);
	}

}

/*		true if object is visible and should be drawn

	does bounding box visibility test, and if visible, will update the depth buffer using the occluder

	call this function in the renderer
*/
bool shouldDraw(const ModelCollection& m) {
	//Transform bounding box into bounding square
	GLfloat minX, maxX, minY, maxY, minZ, maxZ;
	transformBoundingBox(m, minX, maxX, minY, maxY, minZ, maxZ);

	//Depth test
	bool visible = depthTest(minX, maxX, minY, maxY, minZ, maxZ);
	if (visible) {
		updateDepthBuffer(m);
	}

	return visible;
}
