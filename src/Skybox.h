#pragma once

#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"

unsigned int loadCubeTexture();
void createCubeTexture();
void renderSkybox(GLuint programSkybox, glm::mat4 cameraMatrix, glm::mat4 perspectiveMatrix);
void initSkybox();