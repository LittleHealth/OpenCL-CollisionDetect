#include <GL\glut.h>
#include <gl\freeglut.h>
#include <gl\freeglut_ext.h>
#include <gl\freeglut_std.h>
#include <iostream>
#include <math.h>
#include <vector>
#include "Sphere.h"
#include "SphereManager.h"

using namespace std;
const GLfloat Pi = 3.1415926536f;
//摄像机位置
float gl_view_x = 2;
float gl_view_y = 2.5;
float gl_view_z = 1.75;
//视点位置
float gl_target_x = 0;
float gl_target_y = 0;
float gl_target_z = 0;
//缩放比例
float gl_scale = 1.0f;

void init();
void myReshape(GLsizei w, GLsizei h);
void display(void);
//void updateView();
void timer(int id);

SphereManager s(25, 5);

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
	s.updateSphere(1);
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

void drawSphere(const Sphere& s) {
	glPushMatrix();
	glColor3ub(s.red, s.green, s.blue);
	glTranslatef(s.tx, s.ty, s.tz);
	glScalef(s.r, s.r, s.r);
	glutSolidSphere(0.99, 15, 15);
	glPopMatrix();
}

void display(void)
{
	int num = s.num;
	int line = s.line;
	//清除颜色和深度缓存
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(2);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//视角的变化
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(gl_view_x, gl_view_y, gl_view_z, gl_target_x, gl_target_y, gl_target_z, 0, 1, 0);
	glScalef(gl_scale, gl_scale, gl_scale);

	//地板
	drawSolidCube(0, -1, 0, 2, 0, 2, 241, 241, 241);
	//墙壁1
	drawSolidCube(0, 0, -1, 2, 2, 0, 171, 171, 171);
	//墙壁2
	drawSolidCube(-1, 0, 0, 0, 2, 2, 101, 101, 101);


	for (int i = 0; i < num; i++) {
		//drawSphere(s->mParticle[i * 4 + 0], s->mParticle[i * 4 + 1], s->mParticle[i * 4 + 2], s->mParticle[i * 4 + 3], s->mParticle[i * 4 + 3], s->mParticle[i * 4 + 3], (i % line) * colorStep, (i / line) % line * colorStep, (i / line) / line * colorStep);
		drawSphere(s.spheres[i]);
	}

	//交换前后缓冲区
	glutSwapBuffers();
}





