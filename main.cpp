#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <random>
#include <fstream>
#include <string>
#include <ctime>
#include <stdlib.h>
#include "shader.h"
#include "camera.h"
#include "model.h"
#include "gameObject.h"
#include "projectile.h"
#include "enemy.h"
#include "chunk.h"

static void error_callback(int error, const char* description);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

void saveHighscore(int& score, int& highscore);
void AddChunks(Camera& camera, std::vector<Chunk>& chunks, glm::vec3 currentSquare, int numChunks, float chunkWidth, float chunkHeight, Model& groundMdl, Model& treeMdl, std::mt19937& randomGen, std::uniform_real_distribution<float>& spawnXRange, std::uniform_real_distribution<float>& spawnZRange, std::uniform_int_distribution<int>& treeRange);
void AddProjectile(std::vector<Projectile>& projectiles, Camera& camera, Model& bulletMdl, float& shotTimer, float SHOT_DELAY);
void AddEnemies(std::vector<Enemy>& enemies, Model& enemyMdl, Camera& camera, std::mt19937& randomGen, float& enemyTimer, float enemyDelay, bool enemiesEnabled, std::uniform_real_distribution<float>& spawnDirection, std::uniform_real_distribution<float>& spawnHeight, std::uniform_int_distribution<int>& spawnQuadrant);

