#include "models.h"
#include "utility.h"
#include <fstream>

Model cube; //small cube used to show where the light source is

//empty Model constructor
Model::Model() {
}

//copy Model constructor
Model::Model(const Model &m) {
	this->varr = m.varr;
	this->posBuff = m.posBuff;
	this->colBuff = m.colBuff;
	this->nVerts = m.nVerts;
}

//ModelCollection constructor
ModelCollection::ModelCollection() {
	lastSorted = 0;
	dist2ToCamera = 0;
}

//copy ModelCollection constructor
ModelCollection::ModelCollection(const ModelCollection &m) {
	this->main = m.main;
	this->occluder = m.occluder;
	this->box = m.box;
	this->occluderData = m.occluderData;
	this->boxData = m.boxData;
	this->modelMatrix = m.modelMatrix;
	this->marker = m.marker;
	this->lastSorted = m.lastSorted;
	this->transformedCenter = m.transformedCenter;
	this->dist2ToCamera = m.dist2ToCamera;
}

//parse one obj file into a Model -- see header for details
Model parseObj(std::string fileName, GLfloat r, GLfloat g, GLfloat b, std::vector<GLfloat> &posData){
	posData.clear();

	std::string line;
	std::vector<std::string> tokens;
	std::vector<std::vector<std::string>> subtokens; //for numbers within each face element

	std::vector<GLfloat> vertices; //vertex locations specified by v elements
	std::vector<GLfloat> normals; //normal data specified by vn elements

	std::vector<GLfloat> normalData; //normal data to put in GL buffer
	std::vector<GLfloat> colorData; //color data to put in GL buffer

	std::fstream f(fileName); //open file and read all of the v and vn elements first
	while (std::getline(f, line)) {
		tokens = split(line, " ");
		if (tokens.empty()) {
			continue;
		}

		if (tokens[0] == "v") { //vertex element
			GLfloat x = std::stof(tokens[1]);
			GLfloat y = std::stof(tokens[2]);
			GLfloat z = std::stof(tokens[3]);
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
		} else if (tokens[0] == "vn") { //vertex normal element
			GLfloat x = std::stof(tokens[1]);
			GLfloat y = std::stof(tokens[2]);
			GLfloat z = std::stof(tokens[3]);
			normals.push_back(x);
			normals.push_back(y);
			normals.push_back(z);
		}
	}
	f.close(); //close file

	f = std::fstream(fileName); //open file again and read all face elements
	while (std::getline(f, line)) {
		tokens = split(line, " ");
		if (tokens.empty()) {
			continue;
		}

		if (tokens[0] == "f") { //face element
			subtokens.clear();

			for (size_t j = 1; j < tokens.size(); j++) {
				//for:
				//f 1/2/3 7/4/8 ...
				//subtokens are vectors of ["1", "2", "3"] and ["7", "4", "8"], etc...
				subtokens.push_back(split(tokens[j], "/"));
			}

			//turn face into triangles (a triangle has vertices (0, j + 1, j + 2))
			for (size_t j = 0; j < subtokens.size() - 2; j++) { //j + 1 is the first vertex to use that isn't vertex 0
				for (size_t k = 0; k < 3; k++) { //vertex of this triangle
					int vi; //vertex index
					int vni; //vertex normal index

					if (k == 0) { //use first vertex of face
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

	//Now load the model data into GL buffers
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


//parse multiple obj files into a ModelCollection representing one object -- see header for details
ModelCollection parseModelCollection(std::string mainFileName, GLfloat r, GLfloat g, GLfloat b, std::string occluderFileName, std::string boxFileName, std::string markerFileName) {
	ModelCollection m;

	m.main = parseObj(mainFileName, r, g, b);
	m.occluder = parseObj(occluderFileName, 1.0f, 0.0f, 0.0f, m.occluderData); //save occluder data for depth buffer updates
	m.box = parseObj(boxFileName, 1.0f, 1.0f, 0.0f, m.boxData); //save bounding box data for depth buffer tests
	m.boxCenter = modelDataCenter(m.boxData); //get center of bounding box for sorting objects by depth later
	m.marker = parseObj(markerFileName, 0.0f, 1.0f, 1.0f); //marker to show object in scene (to illustrate occlusion effect)

	return m;
}

//return center of position data (format: x, y, z ...)
glm::vec3 modelDataCenter(const std::vector<GLfloat> &data) {
	GLfloat minX = data[0];
	GLfloat maxX = minX;
	GLfloat minY = data[1];
	GLfloat maxY = minY;
	GLfloat minZ = data[2];
	GLfloat maxZ = minZ;

	for (auto it = data.begin(); it != data.end();) {
		minX = std::min(minX, *it);
		maxX = std::max(maxX, *it);
		it++;

		minY = std::min(minY, *it);
		maxY = std::max(maxY, *it);
		it++;

		minZ = std::min(minZ, *it);
		maxZ = std::max(maxZ, *it);
		it++;
	}

	return glm::vec3((minX + maxX) / 2.0f, (minY + maxY) / 2.0f, (minZ + maxZ) / 2.0f); //mean coordinates
}
