//-----------------------------------------------------------------------------
// Models_01.cpp 
// - Creates Mesh class
// - Loads and renders (3) OBJ models
//-----------------------------------------------------------------------------
#include <iostream>
#include <sstream>
#include <string>
#define GLEW_STATIC
#include "GL/glew.h"	// Important - this header must come before glfw3 header
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "ShaderProgram.h"
#include "Texture2D.h"
#include "Camera.h"
#include "Mesh.h"


// Global Variables
const char* APP_TITLE = "Introduction to Modern OpenGL - Loading OBJ Models";
int gWindowWidth = 1024;
int gWindowHeight = 768;
GLFWwindow* gWindow = NULL;
bool gWireframe = false;

FPSCamera fpsCamera(glm::vec3(0.0f, 3.0f, 10.0f));
const double ZOOM_SENSITIVITY = -3.0;
const float MOVE_SPEED = 15.0; // units per second
const float MOUSE_SENSITIVITY = 0.1f;

// Function prototypes
void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode);
void glfw_onFramebufferSize(GLFWwindow* window, int width, int height);
void glfw_onMouseScroll(GLFWwindow* window, double deltaX, double deltaY);
void update(double elapsedTime);
void showFPS(GLFWwindow* window);
bool initOpenGL();

//-----------------------------------------------------------------------------
// Main Application Entry Point
//-----------------------------------------------------------------------------
int main()
{
	if (!initOpenGL())
	{
		// An error occured
		std::cerr << "GLFW initialization failed" << std::endl;
		return -1;
	}

	ShaderProgram shaderProgram;
	shaderProgram.loadShaders("shaders/basic.vert", "shaders/basic.frag");

	// Load meshes and textures
	const int numModels = 10;
	Mesh mesh[numModels];
	Texture2D texture[numModels];

	mesh[0].loadOBJ("models/sphere.obj");
	mesh[1].loadOBJ("models/sphere.obj");
	mesh[2].loadOBJ("models/sphere.obj");
	mesh[3].loadOBJ("models/sphere.obj");
	mesh[4].loadOBJ("models/sphere.obj");
	mesh[5].loadOBJ("models/sphere.obj");
	mesh[6].loadOBJ("models/sphere.obj");
	mesh[7].loadOBJ("models/sphere.obj");
	mesh[8].loadOBJ("models/sphere.obj");
	mesh[9].loadOBJ("models/sphere.obj");
	
	texture[0].loadTexture("textures/sun.jpg", true);
	texture[1].loadTexture("textures/mercury.jpg", true);
	texture[2].loadTexture("textures/venus.jpg", true);
	texture[3].loadTexture("textures/earth.jpg", true);
	texture[4].loadTexture("textures/mars.jpg", true);
	texture[5].loadTexture("textures/jupiter.jpg", true);
	texture[6].loadTexture("textures/saturn.jpg", true);
	texture[7].loadTexture("textures/uranus.jpg", true);
	texture[8].loadTexture("textures/neptune.jpg", true);
	texture[9].loadTexture("textures/pluto.jpg", true);

	// Model positions modifier
	// distance based on jpl.nasa.gov au distance from sun x 5
	float modelPosMod[] = {
		0,	// 0 sun 
		1.9,	// 1 mercury
		3.6,	// 2 venus
		5, 	// 3 earth
		7.6,	// 4 mars
		25.1,	// 5 jupiter
		45.2,	// 6 saturn
		96,	// 7 uranus
		150.3,	// 8 neptune
		197.5 	// 9 pluto
	};

	// Model scale
	glm::vec3 modelScale[] = {
		glm::vec3(1.5f, 1.5f, 1.5f),	// 0 sun 
		glm::vec3(0.1f, 0.1f, 0.1f),	// 1 mercury
		glm::vec3(0.3f, 0.3f, 0.3f),	// 2 venus
		glm::vec3(0.3f, 0.3f, 0.3f),	// 3 earth
		glm::vec3(0.2f, 0.2f, 0.2f),	// 4 mars
		glm::vec3(1.0f, 1.0f, 1.0f),	// 5 jupiter
		glm::vec3(0.8f, 0.8f, 0.8f),	// 6 saturn
		glm::vec3(0.5f, 0.5f, 0.5f),	// 7 uranus
		glm::vec3(0.6f, 0.6f, 0.6f),	// 8 neptune
		glm::vec3(0.05f, 0.05f, 0.05f)		// 9 pluto
	};

	// Model rotation speed modifier
	float modelRot[] = {
		100,	// 0 sun 
		100,	// 1 mercury
		100,	// 2 venus
		100,	// 3 earth
		100,	// 4 mars
		100,	// 5 jupiter
		100,	// 6 saturn
		100,	// 7 uranus
		100,	// 8 neptune
		100		// 9 pluto
	};

	// Model orbit speed modifier
	// assuming earth with a speed of 10
	float modelOrbit[] = {
		0,  	// 0 sun 
		41.4,	// 1 mercury
		16.2,	// 2 venus
		10,  	// 3 earth
		5.3,	// 4 mars
		0.84,	// 5 jupiter
		0.34,	// 6 saturn
		0.12,	// 7 uranus
		0.061,	// 8 neptune
		0.0403	// 9 pluto
	};

	//array variables
	float angle[10] = {0};
	float rotation[10] = {0};
	glm::vec3 modelPos[10] = {glm::vec3(0.0f, 0.0f, 0.0f)};

	double lastTime = glfwGetTime();

	// Rendering loop
	while (!glfwWindowShouldClose(gWindow))
	{
		showFPS(gWindow);

		double currentTime = glfwGetTime();
		double deltaTime = currentTime - lastTime;

		// Poll for and process events
		glfwPollEvents();
		update(deltaTime);

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model(1.0), view(1.0), projection(1.0);

		// Create the View matrix
		view = fpsCamera.getViewMatrix();

		// Create the projection matrix
		projection = glm::perspective(glm::radians(fpsCamera.getFOV()), (float)gWindowWidth / (float)gWindowHeight, 0.1f, 200.0f);

		// Must be called BEFORE setting uniforms because setting uniforms is done
		// on the currently active shader program.
		shaderProgram.use();

		// Pass the matrices to the shader
		shaderProgram.setUniform("view", view);
		shaderProgram.setUniform("projection", projection);

		// Render the scene
		for (int i = 0; i < numModels; i++)
		{
			angle[i] += deltaTime * modelOrbit[i];
			rotation[i] += deltaTime * modelRot[i];
			modelPos[i].x = modelPosMod[i] * sinf(glm::radians(angle[i]));
			modelPos[i].z = modelPosMod[i] * cosf(glm::radians(angle[i]));
			model = glm::translate(glm::mat4(1.0), modelPos[i]) * glm::rotate(glm::mat4(1.0f), glm::radians(rotation[i]), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0), modelScale[i]);
			
			shaderProgram.setUniform("model", model);

			texture[i].bind(0);		// set the texture before drawing.  Our simple OBJ mesh loader does not do materials yet.
			mesh[i].draw();			// Render the OBJ mesh
			texture[i].unbind(0);	
		}

		// Swap front and back buffers
		glfwSwapBuffers(gWindow);

		lastTime = currentTime;
	}

	glfwTerminate();

	return 0;
}

