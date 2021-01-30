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
GLuint texturePlanet1;
GLuint texturePlanet2;

//Models
obj::Model sphereModel;


Core::Shader_Loader shaderLoader;

Core::RenderContext sphereContext;


Core::RenderContext armContext;
std::vector<Core::Node> arm;
int ballIndex;

float cameraAngle = 0;
glm::vec3 cameraPos = glm::vec3(-6, 0, 0);
glm::vec3 cameraDir;

glm::mat4 cameraMatrix, perspectiveMatrix;

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

void renderScene()
{
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.f;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	glUseProgram(program);

	// Macierz statku "przyczpeia" go do kamery. Wrato przeanalizowac te linijke i zrozumiec jak to dziala.
	//glm::vec3 lightPos = glm::vec3(-4, 1, -4);
	glm::vec3 lightPos = glm::vec3(0.0f);
	//glUniform3f(glGetUniformLocation(program, "light_dir"), 2, 1, 0);
	glUniform3f(glGetUniformLocation(program, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	renderSkybox(programSkybox, cameraMatrix, perspectiveMatrix);
	drawObject(program, sphereContext, glm::translate(glm::vec3(0, 0, 0)), glm::vec3(1.0f, 0.0f, 0.0f));

	glUseProgram(0);
	glutSwapBuffers();
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	program = shaderLoader.CreateProgram("shaders/shader_4_1.vert", "shaders/shader_4_1.frag");
	programSun = shaderLoader.CreateProgram("shaders/shader_4_sun.vert", "shaders/shader_4_sun.frag");
	programSkybox = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");

	sphereModel = obj::loadModelFromFile("models/sphere.obj");
	sphereContext.initFromOBJ(sphereModel);
	initSkybox();
}

void shutdown()
{
	shaderLoader.DeleteProgram(program);
	shaderLoader.DeleteProgram(programSkybox);
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 300);
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
