#include "CustomMouseController.h"

CustomMouseController::CustomMouseController(float _horizontalDistance, float _angleAroundPlayer)
{
	horizontalDistance = _horizontalDistance;
	angleAroundPlayer = _angleAroundPlayer;
}

void CustomMouseController::mouseController(int x, int y)
{
    if (firstMouse)
    {
        lastX = x;
        firstMouse = false;
    }

    float xoffset = x - lastX;
    lastX = x;

    float yawSensitivity = 0.35f;

    xoffset *= yawSensitivity;

    angleAroundPlayer += xoffset;
}

void CustomMouseController::mouseKeyController(int button, int state, int x, int y)
{
    float maxCameraHorizontal = 10.0f;
    float cameraHorizontal = 0.2f;

    switch (button)
    {
    case (GLUT_LEFT_BUTTON): if (state == GLUT_UP) firstMouse = true; break;
    case (GLUT_RIGHT_BUTTON): {
        if (state == GLUT_DOWN) angleAroundPlayer = 180.0f; firstMouse = true;
        if (state == GLUT_UP) angleAroundPlayer = 0; firstMouse = true;
    } break;
    case (GLUT_MIDDLE_BUTTON): if (state == GLUT_UP) angleAroundPlayer = 0; break;
    case (3):if (state == GLUT_UP) {
        horizontalDistance -= cameraHorizontal;
        if (horizontalDistance < 2.5f) horizontalDistance = 2.5f;
    } break;
    case (4): if (state == GLUT_UP) {
        horizontalDistance += cameraHorizontal;
        if (horizontalDistance > maxCameraHorizontal) horizontalDistance = maxCameraHorizontal;
    } break;
    }
}

float CustomMouseController::getAngleAroundPlayer()
{
	return angleAroundPlayer;
}

float CustomMouseController::getHorizontalDistance()
{
	return horizontalDistance;
}
