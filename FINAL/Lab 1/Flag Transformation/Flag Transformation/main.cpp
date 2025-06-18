#include <GLUT/glut.h>
#include <cmath>

const float PI = 3.1416;
float scale = 1.0f;   // Scaling factor
bool growing = true;

void drawCircle(float cx, float cy, float r, int segments) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; i++) {
        float theta = 2 * PI * i / segments;
        glVertex2f(cx + r * cos(theta), cy + r * sin(theta));
    }
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Flagpole
    glColor3f(0.54f, 0.40f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(100, 100); glVertex2f(110, 100);
        glVertex2f(110, 370); glVertex2f(100, 370);
    glEnd();

    // Top triangle
    glColor3f(0.4f, 0.2f, 0.1f);
    glBegin(GL_TRIANGLES);
        glVertex2f(100, 370); glVertex2f(110, 370);
        glVertex2f(105, 390);
    glEnd();

    // 🧩 Apply scaling to the flag only
    glPushMatrix();
    glTranslatef(110, 370, 0);       // Move pivot to flag origin
    glScalef(scale, scale, 1.0f);    // Apply scaling
    glTranslatef(-110, -370, 0);     // Move pivot back

    // Flag (green)
    glColor3f(0.f, 0.50f, 0.1f);
    glBegin(GL_QUADS);
        glVertex2f(110, 285); glVertex2f(280, 285);
        glVertex2f(280, 370); glVertex2f(110, 370);
    glEnd();

    // Red circle
    glColor3f(0.957f, 0.0f, 0.0f);
    drawCircle(190, 330, 30, 100);

    glPopMatrix(); // End scaling

    // Base steps
    glColor3f(0.54f, 0.40f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(90, 90); glVertex2f(120, 90);
        glVertex2f(120, 100); glVertex2f(90, 100);
    glEnd();

    glColor3f(0.4f, 0.3f, 0.12f);
    glBegin(GL_QUADS);
        glVertex2f(80, 80); glVertex2f(130, 80);
        glVertex2f(130, 90); glVertex2f(80, 90);
    glEnd();

    glColor3f(0.4f, 0.2f, 0.1f);
    glBegin(GL_QUADS);
        glVertex2f(70, 70); glVertex2f(140, 70);
        glVertex2f(140, 80); glVertex2f(70, 80);
    glEnd();

    glFlush();
}

void idle() {
    // Animate scale
    if (growing) {
        scale += 0.001f;
        if (scale >= 1.2f) growing = false;
    } else {
        scale -= 0.001f;
        if (scale <= 0.8f) growing = true;
    }
    glutPostRedisplay();
}

void init() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, 640, 0, 480);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(640, 480);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Scaling Bangladesh Flag");

    init();
    glutDisplayFunc(display);
    glutIdleFunc(idle); // For animation
    glutMainLoop();
    return 0;
}
