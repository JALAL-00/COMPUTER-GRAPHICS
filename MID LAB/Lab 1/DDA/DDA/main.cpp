#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#include <iostream>
#include <cmath>

using namespace std;

int X1, Y1, X2, Y2;

void DDA(void) {
    float dx = X2 - X1;
    float dy = Y2 - Y1;
    float steps = max(abs(dx), abs(dy));

    float xInc = dx / steps;
    float yInc = dy / steps;

    float x = X1;
    float y = Y1;

    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_POINTS);

    for (int i = 0; i <= steps; i++) {
        glVertex2i(round(x), round(y));
        x += xInc;
        y += yInc;
    }

    glEnd();
    glFlush();
}

void myInit(void) {
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glColor3f(0.0f, 0.0f, 0.0f);
    glPointSize(4.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 640.0, 0.0, 480.0);
}

int main(int argc, char** argv) {
    cout << "Enter the initial point (X1, Y1): ";
    cin >> X1 >> Y1;
    cout << "Enter the final point (X2, Y2): ";
    cin >> X2 >> Y2;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(640, 480);
    glutInitWindowPosition(100, 150);
    glutCreateWindow("DDA Line Algorithm");

    myInit();
    glutDisplayFunc(DDA);
    glutMainLoop();
    return 0;
}
