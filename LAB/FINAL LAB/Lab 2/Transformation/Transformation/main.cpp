#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <cmath>
#include <iostream>

// Constants for transformations
const float TX = 80.0f;       // Increased to shift transformed car to the right
const float TY = 100.0f;
const float ROT_ANGLE = 175.0f;
const float SX = 3.0f;
const float SY = 4.0f;

// Helper function to draw a filled circle (for wheels)
void drawCircle(float cx, float cy, float r, int num_segments) {
    glBegin(GL_POLYGON);
    for (int i = 0; i < num_segments; i++) {
        float theta = 2.0f * M_PI * static_cast<float>(i) / static_cast<float>(num_segments);
        float x = r * cosf(theta);
        float y = r * sinf(theta);
        glVertex2f(x + cx, y + cy);
    }
    glEnd();
}

// Function to draw the car object
void drawCar() {
    // Car body
    glBegin(GL_QUADS);
        glColor3f(0.0f, 0.0f, 1.0f);  // Blue front
        glVertex2f(-25.0f, -10.0f);

        glColor3f(0.0f, 1.0f, 0.0f);  // Green side
        glVertex2f(10.0f, -10.0f);

        glColor3f(1.0f, 0.0f, 0.0f);  // Red roof
        glVertex2f(10.0f, 10.0f);

        glColor3f(0.0f, 0.0f, 1.0f);  // Blue back
        glVertex2f(-15.0f, 10.0f);
    glEnd();

    // Wheels
    glColor3f(0.0f, 1.0f, 0.0f);  // Green
    float wheel_radius = 6.0f;
    float wheel_y_center = -10.0f - wheel_radius;
    drawCircle(-15.0f, wheel_y_center, wheel_radius, 30);
    drawCircle(0.0f, wheel_y_center, wheel_radius, 30);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // --- Draw Original Car ---
    glPushMatrix();
        drawCar();  // Original car
    glPopMatrix();

    // --- Draw Transformed Car ---
    glPushMatrix();
        glTranslatef(TX, TY, 0.0f);           // Translate
        glRotatef(ROT_ANGLE, 0.0f, 0.0f, 1.0f); // Rotate
        glScalef(SX, SY, 1.0f);               // Scale
        drawCar();                            // Transformed car
    glPopMatrix();

    glutSwapBuffers();
}

void reshape(int width, int height) {
    if (height == 0) height = 1;
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Coordinate system as per lab instructions
    gluOrtho2D(-200.0, 200.0, -300.0, 300.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black background
    glShadeModel(GL_SMOOTH);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Transformation Lab - 2 (OpenGL)");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glutMainLoop();
    return 0;
}
