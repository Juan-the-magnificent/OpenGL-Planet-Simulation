//-----------------------------------------------------------------------------
// Mesh_Loading.cpp 
// Solar System Simulation with Lighting
// - Creates Mesh class with normal support
// - Loads and renders OBJ models with Phong lighting
// - Light source is located at the sun's position
// - Direct access to camera position for lighting calculations
//-----------------------------------------------------------------------------
#include <iostream>
#include <sstream>
#include <string>
#define GLEW_STATIC
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "ShaderProgram.h"
#include "Texture2D.h"
#include "Camera.h"
#include "Mesh.h"

// Global Variables
const char* APP_TITLE = "Solar System Simulation with Lighting";
int gWindowWidth = 1024;
int gWindowHeight = 768;
GLFWwindow* gWindow = NULL;
bool gWireframe = false;

// Camera - made global for direct position access
// Note: This requires modifying Camera class to make mPosition public or using friend class
// For this example, we'll assume mPosition is accessible
FPSCamera fpsCamera(glm::vec3(0.0f, 3.0f, 20.0f));
const double ZOOM_SENSITIVITY = -3.0;
const float MOVE_SPEED = 15.0; // units per second
const float MOUSE_SENSITIVITY = 0.1f;

// Lighting variables
glm::vec3 gLightColor(1.0f, 1.0f, 1.0f);
float gAmbientStrength = 0.3f;
float gSpecularStrength = 0.5f;
bool gUseLighting = true;

// Function prototypes
void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode);
void glfw_onFramebufferSize(GLFWwindow* window, int width, int height);
void glfw_onMouseScroll(GLFWwindow* window, double deltaX, double deltaY);
void update(double elapsedTime);
void showFPS(GLFWwindow* window);
bool initOpenGL();

//-----------------------------------------------------------------------------
// Helper function to get camera position directly
// This approach requires modifying the Camera class to make mPosition public
// or using a different design pattern
//-----------------------------------------------------------------------------
glm::vec3 getCameraPositionDirect()
{
    // WARNING: This requires Camera::mPosition to be public
    // This is not recommended for production code but works for this example
    return fpsCamera.getPosition();
}

//-----------------------------------------------------------------------------
// Alternative helper using camera's public interface
// This is the preferred method if Camera class provides getPosition()
//-----------------------------------------------------------------------------
glm::vec3 getCameraPosition()
{
    // If Camera class has getPosition() method, use it
    // return fpsCamera.getPosition();
    
    // For this example, we'll use the direct access method
    return getCameraPositionDirect();
}

