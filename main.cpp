#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <random>
#include "shader.h"
#include "camera.h"
#include "model.h"
#include "chunk.h"
#include "gameObject.h"
#include "projectile.h"
#include "enemy.h"

float PreviousFrameTime = 0.0f;
float TimeElapsed = 0.0f;

Camera camera;
int ScreenWidth = 1600;
int ScreenHeight = 900;
static void error_callback(int error, const char* description);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

const float chunkWidth = 30.0f;
const float chunkHeight = 30.0f;
int numChunks;
int range;
std::vector<Chunk> chunks;
std::mt19937 mRandomGen;
glm::vec3 currentSquare(0.0f);

std::vector<Projectile> bullets;
float shotDelay = 0.1f;
float shotTimer = 0.1f;

std::vector<Enemy> enemies;
float enemyDelay = 5.0f;
float enemyTimer = 0.0f;

float danger = 0.0f;

Model groundMdl;
Model treeMdl;
Model bulletMdl;
Model enemyMdl;
Model skyModel;
glm::vec3 skyBoxColour = glm::vec3(91.0f / 255.0f, 110.0f / 255.0f, 225.0f / 255.0f);

void AddChunks();
void AddBullet();
void AddEnemies();

int main()
{
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
	{
		std::cout << "failed to initialise glfw" << std::endl;
		exit(EXIT_FAILURE);
	}
	//window creation will fail if minimum OpenGL version isn't met
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	GLFWwindow* window = glfwCreateWindow(1600, 900, "fpGame", NULL, NULL);
	if (!window)
	{
		std::cout << "glfw failed to create window" << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //gets current context and gives it to glad
	{
		std::cout << "failed to initialise GLAD" << std::endl;
		return -1;
	}
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glfwSwapInterval(1);

	//so that fragments behind other fragments in the world space are not drawn
	glEnable(GL_DEPTH_TEST);

	Shader objectShader("vShader.vert", "fShader.frag");
	camera.setScreenSize(ScreenWidth, ScreenHeight);

	std::random_device rd{};
	std::mt19937 engine{ rd() };
	mRandomGen = engine;
	range = (int)(camera.getRenderDistance() + 100.0f);
	numChunks = 3;


	groundMdl = Model("forest/ground.obj");
	treeMdl = Model("forest/tree.obj");
	bulletMdl = Model("forest/bullet.obj");
	enemyMdl = Model("forest/enemy.obj");
	skyModel = Model("forest/sky.obj");

	while (!glfwWindowShouldClose(window))
	{
		//main loop

		//update
		float currentFrame = (float)glfwGetTime();
		TimeElapsed = currentFrame - PreviousFrameTime;
		PreviousFrameTime = currentFrame;
		for (unsigned int i = 0; i < bullets.size(); i++)
		{
			bool collided = false;
			for (unsigned int j = 0; j < enemies.size(); j++)
			{
				if (enemies[j].Colliding(bullets[i].getPos()))
				{
					enemies.erase(enemies.begin() + j--);
					collided = true;
				}
			}
			if (collided)
			{
				bullets.erase(bullets.begin() + i--);
			}
		}
		danger = 0.0f;
		for (unsigned int i = 0; i < enemies.size(); i++)
		{
			if (glm::distance(enemies[i].getPos(), camera.getPos()) < 15.0f)
			{
				auto tempDanger =  1.0f - ((glm::distance(enemies[i].getPos(), camera.getPos()) + 1.0f) / 16.0f);
				if (tempDanger > danger)
					danger = tempDanger;
			}
			if (enemies[i].Colliding(camera.getPos()))
			{
				enemies.clear();
				bullets.clear();
				chunks.clear();
			}
		}

		camera.KeyHandler(window, TimeElapsed);
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			AddBullet();
		}
		if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
		{
			chunks.clear();
		}
		if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)
		{
			chunks.clear();
			enemies.clear();
			bullets.clear();
		}


		AddEnemies();

		//-----------------------------------------
		//draw
		glClearColor(0.2f, 0.2f, 0.22f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		objectShader.Use();

		glUniform3fv(objectShader.Location("viewPos"), 1, &camera.getPos()[0]);
		glUniform3fv(objectShader.Location("light.direction"), 1, &glm::vec3(-0.2f, -0.5f, -0.3f)[0]);
		//set shader view and projection matricies
		glm::mat4 view = camera.getViewMatrix();
		glUniformMatrix4fv(objectShader.Location("view"), 1, GL_FALSE, &view[0][0]);
		glm::mat4 projection = camera.getProjectionMatrix();
		glUniformMatrix4fv(objectShader.Location("projection"), 1, GL_FALSE, &projection[0][0]);
		auto dist = camera.getRenderDistance();
		glUniform1fv(objectShader.Location("renderDistance"), 1, &dist);

		auto currentPos = camera.getPos();

		glUniform3fv(objectShader.Location("fogColor"), 1, &glm::vec3(0.0f)[0]);
		glUniform3fv(objectShader.Location("light.ambient"), 1, &glm::vec3(0.7f - (danger / (10.0f/7.0f)))[0]);
		glUniform3fv(objectShader.Location("light.diffuse"), 1, &glm::vec3(0.0f)[0]);
		glUniform3fv(objectShader.Location("light.specular"), 1, &glm::vec3(0.0f)[0]);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, currentPos);
		model = glm::scale(model, glm::vec3(camera.getRenderDistance()));
		glUniformMatrix4fv(objectShader.Location("model"), 1, GL_FALSE, &model[0][0]);
		float shinX = 1.0f;
		glUniform1fv(objectShader.Location("shininess"), 1, &shinX);
		skyModel.Draw(objectShader);

		auto fogColour = (0.7f - (danger / (10.0f / 7.0f))) * skyBoxColour;
		glUniform3fv(objectShader.Location("fogColor"), 1, &fogColour[0]);
		glUniform3fv(objectShader.Location("light.ambient"), 1, &glm::vec3(0.2f + (danger / 10.0f), 0.2f - (danger / 5.0f), 0.2f - (danger / 5.0f))[0]);
		glUniform3fv(objectShader.Location("light.diffuse"), 1, &glm::vec3(0.5f - (danger / 4.0f), 0.5f - (danger / 2.0f), 0.5f - (danger / 2.0f))[0]);
		glUniform3fv(objectShader.Location("light.specular"), 1, &glm::vec3(1.0f - danger)[0]);

		std::vector<Chunk*> collidingChunks;
		for (unsigned int i = 0; i < chunks.size(); i++)
		{
			auto chunkPos = chunks[i].getPos();
			chunkPos.x -= chunkWidth / 2;
			chunkPos.z -= chunkHeight / 2;
			if (currentPos.x > chunkPos.x - 1.0f && currentPos.x < chunkPos.x + chunkWidth + 1.0f &&
				currentPos.z > chunkPos.z - 1.0f && currentPos.z < chunkPos.z + chunkHeight + 1.0f)
			{
				collidingChunks.push_back(&chunks[i]);
			}
			chunks[i].Draw(objectShader, camera);

			if (glm::distance(chunks[i].getPos(), currentPos) > range)
			{
				chunks.erase(chunks.begin() + i--);
			}
		}
		if (collidingChunks.size() == 0)
		{
			chunks.clear();
			currentSquare.x = camera.getPos().x;
			currentSquare.z = camera.getPos().z;
			AddChunks();
		}
		else
		{
			bool chunkFound = false;
			for (unsigned int i = 0; i < collidingChunks.size(); i++)
			{
				if (currentSquare.x == collidingChunks[i]->getPos().x && currentSquare.z == collidingChunks[i]->getPos().z)
				{
					chunkFound = true;
				}
			}
			if (!chunkFound)
			{
				currentSquare.x = collidingChunks[0]->getPos().x;
				currentSquare.z = collidingChunks[0]->getPos().z;
				AddChunks();
			}
		}


		shotTimer += TimeElapsed;
		for (unsigned int i = 0; i < bullets.size(); i++)
		{
			bullets[i].Update(TimeElapsed);
			bullets[i].Draw(objectShader, camera);

			if (glm::distance(bullets[i].getPos(), camera.getPos()) > range * 2)
			{
				bullets.erase(bullets.begin() + i--);
			}
		}
		enemyTimer += TimeElapsed;
		for (unsigned int i = 0; i < enemies.size(); i++)
		{
			enemies[i].Update(TimeElapsed);
			enemies[i].Draw(objectShader, camera);
			auto pos = camera.getPos();
			pos.y -= 0.3f;
			enemies[i].UpdateVelocity(glm::normalize(pos - enemies[i].getPos()));
		}
		//-------------------------------------
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	window = nullptr;
	glfwTerminate();

}

