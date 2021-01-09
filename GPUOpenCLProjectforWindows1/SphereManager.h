#pragma once
const int N = 125;
const int LINE = 5;
const double g = -0.0003;
const double COLOR = 51;

#include "Sphere.h"
#include <vector>
class SphereManager
{
public:
	int num = N, line = LINE;

	std::vector<Sphere> spheres;

	SphereManager(int num = 125, int line = 5);
	~SphereManager();
	void updateSphere(double interval);
	void collisionDetect(double index);
};

