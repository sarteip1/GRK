#pragma once

#include "freeglut.h"

class CustomMouseController {
private:
	float lastX;
	bool firstMouse = true;
	float angleAroundPlayer;
	float horizontalDistance;

public:

	CustomMouseController(float _horizontalDistance, float _angleAroundPlayer);

	void mouseController(int x, int y);

	void mouseKeyController(int button, int state, int x, int y);

	float getAngleAroundPlayer();

	float getHorizontalDistance();

};