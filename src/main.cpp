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

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"

#include "Box.cpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Texture.h"
#include "Physics.h"

float frustumScale = 1.f;

Physics pxScene(9.8);

// fixed timestep for stable and deterministic simulation
const double physicsStepTime = 1.f / 90.0f;
double physicsTimeToProcess = 0;

// physical objects
//PxRigidStatic *planeBody = nullptr;
//PxMaterial *planeMaterial = nullptr;
//std::vector<PxRigidDynamic*> asteroidBodies;
//PxMaterial *asteroidMaterial = nullptr;
//PxRigidStatic *sphereBody = nullptr;
//PxMaterial *sphereMaterial = nullptr;
PxRigidDynamic *shipBody = nullptr;
PxMaterial* shipMaterial = nullptr;

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

//glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -1.0f, -1.0f));
glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, 0.0f);

static const int NUM_ASTEROIDS = 8;
glm::vec3 asteroidPositions[NUM_ASTEROIDS];

struct Renderable {
    obj::Model *model;
    glm::mat4 modelMatrix;
    GLuint textureId;
	GLuint textureNormal;
};
Renderable* ship;
    //rendHandle, rendSpheres[3], rendBox, rendGate;

void keyboard(unsigned char key, int x, int y)
{
	float angleSpeed = 0.1f;
	float moveSpeed = 0.1f;
	switch (key)
	{
	case 'z': cameraAngle -= angleSpeed; break;
	case 'x': cameraAngle += angleSpeed; break;
	case 'w': shipBody->addForce(PxVec3(cameraDir.x * -1 * 1000.0f, 0, cameraDir.z * -1 * 1000.0f), PxForceMode::eFORCE, true); break;
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

	glUniform3f(glGetUniformLocation(program, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	//glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	Core::SetActiveTexture(tex, "textureSampler", program, 0);
	Core::SetActiveTexture(normalmapId, "normalSampler", program, 1);
	Core::DrawModel(model);
	glUseProgram(0);
}

// unsigned int loadCubemap(std::vector<std::string> faces)
// {
// 	unsigned int textureID;
// 	glGenTextures(1, &textureID);
// 	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

// 	int width, height, nrChannels;
// 	for (unsigned int i = 0; i < faces.size(); i++)
// 	{
// 		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
// 		if (data)
// 		{
// 			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
// 				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
// 			);
// 			stbi_image_free(data);
// 		}
// 		else
// 		{
// 			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
// 			stbi_image_free(data);
// 		}
// 	}
// 	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
// 	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
// 	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

// 	return textureID;
// }

void updateTransforms()
{
    // Here we retrieve the current transforms of the objects from the physical simulation.
    auto actorFlags = PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC;
    PxU32 nbActors = pxScene.scene->getNbActors(actorFlags);
    if (nbActors)
    {
        std::vector<PxRigidActor*> actors(nbActors);
        pxScene.scene->getActors(actorFlags, (PxActor**)&actors[0], nbActors);
        for (auto actor : actors)
        {
            // We use the userData of the objects to set up the model matrices
            // of proper renderables.
            if (!actor->userData) continue;
            Renderable *renderable = (Renderable*)actor->userData;

            // get world matrix of the object (actor)
            PxMat44 transform = actor->getGlobalPose();
            auto &c0 = transform.column0;
            auto &c1 = transform.column1;
            auto &c2 = transform.column2;
            auto &c3 = transform.column3;

            // set up the model matrix used for the rendering
            glm::mat4 modelMatrix = glm::mat4(
                c0.x, c0.y, c0.z, c0.w,
                c1.x, c1.y, c1.z, c1.w,
                c2.x, c2.y, c2.z, c2.w,
                c3.x, c3.y, c3.z, c3.w);
            
            if (actor->getName() == "asteroid") modelMatrix = modelMatrix * glm::scale(glm::vec3(20.0f));
            if (actor->userData == ship) modelMatrix = modelMatrix * glm::scale(glm::vec3(0.08f)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, 1, 0)); // IMPORTANT!

            renderable->modelMatrix = modelMatrix;
        }
    }
}

void renderScene()
{
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix(0.1f, 100.0f, frustumScale);
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.f;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	glUseProgram(program);

	glm::mat4 shipModelMatrix =
		glm::translate(cameraPos + cameraDir * 0.85f + glm::vec3(0, -0.25f, 0))
		* glm::rotate(-cameraAngle + glm::radians(90.0f), glm::vec3(0, 1, 0))
		* glm::scale(glm::vec3(0.03f));

	ship->modelMatrix = shipModelMatrix;

	glm::mat4 rotate1, rotate2, rotate3, rotate4, moonRotate;
	rotate1 = glm::rotate((time / 100.0f) * 2 * 3.14159f, glm::vec3(0.0f, 2.0f, 0.0f));
	rotate2 = glm::rotate((time / 120.0f) * 2 * 3.14159f, glm::vec3(0.0f, 2.0f, 0.0f));
	rotate3 = glm::rotate((time / 150.0f) * 2 * 3.14159f, glm::vec3(0.0f, 2.0f, 0.0f));
	rotate4 = glm::rotate((time / 6.0f) * 2 * 3.14159f, glm::vec3(0.0f, 2.0f, 0.0f));
	moonRotate = glm::rotate((time / 15.0f) * 2 * 3.14159f, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 moonScale, sunScale, planetScale1, planetScale2, planetScale3;
	moonScale = glm::scale(glm::vec3(0.3, 0.3, 0.3));
	sunScale = glm::scale(glm::vec3(1.5, 1.5, 1.5));
	planetScale1 = glm::scale(glm::vec3(0.8, 0.8, 0.8));
	planetScale2 = glm::scale(glm::vec3(1.2, 1.2, 1.2));
	planetScale3 = glm::scale(glm::vec3(1.0, 1.0, 1.0));

	renderSkybox(programSkybox, cameraMatrix, perspectiveMatrix);

	//updateTransforms();

	drawObjectTexture(programTexture, &sphereModel, rotate1 * glm::translate(glm::vec3(0, 0, -10)) * planetScale3 * rotate4, textureVenus, textureMarsNormal);
	drawObjectTexture(programTexture, &sphereModel, rotate2 * glm::translate(glm::vec3(0, 0, 25)) * planetScale1 * rotate4, textureMars, textureMarsNormal);
	drawObjectTexture(programTexture, &sphereModel, rotate3 * glm::translate(glm::vec3(0, 0, 20)) * planetScale2 * rotate4, textureEarth, textureEarthNormal);
	drawObjectTexture(programTexture, &sphereModel, rotate3 * glm::translate(glm::vec3(0, 0, 20)) * moonRotate * glm::translate(glm::vec3(0.25, 0.5, 1.5)) *
		moonScale, textureMoon, textureMarsNormal);
	drawObjectTexture(programSun, &sphereModel,glm::translate(glm::vec3(0,0,0)), textureSun, textureMarsNormal);

	for (int i = 0; i < NUM_ASTEROIDS; i++) {
		drawObjectTexture(programTexture, &sphereModel, glm::translate(asteroidPositions[i]) * glm::scale(glm::vec3(0.2f)), textureAsteroid,textureAsteroidNormal);
	}


	drawObjectTexture(programTexture, ship->model, ship->modelMatrix, ship->textureId, ship->textureNormal);

	glutSwapBuffers();
}

void initPhysicsScene()
{
	shipBody = pxScene.physics->createRigidDynamic(PxTransform(0, 0, 0));
    shipMaterial = pxScene.physics->createMaterial(0.82f, 0.8f, 0.f);
    PxShape* shipShape = pxScene.physics->createShape(PxBoxGeometry(1.25f, 0.3f, 1.25f), *shipMaterial);
    shipBody->attachShape(*shipShape);
    shipShape->release();
    shipBody->setMass(5000.0f);
    shipBody->setMassSpaceInertiaTensor(PxVec3(0.f, 250.0f, 0.f));
    //spaceshipBody->setRigidDynamicLockFlags(PxRigidDynamicLockFlag::eLOCK_LINEAR_Y | PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z | PxRigidDynamicLockFlag::eLOCK_ANGULAR_X);
    shipBody->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
    shipBody->userData = ship;

    pxScene.scene->addActor(*shipBody);
}


void init()
{
	glEnable(GL_DEPTH_TEST);
	program = shaderLoader.CreateProgram("shaders/shader_4_1.vert", "shaders/shader_4_1.frag");
	programSun = shaderLoader.CreateProgram("shaders/shader_sun_tex.vert", "shaders/shader_sun_tex.frag");
	programSkybox = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_texture.vert", "shaders/shader_texture.frag");

	textureEarth = Core::LoadTexture("textures/Earth/earth2.png");
	textureSun = Core::LoadTexture("textures/Sun/sunTex.png");
	textureMoon = Core::LoadTexture("textures/Moon/moon2.png");
	textureAsteroid = Core::LoadTexture("textures/Asteroid/asteroid.png");
	textureMars = Core::LoadTexture("textures/Planet/mars.png");
	textureVenus = Core::LoadTexture("textures/Planet/venus.png");

	//textureVenusNormal = Core::LoadTexture("textures/Planet/venus_normal.png");
	textureEarthNormal = Core::LoadTexture("textures/Earth/earth2_normals.png");
	textureAsteroidNormal = Core::LoadTexture("textures/Asteroid/asteroid_normals.png");
	textureSunNormal = Core::LoadTexture("textures/Moon/moon_normal.png");;
	textureMoonNormal = Core::LoadTexture("textures/Moon/moon_normal.png");;
	textureMarsNormal = Core::LoadTexture("textures/Planet/mars_normal.png");;
	shipTextureNormal = Core::LoadTexture("textures/StarSparrow_Normal.png");

	sphereModel = obj::loadModelFromFile("models/sphere.obj");

	shipModel = obj::loadModelFromFile("models/StarSparrow02.obj");
	shipTexture = Core::LoadTexture("textures/StarSparrow_Blue.png");

	ship = new Renderable();
	ship->model = &shipModel;
	ship->textureId = shipTexture;
	ship->textureNormal = shipTextureNormal;


	static const float astRadius = 6.0;
	for (int i = 0; i < NUM_ASTEROIDS; i++) {
		float angle = (float(i) * (2 * glm::pi<float>() / NUM_ASTEROIDS));
		asteroidPositions[i] = glm::vec3(cosf(angle), 0.0f, sinf(angle)) * astRadius + glm::sphericalRand(0.5f);
	}

	initSkybox();
	initPhysicsScene();
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

void onReshape(int width, int height)
{
	frustumScale = (float)width / height;

	glViewport(0, 0, width, height);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(2560, 1440);
	glutCreateWindow("OpenGL Pierwszy Program");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);
	glutReshapeFunc(onReshape);

	glutMainLoop();

	shutdown();

	return 0;
}
