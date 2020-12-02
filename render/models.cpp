#include "models.h"
#include "utility.h"
#include <fstream>

Model::Model() {
}

Model::Model(const Model &m) {
	this->varr = m.varr;
	this->posBuff = m.posBuff;
	this->colBuff = m.colBuff;
	this->nVerts = m.nVerts;
}



ModelCollection::ModelCollection() {
}


ModelCollection::ModelCollection(const ModelCollection &m) {
	this->main = m.main;
	this->occluder = m.occluder;
	this->box = m.box;
	this->occluderData = m.occluderData;
	this->boxData = m.boxData;
	this->modelMatrix = m.modelMatrix;
	this->marker = m.marker;
}


Model box;
Model plant;
Model cube;
ModelCollection office;

ModelCollection parseModelCollection(std::string mainFileName, GLfloat r, GLfloat g, GLfloat b, std::string occluderFileName, std::string boxFileName, std::string markerFileName) {
	ModelCollection m;

	m.main = parseObj(mainFileName, r, g, b);
	m.occluder = parseObj(occluderFileName, 1.0f, 0.0f, 0.0f, m.occluderData);
	m.box = parseObj(boxFileName, 1.0f, 1.0f, 0.0f, m.boxData);
	m.boxCenter = modelDataCenter(m.boxData);
	m.marker = parseObj(markerFileName, 0.0f, 1.0f, 1.0f);

	return m;
}

Model parseObj(std::string fileName, GLfloat r, GLfloat g, GLfloat b, std::vector<GLfloat> &posData){
	posData.clear();

	std::string line;
	std::vector<std::string> tokens;
	std::vector<std::vector<std::string>> subtokens;

	std::vector<GLfloat> vertices;
	std::vector<GLfloat> normals;

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

	return glm::vec3((minX + maxX) / 2.0f, (minY + maxY) / 2.0f, (minZ + maxZ) / 2.0f);
}
