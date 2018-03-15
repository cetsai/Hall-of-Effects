#include "Curve.h"

extern GLint shaderProgramNormal;
extern GLint shaderProgramPhong;
extern glm::vec3 cam_pos;

Curve::Curve(float x, float y, float z, float scale, std::vector<GLfloat> controlPoints)
{
	this->x = x;
	this->y = y;
	this->z = z;

	startX = x;
	startY = y;
	startZ = z;

	scaleX = scale;
	scaleY = scale;
	scaleZ = scale;

	startScaleX = scale;
	startScaleY = scale;
	startScaleZ = scale;

	rotMatrix = glm::mat4(1.0f);

	this->controlPoints = controlPoints;

	toWorld = glm::mat4(1.0f);

	int linesPerCurve = 150;

	glm::vec3 posPrevious = glm::vec3(controlPoints[0], controlPoints[1], controlPoints[2]);
	glm::vec3 pos;
	glm::vec3 posIntermediate = glm::vec3(posPrevious.x, posPrevious.y, posPrevious.z);

	totalLength = 0.0f;

	for (int i = 3; i < controlPoints.size(); i += 3 * 3)
	{
		glm::vec3 p1 = glm::vec3(controlPoints[i], controlPoints[i + 1], controlPoints[i + 2]);
		glm::vec3 p2 = glm::vec3(controlPoints[i + 3], controlPoints[i + 4], controlPoints[i + 5]);
		pos = glm::vec3(controlPoints[i + 6], controlPoints[i + 7], controlPoints[i + 8]);
		
		float lengthSum = 0.0f;

		for (int j = 1; j <= linesPerCurve; j++)
		{
			indices.push_back((GLuint)(vertices.size() / 3) - 1);
			indices.push_back((GLuint)(vertices.size() / 3));
			
			glm::vec3 previousPosIntermediate = glm::vec3(posIntermediate.x, posIntermediate.y, posIntermediate.z);
			//posIntermediate = glm::mix(posPrevious, pos, ((float)j) / ((float)linesPerCurve));
			float t = ((float)j) / ((float)linesPerCurve);
			posIntermediate = (1 - t) * (1 - t) * (1 - t) * posPrevious
				+ 3 * (1 - t) * (1 - t) * t * p1
				+ 3 * (1 - t) * t * t * p2
				+ t * t * t * pos;
			
			lengthSum += glm::distance(previousPosIntermediate, posIntermediate);

			vertices.push_back(posIntermediate.x); vertices.push_back(posIntermediate.y); vertices.push_back(posIntermediate.z);
		}

		length.push_back(lengthSum);
		totalLength += lengthSum;

		posPrevious = glm::vec3(pos.x, pos.y, pos.z);
	}

	// Create array object and buffers. Remember to delete your buffers when the object is destroyed!
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenBuffers(1, &NBO);

	// Bind the Vertex Array Object (VAO) first, then bind the associated buffers to it.
	// Consider the VAO as a container for all your buffers.
	glBindVertexArray(VAO);

	// Now bind a VBO to it as a GL_ARRAY_BUFFER. The GL_ARRAY_BUFFER is an array containing relevant data to what
	// you want to draw, such as vertices, normals, colors, etc.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// glBufferData populates the most recently bound buffer with data starting at the 3rd argument and ending after
	// the 2nd argument number of indices. How does OpenGL know how long an index spans? Go to glVertexAttribPointer.
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
	// Enable the usage of layout location 0 (check the vertex shader to see what this is)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 0. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
		3, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		3 * sizeof(GLfloat), // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

					 // We've sent the vertex data over to OpenGL, but there's still something missing.
					 // In what order should it draw those vertices? That's why we'll need a GL_ELEMENT_ARRAY_BUFFER for this.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	/*glBindBuffer(GL_ARRAY_BUFFER, NBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 0. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
		3, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		3 * sizeof(GLfloat), // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0);*/

	// Unbind the currently bound buffer so that we don't accidentally make unwanted changes to it.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Unbind the VAO now so we don't accidentally tamper with it.
	// NOTE: You must NEVER unbind the element array buffer associated with a VAO!
	glBindVertexArray(0);
}

