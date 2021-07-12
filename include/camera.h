#ifndef MCAMERA_H
#define MCAMERA_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class mCamera_t
{
private:

	glm::vec3 pos;
	glm::vec3 tgt;
	glm::vec3  up;
	glm::vec3 pyr;
	float     spd;
	float     mod;
	float     fov;

public:

	mCamera_t();

	void setSpd(float);
	void addMod(float);
	void setTgt(glm::vec3);
	void addPYR(glm::vec3);
	void adjFov(double, double);
	void update(GLFWwindow*, float);

	glm::mat4 makeViewMat() const;
	float getFov() const;
	glm::vec3 getPos() const;
	glm::vec3 getTgt() const;
};

#endif
