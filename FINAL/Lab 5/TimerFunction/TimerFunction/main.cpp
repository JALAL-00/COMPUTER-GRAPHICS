#include <GLUT/glut.h>
#include <cmath>

float carPosX = -1.2f;

void initGL() {
    glClearColor(0.4f, 0.8f, 1.0f, 1.0f);
}

void drawSun() {
    glColor3f(1.0f, 1.0f, 0.0f);
    float cx = 0.8f, cy = 0.8f, r = 0.1f;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 100; i++) {
        float angle = 2 * M_PI * i / 100;
        glVertex2f(cx + r * cos(angle), cy + r * sin(angle));
    }
    glEnd();
}

void drawBuildings() {
    float x = -1.0f;
    for (int i = 0; i < 6; ++i) {
        float width = 0.25f;
        float height = 0.3f + 0.1f * (i % 3);
        glColor3f(0.3f + 0.1f * (i % 3), 0.3f, 0.3f);
        glBegin(GL_QUADS);
            glVertex2f(x, 0.3f);
            glVertex2f(x + width, 0.3f);
            glVertex2f(x + width, 0.3f + height);
            glVertex2f(x, 0.3f + height);
        glEnd();

        // Windows
        glColor3f(1.0f, 1.0f, 0.8f);
        for (float wx = x + 0.03f; wx < x + width - 0.05f; wx += 0.07f) {
            for (float wy = 0.35f; wy < 0.3f + height - 0.05f; wy += 0.08f) {
                glBegin(GL_QUADS);
                    glVertex2f(wx, wy);
                    glVertex2f(wx + 0.04f, wy);
                    glVertex2f(wx + 0.04f, wy + 0.05f);
                    glVertex2f(wx, wy + 0.05f);
                glEnd();
            }
        }
        x += 0.35f;
    }
}

void drawTree(float x, float y) {
    glColor3f(0.5f, 0.3f, 0.1f);
    glBegin(GL_QUADS);
        glVertex2f(x - 0.01f, y);
        glVertex2f(x + 0.01f, y);
        glVertex2f(x + 0.01f, y + 0.06f);
        glVertex2f(x - 0.01f, y + 0.06f);
    glEnd();

    glColor3f(0.0f, 0.6f, 0.0f);
    float r = 0.03f;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y + 0.1f);
    for (int i = 0; i <= 100; ++i) {
        float angle = 2 * M_PI * i / 100;
        glVertex2f(x + r * cos(angle), y + 0.07f + r * sin(angle));
    }
    glEnd();
}

void drawRoad() {
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
        glVertex2f(-1.0f, -0.2f);
        glVertex2f( 1.0f, -0.2f);
        glVertex2f( 1.0f, -0.6f);
        glVertex2f(-1.0f, -0.6f);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    for (float x = -1.0f; x < 1.0f; x += 0.2f) {
        glBegin(GL_QUADS);
            glVertex2f(x, -0.4f);
            glVertex2f(x + 0.1f, -0.4f);
            glVertex2f(x + 0.1f, -0.42f);
            glVertex2f(x, -0.42f);
        glEnd();
    }
}

void drawCar() {
    glPushMatrix();
    glTranslatef(carPosX, -0.3f, 0.0f);
    glScalef(-1.0f, 1.0f, 1.0f);

    glColor3f(0.9f, 0.0f, 0.1f);
    glBegin(GL_QUADS);
        glVertex2f(-0.2f, 0.0f);
        glVertex2f( 0.2f, 0.0f);
        glVertex2f( 0.2f, 0.1f);
        glVertex2f(-0.2f, 0.1f);
    glEnd();

    glColor3f(0.7f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(-0.1f, 0.1f);
        glVertex2f( 0.1f, 0.1f);
        glVertex2f( 0.05f, 0.2f);
        glVertex2f(-0.05f, 0.2f);
    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(-0.15f, -0.05f);
        glVertex2f(-0.10f, -0.05f);
        glVertex2f(-0.10f,  0.00f);
        glVertex2f(-0.15f,  0.00f);

        glVertex2f( 0.10f, -0.05f);
        glVertex2f( 0.15f, -0.05f);
        glVertex2f( 0.15f,  0.00f);
        glVertex2f( 0.10f,  0.00f);
    glEnd();

    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawSun();
    drawBuildings();
    drawTree(-0.9f, -0.2f);
    drawTree(-0.4f, -0.2f);
    drawTree( 0.2f, -0.2f);
    drawTree( 0.7f, -0.2f);
    drawRoad();
    drawCar();

    glutSwapBuffers();
}

// Update car position
void update(int value) {
    carPosX += 0.015f; // Adjusted speed for timer
    if (carPosX > 1.2f)
        carPosX = -1.2f;

    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // ~60fps (1000ms/60 ≈ 16ms)
}

void reshape(GLsizei width, GLsizei height) {
    if (height == 0) height = 1;
    GLfloat aspect = (GLfloat)width / (GLfloat)height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (width >= height) {
        gluOrtho2D(-1.0f * aspect, 1.0f * aspect, -1.0f, 1.0f);
    } else {
        gluOrtho2D(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect);
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(550, 600);
    glutCreateWindow("Car Animation with Timer - macOS");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, update, 0); // Timer Function
    initGL();
    glutMainLoop();
    return 0;
}
