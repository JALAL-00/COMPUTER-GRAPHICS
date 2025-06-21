#define GL_SILENCE_DEPRECATION
#include <iostream>
#include <GLUT/glut.h>
#include "Graphics/Renderer.h"
#include "Scene/Scene.h"
#include "Core/Timer.h"
#include "Core/StateManager.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
Renderer renderer;
StateManager stateManager;
Timer timer;
Scene scene(renderer, stateManager);

void display();
void reshape(int width, int height);
void keyboard(unsigned char key, int x, int y);
void idle();

int main(int argc, char** argv) {
    std::cout << "Starting 'Changing Seasons: Urban Scenery Transformation'..." << std::endl;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Changing Seasons: Urban Scenery Transformation");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;
}

void idle() {
    float deltaTime = timer.tick();
    stateManager.update(deltaTime);
    scene.update(deltaTime);
    glutPostRedisplay();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    scene.draw();
    glutSwapBuffers();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WINDOW_WIDTH, 0.0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: exit(0); break;
        case '1': stateManager.setSeason(Season::WINTER); break;
        case '2': stateManager.setSeason(Season::SPRING); break;
        case '3': stateManager.setSeason(Season::SUMMER); break;
        case '4': stateManager.setSeason(Season::AUTUMN); break;
        case 'n': case 'N':
            stateManager.toggleDayNightCycle();
            break;
        case 'a': case 'A':
            scene.panCamera(-20.0f);
            break;
        case 'd': case 'D':
            scene.panCamera(20.0f);
            break;
    }
}
