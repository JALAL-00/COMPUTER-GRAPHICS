//
//  Renderer.cpp
//  Changing Seasons
//
//  Created by JALAL on 6/14/25.
//

#define GL_SILENCE_DEPRECATION
#include "Renderer.h"
#include <GLUT/glut.h>
#include <cmath>
#include <string>
void Renderer::drawRect(float x, float y, float width, float height, Color color) {
    glColor3f(color.r, color.g, color.b);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}
void Renderer::drawCircle(float cx, float cy, float radius, Color color, int segments) {
    glColor3f(color.r, color.g, color.b);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; i++) {
        float angle = i * 2.0f * 3.1415926f / segments;
        float x_ = radius * cosf(angle);
        float y_ = radius * sinf(angle);
        glVertex2f(cx + x_, cy + y_);
    }
    glEnd();
}
void Renderer::drawText(float x, float y, const std::string& text, Color color) {
    glColor3f(color.r, color.g, color.b);
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    }
}
