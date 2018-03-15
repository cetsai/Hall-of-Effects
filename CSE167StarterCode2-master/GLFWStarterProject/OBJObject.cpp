#include "OBJObject.h"

extern GLint shaderProgramPhong;
extern glm::vec3 cam_pos;

void OBJObject::addSkyboxFace(GLfloat tlx, GLfloat tly, GLfloat tlz, GLfloat trx, GLfloat try_, GLfloat trz, GLfloat blx, GLfloat bly, GLfloat blz, GLfloat brx, GLfloat bry, GLfloat brz)
{
	GLuint indexBase = vertices.size() / 3;
	
	vertices.push_back(tlx); vertices.push_back(tly); vertices.push_back(tlz);
	normals.push_back((GLfloat)1.0f); normals.push_back((GLfloat)1.0f); normals.push_back((GLfloat)1.0f);
	vertices.push_back(trx); vertices.push_back(try_); vertices.push_back(trz);
	normals.push_back((GLfloat)1.0f); normals.push_back((GLfloat)1.0f); normals.push_back((GLfloat)1.0f);
	vertices.push_back(blx); vertices.push_back(bly); vertices.push_back(blz);
	normals.push_back((GLfloat)1.0f); normals.push_back((GLfloat)1.0f); normals.push_back((GLfloat)1.0f);
	vertices.push_back(brx); vertices.push_back(bry); vertices.push_back(brz);
	normals.push_back((GLfloat)1.0f); normals.push_back((GLfloat)1.0f); normals.push_back((GLfloat)1.0f);

	faces.push_back(glm::ivec3(indexBase + 0, indexBase + 2, indexBase + 1));
	indices.push_back((GLuint)(indexBase + 0));
	indices.push_back((GLuint)(indexBase + 2));
	indices.push_back((GLuint)(indexBase + 1));

	faces.push_back(glm::ivec3(indexBase + 1, indexBase + 2, indexBase + 3));
	indices.push_back((GLuint)(indexBase + 1));
	indices.push_back((GLuint)(indexBase + 2));
	indices.push_back((GLuint)(indexBase + 3));
}

OBJObject::OBJObject(const char *filepath, float x, float y, float z, float scale, glm::vec3 k_d, glm::vec3 k_s, float p_s)
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

	this->k_d = k_d;
	this->k_s = k_s;
	this->p_s = p_s;

	if (filepath == NULL)
	{
		GLfloat r = 1000.0f;

		addSkyboxFace(-r, r, -r, r, r, -r, -r, -r, -r, r, -r, -r);
		addSkyboxFace(r, r, -r, r, r, r, r, -r, -r, r, -r, r);
		addSkyboxFace(r, r, r, -r, r, r, r, -r, r, -r, -r, r);
		addSkyboxFace(-r, r, r, -r, r, -r, -r, -r, r, -r, -r, -r);
		addSkyboxFace(-r, r, r, r, r, r, -r, r, -r, r, r, -r);
		addSkyboxFace(-r, -r, -r, r, -r, -r, -r, -r, r, r, -r, r);

		printf("Skybox is being created.\n");
	}
	else
	{
		parse(filepath);
	}

	createNormalizeMatrices();
	rotMatrix = glm::mat4(1.0f);

	toWorld = glm::mat4(1.0f);

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

	glBindBuffer(GL_ARRAY_BUFFER, NBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 0. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
		3, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		3 * sizeof(GLfloat), // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0);

	// Unbind the currently bound buffer so that we don't accidentally make unwanted changes to it.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Unbind the VAO now so we don't accidentally tamper with it.
	// NOTE: You must NEVER unbind the element array buffer associated with a VAO!
	glBindVertexArray(0);
}

OBJObject::~OBJObject()
{
	// Delete previously generated buffers. Note that forgetting to do this can waste GPU memory in a 
	// large project! This could crash the graphics driver due to memory leaks, or slow down application performance!
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &NBO);
}

