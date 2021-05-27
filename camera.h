#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class Camera
{
public:
	Camera();
	Camera(int width, int height);
	~Camera();
	void KeyHandler(GLFWwindow* window, float timeElapsed);
	void CursorPosCallback(GLFWwindow* window, double xpos, double ypos, float timeElapsed);
	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();
	glm::vec3 getPos();
	glm::vec3 getFront();
	float angleToCamera(glm::vec3 targetPos);
	bool inFov(glm::vec3 targetPos, float size);
	bool inView(glm::vec3 targetPos, float size);
	float getRenderDistance();
	void setScreenSize(int width, int height);
private:
	int screenWidth;
	int screenHeight;

	float fov = 55.0f;
	float pitch = -32.0f;
	float yaw = 0.0f;
	float speed = 7.0f;
	float renderDistance = 70.0f;
	glm::vec3 position = glm::vec3(0.0f, 3.0f, 0.0f);
	glm::vec3 front = glm::vec3(0.0f, 0.0f, -7.0f);
	glm::vec3 horizontalFront = glm::vec3(0.0f, 0.0f, -7.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	bool firstMouseUpdate = true;
	double sensitivity = 0.05f;
	double lastMouseX = 200, lastMouseY = 300;
};



#endif