#include "cmbShader.h"
#include <string>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
using namespace std;

cmbShader_t::cmbShader_t(const char* vfn, const char* ffn)
{
	const char* vCode;
	const char* fCode;
	ifstream vFile, fFile;
	stringstream vStr, fStr;
	unsigned int vSh, fSh;
	char iLog[512];
	string dumb;
	int tmp;

	/* Read the files into the stringstreams */
	vFile.exceptions(ifstream::failbit | ifstream::badbit);
	fFile.exceptions(ifstream::failbit | ifstream::badbit);
	try
	{
		vFile.open(vfn);
		fFile.open(ffn);
		vStr << vFile.rdbuf();
		fStr << fFile.rdbuf();
		vFile.close();
		fFile.close();
	}
	catch(ifstream::failure e)
	{
		fprintf(stderr, "Could not read shaders\n");
	}

	/* Compile the vertex shader */
	dumb = vStr.str();
	vCode = dumb.c_str();
	vSh = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vSh, 1, &vCode, NULL);
	glCompileShader(vSh);
	glGetShaderiv(vSh, GL_COMPILE_STATUS, &tmp);
	if(!tmp)
	{
		glGetShaderInfoLog(vSh, 512, NULL, iLog);
		fprintf(stderr, "vShader compilation failed\n%s\n", iLog);
	}

	/* Compile the fragment shader */
	dumb = fStr.str();
	fCode = dumb.c_str();
	fSh = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fSh, 1, &fCode, NULL);
	glCompileShader(fSh);
	glGetShaderiv(fSh, GL_COMPILE_STATUS, &tmp);
	if(!tmp)
	{
		glGetShaderInfoLog(fSh, 512, NULL, iLog);
		fprintf(stderr, "fShader compilation failed\n%s\n", iLog);
	}

	/* Link the shaders */
	id = glCreateProgram();
	glAttachShader(id, vSh);
	glAttachShader(id, fSh);
	glLinkProgram(id);
	glGetProgramiv(id, GL_LINK_STATUS, &tmp);
	if(!tmp)
	{
		glGetProgramInfoLog(id, 512, NULL, iLog);
		fprintf(stderr, "Linking failed\n%s\n", iLog);
	}

	glDeleteShader(vSh);
	glDeleteShader(fSh);
}

void cmbShader_t::use()
{
	glUseProgram(id);
}

void cmbShader_t::del()
{
	glDeleteShader(id);
}

unsigned int cmbShader_t::loc(const char* n) const
{
	return(glGetUniformLocation(id, n));
}

void cmbShader_t::set1i(const char* n, int v) const
{
	glUniform1i(glGetUniformLocation(id, n), v);
}

void cmbShader_t::set2i(const char* n, int v1, int v2) const
{
	glUniform2i(glGetUniformLocation(id, n), v1, v2);
}

void cmbShader_t::set2i(const char* n, const glm::ivec2 &v) const
{
	glUniform2iv(glGetUniformLocation(id, n), 1, &v[0]);
}

void cmbShader_t::set3i(const char* n, int v1, int v2, int v3) const
{
	glUniform3i(glGetUniformLocation(id, n), v1, v2, v3);
}

void cmbShader_t::set3i(const char* n, const glm::ivec3 &v) const
{
	glUniform3iv(glGetUniformLocation(id, n), 1, &v[0]);
}

void cmbShader_t::set1f(const char* n, float v) const
{
	glUniform1f(glGetUniformLocation(id, n), v);
}

void cmbShader_t::set2f(const char* n, float v1, float v2) const
{
	glUniform2f(glGetUniformLocation(id, n), v1, v2);
}

void cmbShader_t::set2f(const char* n, const glm::vec2 &v) const
{
	glUniform2fv(glGetUniformLocation(id, n), 1, &v[0]);
}

void cmbShader_t::set3f(const char* n, float v1, float v2, float v3) const
{
	glUniform3f(glGetUniformLocation(id, n), v1, v2, v3);
}

void cmbShader_t::set3f(const char* n, const glm::vec3 &v) const
{
	glUniform3fv(glGetUniformLocation(id, n), 1, &v[0]);
}

void cmbShader_t::set4f(const char* n, const glm::vec4 &v) const
{
	glUniform4fv(glGetUniformLocation(id, n), 1, &v[0]);
}

void cmbShader_t::set3mf(const char* n, const glm::mat3 &v) const
{
	unsigned int loc;

	loc = glGetUniformLocation(id, n);
	glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(v));
}

void cmbShader_t::set4mf(const char* n, const glm::mat4 &v) const
{
	unsigned int loc;

	loc = glGetUniformLocation(id, n);
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(v));
}
