#define _CRT_SECURE_NO_WARNINGS

#ifndef CURVE_H
#define CURVE_H

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
// Use of degrees is deprecated. Use radians instead.
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <limits>
#include "../Window.h"

class Curve
{
private:
	std::vector<glm::vec3> colors;
	std::vector<glm::ivec3> faces;
	std::vector<GLfloat> normals;
	std::vector<GLfloat> vertices;
	std::vector<GLuint> indices;
	glm::mat4 toWorld;
	glm::mat4 rotMatrix;
	std::vector<GLfloat> controlPoints;
	std::vector<GLfloat> length;
	float totalLength;

	float startX, startY, startZ;
	float x, y, z;
	float startScaleX, startScaleY, startScaleZ;
	float scaleX, scaleY, scaleZ;
	float p_s;
	glm::vec3 ambientColor, k_d, k_s;

public:
	Curve(float, float, float, float, std::vector<GLfloat>);
	~Curve();

	void draw(glm::mat4);
	glm::vec3 getPos(float);

	GLuint VBO, VAO, EBO, NBO;
	GLuint uProjection, uView, uModel, uAmbientColor, uK_d, uK_s, uP_s, uCameraPosition;
};

#endif