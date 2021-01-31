#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <ctime>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"

#include "Box.cpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Skybox.h"
#include "Texture.h"

//programs
GLuint program;
GLuint programSun;
GLuint programSkybox;
GLuint programColor;
GLuint programTexture;

//Textures
GLuint textureEarth;
GLuint textureAsteroid;
GLuint textureSun;
GLuint textureMoon;
GLuint textureMars;
GLuint textureVenus;
GLuint shipTexture;

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

glm::vec3 lightPos = glm::vec3(0.0f, 30.0f, -5000.0f);

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

void drawObjectTexture(GLuint programTex, Core::RenderContext context, glm::mat4 modelMatrix, GLuint tex)
{
	glUseProgram(programTex);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniform3f(glGetUniformLocation(programTex, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(glGetUniformLocation(programTex, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniformMatrix4fv(glGetUniformLocation(programTex, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniformMatrix4fv(glGetUniformLocation(programTex, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	Core::SetActiveTexture(tex, "textureSampler", programTex, 0);
	Core::DrawContext(context);
	glUseProgram(0);
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

	renderSkybox(programSkybox, cameraMatrix, perspectiveMatrix);
	//drawObject(program,sphereContext, rotate1 * glm::translate(glm::vec3(0, 0, 10)) * planetScale3 * rotate3, glm::vec3(0.5f, 0.0f, 0.5f)); //darkred
	drawObjectTexture(programTexture, sphereContext, rotate1 * glm::translate(glm::vec3(0, 0, 10)) * planetScale3 * rotate3, textureVenus);
	drawObjectTexture(programTexture, sphereContext, rotate2 * glm::translate(glm::vec3(0, 0, -7)) * planetScale1 * rotate3, textureMars);
	drawObjectTexture(programTexture, sphereContext, rotate3 * glm::translate(glm::vec3(0, 0, 4)) * planetScale2 * rotate3, textureEarth); //Earth
	drawObjectTexture(programTexture, sphereContext, rotate3 * glm::translate(glm::vec3(0, 0, 4)) * moonRotate * glm::translate(glm::vec3(0.25, 0.5, 1.5)) *
		moonScale, textureMoon);//moon
	drawObjectTexture(programSun,sphereContext,glm::translate(glm::vec3(0,0,0)),textureSun); //Sun


	drawObjectTexture(programTexture,shipContext, shipModelMatrix, shipTexture);

	glutSwapBuffers();
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	program = shaderLoader.CreateProgram("shaders/shader_4_1.vert", "shaders/shader_4_1.frag");
	programSun = shaderLoader.CreateProgram("shaders/shader_sun_tex.vert", "shaders/shader_sun_tex.frag");
	programSkybox = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_texture.vert", "shaders/shader_texture.frag");

	textureEarth = Core::LoadTexturePNG("textures/Earth/earth2.png");
	textureSun = Core::LoadTexturePNG("textures/Sun/sunTex.png");
	textureMoon = Core::LoadTexturePNG("textures/Moon/moon2.png");
	textureAsteroid = Core::LoadTexturePNG("textures/Asteroid/asteroid.png");
	textureMars = Core::LoadTexturePNG("textures/Planet/mars.png");
	textureVenus = Core::LoadTexturePNG("textures/Planet/venus.png");

	sphereModel = obj::loadModelFromFile("models/sphere.obj");
	shipModel = obj::loadModelFromFile("models/StarSparrow02.obj");
	sphereContext.initFromOBJ(sphereModel);
	shipContext.initFromOBJ(shipModel);

	shipTexture = Core::LoadTexturePNG("textures/StarSparrow_Blue.png");

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
