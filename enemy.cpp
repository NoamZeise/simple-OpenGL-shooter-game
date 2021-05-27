#include "enemy.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gameObject.h"
#include "model.h"

Enemy::Enemy(glm::vec3 position, Model* objectModel) : GameObject(position, objectModel)
{
	this->velocity = velocity;
}


void Enemy::Update(float timeElapsed)
{
	if (position.y < 0.0f)
	{
		position.y = 0.0f;
		velocity.y *= -1.0f;
	}
	position += speed * velocity * timeElapsed;
}

void Enemy::UpdateVelocity(glm::vec3 velocity)
{
	this->velocity = velocity;
}

bool Enemy::Colliding(glm::vec3 pos)
{
	return glm::distance(pos, position) < radius;
}