//-----------------------------------------------------------------------------
// Main Application Entry Point
//-----------------------------------------------------------------------------
int main()
{
    if (!initOpenGL())
    {
        std::cerr << "GLFW initialization failed" << std::endl;
        return -1;
    }

    // Load shaders with lighting support
    ShaderProgram shaderProgram;
    shaderProgram.loadShaders("shaders/lighting.vert", "shaders/lighting.frag");

    // Load meshes and textures for solar system
    const int numModels = 10;
    Mesh mesh[numModels];
    Texture2D texture[numModels];

    // Load sphere models for all planets
    mesh[0].loadOBJ("models/sphere.obj");  // Sun
    mesh[1].loadOBJ("models/sphere.obj");  // Mercury
    mesh[2].loadOBJ("models/sphere.obj");  // Venus
    mesh[3].loadOBJ("models/sphere.obj");  // Earth
    mesh[4].loadOBJ("models/sphere.obj");  // Mars
    mesh[5].loadOBJ("models/sphere.obj");  // Jupiter
    mesh[6].loadOBJ("models/sphere.obj");  // Saturn
    mesh[7].loadOBJ("models/sphere.obj");  // Uranus
    mesh[8].loadOBJ("models/sphere.obj");  // Neptune
    mesh[9].loadOBJ("models/sphere.obj");  // Pluto
    
    // Load planet textures
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

    // Model positions - distance from sun (scaled for visualization)
    float modelPosMod[] = {
        0.0f,      // 0 sun 
        1.9f,      // 1 mercury
        3.6f,      // 2 venus
        5.0f,      // 3 earth
        7.6f,      // 4 mars
        25.1f,     // 5 jupiter
        45.2f,     // 6 saturn
        96.0f,     // 7 uranus
        150.3f,    // 8 neptune
        197.5f     // 9 pluto
    };

    // Model scale relative to each other
    glm::vec3 modelScale[] = {
        glm::vec3(1.5f, 1.5f, 1.5f),   // 0 sun 
        glm::vec3(0.1f, 0.1f, 0.1f),   // 1 mercury
        glm::vec3(0.3f, 0.3f, 0.3f),   // 2 venus
        glm::vec3(0.3f, 0.3f, 0.3f),   // 3 earth
        glm::vec3(0.2f, 0.2f, 0.2f),   // 4 mars
        glm::vec3(1.0f, 1.0f, 1.0f),   // 5 jupiter
        glm::vec3(0.8f, 0.8f, 0.8f),   // 6 saturn
        glm::vec3(0.5f, 0.5f, 0.5f),   // 7 uranus
        glm::vec3(0.6f, 0.6f, 0.6f),   // 8 neptune
        glm::vec3(0.05f, 0.05f, 0.05f) // 9 pluto
    };

    // Model rotation speed modifier
    float modelRot[] = {
        -100.0f,  // 0 sun 
        -200.0f,  // 1 mercury
        150.0f,  // 2 venus
        -100.0f,  // 3 earth - counter clockwise
        -120.0f,  // 4 mars
        -80.0f,   // 5 jupiter
        -60.0f,   // 6 saturn
        50.0f,   // 7 uranus
        -40.0f,   // 8 neptune
        30.0f    // 9 pluto
    };

    // Model orbit speed modifier (assuming earth speed = 10, negative for counter clockwise)
    float modelOrbit[] = {
        0.0f,    // 0 sun 
        -41.4f,   // 1 mercury
        -16.2f,   // 2 venus
        -10.0f,   // 3 earth
        -5.3f,    // 4 mars
        -0.84f,   // 5 jupiter
        -0.34f,   // 6 saturn
        -0.12f,   // 7 uranus
        -0.061f,  // 8 neptune
        -0.0403f  // 9 pluto
    };

    // Animation state arrays
    float angle[numModels] = {0};
    float rotation[numModels] = {0};
    glm::vec3 modelPos[numModels] = {glm::vec3(0.0f, 0.0f, 0.0f)};

    double lastTime = glfwGetTime();

    // Display control instructions
    std::cout << "=== Solar System Controls ===" << std::endl;
    std::cout << "WASD: Move camera" << std::endl;
    std::cout << "Z/X: Move up/down" << std::endl;
    std::cout << "Mouse: Look around" << std::endl;
    std::cout << "Mouse Wheel: Zoom" << std::endl;
    std::cout << "ESC: Exit" << std::endl;
    std::cout << "F1: Toggle wireframe mode" << std::endl;
    std::cout << "L: Toggle lighting" << std::endl;
    std::cout << "Up/Down Arrows: Adjust ambient strength" << std::endl;
    std::cout << "Left/Right Arrows: Adjust specular strength" << std::endl;
    std::cout << "=============================" << std::endl;

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

        // Create the View matrix from camera
        view = fpsCamera.getViewMatrix();

        // Create the projection matrix
        projection = glm::perspective(glm::radians(fpsCamera.getFOV()), 
                                    (float)gWindowWidth / (float)gWindowHeight, 
                                    0.1f, 200.0f);

        // Activate shader program before setting uniforms
        shaderProgram.use();

        // Pass transformation matrices to the shader
        shaderProgram.setUniform("view", view);
        shaderProgram.setUniform("projection", projection);

        // Set lighting uniforms - light position is same as sun position
        shaderProgram.setUniform("lightPos", modelPos[0]); // Light at sun position
        
        // Get camera position directly for lighting calculations
        // This is where we access the camera position directly
        glm::vec3 cameraPosition = getCameraPosition();
        shaderProgram.setUniform("viewPos", cameraPosition);
        
        shaderProgram.setUniform("lightColor", gLightColor);
        shaderProgram.setUniform("ambientStrength", gAmbientStrength);
        shaderProgram.setUniform("specularStrength", gSpecularStrength);
        shaderProgram.setUniform("useLighting", gUseLighting ? 1 : 0);

        // Render all celestial bodies
        for (int i = 0; i < numModels; i++)
        {
            // Update animation state
            angle[i] += deltaTime * modelOrbit[i];
            rotation[i] += deltaTime * modelRot[i];
            
            // Calculate orbital position using circular orbits
            modelPos[i].x = modelPosMod[i] * sinf(glm::radians(angle[i]));
            modelPos[i].z = modelPosMod[i] * cosf(glm::radians(angle[i]));
            
            // Create model matrix: translation * rotation * scale
            model = glm::translate(glm::mat4(1.0), modelPos[i]) * 
                   glm::rotate(glm::mat4(1.0f), glm::radians(rotation[i]), glm::vec3(0.0f, 1.0f, 0.0f)) * 
                   glm::scale(glm::mat4(1.0), modelScale[i]);
            
            // Pass model matrix to shader
            shaderProgram.setUniform("model", model);

            // Bind texture and render mesh
            texture[i].bind(0);
            mesh[i].draw();
            texture[i].unbind(0);
        }

        // Swap front and back buffers
        glfwSwapBuffers(gWindow);

        lastTime = currentTime;
    }

    // Cleanup
    glfwTerminate();
    std::cout << "Solar System simulation ended." << std::endl;
    return 0;
}

