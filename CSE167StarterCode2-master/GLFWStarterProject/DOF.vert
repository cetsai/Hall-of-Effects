#version 330 core

layout (location = 0) in vec2 position;

uniform mat4 modelMatrix;

out vec2 ioTexCoord;

void main()
{
    gl_Position = modelMatrix * vec4(position, 0.0, 1.0);
	ioTexCoord = position * 0.5 + 0.5;
}
