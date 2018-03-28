#pragma once


#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#else
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>

struct Vertex{
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 tc;
};

struct Material {
	std::string name;
	GLuint texture;
};


class Mesh{
public:
	Mesh(std::vector<Vertex> vs, std::vector<GLuint> is, Material m);	
	~Mesh();

	void draw(GLuint shaderProgram);

private:
	GLuint VBO, VAO, EBO;

	glm::mat4 toWorld;
	
	Material mat;
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

};
