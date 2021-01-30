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
	vec3 lightDir = normalize(lightPos - fragPos);
	vec3 V = normalize(cameraPos - fragPos);
	vec3 normal = normalize(interpNormal);
	vec3 R = reflect(-lightDir, normal);

	vec4 textureColor = texture2D(textureSampler, vertTexCoord);

	float specular = pow(max(0, dot(R, V)), 5);
	float diffuse = max(0, dot(normal, lightDir));
	float ambient = 0.2;
	gl_FragColor.rgb = mix(textureColor.xyz, textureColor.xyz * diffuse + vec3(0.2f) * specular, 1.0 - ambient);
	gl_FragColor.a = 1.0;
}