//-----------------------------------------------------------------------------
// Initialize GLFW and OpenGL
//-----------------------------------------------------------------------------
bool initOpenGL()
{
	// Intialize GLFW 
	// GLFW is configured.  Must be called before calling any GLFW functions
	if (!glfwInit())
	{
		// An error occured
		std::cerr << "GLFW initialization failed" << std::endl;
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);	// forward compatible with newer versions of OpenGL as they become available but not backward compatible (it will not run on devices that do not support OpenGL 3.3


	// Create an OpenGL 3.3 core, forward compatible context window
	gWindow = glfwCreateWindow(gWindowWidth, gWindowHeight, APP_TITLE, NULL, NULL);
	if (gWindow == NULL)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}

	// Make the window's context the current one
	glfwMakeContextCurrent(gWindow);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return false;
	}

	// Set the required callback functions
	glfwSetKeyCallback(gWindow, glfw_onKey);
	glfwSetFramebufferSizeCallback(gWindow, glfw_onFramebufferSize);
	glfwSetScrollCallback(gWindow, glfw_onMouseScroll);

	// Hides and grabs cursor, unlimited movement
	glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Define the viewport dimensions
	glViewport(0, 0, gWindowWidth, gWindowHeight);
	glEnable(GL_DEPTH_TEST);

	return true;
}