int main()
{
	float PreviousFrameTime = 0.0f;
	float TimeElapsed = 0.0f;

	Camera camera;
	int ScreenWidth = 1600;
	int ScreenHeight = 900;


	const float CHUNK_WIDTH = 30.0f;
	const float CHUNK_HEIGHT = 30.0f;
	const int MAX_TREES = 30;
	int numChunks;
	int range;
	std::vector<Chunk> chunks;
	std::mt19937 randomGen(time(0));
	std::uniform_real_distribution<float> spawnXRange = std::uniform_real_distribution<float>(-(CHUNK_WIDTH / 2), (CHUNK_WIDTH / 2));
	std::uniform_real_distribution<float> spawnZRange = std::uniform_real_distribution<float>(-(CHUNK_HEIGHT / 2), (CHUNK_HEIGHT / 2));
	std::uniform_int_distribution<int> treeRange = std::uniform_int_distribution<int>(0, MAX_TREES);
	glm::vec3 currentSquare(0.0f);

	std::vector<Projectile> projectiles;
	const float SHOT_DELAY = 0.1f;
	float shotTimer = 0.1f;

	std::vector<Enemy> enemies;
	bool enemiesEnabled = true;
	float enemyDelay = 6.0f;
	const float INITIAL_ENEMY_DELAY = 6.0f;
	float enemyTimer = 0.0f;
	const float DIFFICULTY_DELAY = 5.0f;
	float difficultyTimer = 0.0f;
	std::uniform_real_distribution<float> spawnDirection = std::uniform_real_distribution<float>(0.0f, 90.0f);
	std::uniform_real_distribution<float> spawnHeight = std::uniform_real_distribution<float>(0.1f, 10.0f);
	std::uniform_int_distribution<int> spawnQuadrant = std::uniform_int_distribution<int>(0, 1);

	int highscore = 0;
	int score = 0;

	const float DANGER_RANGE = 30.0f;
	float danger = 0.0f;

	Model groundMdl;
	Model treeMdl;
	Model bulletMdl;
	Model enemyMdl;
	Model skyModel;
	glm::vec3 skyBoxColour = glm::vec3(91.0f / 255.0f, 110.0f / 255.0f, 225.0f / 255.0f);


	std::ifstream loadHscore{ "highscore" };
	if (loadHscore)
	{
		std::string fileData;
		loadHscore >> fileData;
		errno = 0;
		char* endptr;
		const char* buffer = fileData.c_str();
		highscore = strtol(buffer, &endptr, 0);

		if (errno == ERANGE || endptr == buffer)
		{
			std::cout << "invalid highsore file" << std::endl;
			highscore = 0;
		}
	}
	loadHscore.close();

	std::cout << "\n\n\n\n\n\n\n\n\n\n\nHighscore: " << highscore << std::endl;

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
	randomGen = engine;
	range = (int)(camera.getRenderDistance() + 100.0f);
	numChunks = 3;


	groundMdl = Model("assets/ground.obj");
	treeMdl = Model("assets/tree.obj");
	bulletMdl = Model("assets/bullet.obj");
	enemyMdl = Model("assets/enemy.obj");
	skyModel = Model("assets/sky.obj");

	while (!glfwWindowShouldClose(window))
	{
		//main loop

		//update
		float currentFrame = (float)glfwGetTime();
		TimeElapsed = currentFrame - PreviousFrameTime;
		PreviousFrameTime = currentFrame;
		for (unsigned int i = 0; i < projectiles.size(); i++)
		{
			bool collided = false;
			for (unsigned int j = 0; j < enemies.size(); j++)
			{
				if (enemies[j].Colliding(projectiles[i].getPos()))
				{
					enemies.erase(enemies.begin() + j--);
					collided = true;
					score++;
					std::cout << "\n\n\n\n\n\n\n\n\n\n\nHighscore: " << highscore <<  "\nScore:     " << score << std::endl;
				}
			}
			if (collided)
			{
				projectiles.erase(projectiles.begin() + i--);
			}
		}
		danger = 0.0f;
		for (unsigned int i = 0; i < enemies.size(); i++)
		{
			if (glm::distance(enemies[i].getPos(), camera.getPos()) < DANGER_RANGE)
			{
				auto tempDanger =  1.0f - ((glm::distance(enemies[i].getPos(), camera.getPos()) + 1.0f) / DANGER_RANGE);
				if (tempDanger > danger)
					danger = tempDanger;
			}
			if (enemies[i].Colliding(camera.getPos()))
			{
				enemies.clear();
				projectiles.clear();
				chunks.clear();
				std::cout << "\n\n\n\n\n\n\n\n\n\n\nYOU DIED\nHighscore: " << highscore << "\nFinal Score: " << score << std::endl;
				if(score > highscore)
					highscore = score;
				score = 0;
				enemyDelay = INITIAL_ENEMY_DELAY;
			}
		}

		difficultyTimer += TimeElapsed;
		if (difficultyTimer > DIFFICULTY_DELAY)
		{
			difficultyTimer = 0.0f;
			enemyDelay -= 0.2f;
			if (enemyDelay < 1.0f)
				enemyDelay = 1.0f;
		}

		camera.KeyHandler(window, TimeElapsed);
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos , &yPos);
		camera.CursorPosCallback(window, xPos, yPos, TimeElapsed);
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			AddProjectile(projectiles, camera, bulletMdl, shotTimer, SHOT_DELAY);
		}
		if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
		{
			chunks.clear();
		}
		if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)
		{
			chunks.clear();
			enemies.clear();
			projectiles.clear();
		}


		AddEnemies(enemies, enemyMdl, camera, randomGen, enemyTimer, enemyDelay, enemiesEnabled, spawnDirection, spawnHeight, spawnQuadrant);

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
			chunkPos.x -= CHUNK_WIDTH / 2;
			chunkPos.z -= CHUNK_HEIGHT / 2;
			if (currentPos.x > chunkPos.x - 1.0f && currentPos.x < chunkPos.x + CHUNK_WIDTH + 1.0f &&
				currentPos.z > chunkPos.z - 1.0f && currentPos.z < chunkPos.z + CHUNK_HEIGHT + 1.0f)
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
			AddChunks(camera, chunks, currentSquare, numChunks, CHUNK_WIDTH, CHUNK_HEIGHT, groundMdl, treeMdl, randomGen, spawnXRange, spawnZRange, treeRange);
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
				AddChunks(camera, chunks, currentSquare, numChunks, CHUNK_WIDTH, CHUNK_HEIGHT, groundMdl, treeMdl, randomGen, spawnXRange, spawnZRange, treeRange);
			}
		}


		shotTimer += TimeElapsed;
		for (unsigned int i = 0; i < projectiles.size(); i++)
		{
			projectiles[i].Update(TimeElapsed);
			projectiles[i].Draw(objectShader, camera);

			if (glm::distance(projectiles[i].getPos(), camera.getPos()) > range * 2)
			{
				projectiles.erase(projectiles.begin() + i--);
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

	saveHighscore(score, highscore);
}

void saveHighscore(int& score, int& highscore)
{
	if (score > highscore)
		highscore = score;
	std::ofstream hscoreFile{ "highscore" };
	hscoreFile.clear();
	hscoreFile << highscore;
	hscoreFile.close();
}

void AddChunks(Camera& camera, std::vector<Chunk>& chunks, glm::vec3 currentSquare, int numChunks, float chunkWidth, float chunkHeight, Model& groundMdl, Model& treeMdl, std::mt19937& randomGen, std::uniform_real_distribution<float>& spawnXRange, std::uniform_real_distribution<float>& spawnZRange, std::uniform_int_distribution<int>& treeRange)
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
				chunks.push_back(Chunk(glm::vec3(x, 1.0f, z), chunkWidth, chunkHeight, &groundMdl, &treeMdl, randomGen, spawnXRange, spawnZRange, treeRange));
			}
		}
	}
}

void AddProjectile(std::vector<Projectile>& projectiles, Camera& camera, Model& bulletMdl, float& shotTimer, float SHOT_DELAY)
{
	if (shotTimer > SHOT_DELAY)
	{
		projectiles.push_back(Projectile(camera.getPos(), glm::normalize(camera.getFront()), &bulletMdl));
		shotTimer = 0.0f;
	}
}

void AddEnemies(std::vector<Enemy>& enemies, Model& enemyMdl ,Camera& camera, std::mt19937& randomGen, float& enemyTimer, float enemyDelay, bool enemiesEnabled, std::uniform_real_distribution<float>& spawnDirection, std::uniform_real_distribution<float>& spawnHeight, std::uniform_int_distribution<int>& spawnQuadrant)
{
	if (enemyTimer > enemyDelay && enemiesEnabled)
	{
		enemyTimer = 0;
		auto playerPos = camera.getPos();
		auto direction = glm::vec3(spawnDirection(randomGen), spawnHeight(randomGen), spawnDirection(randomGen));
		direction.x = cos(glm::radians(direction.x));
		direction.z = sin(glm::radians(direction.z));
		if (spawnQuadrant(randomGen) == 0)
			direction.x *= -1;
		if (spawnQuadrant(randomGen) == 0)
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
	
}