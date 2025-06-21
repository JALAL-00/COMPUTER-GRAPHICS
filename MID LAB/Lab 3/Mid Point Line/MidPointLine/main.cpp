#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <iostream>
#include <cmath>


using namespace std;

int X1, Y1, X2, Y2, r, cx, cy;

void MidpointLine(void)
{
    int dx = X2 - X1;
    int dy = Y2 - Y1;
    int x = X1, y = Y1;
    int d = 2 * dy - dx;
    int deltaE = 2 * dy;
    int deltaNE = 2 * (dy - dx);

    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_POINTS);
    glVertex2i(x, y);

    while (x < X2)
    {
        if (d <= 0)
        {
            d += deltaE;
            x++;
        }
        else
        {
            d += deltaNE;
            x++;
            y++;
        }
        glVertex2i(x, y);
    }

    glEnd();
    glFlush();
}

void init(void)
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glColor3f(1.0, 1.0, 1.0);
    glPointSize(1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 500.0, 0.0, 500.0);
}

void display(void)
{
    MidpointLine();
}

int main(int argc, char **argv)
{
    cout << "Enter two endpoints (X1, Y1, X2, Y2): ";
    cin >> X1 >> Y1 >> X2 >> Y2;

    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Midpoint Line Algorithm");

    init();

    glutDisplayFunc(display);

    glutMainLoop();

    return 0;
}