void OBJObject::createNormalizeMatrices()
{
	float inf = std::numeric_limits<float>::infinity();
	float xMin = inf, yMin = inf, zMin = inf;
	float xMax = -inf, yMax = -inf, zMax = -inf;

	for (unsigned int i = 0; i < vertices.size(); i += 3)
	{
		if (vertices[i] < xMin) xMin = vertices[i];
		if (vertices[i + 1] < yMin) yMin = vertices[i + 1];
		if (vertices[i + 2] < zMin) zMin = vertices[i + 2];
		if (vertices[i] > xMax) xMax = vertices[i];
		if (vertices[i + 1] > yMax) yMax = vertices[i + 1];
		if (vertices[i + 2] > zMax) zMax = vertices[i + 2];
	}

	createNormalizeTransMatrix(xMin, yMin, zMin, xMax, yMax, zMax);
	createNormalizeScaleMatrix(xMin, yMin, zMin, xMax, yMax, zMax);
}

void OBJObject::createNormalizeTransMatrix(float xMin, float yMin, float zMin, float xMax, float yMax, float zMax)
{
	normalizeTransMatrix = glm::translate(glm::mat4(1.0), glm::vec3(-(xMax + xMin) / 2, -(yMax + yMin) / 2, -(zMax + zMin) / 2));
}

void OBJObject::createNormalizeScaleMatrix(float xMin, float yMin, float zMin, float xMax, float yMax, float zMax)
{
	float scaleFactor = 2.0f / std::max(xMax - xMin, std::max(yMax - yMin, zMax - zMin));
	normalizeScaleMatrix = glm::scale(glm::mat4(1.0), glm::vec3(scaleFactor, scaleFactor, scaleFactor));
}

void OBJObject::parse(const char *filepath)
{
	//TODO parse the OBJ file
	// Populate the face indices, vertices, and normals vectors with the OBJ Object data

	FILE* fp;
	float x, y, z;
	float r, g, b;
	float nx, ny, nz;
	int v0, v1, v2;
	int n0, n1, n2;

	fp = fopen(filepath, "rb");

	if (fp == NULL) {
		printf("The file could not be opened.");
	}

	bool run = true;
	int c;
	glm::vec3 normal;

	while (run) {
		c = fgetc(fp);
		//printf("first c: %c\n", c);

		switch (c) {
		case 'v':
			c = fgetc(fp);
			//printf("second c: %c\n", c);

			if (c == ' ') {
				fscanf(fp, "%f %f %f", &x, &y, &z);

				if (fgetc(fp) == ' ')
				{
					fscanf(fp, "%f %f %f", &r, &g, &b);
				}

				//printf("x: %f, y: %f, z: %f\n", x, y, z);
				vertices.push_back((GLfloat)x);
				vertices.push_back((GLfloat)y);
				vertices.push_back((GLfloat)z);
				//printf("vertices: %f, %f, %f\n", (GLfloat)x, (GLfloat)y, (GLfloat)z);
			}
			else {
				fscanf(fp, " %f %f %f", &nx, &ny, &nz);
				normal = glm::normalize(glm::vec3(nx, ny, nz));
				//printf("nx: %f, ny: %f, nz: %f\n", nx, ny, nz);
				normals.push_back((GLfloat)normal.x);
				normals.push_back((GLfloat)normal.y);
				normals.push_back((GLfloat)normal.z);
				//printf("%f, %f, %f\n", normal.x, normal.y, normal.z);
			}

			break;

		case 'f':
			fscanf(fp, " %d//%d %d//%d %d//%d", &v0, &n0, &v1, &n1, &v2, &n2);
			faces.push_back(glm::ivec3(v0 - 1, v1 - 1, v2 - 1));
			indices.push_back((GLuint)(v0 - 1));
			indices.push_back((GLuint)(v1 - 1));
			indices.push_back((GLuint)(v2 - 1));
			//printf("face: %d %d %d\n", (GLuint)(v0 - 1), (GLuint)(v1 - 1), (GLuint)(v2 - 1));
			break;
		}

		// Skips the rest of the line.
		while (true) {
			c = fgetc(fp);

			if (c == EOF) {
				run = false;
				break;
			}
			else if (c == '\n') {
				break;
			}
		}
	}

	fclose(fp);

	//printf("File parsed.\n");
}

