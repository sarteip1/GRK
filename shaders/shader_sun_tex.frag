#version 430 core

//uniform vec3 objectColor;
//uniform vec3 lightDir;
uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform sampler2D textureSampler;

in vec3 interpNormal;
in vec3 fragPos;
in vec2 vertTexCoord;

void main()
{
	vec3 normal = normalize(interpNormal);
	vec3 V = normalize(cameraPos-fragPos);
	float coef = max(0,dot(V,normal));

	vec4 textureColor = texture2D(textureSampler, vertTexCoord);

	gl_FragColor.rgb = mix(textureColor.xyz, textureColor.xyz * vec3(3.0f) * coef, 1.0 - 0.1);
	gl_FragColor.a = 1.0;
}
