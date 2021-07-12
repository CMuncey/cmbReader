#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <stdint.h>
#include "glm/glm.hpp"
#include "shader.h"
#include "cmb.h"
using namespace std;

struct cmbMaterial_t
{
	uint32_t   tex0;
	uint32_t   tex1;
	uint32_t   tex2;
	glm::vec3 ambiC;
	glm::vec3 diffC;
	glm::vec3 specC;
	glm::vec3 emisC;
	float     shiny;
};

struct cmbMesh_t
{
	unsigned int hasFCol, hasTex0;
	unsigned int hasTex1, hasTex2;
	unsigned int     VAO, VBOs[8];
	unsigned int     EBO, TEXs[3];
	cmbMaterial_t             mat;
}

makeCmbMesh(cmbMesh_t*, int, const cmb_t*, const glm::mat4*);

/*
class mMesh_t
{
private:

	unsigned int hasFCol, VAO, EBO, VBOs[8], TEXs[3];
	unsigned int   numInd, hasTex0, hasTex1, hasTex2;
	mMaterial_t                             material;
	void makeMesh();

public:

	mMesh_t() {};
	mMesh_t(const cmb_t*, int, vector<glm::mat4>);

	void draw(mShader_t&);
	void del();
};
*/

#endif
