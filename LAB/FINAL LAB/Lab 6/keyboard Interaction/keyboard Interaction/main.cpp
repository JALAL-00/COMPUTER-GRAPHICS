//All my code is only for macOS users and cannot be used on Windows systems.

#define GL_SILENCE_DEPRECATION
#include <cstdio>
#include <cmath>
#include <GLUT/glut.h>

// Helicopter state
GLfloat helicopterX = 1.0f;
GLfloat helicopterY = 0.0f;
GLfloat helicopterSpeedX = -0.01f;
GLfloat helicopterSpeedY = 0.0f;
GLfloat savedSpeedX = -0.01f;


GLfloat rotorAngle = 0.0f;
GLfloat rotorSpeed = 15.0f;


// Cloud position
GLfloat cloudX = -0.8f;
GLfloat cloudSpeed = 0.005f;

// Draw circle
void drawCircle(float cx, float cy, float r, int num_segments, float R, float G, float B) {
    glColor3f(R, G, B);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int ii = 0; ii <= num_segments; ii++) {
        float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);
        float x = r * cosf(theta);
        float y = r * sinf(theta);
        glVertex2f(x + cx, y + cy);
    }
    glEnd();
}

//sun
void drawSun() {
    glPushMatrix();
    glTranslatef(-0.7f, 0.7f, 0.0f);
    drawCircle(0.0f, 0.0f, 0.15f, 100, 1.0f, 1.0f, 0.0f);
    glColor3f(1.0f, 0.8f, 0.0f);
    for(int i = 0; i < 12; ++i) {
        glRotatef(30.0f, 0.0f, 0.0f, 1.0f);
        glBegin(GL_TRIANGLES);
            glVertex2f(0.0f, 0.16f);
            glVertex2f(-0.02f, 0.22f);
            glVertex2f(0.02f, 0.22f);
        glEnd();
    }
    glPopMatrix();
}

//Cloud
void drawCloud() {
    glPushMatrix();
    glTranslatef(cloudX, 0.5f, 0.0f);
    drawCircle(0.0f, 0.0f, 0.1f, 50, 0.9f, 0.9f, 0.9f);
    drawCircle(0.1f, 0.05f, 0.08f, 50, 0.95f, 0.95f, 0.95f);
    drawCircle(-0.08f, 0.03f, 0.09f, 50, 0.85f, 0.85f, 0.85f);
    drawCircle(0.05f, -0.05f, 0.07f, 50, 0.9f, 0.9f, 0.9f);
    glPopMatrix();
}

//Helicopter
void drawHelicopter() {
    glPushMatrix();
    glTranslatef(helicopterX, helicopterY, 0.0f);

    // Helicopter body
    glColor3f(0.2f, 0.2f, 0.5f);
    glBegin(GL_QUADS);
        glVertex2f(-0.15f, -0.05f);
        glVertex2f(0.15f, -0.05f);
        glVertex2f(0.2f, 0.05f);
        glVertex2f(-0.2f, 0.05f);
    glEnd();

    // Window
    glColor3f(0.5f, 0.8f, 1.0f);
    glBegin(GL_QUADS);
        glVertex2f(-0.18f, 0.0f);
        glVertex2f(-0.05f, 0.0f);
        glVertex2f(-0.03f, 0.04f);
        glVertex2f(-0.18f, 0.04f);
    glEnd();

    // Tail
    glColor3f(0.3f, 0.3f, 0.6f);
    glBegin(GL_QUADS);
        glVertex2f(0.15f, -0.02f);
        glVertex2f(0.4f, -0.01f);
        glVertex2f(0.4f, 0.01f);
        glVertex2f(0.15f, 0.02f);
    glEnd();

    // Tail rotor
    glPushMatrix();
    glTranslatef(0.4f, 0.0f, 0.0f);
    glRotatef(rotorAngle * 2.0f, 0.0f, 0.0f, 1.0f);
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_LINES);
        glVertex2f(-0.04f, 0.0f); glVertex2f(0.04f, 0.0f);
        glVertex2f(0.0f, -0.04f); glVertex2f(0.0f, 0.04f);
    glEnd();
    glPopMatrix();

    // Legs
    glColor3f(0.4f, 0.4f, 0.4f);
    glBegin(GL_QUADS);
        glVertex2f(-0.1f, -0.1f);
        glVertex2f(0.1f, -0.1f);
        glVertex2f(0.1f, -0.08f);
        glVertex2f(-0.1f, -0.08f);
    glEnd();
    glBegin(GL_LINES);
        glVertex2f(-0.08f, -0.05f); glVertex2f(-0.08f, -0.08f);
        glVertex2f(0.08f, -0.05f);  glVertex2f(0.08f, -0.08f);
    glEnd();

    // Top rotor
    glPushMatrix();
    glTranslatef(0.0f, 0.05f, 0.0f);
    glRotatef(rotorAngle, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.6f, 0.6f, 0.6f, 0.6f);
    glBegin(GL_QUADS);
        glVertex2f(-0.3f, -0.01f); glVertex2f(0.3f, -0.01f);
        glVertex2f(0.3f, 0.01f); glVertex2f(-0.3f, 0.01f);
        glVertex2f(-0.01f, -0.3f); glVertex2f(0.01f, -0.3f);
        glVertex2f(0.01f, 0.3f); glVertex2f(-0.01f, 0.3f);
    glEnd();
    glDisable(GL_BLEND);
    glPopMatrix();

    glPopMatrix();
}


