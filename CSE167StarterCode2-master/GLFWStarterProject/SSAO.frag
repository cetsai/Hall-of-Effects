#version 330 core

const vec3 noise[16] = vec3[16](
	vec3(-0.7712,0.0895,0.0),
	vec3(-0.9634,-0.9824,0.0),
	vec3(-0.2158,-0.8968,0.0),
	vec3(0.8886,-0.0464,0.0),
	vec3(0.8228,-0.429,0.0),
	vec3(-0.2216,0.4194,0.0),
	vec3(0.674,0.8976,0.0),
	vec3(0.911,0.7538,0.0),
	vec3(0.8924,-0.59,0.0),
	vec3(-0.4104,0.9018,0.0),
	vec3(-0.7862,-0.0921,0.0),
	vec3(0.633,-0.284,0.0),
	vec3(0.764,-0.8878,0.0),
	vec3(-0.845,-0.4348,0.0),
	vec3(-0.6242,-0.4822,0.0),
	vec3(0.9616,-0.6778,0.0)
);

uniform sampler2D texColor;
uniform sampler2D texNormal;
uniform sampler2D texDepth;
uniform int debugMode;

in vec2 ioTexCoord;

out vec4 color;

void main()
{
	if (debugMode == 0)
	{
		// Debugging disabled. Shows post-processed result.
		color = texture(texColor, ioTexCoord);

		//color = vec4(noise[int(mod(gl_FragCoord.y, 4.0) * 4.0 + mod(gl_FragCoord.x, 4.0))] * 0.5 + 0.5, 1.0);
	}
	else if (debugMode == 1)
	{
		// Shows normals
		color = texture(texNormal, ioTexCoord);
	}
	else if (debugMode == 2)
	{
		// Shows depth
		color = vec4(vec3(texture(texDepth, ioTexCoord).r), 1.0);
	}
}
