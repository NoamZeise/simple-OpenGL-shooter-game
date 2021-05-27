#include "projectile.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gameObject.h"
#include "model.h"

Projectile::Projectile(glm::vec3 position, glm::vec3 velocity, Model* objectModel) : GameObject(position, objectModel)
{
	this->position.y -= 0.5f;
	this->velocity = velocity;
}


void Projectile::Update(float timeElapsed)
{
	if (position.y < 0.0f)
	{
		position.y = 0.0f;
		velocity.y *= -1.0f;
	}
	position += speed * velocity * timeElapsed;
}