#include "SphereManager.h"
#include "Sphere.h"

SphereManager::SphereManager(int _num, int _line) {
	num = _num;
	line = _line;
	for (int i = 0; i < num; i++) {
		int index = i;
		int x_i = index % line;
		index = index / line;
		int y_i = index % line;
		index = index / line;
		int z_i = index;

		Sphere sphere(
			(x_i - 2) * 0.3 - 0.001 * (y_i - 5) - 0.001 * (z_i - 5),
			(y_i - 2) * 0.3 - 0.001 * (z_i - 5) - 0.001 * (x_i - 5),
			(z_i - 2) * 0.3 - 0.001 * (x_i - 5) - 0.001 * (y_i - 5),
			i % 7 == 0 ? 0.15 : 0.1
		);
		sphere.setMass(i % 5 == 0 ? 2 : 1);
		sphere.setColor((i % line) * COLOR, (i+1 % line) * COLOR, (i+2 % line) * COLOR);
		if (i % 11 == 0) sphere.setVelocity(0.003, -0.01, 0.002);
		spheres.push_back(sphere);
	}
}

SphereManager::~SphereManager()
{
}

void SphereManager::updateSphere(double interval)
{
	for (int i = 0; i < spheres.size(); i++) {
		collisionDetect(i);
	}

	for (Sphere& sphere : spheres) {
		sphere.vy += g * interval;
		sphere.updatePosition(interval);
		sphere.collisionWall();
	}
}

void SphereManager::collisionDetect(double index)
{
	Sphere& sphere = spheres[index];
	double force[3] = { 0.0, 0.0, 0.0 };
	for (int i = 0; i < spheres.size(); i++) {
		if (i == index) continue;
		double* dforce = sphere.collideSphere(spheres[i]);
		for (int i = 0; i < 3; i++) force[i] += dforce[i];
	}
	sphere.vx += force[0] / sphere.mass;
	sphere.vy += force[1] / sphere.mass;
	sphere.vz += force[2] / sphere.mass;
}
