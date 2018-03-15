#version 330 core
// This is a sample fragment shader.

// Inputs to the fragment shader are the outputs of the same name from the vertex shader.
// Note that you do not have access to the vertex shader's default output, gl_Position.
in float sampleExtraOutput;

in vec3 vNormal;
in vec3 textureDir; // direction vector representing a 3D texture coordinate

uniform samplerCube cubemap; // cubemap texture sampler

// You can output many things. The first vec4 type output determines the color of the fragment
layout(location = 0) out vec4 color;
layout(location = 1) out vec4 oNormal;

void main()
{
    color = texture(cubemap, textureDir);

	oNormal = vec4(0.0, 0.0, 0.0, 1.0);
}
