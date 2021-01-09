#include "Sphere.h"
#include <GL\glut.h>
#include <gl\freeglut.h>
#include <gl\freeglut_ext.h>
#include <gl\freeglut_std.h>
#include <math.h>

Sphere::Sphere(double _tx, double _ty, double _tz, double _r){
	tx = _tx; ty = _ty; tz = _tz;
	r = _r;
	red = 0.1; green = 0.2; blue = 0.3;
	vx = 0; vy = 0; vz = 0;
	mass = 1;
}

void Sphere::drawSphere()
{
	glPushMatrix();
	glColor3ub(red, green, blue);
	glTranslatef(tx, ty, tz);
	glScalef(r, r, r);
	glutSolidSphere(0.99, 15, 15);
	glPopMatrix();
}

void Sphere::setMass(int _mass)
{
	mass = _mass;
}

void Sphere::setColor(double _red, double _green, double _blue)
{
	red = _red; green = _green; blue = _blue;
}

void Sphere::setVelocity(double _vx, double _vy, double _vz)
{
	vx = _vx; vy = _vy; vz = _vz;
}

void Sphere::updatePosition(int interval)
{
	tx += vx * interval;
	ty += vy * interval;
	tz += vz * interval;
}

void Sphere::collisionWall()
{
	if(tx < -xBound + r) {
		tx = -xBound + r;
		vx *= Decay;
	}
	if (tx > xBound - r) {
		tx = xBound - r;
		vx *= Decay;
	}
	if (ty < -yBound + r) {
		ty = -yBound + r;
		vy *= Decay;
	}
	if (ty > yBound - r) {
		ty = yBound - r;
		vy *= Decay;
	}
	if (tz < -zBound + r) {
		tz = -zBound + r;
		vz *= Decay;
	}
	if (tz > zBound - r) {
		tz = zBound - r;
		vz *= Decay;
	}
}

double* Sphere::collideSphere(const Sphere& s)
{
	double dforce[3] = { 0.0, 0.0, 0.0 };
	double d[3] = { tx - s.tx, ty - s.ty, tz - s.tz };
	double dist = sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
	double collideDist = r + s.r;
	if (dist < collideDist) {
		double norm[3] = { d[0] / dist, d[1] / dist, d[2] / dist };
		double dv[3] = { vx - s.vx, vy - s.vy, vz - s.vz };
		double vDotNorm = dv[0] * norm[0] + dv[1] * norm[1] + dv[2] * norm[2];

		double tanv[3] = { dv[0] - vDotNorm * norm[0], dv[1] - vDotNorm * norm[1] , dv[2] - vDotNorm * dv[2] };

		double factor = -0.005 * (collideDist - dist);
		for (int i = 0; i < 3; i++) {
			dforce[i] = factor * norm[i] + 0.002 * dv[i] + 0.001 * tanv[i];
		}
	}
	return dforce;
}
