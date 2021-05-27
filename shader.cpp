#include "shader.h"

#include <glad/glad.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

Shader::Shader(const char* VertexShaderPath, const char* FragmentShaderPath)
{
	//create shader
	unsigned int vShader, fShader;

	vShader = compileShader(VertexShaderPath, false);
	fShader = compileShader(FragmentShaderPath, true);
	
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vShader);
	glAttachShader(shaderProgram, fShader);
	glLinkProgram(shaderProgram);

	int isLinked = 0;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isLinked);
	if (!isLinked)
	{
		int logSize = 0;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logSize);

		char* errorLog = new char[logSize];
		glGetShaderInfoLog(shaderProgram, logSize, &logSize, errorLog);

		std::cout << "failed to link shader program\n" << errorLog << std::endl;

		delete[] errorLog;
		errorLog = nullptr;
		glDeleteProgram(shaderProgram);
	}
	glDetachShader(shaderProgram, vShader);
	glDetachShader(shaderProgram, fShader);
	glDeleteShader(vShader);
	glDeleteShader(fShader);
}

Shader::~Shader()
{
	glDeleteProgram(shaderProgram);
}

unsigned int Shader::compileShader(const char* path, bool isFragmentShader)
{
	std::string dir = path;
	unsigned int shader;
	if(isFragmentShader)
		shader = glCreateShader(GL_FRAGMENT_SHADER);
	else
		shader = glCreateShader(GL_VERTEX_SHADER);

	//load shader source file into string
	std::ifstream in(path);
	std::string shaderSource((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	const char* source = shaderSource.c_str();
	glShaderSource(shader, 1, &source, NULL);

	glCompileShader(shader);

	int isCompiled;
	char infoLog[512];

	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (!isCompiled)
	{
		int logSize = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

		char* errorLog = new char[logSize];
		glGetShaderInfoLog(shader, logSize, &logSize, errorLog);

		std::cout << "failed to compile shader " << path << "\n" << errorLog << std::endl;

		delete[] errorLog;
		errorLog = nullptr;
		glDeleteShader(shader);
	}

	return shader;
}

void Shader::Use()
{
	glUseProgram(shaderProgram);
}

unsigned int Shader::Location(const std::string &uniformName) const
{
	return glGetUniformLocation(shaderProgram, uniformName.c_str());
}