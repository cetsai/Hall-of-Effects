#define _CRT_SECURE_NO_WARNINGS

#ifndef NODE_H
#define NODE_H

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
#include <list>
#include <string>
#include "../Window.h"
#include "OBJObject.h"

class OBJObject;

class Node
{
private:
	Node* parent;
public:
	virtual void draw(glm::mat4 C) = 0;
	virtual void update() = 0;
};

class Transform : public Node
{
private:
	glm::mat4 M;
	glm::mat4 initialM;
	std::list<Node*> children;
	std::string type;
	float timeOffset;

public:
	Transform(glm::mat4, std::string, float);

	void addChild(Node*);
	void removeChild(Node*);
	void draw(glm::mat4 C);
	void update();
	glm::mat4 getMatrix();
	void setMatrix(glm::mat4);
};

class Geometry : public Node
{
private:
	OBJObject* obj;

public:
	Geometry(std::string);

	void init(std::string);
	void draw(glm::mat4 C);
	void update();
};

#endif