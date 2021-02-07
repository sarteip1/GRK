#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in vec3 vertexTangent;
layout(location = 4) in vec3 vertexBitangent;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelMatrix;

uniform vec3 lightPos;
uniform vec3 cameraPos;

out vec2 interpTexCoord;
out vec3 lightDirTS;
out vec3 viewDirTS;

void main()
{
	vec3 vertPos;

	vertPos = (modelMatrix * vec4(vertexPosition, 1.0)).xyz;

	vec3 lightDir = normalize(vertPos - lightPos);
	
	vec3 normal = vec3(modelMatrix * vec4(vertexNormal,0.0));
	vec3 tangent = vec3(modelMatrix * vec4(vertexTangent,0.0));
	vec3 bitangent = vec3(modelMatrix * vec4(vertexBitangent,0.0));

	mat3x3 TBN = transpose(mat3(tangent,bitangent,normal));
	
	vec3 viewDir = normalize(cameraPos - vertPos);
	lightDirTS = TBN * lightDir;
	viewDirTS = TBN * viewDir;

	gl_Position = modelViewProjectionMatrix * vec4(vertexPosition, 1.0);
	interpTexCoord = vertexTexCoord;
}
