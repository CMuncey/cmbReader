#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include <signal.h>

/* My stuff */
#include "cmbShader.h"
#include "camera.h"
#include "cmb.h"
#include "sklmChunk.h"
#include "cmbModel.h"

/* Math library for matrix/vectors */
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define SWAP_INTERVAL 1
#define WINDOW_NAME "Test"
#define WINDOW_X 1280
#define WINDOW_Y 720
#define CAM_SPEED 2.5f
#define SENSITIVITY 0.1f
#define NEAR_PLANE 0.1f
#define FAR_PLANE 200.0f

cmbModel_t model;
const char* windowName;
char wf, fm, ce;
float pX, pY, winX, winY, scale, gammaC;
mCamera_t cam;
glm::mat4 proj;

/* {{{1 User input */
void framebuffer_size_callback(GLFWwindow* w, int x, int y)
{
	winX = x, winY = y;
	glViewport(0, 0, winX, winY);
	proj = glm::perspective(glm::radians(cam.getFov()), winX / winY, NEAR_PLANE, FAR_PLANE);
}

void key_callback(GLFWwindow* w, int key, int scancode, int action, int mods)
{
	int tmp;

	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(w, 1);
	if(key == GLFW_KEY_TAB && action == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, (wf) ? GL_FILL : GL_LINE), wf = !wf;
	if(key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
		cam.addMod(1.0f);
	if(key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
		cam.addMod(-1.0f);
	if(key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		tmp = (ce) ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
		glfwSetInputMode(w, GLFW_CURSOR, tmp), ce = !ce, fm = 1;
	}
	if(key == GLFW_KEY_UP && action == GLFW_PRESS)
		scale += 0.001;
	if(key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		scale -= 0.001;
	if(key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		if(model.meshNum > -1)
			model.meshNum--;
	if(key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		if(model.meshNum < model.nMeshes-1)
			model.meshNum++;
	if(key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS)
		gammaC -= 0.01;
	if(key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS)
		gammaC += 0.01;
	if(key == GLFW_KEY_BACKSLASH && action == GLFW_PRESS)
		printf("Gamma correction: %4.3f\n", gammaC);
	
}

void cursor_callback(GLFWwindow* w, double xpos, double ypos)
{
	float dX, dY;

	/* Don't move if cursor is enabled */
	if(ce == 1)
		return;

	if(fm)
		pX = xpos, pY = ypos, fm = 0;

	dX = (xpos - pX) * SENSITIVITY;
	dY = (pY - ypos) * SENSITIVITY;
	pX = xpos;
	pY = ypos;

	cam.addPYR(glm::vec3(dY, dX, 1.0f));
}

void scroll_callback(GLFWwindow* w, double x, double y)
{
	cam.adjFov(x, y);
	proj = glm::perspective(glm::radians(cam.getFov()), winX / winY, NEAR_PLANE, FAR_PLANE);
}
/* 1}}} */

void debug_sigsegv_handler(int junk)
{
	printf("%s crashed\n", windowName);
	exit(1);
}

int main(int argc, char** argv)
{
	GLFWwindow* window;
	float cTime, pTime, fTime;
	float* vertexData;
	unsigned int EBO, VBO, VAO;
	uint32_t* indices;
	int tmp, i, j, k, s1, s2, nInd;
	glm::mat3 normMat;
	glm::mat4 modelMat, viewMat;
	cmbShader_t shader;
	cmb_t cmb;
	sepdChunk_t* sepdC;
	mshsChunk_t* mshsC;

	if(argc < 2)
	{
		fprintf(stderr, "Usage: %s file.cmb\n", argv[0]);
		return(1);
	}

	scale = 0.005f;
	wf = ce = 0, fm = 1;
	cTime = pTime = fTime = 0.0f;
	gammaC = 1.2f;
	winX = WINDOW_X, winY = WINDOW_Y;
	windowName = argv[1];

	signal(SIGSEGV, debug_sigsegv_handler);

	/* {{{1 Boring stuff */
	/* Init and set OpenGL version */
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	/* Create GLFW window, make sure it worked */
	window = glfwCreateWindow(WINDOW_X, WINDOW_Y, argv[1], NULL, NULL);
	//window = glfwCreateWindow(WINDOW_X, WINDOW_Y, argv[1], glfwGetPrimaryMonitor(), NULL);
	if(window == NULL)
	{
		fprintf(stderr, "Failed to make window\n");
		glfwTerminate();
		return(1);
	}
	glfwMakeContextCurrent(window);

	/* Start up GLAD, make sure it works */
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		fprintf(stderr, "Failed to start GLAD\n");
		return(-1);
	}

	/* Create visible window, set resize function */
	glViewport(0, 0, WINDOW_X, WINDOW_Y);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSwapInterval(SWAP_INTERVAL);
	glEnable(GL_DEPTH_TEST);
	//glClearColor(1.0f, 0.0f, 0.75f, 1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	cam.setSpd(CAM_SPEED);
	/* 1}}} */

	/* Create and compile shaders */
	//shader = cmbShader_t("src/vshader.glsl", "src/2_fshader.glsl");
	shader = cmbShader_t("src/vshader.glsl", "src/fshader.glsl");

	/* Make the model */
	readCmb(&cmb, argv[1]);
	makeCmbModel(&model, &cmb);
	delCmb(cmb);

	/* Constant matrices */
	proj = glm::perspective(glm::radians(cam.getFov()), winX / winY, NEAR_PLANE, FAR_PLANE);

	/* Set directional lighting, spec is unused */
	shader.use();
	shader.set3f("dLight.dir", -1.0f, -1.0f, -1.0f);
	shader.set3f("dLight.ambi", glm::vec3(0.007f));
	shader.set3f("dLight.diff", glm::vec3(0.005f));
	shader.set3f("dLight.spec", glm::vec3(0.001f));

	/* Render loop */
	tmp = 0, fTime = glfwGetTime();
	while(!glfwWindowShouldClose(window))
	{
		/* Get frame start time */
		cTime = glfwGetTime();
		if(glfwGetTime() - fTime >= 1.0f)
		{
			printf("FPS: %d\n", tmp);
			tmp = 0, fTime = glfwGetTime();
		}

		/* Clear the frame buffer */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* Make the view matrix */
		cam.update(window, cTime - pTime);
		viewMat = cam.makeViewMat();

		/* Scale the model by scale variable */
		modelMat = glm::mat4(1.0f);
		modelMat = glm::scale(modelMat, glm::vec3(scale));
		normMat = glm::mat3(glm::transpose(glm::inverse(modelMat)));

		/* Update the shader stuff */
		shader.use();
		shader.set1f("gammaC", gammaC);
		shader.set3f("camPos", cam.getPos());
		shader.set4mf("view", viewMat);
		shader.set4mf("proj", proj);
		shader.set4mf("model", modelMat);
		shader.set3mf("norm", normMat);

		/* Draw the model with both shaders */
		drawCmbModel(&model, &shader);

		/* Check events, swap buffers */
		glfwSwapBuffers(window);
		glfwPollEvents();
		pTime = cTime;
		++tmp;
	}

	/* Clean up */
	delCmbModel(&model);
	shader.del();
	glfwTerminate();
	return(0);
}