glm::vec3 Curve::getPos(float t)
{
	while (t > totalLength)
	{
		t -= totalLength;
	}
	//t = glm::mod(t, totalLength);

	int curveCount = 5; //(controlPoints.size() - 3) / (3 * 3) + 1;

	int curveIndex;
	float lengthSum = 0.0f;
	for (int i = 0; i < curveCount; i++)
	{
		lengthSum += length[i];
		if (t <= lengthSum)
		{
			curveIndex = i;
			break;
		}
	}

	int curve = curveIndex * 3 * 3;

	glm::vec3 posPrevious = glm::vec3(controlPoints[curve], controlPoints[curve + 1], controlPoints[curve + 2]);
	glm::vec3 pos;
	glm::vec3 posIntermediate;

	glm::vec3 p1 = glm::vec3(controlPoints[curve + 3], controlPoints[curve + 4], controlPoints[curve + 5]);
	glm::vec3 p2 = glm::vec3(controlPoints[curve + 6], controlPoints[curve + 7], controlPoints[curve + 8]);
	pos = glm::vec3(controlPoints[curve + 9], controlPoints[curve + 10], controlPoints[curve + 11]);

	t = (t - (lengthSum - length[curveIndex])) / length[curveIndex];
	posIntermediate = (1 - t) * (1 - t) * (1 - t) * posPrevious
		+ 3 * (1 - t) * (1 - t) * t * p1
		+ 3 * (1 - t) * t * t * p2
		+ t * t * t * pos;

	return posIntermediate;

	/*int curveCount = (controlPoints.size() - 3) / (3 * 3);
	
	int curveIndex = (((int)t) % curveCount) * 3 * 3;

	glm::vec3 posPrevious = glm::vec3(controlPoints[curveIndex], controlPoints[curveIndex + 1], controlPoints[curveIndex + 2]);
	glm::vec3 pos;
	glm::vec3 posIntermediate;

	glm::vec3 p1 = glm::vec3(controlPoints[curveIndex + 3], controlPoints[curveIndex + 4], controlPoints[curveIndex + 5]);
	glm::vec3 p2 = glm::vec3(controlPoints[curveIndex + 6], controlPoints[curveIndex + 7], controlPoints[curveIndex + 8]);
	pos = glm::vec3(controlPoints[curveIndex + 9], controlPoints[curveIndex + 10], controlPoints[curveIndex + 11]);

	t = glm::fract(t);
	posIntermediate = (1 - t) * (1 - t) * (1 - t) * posPrevious
			+ 3 * (1 - t) * (1 - t) * t * p1
			+ 3 * (1 - t) * t * t * p2
			+ t * t * t * pos;

	return posIntermediate;*/
}

Curve::~Curve()
{
	// Delete previously generated buffers. Note that forgetting to do this can waste GPU memory in a 
	// large project! This could crash the graphics driver due to memory leaks, or slow down application performance!
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	//glDeleteBuffers(1, &NBO);
}

void Curve::draw(glm::mat4 C)
{
	GLint shaderProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &shaderProgram);

	// We need to calcullate this because modern OpenGL does not keep track of any matrix other than the viewport (D)
	// Consequently, we need to forward the projection, view, and model matrices to the shader programs
	// Get the location of the uniform variables "projection" and "modelview"
	uProjection = glGetUniformLocation(shaderProgram, "projection");
	uView = glGetUniformLocation(shaderProgram, "view");
	uModel = glGetUniformLocation(shaderProgram, "model");

	toWorld = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));

	// Now send these values to the shader program
	glUniformMatrix4fv(uProjection, 1, GL_FALSE, &Window::P[0][0]);
	glUniformMatrix4fv(uView, 1, GL_FALSE, &C[0][0]);
	glUniformMatrix4fv(uModel, 1, GL_FALSE, &toWorld[0][0]);

	// Now draw the cube. We simply need to bind the VAO associated with it.
	glBindVertexArray(VAO);
	// Tell OpenGL to draw with triangles, using 36 indices, the type of the indices, and the offset to start from
	glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);
	// Unbind the VAO when we're done so we don't accidentally draw extra stuff or tamper with its bound buffers
	glBindVertexArray(0);
}