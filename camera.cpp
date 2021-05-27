#include "camera.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Camera::Camera()
{
	setScreenSize(1600, 900);
}

Camera::Camera(int width, int height)
{
	setScreenSize(width, height);
}

Camera::~Camera()
{

}

void Camera::KeyHandler(GLFWwindow* window, float timeElapsed)
{
	float velocity = speed * timeElapsed;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		velocity *= 2.0f;
	//horizontalFront.y = 0.0f;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		position += horizontalFront * velocity;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		position -= glm::normalize(glm::cross(horizontalFront, up)) * velocity;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		position -= horizontalFront * velocity;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		position += glm::normalize(glm::cross(horizontalFront, up)) * velocity;

	if (position.y != 3.0f)
		position.y = 3.0f;
}

void Camera::CursorPosCallback(GLFWwindow* window, double xpos, double ypos, float timeElapsed)
{
	if (firstMouseUpdate)
	{
		lastMouseX = xpos;
		lastMouseY = ypos;
		firstMouseUpdate = false;
	}

	double dX = xpos - lastMouseX;
	double dY = lastMouseY - ypos;
	lastMouseX = xpos;
	lastMouseY = ypos;
	dX *= sensitivity;
	dY *= sensitivity;

	yaw += (float)dX;
	pitch += (float)dY;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(direction);
	direction.y = 0.0f;
	horizontalFront = glm::normalize(direction);
}

void Camera::setScreenSize(int width, int height)
{
	screenWidth = width;
	screenHeight = height;
}

glm::mat4 Camera::getViewMatrix()
{
	view = glm::lookAt(position, position + front, up);
	return view;
}

glm::mat4 Camera::getProjectionMatrix()
{
	projection = glm::perspective(glm::radians(fov), (float)screenWidth / (float)screenHeight, 0.1f, renderDistance);
	return projection;
}


glm::vec3 Camera::getPos()
{
	return position;
}
glm::vec3 Camera::getFront()
{
	return front;
}

float Camera::getRenderDistance()
{
	return renderDistance;
}

float Camera::angleToCamera(glm::vec3 targetPos)
{
	glm::vec3 dirToCamera = glm::normalize(position - targetPos);
	auto angle = atan2(front.z, front.x) - atan2(dirToCamera.z, dirToCamera.x);

	if (angle < 0)
		angle += glm::radians(360.0f);
	return angle - glm::radians(180.0f);
}
bool Camera::inFov(glm::vec3 targetPos, float size)
{
	auto angle = angleToCamera(targetPos);
	return (angle < glm::radians(fov) && angle > glm::radians(-fov)) || glm::distance(targetPos, position) < size;
}

bool Camera::inView(glm::vec3 targetPos, float size)
{
	return (glm::distance(position, targetPos) < renderDistance + size);
}