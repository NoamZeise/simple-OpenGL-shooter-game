#ifndef ENEMY_H
#define ENEMY_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gameObject.h"
#include "model.h"

class Enemy : public GameObject
{
public:
	Enemy(glm::vec3 position, Model* objectModel);
	void Update(float timeElapsed);
	void UpdateVelocity(glm::vec3 velocity);
	bool Colliding(glm::vec3 pos);

private:
	glm::vec3 velocity;
	float speed = 14.5f;
	float radius = 1.4f;
};


#endif
