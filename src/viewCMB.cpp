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
#define RESOLUTION 1
#define FULLSCREEN 0
#define CAM_SPEED 2.5f
#define SENSITIVITY 0.1f
#define NEAR_PLANE 0.1f
#define FAR_PLANE 200.0f

const float resolutions[] = {
1280.0, 720.0,   //0 -> 720p  16:9
1920.0, 1080.0,  //1 -> 1080p 16:9
2560.0, 1440.0,  //2 -> 1440p 16:9
2560.0, 1080.0,  //3 -> 1080p 21:9
3440.0, 1440.0}; //4 -> 1440p 21:9

cmbModel_t model;
const char* windowName;
char wf, fm, ce;
float pX, pY, winX, winY, scale;
mCamera_t cam;
glm::mat4 proj;

void framebuffer_size_callback(GLFWwindow* w, int x, int y)
{
	float fov;

	/* Resize the viewport when window size is adjusted */
	winX = x, winY = y;
	glViewport(0, 0, winX, winY);
	fov = glm::radians(cam.getFov());
	proj = glm::perspective(fov, winX / winY, NEAR_PLANE, FAR_PLANE);
}

void key_callback(GLFWwindow* w, int key, int scancode, int action, int mods)
{
	int tmp;

	/* Close on escape */
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(w, 1);

	/* Toggle wireframe on tab */
	if(key == GLFW_KEY_TAB && action == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, (wf) ? GL_FILL : GL_LINE), wf = !wf;

	/* "Sprint" on shift */
	if(key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
		cam.addMod(1.0f);
	if(key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
		cam.addMod(-1.0f);

	/* Toggle cursor on C */
	if(key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		tmp = (ce) ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
		glfwSetInputMode(w, GLFW_CURSOR, tmp), ce = !ce, fm = 1;
	}

	/* Scale up and down on up/down */
	if(key == GLFW_KEY_UP && action == GLFW_PRESS)
		scale += 0.001;
	if(key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		scale -= 0.001;

	/* Swap meshes on left/right */
	if(key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		if(model.meshNum > -1)
			model.meshNum--;
	if(key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		if(model.meshNum < model.nMeshes-1)
			model.meshNum++;
}

void cursor_callback(GLFWwindow* w, double xpos, double ypos)
{
	float dX, dY;

	/* Don't move if cursor is enabled */
	if(ce == 1)
		return;

	/* Remove jump on first movement */
	if(fm)
		pX = xpos, pY = ypos, fm = 0;

	/* Get new camera angles */
	dX = (xpos - pX) * SENSITIVITY;
	dY = (pY - ypos) * SENSITIVITY;
	pX = xpos;
	pY = ypos;

	/* Update the camera pitch/yaw */
	cam.addPYR(glm::vec3(dY, dX, 1.0f));
}

void scroll_callback(GLFWwindow* w, double x, double y)
{
	float fov;

	/* Zoom in/out on scroll */
	cam.adjFov(x, y);
	fov = glm::radians(cam.getFov());
	proj = glm::perspective(fov, winX / winY, NEAR_PLANE, FAR_PLANE);
}

void debug_sigsegv_handler(int junk)
{
	printf("%s crashed\n", windowName);
	exit(1);
}

int main(int argc, char** argv)
{
	cmb_t cmb;
	GLFWwindow* window;
	GLFWmonitor* monitor;
	float cTime, pTime, fTime, fov;
	unsigned int tmp;
	glm::mat3 normMat;
	glm::mat4 modelMat, viewMat;

	if(argc < 2)
	{
		fprintf(stderr, "Usage: %s file.cmb\n", argv[0]);
		return(1);
	}

	/* Initalise some stuff */
	scale = 0.005f;
	wf = ce = 0, fm = 1;
	cTime = pTime = fTime = 0.0f;
	winX = resolutions[(RESOLUTION * 2)];
	winY = resolutions[(RESOLUTION * 2) + 1];
	windowName = argv[1];

	signal(SIGSEGV, debug_sigsegv_handler);

	/* Init GLFW and set OpenGL version */
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Get the monitor */
	if(FULLSCREEN)
		monitor = glfwGetPrimaryMonitor();
	else
		monitor = NULL;

	/* Make the window, make sure it worked */
	window = glfwCreateWindow(winX, winY, argv[1], monitor, NULL);
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
	glViewport(0, 0, winX, winY);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSwapInterval(SWAP_INTERVAL);
	glEnable(GL_DEPTH_TEST);
	//glClearColor(1.0f, 0.0f, 0.75f, 1.0f); // pink for testing alpha
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	cam.setSpd(CAM_SPEED);

	/* Make the model */
	readCmb(&cmb, argv[1]);
	makeCmbModel(&model, &cmb);
	delCmb(cmb);

	/* Make the initial projection matrix */
	fov = glm::radians(cam.getFov());
	proj = glm::perspective(fov, winX / winY, NEAR_PLANE, FAR_PLANE);

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

		/* Send the transform matrices to the model */
		model.modelMat = modelMat;
		model.normMat = normMat;
		model.viewMat = viewMat;
		model.projMat = proj;

		/* Draw the model with both shaders */
		drawCmbModel(&model);

		/* Check events, swap buffers */
		glfwSwapBuffers(window);
		glfwPollEvents();
		pTime = cTime;
		++tmp;
//glfwSetWindowShouldClose(window, 1); //debug
	}

	/* Clean up */
	delCmbModel(&model);
	glfwTerminate();
	return(0);
}
