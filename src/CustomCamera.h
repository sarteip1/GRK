#pragma once

#include "ext.hpp"
#include "Texture.h"
#include "Physics.h"

class CustomCamera {
private:
    glm::vec3 cameraPos;
    glm::vec3 cameraDir;
    glm::vec3 cameraSide;
    float cameraAngle;
public:
    CustomCamera(glm::vec3 _cameraPos);

    glm::mat4 createPerspectiveMatrix(float zNear, float zFar, float fov, float frustumScale = 1.0f);

    glm::vec3 calculateCameraPosition(float horizontalDis, float verticalDis, float rotY, glm::vec3 position, float angleAroundPlayer);

    glm::mat4 createCustomCameraMatrix(PxRigidActor* actor, float horizontalDistance, float verticalDistance, float angleAroundPlayer);

    glm::vec3 getCameraPos();

    glm::vec3 getCameraDir();

    glm::vec3 getCameraSide();

};