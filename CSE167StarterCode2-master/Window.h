#ifndef _WINDOW_H_
#define _WINDOW_H_

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
#include <string>
#include "Cube.h"
#include "shader.h"
#include "GLFWStarterProject/OBJObject.h"
#include "GLFWStarterProject/Node.h"

class Curve;

struct s_directionalLight {
	int enabled;
	glm::vec3 color;
	glm::vec3 direction;
	glm::vec3 ambientColor;
};

struct s_pointLight {
	int enabled;
	glm::vec3 color;
	glm::vec3 position;
	glm::vec3 ambientColor;
	float radius;
};

struct s_spotLight {
	int enabled;
	glm::vec3 color;
	glm::vec3 position;
	glm::vec3 direction;
	float cutoff;
	float exponent;
	glm::vec3 ambientColor;
	float radius;
};

class Window
{
public:
	static int width;
	static int height;
	static glm::mat4 P; // P for projection
	static glm::mat4 V; // V for view
	static s_directionalLight* currentDirectionalLight;
	static s_pointLight* currentPointLight;
	static s_spotLight* currentSpotLight;
	static int time;
	static std::vector<GLfloat> Window::vertices;

	static void initialize_objects();
	static void clean_up();
	static GLFWwindow* create_window(int width, int height);
	static void resize_callback(GLFWwindow* window, int width, int height);
	static void idle_callback();
	static void display_callback(GLFWwindow*);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

	static void cursorPos_callback(GLFWwindow*, double, double);
	static void mouseButton_callback(GLFWwindow*, int, int, int);
	static void scroll_callback(GLFWwindow*, double, double);
};

#endif