//-----------------------------------------------------------------------------
// Is called whenever a key is pressed/released via GLFW
//-----------------------------------------------------------------------------
void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	
	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		gWireframe = !gWireframe;
		if (gWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

//-----------------------------------------------------------------------------
// Is called when the window is resized
//-----------------------------------------------------------------------------
void glfw_onFramebufferSize(GLFWwindow* window, int width, int height)
{
	gWindowWidth = width;
	gWindowHeight = height;
	glViewport(0, 0, gWindowWidth, gWindowHeight);
}

//-----------------------------------------------------------------------------
// Called by GLFW when the mouse wheel is rotated
//-----------------------------------------------------------------------------
void glfw_onMouseScroll(GLFWwindow* window, double deltaX, double deltaY)
{
	double fov = fpsCamera.getFOV() + deltaY * ZOOM_SENSITIVITY;

	fov = glm::clamp(fov, 1.0, 120.0);

	fpsCamera.setFOV((float)fov);
}

//-----------------------------------------------------------------------------
// Update stuff every frame
//-----------------------------------------------------------------------------
void update(double elapsedTime)
{
	// Camera orientation
	double mouseX, mouseY;

	// Get the current mouse cursor position delta
	glfwGetCursorPos(gWindow, &mouseX, &mouseY);

	// Rotate the camera the difference in mouse distance from the center screen.  Multiply this delta by a speed scaler
	fpsCamera.rotate((float)(gWindowWidth / 2.0 - mouseX) * MOUSE_SENSITIVITY, (float)(gWindowHeight / 2.0 - mouseY) * MOUSE_SENSITIVITY);

	// Clamp mouse cursor to center of screen
	glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);

	// Camera FPS movement

	// Forward/backward
	if (glfwGetKey(gWindow, GLFW_KEY_W) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * fpsCamera.getLook());
	else if (glfwGetKey(gWindow, GLFW_KEY_S) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -fpsCamera.getLook());

	// Strafe left/right
	if (glfwGetKey(gWindow, GLFW_KEY_A) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -fpsCamera.getRight());
	else if (glfwGetKey(gWindow, GLFW_KEY_D) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * fpsCamera.getRight());

	// Up/down
	if (glfwGetKey(gWindow, GLFW_KEY_Z) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * fpsCamera.getUp());
	else if (glfwGetKey(gWindow, GLFW_KEY_X) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -fpsCamera.getUp());
}

//-----------------------------------------------------------------------------
// Code computes the average frames per second, and also the average time it takes
// to render one frame.  These stats are appended to the window caption bar.
//-----------------------------------------------------------------------------
void showFPS(GLFWwindow* window)
{
	static double previousSeconds = 0.0;
	static int frameCount = 0;
	double elapsedSeconds;
	double currentSeconds = glfwGetTime(); // returns number of seconds since GLFW started, as double float

	elapsedSeconds = currentSeconds - previousSeconds;

	// Limit text updates to 4 times per second
	if (elapsedSeconds > 0.25)
	{
		previousSeconds = currentSeconds;
		double fps = (double)frameCount / elapsedSeconds;
		double msPerFrame = 1000.0 / fps;

		// The C++ way of setting the window title
		std::ostringstream outs;
		outs.precision(3);	// decimal places
		outs << std::fixed
			<< APP_TITLE << "    "
			<< "FPS: " << fps << "    "
			<< "Frame Time: " << msPerFrame << " (ms)";
		glfwSetWindowTitle(window, outs.str().c_str());

		// Reset for next average.
		frameCount = 0;
	}

	frameCount++;
}