#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <ctime>

#include "Skybox.h"

#include <fstream>
#include <iterator>
#include <vector>
#include "stb_image.h"

//#include <fstream>
//#include <iterator>
//#include <vector>
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
//#include <iostream>
//#include "glew.h"
//#include "freeglut.h"
//#include "glm.hpp"

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"

#include "Box.cpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Texture.h"

//programs
GLuint program;
GLuint programSun;
GLuint programSkybox;
GLuint programColor;
GLuint programTexture;

//Textures
GLuint textureEarth, textureEarthNormal;
GLuint textureAsteroid, textureAsteroidNormal;
GLuint textureSun, textureSunNormal;
GLuint textureMoon, textureMoonNormal;
GLuint textureMars, textureMarsNormal;
GLuint textureVenus, textureVenusNormal;
GLuint shipTexture, shipTextureNormal;

//Models
obj::Model sphereModel;
obj::Model shipModel;

Core::Shader_Loader shaderLoader;

Core::RenderContext sphereContext;
Core::RenderContext shipContext;

float cameraAngle = 0;
glm::vec3 cameraPos = glm::vec3(-6, 0, 0);
glm::vec3 cameraDir;

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -1.0f, -1.0f));

void keyboard(unsigned char key, int x, int y)
{
	float angleSpeed = 0.1f;
	float moveSpeed = 0.1f;
	switch (key)
	{
	case 'z': cameraAngle -= angleSpeed; break;
	case 'x': cameraAngle += angleSpeed; break;
	case 'w': cameraPos += cameraDir * moveSpeed; break;
	case 's': cameraPos -= cameraDir * moveSpeed; break;
	case 'd': cameraPos += glm::cross(cameraDir, glm::vec3(0, 1, 0)) * moveSpeed; break;
	case 'a': cameraPos -= glm::cross(cameraDir, glm::vec3(0, 1, 0)) * moveSpeed; break;
	case 'e': cameraPos += glm::cross(cameraDir, glm::vec3(1, 0, 0)) * moveSpeed; break;
	case 'q': cameraPos -= glm::cross(cameraDir, glm::vec3(1, 0, 0)) * moveSpeed; break;
	}
}

glm::mat4 createCameraMatrix()
{
	cameraDir = glm::vec3(cosf(cameraAngle), 0.0f, sinf(cameraAngle));
	glm::vec3 up = glm::vec3(0, 1, 0);

	return Core::createViewMatrix(cameraPos, cameraDir, up);
}

void drawObject(GLuint program, Core::RenderContext context, glm::mat4 modelMatrix, glm::vec3 color)
{
	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);

	Core::DrawContext(context);
	glUseProgram(0);
}

void drawObjectTexture(GLuint program, obj::Model *model, glm::mat4 modelMatrix, GLuint tex, GLuint normalmapId)
{
	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	Core::SetActiveTexture(tex, "textureSampler", program, 0);
	Core::SetActiveTexture(normalmapId, "normalSampler", program, 1);
	Core::DrawModel(model);
	glUseProgram(0);
}

//GLuint skyboxVAO;
//unsigned int cubemapTexture;

//std::vector<std::string> faces
//{
//	"textures/Skybox/vision6/right.png",
//	"textures/Skybox/vision6/left.png",
//	"textures/Skybox/vision6/top.png",
//	"textures/Skybox/vision6/bottom.png",
//	"textures/Skybox/vision6/front.png",
//	"textures/Skybox/vision6/back.png"
//};

unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void renderScene()
{
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.f;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	glUseProgram(program);

	glm::mat4 shipModelMatrix =
		glm::translate(cameraPos + cameraDir * 0.5f + glm::vec3(0, -0.25f, 0))
		* glm::rotate(-cameraAngle + glm::radians(90.0f), glm::vec3(0, 1, 0))
		* glm::scale(glm::vec3(0.03f));

	glm::mat4 rotate1, rotate2, rotate3, moonRotate;
	rotate1 = glm::rotate((time / 10.0f) * 2 * 3.14159f, glm::vec3(0.0f, 2.0f, 0.0f));
	rotate2 = glm::rotate((time / 12.0f) * 2 * 3.14159f, glm::vec3(0.0f, 2.0f, 0.0f));
	rotate3 = glm::rotate((time / 15.0f) * 2 * 3.14159f, glm::vec3(0.0f, 2.0f, 0.0f));
	moonRotate = glm::rotate((time / 15.0f) * 2 * 3.14159f, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 moonScale, sunScale, planetScale1, planetScale2, planetScale3;
	moonScale = glm::scale(glm::vec3(0.3, 0.3, 0.3));
	sunScale = glm::scale(glm::vec3(1.5, 1.5, 1.5));
	planetScale1 = glm::scale(glm::vec3(0.8, 0.8, 0.8));
	planetScale2 = glm::scale(glm::vec3(1.2, 1.2, 1.2));
	planetScale3 = glm::scale(glm::vec3(1.0, 1.0, 1.0));

	
	//drawObject(program,sphereContext, rotate1 * glm::translate(glm::vec3(0, 0, 10)) * planetScale3 * rotate3, glm::vec3(0.5f, 0.0f, 0.5f));

	//glDepthMask(GL_FALSE);
	//programSkybox.use();
	//glBindVertexArray(skyboxVAO);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	//glDrawArrays(GL_TRIANGLES, 0, 36);
	//glDepthMask(GL_TRUE);

	renderSkybox(programSkybox, cameraMatrix, perspectiveMatrix);

	drawObjectTexture(programTexture, &sphereModel, rotate1 * glm::translate(glm::vec3(0, 0, 10)) * planetScale3 * rotate3, textureVenus, textureVenusNormal);
	drawObjectTexture(programTexture, &sphereModel, rotate2 * glm::translate(glm::vec3(0, 0, -7)) * planetScale1 * rotate3, textureMars, textureMarsNormal);
	drawObjectTexture(programTexture, &sphereModel, rotate3 * glm::translate(glm::vec3(0, 0, 4)) * planetScale2 * rotate3, textureEarth, textureEarthNormal);
	drawObjectTexture(programTexture, &sphereModel, rotate3 * glm::translate(glm::vec3(0, 0, 4)) * moonRotate * glm::translate(glm::vec3(0.25, 0.5, 1.5)) *
		moonScale, textureMoon, textureMoonNormal);
	drawObjectTexture(programSun, &sphereModel,glm::translate(glm::vec3(0,0,0)), textureSun, textureMoonNormal);


	drawObjectTexture(programTexture, &shipModel, shipModelMatrix, shipTexture, shipTextureNormal);

	glutSwapBuffers();
}


void init()
{
	glEnable(GL_DEPTH_TEST);
	program = shaderLoader.CreateProgram("shaders/shader_4_1.vert", "shaders/shader_4_1.frag");
	programSun = shaderLoader.CreateProgram("shaders/shader_sun_tex.vert", "shaders/shader_sun_tex.frag");
	programSkybox = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");

	textureEarth = Core::LoadTexture("textures/Earth/earth2.png");
	textureSun = Core::LoadTexture("textures/Sun/sunTex.png");
	textureMoon = Core::LoadTexture("textures/Moon/moon2.png");
	textureAsteroid = Core::LoadTexture("textures/Asteroid/asteroid.png");
	textureMars = Core::LoadTexture("textures/Planet/mars.png");
	textureVenus = Core::LoadTexture("textures/Planet/venus.png");

	textureVenusNormal = Core::LoadTexture("textures/Planet/venus_normal.png");
	textureEarthNormal = Core::LoadTexture("textures/Earth/earth2_normals.png");
	//textureAsteroidNormal;
	textureSunNormal = Core::LoadTexture("textures/Moon/moon_normal.png");;
	textureMoonNormal = Core::LoadTexture("textures/Moon/moon_normal.png");;
	textureMarsNormal = Core::LoadTexture("textures/Planet/mars_normal.png");;
	shipTextureNormal = Core::LoadTexture("textures/StarSparrow_Normal.png");

	sphereModel = obj::loadModelFromFile("models/sphere.obj");
	shipModel = obj::loadModelFromFile("models/StarSparrow02.obj");
	//sphereContext.initFromOBJ(sphereModel);
	//shipContext.initFromOBJ(shipModel);

	shipTexture = Core::LoadTexture("textures/StarSparrow_Blue.png");

	//cubemapTexture = loadCubemap(faces);

	initSkybox();
}

void shutdown()
{
	shaderLoader.DeleteProgram(program);
	shaderLoader.DeleteProgram(programSun);
	shaderLoader.DeleteProgram(programSkybox);
	shaderLoader.DeleteProgram(programTexture);
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(600, 600);
	glutCreateWindow("OpenGL Pierwszy Program");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}
