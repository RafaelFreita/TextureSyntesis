#include "Shader.h"

Shader::~Shader()
{
}

GLint Shader::Attribute(const GLchar* name)
{
	return glGetAttribLocation(program, name);
}

GLint Shader::Uniform(const GLchar * name)
{
	return glGetUniformLocation(program, name);
}

Shader* Shader::currentShader = nullptr;