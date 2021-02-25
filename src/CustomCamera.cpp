#include "CustomCamera.h"

#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>
#include <stdlib.h>
#include <cstdio>

#include "Physics.h"

CustomCamera::CustomCamera(glm::vec3 _cameraPos) { cameraPos = _cameraPos; }

glm::mat4 CustomCamera::createPerspectiveMatrix(float zNear, float zFar, float fov, float aspectRatio)
{
    float tanFov = tan(glm::radians(fov) * 0.5f);

    glm::mat4 perspective;
    perspective[0][0] = 1.0f / (tanFov * aspectRatio);
    perspective[1][1] = 1.0f / tanFov;
    perspective[2][2] = (zFar + zNear) / (zNear - zFar);
    perspective[3][2] = (2 * zFar * zNear) / (zNear - zFar);
    perspective[2][3] = -1;
    perspective[3][3] = 0;

    return perspective;
}

glm::vec3 CustomCamera::calculateCameraPosition(float horizontalDis, float verticalDis, float rotY, glm::vec3 position, float angleAroundPlayer) {

    glm::vec3 cameraPosition;
    float theta = glm::radians(angleAroundPlayer) + rotY;
    float offsetX = horizontalDis * sin(theta);
    float offsetZ = horizontalDis * cos(theta);

    cameraPosition.x = position.x - offsetX;
    cameraPosition.z = position.z + offsetZ;
    cameraPosition.y = position.y + verticalDis;

    return cameraPosition;

}

glm::mat4 CustomCamera::createCustomCameraMatrix(PxRigidActor* actor, float horizontalDistance, float verticalDistance, float angleAroundPlayer)
{
    // ACTOR MUST BE THE LAST OBJECT ADDED TO PHYSX SCENE!
    glm::vec3 up = glm::vec3(0, 1, 0);
    glm::vec3 spaceshipPosition;
    PxVec3 spaceshipPositionVector;
    PxQuat spaceshipPositionQuat;
    glm::quat glmQuatSpPos;

    PxTransform spaceshipGlobalPosition = actor->getGlobalPose();
    PxMat44 spMatrix = spaceshipGlobalPosition;
    spaceshipPositionVector = spaceshipGlobalPosition.p;
    spaceshipPositionQuat = spaceshipGlobalPosition.q;

    spaceshipPosition = glm::vec3(spaceshipPositionVector.x, spaceshipPositionVector.y, spaceshipPositionVector.z);

    float angle = acos(spMatrix[0][0]);

    PxVec3 basisVec0 = spaceshipPositionQuat.getBasisVector0();
    PxVec3 basisVec2 = spaceshipPositionQuat.getBasisVector2();

    if (basisVec0.z < 0 && basisVec2.x > 0) cameraAngle = -1.0f * angle;
    if (basisVec0.z > 0 && basisVec2.x < 0) cameraAngle = 1.0f * angle;

    glmQuatSpPos = glm::quat(spaceshipPositionQuat.w, spaceshipPositionQuat.x, spaceshipPositionQuat.y, spaceshipPositionQuat.z);

    cameraDir = glm::inverse(glmQuatSpPos) * glm::vec3(0, 0, -1);
    cameraSide = glm::inverse(glmQuatSpPos) * glm::vec3(1, 0, 0);

    cameraPos = CustomCamera::calculateCameraPosition(horizontalDistance, verticalDistance, cameraAngle, spaceshipPosition, angleAroundPlayer);
    return glm::lookAt(cameraPos, spaceshipPosition, up);
}

glm::vec3 CustomCamera::getCameraPos() { return cameraPos; }

glm::vec3 CustomCamera::getCameraDir() { return cameraDir; }

glm::vec3 CustomCamera::getCameraSide() { return cameraSide; }