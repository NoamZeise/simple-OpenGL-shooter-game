#include "gameObject.h"

#include "model.h"
#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

GameObject::GameObject(glm::vec3 position, Model* objectModel)
{
	this->position = position;
	this->objectModel = objectModel;
}

GameObject::~GameObject()
{
	objectModel = nullptr;
}


void GameObject::Update(float timeElapsed)
{

}

void GameObject::Draw(Shader& shader, Camera& camera)
{
	if (camera.inFov(position, 2.0f) && camera.inView(position, 2.0f))
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, position);
		glUniformMatrix4fv(shader.Location("model"), 1, GL_FALSE, &model[0][0]);
		glUniform1fv(shader.Location("shininess"), 1, &shininess);
		objectModel->Draw(shader);
	}
}

glm::vec3 GameObject::getPos()
{
	return position;
}