void update(int value) {
    rotorAngle += rotorSpeed;
    if (rotorAngle > 360.0f) rotorAngle -= 360.0f;

    cloudX += cloudSpeed;
    if (cloudX > 1.5f) cloudX = -1.5f;
    cloudSpeed = 0.004f + 0.002f * sinf(glutGet(GLUT_ELAPSED_TIME) * 0.001f);

    helicopterX += helicopterSpeedX;
    helicopterY += helicopterSpeedY;
    helicopterSpeedY *= 0.9f;

    if (helicopterX < -1.5f) helicopterX = 1.5f;
    if (helicopterX > 1.5f) helicopterX = -1.5f;

    if (helicopterY > 0.8f) helicopterY = 0.8f;
    if (helicopterY < -0.8f) helicopterY = -0.8f;

    glutPostRedisplay();
    glutTimerFunc(30, update, 0);
}

void display() {
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.3f, 0.6f, 0.2f);
    glBegin(GL_QUADS);
        glVertex2f(-1.0f, -1.0f);
        glVertex2f(1.0f, -1.0f);
        glVertex2f(1.0f, -0.9f);
        glVertex2f(-1.0f, -0.9f);
    glEnd();

    drawSun();
    drawCloud();
    drawHelicopter();

    glFlush();
}

//Helicopter Up & down by 'w' & 's' key press
void handleKeypress(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': case 'W':
            if (helicopterSpeedY < 0.2f) helicopterSpeedY += 0.01f;
            break;
        case 's': case 'S':
            if (helicopterSpeedY > -0.2f) helicopterSpeedY -= 0.01f;
            break;
        case 27:
            exit(0);
    }
    glutPostRedisplay();
}

//Helicopter Speed increase & decrease by arrow left & Right keys press
void handleSpecialKeypress(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:
            if (helicopterSpeedX > -0.05f) helicopterSpeedX -= 0.005f;
            break;
        case GLUT_KEY_RIGHT:
            if (helicopterSpeedX < 0.05f) helicopterSpeedX += 0.005f;
            break;
    }
}

//Helicopter Stop by press Mouse Right Button & Resume by pree Left Button
void handleMouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        if (button == GLUT_LEFT_BUTTON) {
            helicopterSpeedX = savedSpeedX; // resume
        } else if (button == GLUT_RIGHT_BUTTON) {
            savedSpeedX = helicopterSpeedX; // save
            helicopterSpeedX = 0.0f;        // stop
        }
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("Helicopter with Mouse Control");
    glutDisplayFunc(display);
    glutTimerFunc(30, update, 0);
    glutKeyboardFunc(handleKeypress);
    glutSpecialFunc(handleSpecialKeypress);
    glutMouseFunc(handleMouse); // <-- added
    glutMainLoop();
    return 0;
}
