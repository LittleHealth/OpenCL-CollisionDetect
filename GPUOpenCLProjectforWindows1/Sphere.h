#pragma once
const double xBound = 1.0, yBound = 1.0, zBound=1.0;
const double Decay = -0.5;

class Sphere
{
public:
	Sphere(double _tx, double _ty, double _tz, double _r);

	double tx, ty, tz;
	double r;
	double red, green, blue;

	int mass; // 质量
	double vx, vy, vz; // 速度

	void drawSphere();

	void setMass(int _mass);
	void setColor(double _red, double _green, double _blue);
	void setVelocity(double _vx, double _vy, double _vz);
	void updatePosition(int interval);

	void  collisionWall();

	double* collideSphere(const Sphere& s);
};

