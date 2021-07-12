#ifndef SHADER_H
#define SHADER_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
using namespace std;

class cmbShader_t
{
private:
	unsigned int id;

public:
	cmbShader_t() {};
	cmbShader_t(const char*, const char*);

	void use();
	void del();

	unsigned int loc(const char*) const;

	void set1i(const char*, int) const;
	void set2i(const char*, int, int) const;
	void set2i(const char*, const glm::ivec2&) const;
	void set3i(const char*, int, int, int) const;
	void set3i(const char*, const glm::ivec3&) const;

	void set1f(const char*, float) const;
	void set2f(const char*, float, float) const;
	void set2f(const char*, const glm::vec2&) const;
	void set3f(const char*, float, float, float) const;
	void set3f(const char*, const glm::vec3&) const;
	void set4f(const char*, const glm::vec4&) const;

	void set3mf(const char*, const glm::mat3&) const;
	void set4mf(const char*, const glm::mat4&) const;
};

/*
extern const char* cmbVS;
extern const char* cmbFSHead;
extern const char* cmbFSEnd;
*/

#endif