void AddChunks()
{
	//std::cout << "adding chunks" << std::endl;
	glm::vec3 currentPos = glm::vec3(camera.getPos());
	bool inSquare = false;
	for (int i = -numChunks; i <= numChunks; i++)
	{
		for (int j = -numChunks; j <= numChunks; j++)
		{
			auto x = currentSquare.x + (float)i * chunkWidth;
			auto z = currentSquare.z + (float)j * chunkHeight;

			//std::cout << "x: " << x << std::endl;
			//std::cout << "z: " << z << std::endl << std::endl;
			//std::cout << "dist: " << glm::distance(camera.getPos(), glm::vec3(x, 0.0f, z)) << std::endl << std::endl;
			bool chunkFound = false;
			for (unsigned int k = 0; k < chunks.size(); k++)
			{
				if (chunks[k].getPos().x == x && chunks[k].getPos().z == z)
					chunkFound = true;
			}
			if (!chunkFound)
			{
				//std::cout << "chunk added" << std::endl;
				chunks.push_back(Chunk(glm::vec3(x, 0.0f, z), chunkWidth, chunkHeight, &groundMdl, &treeMdl, &mRandomGen));
			}
		}
	}
}

void AddBullet()
{
	if (shotTimer > shotDelay)
	{
		bullets.push_back(Projectile(camera.getPos(), glm::normalize(camera.getFront()), &bulletMdl));
		shotTimer = 0.0f;
	}
}