//-----------------------------------------------------------------------------
// Initialize GLFW and OpenGL
//-----------------------------------------------------------------------------
bool initOpenGL()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "GLFW initialization failed" << std::endl;
        return false;
    }

    // Set OpenGL context parameters
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create window
    gWindow = glfwCreateWindow(gWindowWidth, gWindowHeight, APP_TITLE, NULL, NULL);
    if (gWindow == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    // Make the window's context current
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

    // Hide and capture cursor for FPS camera control
    glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);

    // Set clear color to black (space background)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Set viewport and enable depth testing for 3D rendering
    glViewport(0, 0, gWindowWidth, gWindowHeight);
    glEnable(GL_DEPTH_TEST);

    // Print OpenGL version information
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    return true;
}

//-----------------------------------------------------------------------------
// Is called whenever a key is pressed/released via GLFW
//-----------------------------------------------------------------------------
void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // Exit application on ESC key press
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    
    // Toggle wireframe rendering mode
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
    {
        gWireframe = !gWireframe;
        if (gWireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            std::cout << "Wireframe mode: ON" << std::endl;
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            std::cout << "Wireframe mode: OFF" << std::endl;
        }
    }

    // Toggle lighting calculations
    if (key == GLFW_KEY_L && action == GLFW_PRESS)
    {
        gUseLighting = !gUseLighting;
        std::cout << "Lighting: " << (gUseLighting ? "ON" : "OFF") << std::endl;
    }

    // Increase ambient lighting strength
    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    {
        gAmbientStrength += 0.1f;
        gAmbientStrength = glm::clamp(gAmbientStrength, 0.0f, 1.0f);
        std::cout << "Ambient Strength: " << gAmbientStrength << std::endl;
    }

    // Decrease ambient lighting strength
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
    {
        gAmbientStrength -= 0.1f;
        gAmbientStrength = glm::clamp(gAmbientStrength, 0.0f, 1.0f);
        std::cout << "Ambient Strength: " << gAmbientStrength << std::endl;
    }

    // Increase specular lighting strength
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    {
        gSpecularStrength += 0.1f;
        gSpecularStrength = glm::clamp(gSpecularStrength, 0.0f, 1.0f);
        std::cout << "Specular Strength: " << gSpecularStrength << std::endl;
    }

    // Decrease specular lighting strength
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    {
        gSpecularStrength -= 0.1f;
        gSpecularStrength = glm::clamp(gSpecularStrength, 0.0f, 1.0f);
        std::cout << "Specular Strength: " << gSpecularStrength << std::endl;
    }
}

