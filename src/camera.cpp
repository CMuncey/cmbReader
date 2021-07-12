#include "camera.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
using namespace std;

mCamera_t::mCamera_t()
{
	pos = glm::vec3(0.0f, 0.0f, 5.0f);
	tgt = glm::vec3(0.0f, 0.0f, 0.0f);
	up  = glm::vec3(0.0f, 1.0f, 0.0f);
	pyr = glm::vec3(0.0f, -90.0f, 0.0f);
	spd = mod = 1.0f;
	fov = 45.0f;
}

void mCamera_t::setSpd(float s)
{
	spd = s;
}

void mCamera_t::addMod(float m)
{
	mod += m;
}

void mCamera_t::setTgt(glm::vec3 a)
{
	tgt = glm::normalize(a);
}

void mCamera_t::addPYR(glm::vec3 a)
{
	glm::vec3 dir;

	/* Add the offsets to pitch/yaw/roll */
	pyr += a;

	/* Constrain up/down */
	if(pyr.x > 89.0f)
		pyr.x = 89.0f;
	if(pyr.x < -89.0f)
		pyr.x = -89.0f;

	/* Calculate direction and new target */
	dir.x = cos(glm::radians(pyr.x)) * cos(glm::radians(pyr.y));
	dir.y = sin(glm::radians(pyr.x));
	dir.z = cos(glm::radians(pyr.x)) * sin(glm::radians(pyr.y));
	tgt   = glm::normalize(dir);
}

void mCamera_t::adjFov(double x, double y)
{
	/* Calculate new fov */
	fov -= (float)y;
	if(fov < 1.0f)
		fov = 1.0f;
	if(fov > 45.0f)
		fov = 45.0f;
}

void mCamera_t::update(GLFWwindow* w, float dTime)
{
	float rSpd;

	rSpd = spd * mod * dTime;
	if(glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS)
		pos += rSpd * tgt;
	if(glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS)
		pos -= rSpd * tgt;
	if(glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS)
		pos -= rSpd * glm::normalize(glm::cross(tgt, up));
	if(glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS)
		pos += rSpd * glm::normalize(glm::cross(tgt, up));
	if(glfwGetKey(w, GLFW_KEY_SPACE) == GLFW_PRESS)
		pos += rSpd * up;
	if(glfwGetKey(w, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		pos -= rSpd * up;
}

glm::mat4 mCamera_t::makeViewMat() const
{
	return(glm::lookAt(pos, pos + tgt, up));
}

float mCamera_t::getFov() const
{
	return(fov);
}

glm::vec3 mCamera_t::getPos() const
{
	return(pos);
}

glm::vec3 mCamera_t::getTgt() const
{
	return(tgt);
}
