#include "chunk.h"

#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <random>
#include <iostream>
#include "camera.h"


Chunk::Chunk(glm::vec3 position, float chunkWidth, float chunkHeight, Model* ground, Model* tree, std::mt19937* mRandomGen)
{
	this->ground = ground;
	this->tree = tree;
	this->chunkWidth = chunkWidth;
	this->chunkHeight = chunkHeight;
	this->position = position;

	std::uniform_real_distribution<float> spawnXRange = std::uniform_real_distribution<float>(-(chunkWidth / 2), (chunkWidth / 2));
	std::uniform_real_distribution<float> spawnZRange = std::uniform_real_distribution<float>(-(chunkHeight / 2), (chunkHeight / 2));
	std::uniform_int_distribution<int> treeRange = std::uniform_int_distribution<int>(0, maxTrees);

	unsigned int numTrees = treeRange(*mRandomGen);
	for (unsigned int i = 0; i < numTrees; i++)
	{
		glm::vec3 pos = position;
		pos.x += spawnXRange(*mRandomGen);
		pos.z += spawnZRange(*mRandomGen);
		treePositions.push_back(pos);
	}
}
Chunk::~Chunk()
{
	treePositions.~vector();
	tree = nullptr;
	ground = nullptr;
}


void Chunk::Draw(Shader& shader, Camera& camera)
{
	if (camera.inView(position, chunkWidth))
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, position);
		glUniformMatrix4fv(shader.Location("model"), 1, GL_FALSE, &model[0][0]);
		glUniform1fv(shader.Location("shininess"), 1, &groundShininess);
		ground->Draw(shader);
	}
	glUniform1fv(shader.Location("shininess"), 1, &treeShininess);
	for (unsigned int i = 0; i < treePositions.size(); i++)
	{
		if (camera.inFov(treePositions[i], 10.0f) && camera.inView(treePositions[i], 10.0f))
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, treePositions[i]);
			glUniformMatrix4fv(shader.Location("model"), 1, GL_FALSE, &model[0][0]);
			tree->Draw(shader);
		}
	}
}

glm::vec3 Chunk::getPos()
{
	return position;
}