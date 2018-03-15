#include "Node.h"

Transform::Transform(glm::mat4 M, std::string type, float timeOffset)
{
	this->M = M;
	initialM = M;
	this->type = type;
	this->timeOffset = timeOffset;
}

void Transform::addChild(Node* node)
{
	children.push_front(node);
}

void Transform::removeChild(Node* node)
{
	children.remove(node);
}

void Transform::draw(glm::mat4 C)
{
	for (auto it : children)
	{
		it->draw(C * M);
	}
}

void Transform::update()
{
	if (type == "limb")
	{
		M = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, -1.0));
		M = glm::rotate(glm::mat4(1.0f), glm::sin(timeOffset + Window::time * 0.01f) * 0.8f, glm::vec3(1.0, 0.0, 0.0)) * M;
		M = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 1.0)) * M;
		M = initialM * M;
	}

	for (auto it : children)
	{
		it->update();
	}
}

glm::mat4 Transform::getMatrix()
{
	return M;
}

void Transform::setMatrix(glm::mat4 M)
{
	this->M = M;
}

Geometry::Geometry(std::string filename)
{
	init(filename);
}

void Geometry::init(std::string filename)
{
	obj = new OBJObject(filename.c_str(), 0.0f, 0.0f, 0.0f, 1.0f, glm::vec3(1.0, 0.0, 0.0), glm::vec3(1.0), 16.0f);
}

void Geometry::draw(glm::mat4 C)
{
	obj->draw(C);
}

void Geometry::update()
{

}