void AddEnemies()
{
	if (enemyTimer > enemyDelay)
	{
		enemyTimer = 0;
		auto playerPos = camera.getPos();
		std::uniform_real_distribution<float> spawnDirection = std::uniform_real_distribution<float>(0.0f, 90.0f);
		std::uniform_real_distribution<float> spawnHeight = std::uniform_real_distribution<float>(0.1f, 10.0f);
		std::uniform_int_distribution<int> spawnQuadrant = std::uniform_int_distribution<int>(0, 1);
		auto direction = glm::vec3(spawnDirection(mRandomGen), spawnHeight(mRandomGen), spawnDirection(mRandomGen));
		direction.x = cos(glm::radians(direction.x));
		direction.z = sin(glm::radians(direction.z));
		if (spawnQuadrant(mRandomGen) == 0)
			direction.x *= -1;
		if (spawnQuadrant(mRandomGen) == 0)
			direction.z *= -1;
		direction.x *= (camera.getRenderDistance() + 10.0f);
		direction.z *= (camera.getRenderDistance() + 10.0f);
		direction.x += playerPos.x;
		direction.z += playerPos.z;
		//std::cout << "x: " << direction.x << std::endl;
		//std::cout << "z: " << direction.z << std::endl;
		enemies.push_back(Enemy(direction, &enemyMdl));
	}
}

static void error_callback(int error, const char* description)
{
	std::cout << stderr << "Error: %s\n" << description << std::endl;
}


static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	camera.CursorPosCallback(window, xpos, ypos, TimeElapsed);
}