//-----------------------------------------------------------------------------
// Is called when the window is resized
//-----------------------------------------------------------------------------
void glfw_onFramebufferSize(GLFWwindow* window, int width, int height)
{
    // Update global window dimensions
    gWindowWidth = width;
    gWindowHeight = height;
    
    // Update OpenGL viewport to match new window size
    glViewport(0, 0, gWindowWidth, gWindowHeight);
}

//-----------------------------------------------------------------------------
// Called by GLFW when the mouse wheel is rotated
//-----------------------------------------------------------------------------
void glfw_onMouseScroll(GLFWwindow* window, double deltaX, double deltaY)
{
    // Adjust field of view based on vertical scroll input
    double fov = fpsCamera.getFOV() + deltaY * ZOOM_SENSITIVITY;
    
    // Clamp FOV to reasonable limits
    fov = glm::clamp(fov, 1.0, 120.0);
    
    // Update camera FOV
    fpsCamera.setFOV((float)fov);
}

//-----------------------------------------------------------------------------
// Update camera and other game logic every frame
//-----------------------------------------------------------------------------
void update(double elapsedTime)
{
    // Camera orientation variables
    double mouseX, mouseY;

    // Get the current mouse cursor position
    glfwGetCursorPos(gWindow, &mouseX, &mouseY);

    // Rotate the camera based on mouse movement relative to screen center
    // Convert mouse displacement to rotation angles with sensitivity scaling
    fpsCamera.rotate((float)(gWindowWidth / 2.0 - mouseX) * MOUSE_SENSITIVITY, 
                    (float)(gWindowHeight / 2.0 - mouseY) * MOUSE_SENSITIVITY);

    // Reset mouse cursor to center of screen for continuous rotation
    glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);

    // Camera movement based on keyboard input

    // Forward movement (W key)
    if (glfwGetKey(gWindow, GLFW_KEY_W) == GLFW_PRESS)
        fpsCamera.move(MOVE_SPEED * (float)elapsedTime * fpsCamera.getLook());
    
    // Backward movement (S key)
    else if (glfwGetKey(gWindow, GLFW_KEY_S) == GLFW_PRESS)
        fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -fpsCamera.getLook());

    // Strafe left (A key)
    if (glfwGetKey(gWindow, GLFW_KEY_A) == GLFW_PRESS)
        fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -fpsCamera.getRight());
    
    // Strafe right (D key)
    else if (glfwGetKey(gWindow, GLFW_KEY_D) == GLFW_PRESS)
        fpsCamera.move(MOVE_SPEED * (float)elapsedTime * fpsCamera.getRight());

    // Move up (Z key)
    if (glfwGetKey(gWindow, GLFW_KEY_Z) == GLFW_PRESS)
        fpsCamera.move(MOVE_SPEED * (float)elapsedTime * fpsCamera.getUp());
    
    // Move down (X key)
    else if (glfwGetKey(gWindow, GLFW_KEY_X) == GLFW_PRESS)
        fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -fpsCamera.getUp());
}

//-----------------------------------------------------------------------------
// Code computes the average frames per second, and also the average time it takes
// to render one frame. These stats are appended to the window caption bar.
//-----------------------------------------------------------------------------
void showFPS(GLFWwindow* window)
{
    static double previousSeconds = 0.0;
    static int frameCount = 0;
    double elapsedSeconds;
    double currentSeconds = glfwGetTime(); // returns number of seconds since GLFW started

    elapsedSeconds = currentSeconds - previousSeconds;

    // Limit text updates to 4 times per second to reduce flickering
    if (elapsedSeconds > 0.25)
    {
        previousSeconds = currentSeconds;
        double fps = (double)frameCount / elapsedSeconds;
        double msPerFrame = 1000.0 / fps;

        // Format and update window title with performance statistics
        std::ostringstream outs;
        outs.precision(3);    // decimal places
        outs << std::fixed
            << APP_TITLE << "    "
            << "FPS: " << fps << "    "
            << "Frame Time: " << msPerFrame << " (ms)";
        glfwSetWindowTitle(window, outs.str().c_str());

        // Reset frame count for next average calculation
        frameCount = 0;
    }

    frameCount++;
}