#include "Mesh.h"
#include "Window.h"

Mesh::Mesh(std::vector<Vertex> vs, std::vector<GLuint> is, Material m){

	toWorld = glm::scale(glm::mat4(1.0), glm::vec3(10, 10, 10));

	vertices = vs;
	mat = m;
	indices = is;
	
	glGenVertexArrays(1,&VAO);
	glGenBuffers(1,&VBO);
	glGenBuffers(1,&EBO);
	
	glBindVertexArray(VAO);
	
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
	

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
	
	// position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,
						  3,
						 GL_FLOAT,
						 GL_FALSE,
						 sizeof(Vertex),
						 (GLvoid*) 0);
	
	// normal attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,
						  3,
						 GL_FLOAT,
						 GL_FALSE,
						 sizeof(Vertex),
						(GLvoid*)offsetof(Vertex, norm));

	// texture coordinate attribute
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(GLvoid*)offsetof(Vertex, tc));
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
}

Mesh::~Mesh(){
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void Mesh::draw(GLuint shaderProgram){

	//set uniforms
	GLuint uColor = glGetUniformLocation(shaderProgram, "aColor");
	GLuint uTexture = glGetUniformLocation(shaderProgram, "aTexture");
	
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &toWorld[0][0]);
	glUniform4f(uColor, 0.5, 0, 0.5, 1.0);
	
	// set texture
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(uTexture, 0);
	glBindTexture(GL_TEXTURE_2D, mat.texture);

	// draw vertices
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}