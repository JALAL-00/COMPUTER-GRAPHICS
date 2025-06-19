//
//  Scene.cpp
//  Changing Seasons
//
//  Created by JALAL on 6/14/25.
//

#include "Scene.h"
#include <GLUT/glut.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <string>
#include <iomanip>
#include <sstream>
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
Scene::Scene(Renderer& renderer, StateManager& stateManager) : renderer(renderer), stateManager(stateManager) {
    initializeClouds();
    initializeTrees();
    initializeStars();
    particleSystem = std::make_unique<ParticleSystem>(500, renderer);
}
void Scene::initializeClouds() {
    clouds.push_back(Cloud{100.0f, 600.0f, 150.0f, 40.0f, 50.0f});
    clouds.push_back(Cloud{400.0f, 650.0f, 200.0f, 50.0f, 35.0f});
    clouds.push_back(Cloud{800.0f, 620.0f, 180.0f, 45.0f, 60.0f});
    clouds.push_back(Cloud{-200.0f, 630.0f, 160.0f, 40.0f, 45.0f});
}
void Scene::initializeTrees() {
    trees.push_back({700.0f, WINDOW_HEIGHT / 4.0f, 15.0f, 80.0f, 50.0f});
    trees.push_back({50.0f, WINDOW_HEIGHT / 4.0f, 12.0f, 70.0f, 40.0f});
    trees.push_back({950.0f, WINDOW_HEIGHT / 4.0f, 20.0f, 100.0f, 60.0f});
}
void Scene::initializeStars() {
    for (int i = 0; i < 200; ++i) {
        stars.push_back({(float)(rand() % WINDOW_WIDTH), (float)(rand() % WINDOW_HEIGHT), 1.0f + (rand() / (float)RAND_MAX) * 1.5f});
    }
}
void Scene::update(float deltaTime) {
    updateClouds(deltaTime);
    Season currentSeason = stateManager.getCurrentSeason();
    if (currentSeason == Season::WINTER) { particleSystem->setEffect(ParticleEffect::SNOW); }
    else if (currentSeason == Season::SPRING) { particleSystem->setEffect(ParticleEffect::RAIN); }
    else if (currentSeason == Season::AUTUMN) { particleSystem->setEffect(ParticleEffect::LEAVES); }
    else { particleSystem->setEffect(ParticleEffect::NONE); }
    particleSystem->update(deltaTime);
}
void Scene::updateClouds(float deltaTime) {
    for (auto& cloud : clouds) {
        cloud.x += cloud.speed * deltaTime;
        if (cloud.x > WINDOW_WIDTH) {
            cloud.x = -cloud.width;
            cloud.y = 550 + (rand() % 150);
            cloud.speed = 30 + (rand() % 40);
        }
    }
}
void Scene::draw() {
    glLoadIdentity();
    glTranslatef(-cameraX, 0.0f, 0.0f);
    drawSky();
    drawStars();
    drawCelestialObjects();
    drawClouds();
    particleSystem->draw();
    drawGround();
    drawTrees();
    drawBuildings();
    drawUI();
}
Renderer::Color Scene::lerpColor(const Renderer::Color& a, const Renderer::Color& b, float t) {
    t = std::max(0.0f, std::min(1.0f, t));
    return { a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t };
}
void Scene::drawSky() {
    float time = stateManager.getTimeOfDay();
    const Renderer::Color DAY_COLOR_SUMMER = {0.53f, 0.81f, 0.92f};
    const Renderer::Color DUSK_COLOR = {0.99f, 0.55f, 0.38f};
    const Renderer::Color NIGHT_COLOR = {0.05f, 0.05f, 0.15f};
    const Renderer::Color DAWN_COLOR = {0.83f, 0.68f, 0.78f};
    Renderer::Color skyColor;
    if (time >= 0.25f && time < 0.5f) { skyColor = lerpColor(DAWN_COLOR, DAY_COLOR_SUMMER, (time - 0.25f) * 4.0f); }
    else if (time >= 0.5f && time < 0.75f) { skyColor = lerpColor(DAY_COLOR_SUMMER, DUSK_COLOR, (time - 0.5f) * 4.0f); }
    else if (time >= 0.75f && time < 1.0f) { skyColor = lerpColor(DUSK_COLOR, NIGHT_COLOR, (time - 0.75f) * 4.0f); }
    else { skyColor = lerpColor(NIGHT_COLOR, DAWN_COLOR, time * 4.0f); }
    glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
}
void Scene::drawStars() {
    float time = stateManager.getTimeOfDay();
    if (time < 0.25f || time > 0.75f) {
        float opacity = (time < 0.25f) ? (1.0f - time * 4.0f) : ((time - 0.75f) * 4.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 0.9f, opacity);
        for (const auto& star : stars) {
            glBegin(GL_QUADS);
            glVertex2f(star.x, star.y);
            glVertex2f(star.x + star.size, star.y);
            glVertex2f(star.x + star.size, star.y + star.size);
            glVertex2f(star.x, star.y + star.size);
            glEnd();
        }
        glDisable(GL_BLEND);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
}
void Scene::drawCelestialObjects() {
    float time = stateManager.getTimeOfDay();
    const float PI = 3.14159265f;
    float angle = time * PI;
    float sunX = WINDOW_WIDTH * (1.0f - time);
    float sunY = (WINDOW_HEIGHT / 2.0f) + sin(angle) * (WINDOW_HEIGHT / 2.0f);
    float moonAngle = (time > 0.5f ? time - 0.5f : time + 0.5f) * PI;
    float moonX = WINDOW_WIDTH * (1.0f - (time > 0.5f ? time - 0.5f : time + 0.5f));
    float moonY = (WINDOW_HEIGHT / 2.0f) + sin(moonAngle) * (WINDOW_HEIGHT / 2.0f);
    if (time > 0.25f && time < 0.75f) { renderer.drawCircle(sunX, sunY, 50.0f, {1.0f, 1.0f, 0.0f}); }
    if (time < 0.25f || time > 0.75f) { renderer.drawCircle(moonX, moonY, 40.0f, {0.9f, 0.9f, 0.85f}); }
}
void Scene::drawClouds() {
    Renderer::Color cloudColor = {1.0f, 1.0f, 1.0f};
    for (const auto& cloud : clouds) {
        float ellipseHeight = cloud.height;
        float ellipseWidth = cloud.width / 2.0f;
        glPushMatrix();
        glTranslatef(cloud.x, cloud.y, 0);
        glPushMatrix();
        glTranslatef(ellipseWidth * 0.5f, 0, 0);
        glScalef(1.0, 0.7, 1.0);
        renderer.drawCircle(0, 0, ellipseWidth * 0.5f, cloudColor);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(ellipseWidth * 1.5f, 0, 0);
        glScalef(1.0, 0.6, 1.0);
        renderer.drawCircle(0, 0, ellipseWidth * 0.5f, cloudColor);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(ellipseWidth, ellipseHeight * 0.4f, 0);
        glScalef(1.2, 1.0, 1.0);
        renderer.drawCircle(0, 0, ellipseWidth * 0.6f, cloudColor);
        glPopMatrix();
        glPopMatrix();
    }
}
void Scene::drawGround() {
    Season currentSeason = stateManager.getCurrentSeason();
    Renderer::Color groundColor;
    if (currentSeason == Season::WINTER) { groundColor = {0.9f, 0.9f, 0.95f}; }
    else { groundColor = {0.34f, 0.49f, 0.27f}; }
    renderer.drawRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT / 4.0f, groundColor);
}
void Scene::drawTrees() {
    Season currentSeason = stateManager.getCurrentSeason();
    Renderer::Color trunkColor = {0.4f, 0.26f, 0.13f};
    for (const auto& tree : trees) {
        renderer.drawRect(tree.x, tree.y, tree.trunkWidth, tree.trunkHeight, trunkColor);
        if (currentSeason != Season::WINTER) {
            Renderer::Color canopyColor = {0.13f, 0.55f, 0.13f};
            if (currentSeason == Season::SPRING) { canopyColor = {0.56f, 0.93f, 0.56f}; }
            else if (currentSeason == Season::AUTUMN) { canopyColor = {0.8f, 0.4f, 0.1f}; }
            renderer.drawCircle(tree.x + tree.trunkWidth / 2.0f, tree.y + tree.trunkHeight, tree.canopyRadius, canopyColor);
        }
    }
}
void Scene::drawBuildings() {
    Season currentSeason = stateManager.getCurrentSeason();
    renderer.drawRect(100, WINDOW_HEIGHT / 4.0f, 150, 300, {0.2f, 0.2f, 0.2f});
    renderer.drawRect(280, WINDOW_HEIGHT / 4.0f, 120, 250, {0.3f, 0.3f, 0.3f});
    renderer.drawRect(500, WINDOW_HEIGHT / 4.0f, 200, 450, {0.15f, 0.15f, 0.15f});
    renderer.drawRect(800, WINDOW_HEIGHT / 4.0f, 180, 350, {0.25f, 0.25f, 0.25f});
    float time = stateManager.getTimeOfDay();
    Renderer::Color windowOff = {0.1f, 0.1f, 0.15f};
    Renderer::Color windowOn = {0.9f, 0.8f, 0.3f};
    Renderer::Color windowColor;
    if (time >= 0.7f && time < 0.8f) { windowColor = lerpColor(windowOff, windowOn, (time - 0.7f) * 10.0f); }
    else if (time >= 0.2f && time < 0.3f) { windowColor = lerpColor(windowOn, windowOff, (time - 0.2f) * 10.0f); }
    else if (time < 0.2f || time >= 0.8f) { windowColor = windowOn; }
    else { windowColor = windowOff; }
    for (int y = 0; y < 5; ++y) {
        for (int x = 0; x < 3; ++x) {
            float winX = 100 + 20 + x * 40;
            float winY = WINDOW_HEIGHT / 4.0f + 20 + y * 55;
            renderer.drawRect(winX, winY, 25, 35, windowColor);
        }
    }
    if (currentSeason == Season::WINTER) {
        Renderer::Color snowColor = {0.9f, 0.9f, 0.95f};
        float snowDepth = 10.0f;
        renderer.drawRect(100, WINDOW_HEIGHT / 4.0f + 300, 150, snowDepth, snowColor);
        renderer.drawRect(280, WINDOW_HEIGHT / 4.0f + 250, 120, snowDepth, snowColor);
        renderer.drawRect(500, WINDOW_HEIGHT / 4.0f + 450, 200, snowDepth, snowColor);
        renderer.drawRect(800, WINDOW_HEIGHT / 4.0f + 350, 180, snowDepth, snowColor);
    }
}
void Scene::drawUI() {
    glLoadIdentity();
    Renderer::Color textColor = {1.0f, 1.0f, 1.0f};
    std::string seasonText = "Season: ";
    switch(stateManager.getCurrentSeason()) {
        case Season::WINTER: seasonText += "Winter"; break;
        case Season::SPRING: seasonText += "Spring"; break;
        case Season::SUMMER: seasonText += "Summer"; break;
        case Season::AUTUMN: seasonText += "Autumn"; break;
    }
    float time = stateManager.getTimeOfDay();
    int hour = (int)(time * 24.0f);
    int minute = (int)(((time * 24.0f) - hour) * 60.0f);
    std::stringstream timeStream;
    timeStream << "Time: " << std::setw(2) << std::setfill('0') << hour << ":" << std::setw(2) << std::setfill('0') << minute;
    std::string timeText = timeStream.str();
    std::string controlsText1 = "Controls: [1-4] Change Season | [N] Toggle Day/Night Cycle";
    std::string controlsText2 = "[A/D] Pan Camera | [ESC] Exit";
    renderer.drawText(10, WINDOW_HEIGHT - 20, seasonText, textColor);
    renderer.drawText(10, WINDOW_HEIGHT - 40, timeText, textColor);
    renderer.drawText(10, 30, controlsText1, textColor);
    renderer.drawText(10, 10, controlsText2, textColor);
}
void Scene::panCamera(float dx) {
    cameraX += dx;
}
