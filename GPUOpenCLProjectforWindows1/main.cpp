#include <GL\glut.h>
#include <gl\freeglut.h>
#include <gl\freeglut_ext.h>
#include <gl\freeglut_std.h>
#include <iostream>
#include <math.h>
#include "config.h"
using namespace std;

const GLfloat Pi = 3.1415926536f;

void initSpheres(void);
void myReshape(GLsizei w, GLsizei h);
void display(void);
//void updateView();
void timer(int id);

double tx[N], ty[N], tz[N], r[N], vx[N], vy[N], vz[N], m[N], red[N], green[N], blue[N];

//主函数
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	//初始化OPENGL显示方式
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	//设定OPENGL窗口位置和大小
	glutInitWindowSize(800, 800);
	glutInitWindowPosition(100, 100);
	//打开窗口
	glutCreateWindow("opengl3d");
	glEnable(GL_DEPTH_TEST);

	glutTimerFunc(10, timer, 1);
	//设定窗口大小变化的回调函数
	glutReshapeFunc(myReshape);
	//开始OPENGL的循环
	glutDisplayFunc(display);
	glutMainLoop();

	return 0;
}

void timer(int id) {
	//重新计算速度位置
	updateSpheres(0.5);
	glutPostRedisplay();
	glutTimerFunc(10, timer, 1);
}

void myReshape(GLsizei w, GLsizei h)//设定窗口大小变化的回调函数
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (GLfloat)w / (GLfloat)h, 0.1, 300);
}

void drawSolidCube(GLfloat x, GLfloat y, GLfloat z, GLfloat xl, GLfloat yl, GLfloat zl, GLubyte red, GLubyte green, GLubyte blue) {
	glPushMatrix();
	glColor3ub(red, green, blue);
	glTranslatef(x, y, z);
	glScalef(xl, yl, zl);
	glutSolidCube(1);
	glPopMatrix();
}

void drawSphere(GLfloat x, GLfloat y, GLfloat z, GLfloat r, GLubyte red, GLubyte green, GLubyte blue) {
	glPushMatrix();
	glColor3ub(red, green, blue);
	glTranslatef(x, y, z);
	glScalef(r, r, r);
	glutSolidSphere(0.99, 15, 15);
	glPopMatrix();
}

void display(void)
{
	//清除颜色和深度缓存
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(2);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//视角的变化
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(gl_view_x, gl_view_y, gl_view_z, gl_target_x, gl_target_y, gl_target_z, 0, 1, 0);
	glScalef(gl_scale, gl_scale, gl_scale);

	drawSolidCube(0, -1, 0, 2, 0, 2, 241, 241, 241);
	drawSolidCube(0, 0, -1, 2, 2, 0, 171, 171, 171);
	drawSolidCube(-1, 0, 0, 0, 2, 2, 101, 101, 101);


	for (int i = 0; i < num; i++) {
		drawSphere(tx[i], ty[i], tz[i], r[i], red[i], green[i], blue[i]);
	}
	glutSwapBuffers();
}

void initSpheres(void)
{
	for (int i = 0; i < num; i++) {
		int index = i;
		int x_i = index % line;
		index = index / line;
		int y_i = index % line;
		index = index / line;
		int z_i = index;
		tx[i] = (x_i - 2) * 0.3 - 0.001 * (y_i - 5) - 0.001 * (z_i - 5);
		ty[i] = (y_i - 2) * 0.3 - 0.001 * (z_i - 5) - 0.001 * (x_i - 5);
		tz[i] = (z_i - 2) * 0.3 - 0.001 * (x_i - 5) - 0.001 * (y_i - 5);
		r[i] = i % 7 == 0 ? 0.15 : 0.1;
		m[i] = i % 5 == 0 ? 2 : 1;
		red[i] = (i % line) * COLOR;
		green[i] = (i + 1 % line) * COLOR;
		blue[i] = (i + 2 % line) * COLOR;
		if (i % 11 == 0) {
			vx[i] = 0.003; vy[i] = -0.01; vz[i] = 0.002;
		}
		else vx[i] = vy[i] = vz[i] = 0;
	};
}


void updateSpheres(double interval) {
	for (int i = 0; i < num; i++) {
		collideDetect(i);
	}
	for (int i = 0; i < num; i++) {
		vy[i] += G * interval;
		tx[i] += vx[i] * interval;
		ty[i] += vy[i] * interval;
		tz[i] += vz[i] * interval;
		if (tx[i] < -xBound + r[i]) {
			tx[i] = -xBound + r[i];
			vx[i] *= Decay;
		}
		if (tx[i] > xBound - r[i]) {
			tx[i] = xBound - r[i];
			vx[i] *= Decay;
		}
		if (ty[i] < -yBound + r[i]) {
			ty[i] = -yBound + r[i];
			vy[i] *= Decay;
		}
		if (ty[i] > yBound - r[i]) {
			ty[i] = yBound - r[i];
			vy[i] *= Decay;
		}
		if (tz[i] < -zBound + r[i]) {
			tz[i] = -zBound + r[i];
			vz[i] *= Decay;
		}
		if (tz[i] > zBound - r[i]) {
			tz[i] = zBound - r[i];
			vz[i] *= Decay;
		}
	}
}

void collideDetect(int index) {
	double force[3] = { 0.0, 0.0, 0.0 };
	for (int i = 0; i < num; i++) {
		if (i == index) continue;
		double dforce[3] = { 0.0, 0.0, 0.0 };
		double d[3] = { tx[index] - tx[i], ty[index] - ty[i], tz[index] - tz[i] };
		double dist = sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
		double collideDist = r[index] + r[i];
		if (dist < collideDist) {
			double norm[3] = { d[0] / dist, d[1] / dist, d[2] / dist };
			double dv[3] = { vx[index] - vx[i], vy[index] - vy[i], vz[index] - vz[i] };
			double vDotNorm = dv[0] * norm[0] + dv[1] * norm[1] + dv[2] * norm[2];

			double tanv[3] = { dv[0] - vDotNorm * norm[0], dv[1] - vDotNorm * norm[1] , dv[2] - vDotNorm * dv[2] };

			double factor = -0.005 * (collideDist - dist);
			for (int j = 0; j < 3; j++) {
				force[i] += factor * norm[i] + 0.002 * dv[i] + 0.001 * tanv[i];
			}
		}
	}
	vx[index] += force[0] / m[index];
	vy[index] += force[1] / m[index];
	vz[index] += force[2] / m[index];
}
