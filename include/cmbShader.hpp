#ifndef SHADER_H
#define SHADER_H

#include <string>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "cmb.h"
#include "matsChunk.h"
using namespace std;

class cmbShader_t
{
private:
	unsigned int id;

	/* Pica200 doesn't have fragment shaders */
	/* only texture combiners, so this emulates */
	/* them by making a fragment shader for every */
	/* material. This is faster than one generic */
	/* fragment shader, and passing the TCs to it */ 
	string makeTCSrc(uint16_t);
	string makeTCOp(uint16_t, uint16_t);
	string makeTCCombine(uint16_t);
	string makeTCScale(uint16_t, uint16_t);
	string makeTCBuf(uint16_t);
	string makeTC(texCombiner_t*);
	string makeAlphaTest(uint16_t, uint8_t);
	string makeFS(const cmb_t*, int);

public:
	cmbShader_t() {};
	cmbShader_t(const cmb_t*, int);

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

#endif
