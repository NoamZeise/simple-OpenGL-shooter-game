#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class Shader
{
public:
	Shader(const char* VertexShaderPath, const char* FragmentShaderPath);
	~Shader();
	void Use();
	unsigned int Location(const std::string& uniformName) const;
	
private:
	unsigned int shaderProgram;
	unsigned int compileShader(const char* path, bool isFragmentShader);
};


#endif // !SHADER_H
