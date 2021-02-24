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
#include "Physics.h"
#include "Texture.h"

float frustumScale = 1.f;
Physics pxScene(9.8);
const double physicsStepTime = 1.f / 90.0f;
double physicsTimeToProcess = 0;
PxRigidDynamic *shipBody = nullptr;
PxMaterial* shipMaterial = nullptr;
GLuint program, programSun, programSkybox, programColor, programTexture, textureEarth, textureEarthNormal, textureAsteroid, textureAsteroidNormal, textureSun, textureSunNormal, textureMoon, textureMoonNormal, textureMars, textureMarsNormal, textureVenus, textureVenusNormal, shipTexture, shipTextureNormal;
obj::Model sphereModel, shipModel;
Core::Shader_Loader shaderLoader;
Core::RenderContext sphereContext, shipContext;
float cameraAngle = 0;
glm::vec3 cameraPos = glm::vec3(-6, 0, 0), cameraDir, lightPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::mat4 cameraMatrix, perspectiveMatrix;
struct Renderable {
    obj::Model *model;
	Core::RenderContext* context;
    glm::mat4 modelMatrix;
    GLuint textureId;
	GLuint textureNormal;
};
Renderable* ship;

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

	pxScene.step(physicsStepTime);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	glUseProgram(program);

	glm::mat4 shipModelMatrix =
		glm::translate(cameraPos + cameraDir * 0.85f + glm::vec3(0, -0.25f, 0))
		* glm::rotate(-cameraAngle + glm::radians(90.0f), glm::vec3(0, 1, 0))
		* glm::scale(glm::vec3(0.03f));

	ship->modelMatrix = shipModelMatrix;

	glm::mat4 sunScale = glm::scale(glm::vec3(1.5, 1.5, 1.5));

	renderSkybox(programSkybox, cameraMatrix, perspectiveMatrix);
	//updateTransforms();
	Core::drawObjectTexture(programSun, &sphereModel,glm::translate(glm::vec3(0,0,0)), textureSun, textureMarsNormal, cameraMatrix, perspectiveMatrix, cameraPos, lightPos);
	Core::drawObjectTexture(programTexture, ship->model, ship->modelMatrix, ship->textureId, ship->textureNormal, cameraMatrix, perspectiveMatrix, cameraPos, lightPos);
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
	programSun = shaderLoader.CreateProgram("shaders/shader_sun_tex.vert", "shaders/shader_sun_tex.frag");
	programSkybox = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_texture.vert", "shaders/shader_texture.frag");

	textureSun = Core::LoadTexture("textures/Sun/sunTex.png");
	textureEarthNormal = Core::LoadTexture("textures/Earth/earth2_normals.png");
	shipTextureNormal = Core::LoadTexture("textures/StarSparrow_Normal.png");
	sphereModel = obj::loadModelFromFile("models/sphere.obj");
	shipModel = obj::loadModelFromFile("models/StarSparrow02.obj");
	shipTexture = Core::LoadTexture("textures/StarSparrow_Blue.png");
	shipContext.initFromOBJ(shipModel);
	ship = new Renderable();
	ship->model = &shipModel;
	ship->textureId = shipTexture;
	ship->textureNormal = shipTextureNormal;
	ship->context = &shipContext;

	initSkybox();
	initPhysicsScene();
}

void shutdown()
{
	shaderLoader.DeleteProgram(programSun); shaderLoader.DeleteProgram(programSkybox); shaderLoader.DeleteProgram(programTexture);
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
