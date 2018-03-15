#version 330 core
// This is a sample fragment shader.

struct s_directionalLight {
	int enabled;
	vec3 color;
	vec3 direction;
	vec3 ambientColor;
};

struct s_pointLight {
	int enabled;
	vec3 color;
	vec3 position;
	vec3 ambientColor;
	float radius;
};

struct s_spotLight {
	int enabled;
	vec3 color;
	vec3 position;
	vec3 direction;
	float cutoff;
	float exponent;
	vec3 ambientColor;
	float radius;
};

// Inputs to the fragment shader are the outputs of the same name from the vertex shader.
// Note that you do not have access to the vertex shader's default output, gl_Position.
in float sampleExtraOutput;

in vec3 vNormal;
in vec3 vPosition;
in vec3 vCameraPosition;
in float ioDepth;

uniform s_directionalLight directionalLight;
uniform s_pointLight pointLight;
uniform s_spotLight spotLight;
uniform vec3 k_d;
uniform vec3 k_s;
uniform float p_s;

// You can output many things. The first vec4 type output determines the color of the fragment
layout(location = 0) out vec4 color;
layout(location = 1) out vec4 oNormal;

void main()
{
	color = vec4(0.0, 0.0, 0.0, sampleExtraOutput);
	
	float k = 0.3;

	if (directionalLight.enabled == 1)
	{
		vec3 L = normalize(-directionalLight.direction);

		float LDotN = dot(L, normalize(vNormal));

		float diffuseFactor = max(0.0, LDotN);
	
		vec3 R = normalize(2.0 * LDotN * normalize(vNormal) - L);
		vec3 e = normalize(vCameraPosition - vPosition);
		float specularFactor = pow(max(0.0, dot(R, e)), p_s);

		color.rgb += directionalLight.color * (k_d * diffuseFactor + k_s * specularFactor + directionalLight.ambientColor);
	}

	if (pointLight.enabled == 1)
	{
		vec3 L = normalize(pointLight.position - vPosition);

		float LDotN = dot(L, normalize(vNormal));

		float diffuseFactor = max(0.0, LDotN);
	
		vec3 R = normalize(2.0 * LDotN * normalize(vNormal) - L);
		vec3 e = normalize(vCameraPosition - vPosition);
		float specularFactor = pow(max(0.0, dot(R, e)), p_s);

		float attenuation = distance(pointLight.position, vPosition) * k;
		color.rgb += pointLight.color * (k_d * diffuseFactor + k_s * specularFactor + pointLight.ambientColor) / attenuation;
	}

	if (spotLight.enabled == 1)
	{
		vec3 L = normalize(spotLight.position - vPosition);

		float LDotN = dot(L, normalize(vNormal));

		float diffuseFactor = max(0.0, LDotN);
	
		vec3 R = normalize(2.0 * LDotN * normalize(vNormal) - L);
		vec3 e = normalize(vCameraPosition - vPosition);
		float specularFactor = pow(max(0.0, dot(R, e)), p_s);

		float attenuation = pow(distance(spotLight.position, vPosition), 2.0) * k;
		float cutoffFactor = 0.0;
		
		if (dot(-L, normalize(spotLight.direction)) > cos(spotLight.cutoff))
		{
			cutoffFactor = pow(dot(-L, normalize(spotLight.direction)), spotLight.exponent);
		}

		color.rgb += spotLight.color * (k_d * diffuseFactor + k_s * specularFactor + spotLight.ambientColor) * cutoffFactor / attenuation;
	}

	oNormal = vec4(normalize(vNormal) * 0.5 + 0.5, 1.0);
	//color = vec4(vec3(ioDepth * 0.01), 1.0);
}