void OBJObject::update()
{
	toWorld = glm::mat4(1.0);

	toWorld = glm::translate(toWorld, glm::vec3(x, y, z));
	toWorld = glm::scale(toWorld, glm::vec3(scaleX, scaleY, scaleZ));

	toWorld *= rotMatrix;

	toWorld *= normalizeScaleMatrix;
	toWorld *= normalizeTransMatrix;
}

void OBJObject::draw(glm::mat4 C)
{
	s_directionalLight* directionalLight = Window::currentDirectionalLight;

	GLint shaderProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &shaderProgram);

	// We need to calcullate this because modern OpenGL does not keep track of any matrix other than the viewport (D)
	// Consequently, we need to forward the projection, view, and model matrices to the shader programs
	// Get the location of the uniform variables "projection" and "modelview"
	uProjection = glGetUniformLocation(shaderProgram, "projection");
	uView = glGetUniformLocation(shaderProgram, "view");
	uModel = glGetUniformLocation(shaderProgram, "model");
	
	toWorld = normalizeScaleMatrix * normalizeTransMatrix;

	// Now send these values to the shader program
	glUniformMatrix4fv(uProjection, 1, GL_FALSE, &Window::P[0][0]);
	glUniformMatrix4fv(uView, 1, GL_FALSE, &C[0][0]);
	glUniformMatrix4fv(uModel, 1, GL_FALSE, &toWorld[0][0]);
	
	uAmbientColor = glGetUniformLocation(shaderProgram, "ambientColor");
	glUniform3f(uAmbientColor, ambientColor.x, ambientColor.y, ambientColor.z);

	glUniform1i(glGetUniformLocation(shaderProgram, "directionalLight.enabled"), directionalLight->enabled);
	glUniform3f(glGetUniformLocation(shaderProgram, "directionalLight.color"), directionalLight->color.x, directionalLight->color.y, directionalLight->color.z);
	glUniform3f(glGetUniformLocation(shaderProgram, "directionalLight.direction"), directionalLight->direction.x, directionalLight->direction.y, directionalLight->direction.z);
	glUniform3f(glGetUniformLocation(shaderProgram, "directionalLight.ambientColor"), directionalLight->ambientColor.x, directionalLight->ambientColor.y, directionalLight->ambientColor.z);

	glUniform1i(glGetUniformLocation(shaderProgram, "pointLight.enabled"), false);
	glUniform3f(glGetUniformLocation(shaderProgram, "pointLight.color"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(shaderProgram, "pointLight.position"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(shaderProgram, "pointLight.ambientColor"), 0.0f, 0.0f, 0.0f);
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLight.radius"), 0.0f);

	glUniform1i(glGetUniformLocation(shaderProgram, "spotLight.enabled"), false);
	glUniform3f(glGetUniformLocation(shaderProgram, "spotLight.color"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(shaderProgram, "spotLight.position"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(shaderProgram, "spotLight.direction"), 0.0f, 0.0f, 0.0f);
	glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.cutoff"), 0.0f);
	glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.exponent"), 1.0f);
	glUniform3f(glGetUniformLocation(shaderProgram, "spotLight.ambientColor"), 0.0f, 0.0f, 0.0f);
	glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.radius"), 0.0f);

	uK_d = glGetUniformLocation(shaderProgram, "k_d");
	glUniform3f(uK_d, k_d.x, k_d.y, k_d.z);

	uK_s = glGetUniformLocation(shaderProgram, "k_s");
	glUniform3f(uK_s, k_s.x, k_s.y, k_s.z);

	uP_s = glGetUniformLocation(shaderProgram, "p_s");
	glUniform1f(uP_s, p_s);

	uCameraPosition = glGetUniformLocation(shaderProgram, "cameraPosition");
	glUniform3f(uCameraPosition, cam_pos.x, cam_pos.y, cam_pos.z);

	// Now draw the cube. We simply need to bind the VAO associated with it.
	glBindVertexArray(VAO);
	// Tell OpenGL to draw with triangles, using 36 indices, the type of the indices, and the offset to start from
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	// Unbind the VAO when we're done so we don't accidentally draw extra stuff or tamper with its bound buffers
	glBindVertexArray(0);
}