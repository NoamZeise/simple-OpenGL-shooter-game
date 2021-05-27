#ifndef CHUNK_H
#define CHUNK_H

#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <random>
#include "model.h"
#include "camera.h"

class Chunk
{
public:
	Chunk(glm::vec3 position, float chunkWidth, float chunkHeight, Model* ground, Model* tree, std::mt19937& randomGen, std::uniform_real_distribution<float>& spawnXRange, std::uniform_real_distribution<float>& spawnZRange, std::uniform_int_distribution<int>& treeRange);
	~Chunk();
	void Draw(Shader& shader, Camera& camera);
	glm::vec3 getPos();
	bool isRemoved = false;
private:
	glm::vec3 position;
	std::vector<glm::vec3> treePositions;
	float chunkWidth, chunkHeight;
	Model* ground, *tree;
	float treeShininess = 5.0f;
	float groundShininess = 10.0f;
};





#endif