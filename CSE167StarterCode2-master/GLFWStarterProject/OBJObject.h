#define _CRT_SECURE_NO_WARNINGS

#ifndef OBJOBJECT_H
#define OBJOBJECT_H

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

struct s_directionalLight;
struct s_pointLight;
struct s_spotLight;

class OBJObject
{
private:
	std::vector<glm::vec3> colors;
	std::vector<glm::ivec3> faces;
	std::vector<GLfloat> normals;
	std::vector<GLfloat> vertices;
	std::vector<GLuint> indices;
	glm::mat4 toWorld;
	glm::mat4 normalizeTransMatrix;
	glm::mat4 normalizeScaleMatrix;
	glm::mat4 rotMatrix;

	float startX, startY, startZ;
	float x, y, z;
	float startScaleX, startScaleY, startScaleZ;
	float scaleX, scaleY, scaleZ;
	float p_s;
	glm::vec3 ambientColor, k_d, k_s;

public:
	OBJObject(const char*, float, float, float, float, glm::vec3, glm::vec3, float);
	~OBJObject();

	void parse(const char*);
	void createNormalizeMatrices();
	void createNormalizeTransMatrix(float, float, float, float, float, float);
	void createNormalizeScaleMatrix(float, float, float, float, float, float);
	void update();
	void draw(glm::mat4);
	void addSkyboxFace(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);

	GLuint VBO, VAO, EBO, NBO;
	GLuint uProjection, uView, uModel, uAmbientColor, uK_d, uK_s, uP_s, uCameraPosition;
};

#endif