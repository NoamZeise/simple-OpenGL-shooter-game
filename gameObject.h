#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gameObject.h"
#include "model.h"
#include "camera.h"

class GameObject
{
public:
	GameObject(glm::vec3 postion, Model* objectModel);
	~GameObject();

	void Draw(Shader& shader, Camera& camera);
	void Update(float timeElapsed);

	glm::vec3 getPos();
	bool isRemoved = false;

protected:
	glm::vec3 position;
	float shininess = 20.0f;
private:
	Model* objectModel;
};





#endif
