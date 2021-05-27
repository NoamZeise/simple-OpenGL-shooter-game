#ifndef BLT_H
#define BLT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gameObject.h"
#include "model.h"

class Projectile : public GameObject
{
public:
	Projectile(glm::vec3 position, glm::vec3 velocity, Model* objectModel);
	void Update(float timeElapsed);

private:
	glm::vec3 velocity;
	float speed = 40.0f;
};


#endif
