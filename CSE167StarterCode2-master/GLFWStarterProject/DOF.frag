#version 330 core

uniform sampler2D texColor2;
uniform sampler2D texDepth;
uniform int debugMode;

in vec2 ioTexCoord;

out vec4 color;

void main()
{
	if (debugMode == 0)
	{
		// Debugging disabled. Shows post-processed result.
		color = vec4(texture(texColor2, ioTexCoord).rgb * (texture(texDepth, ioTexCoord).x - 0.9) * 10.0, 1.0);
	}
}
