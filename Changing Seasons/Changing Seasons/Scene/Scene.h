//
//  Scene.h
//  Changing Seasons
//
//  Created by JALAL on 6/14/25.
//

#pragma once
#include <vector>
#include <memory>
#include "../Graphics/Renderer.h"
#include "../Core/StateManager.h"
#include "../Graphics/ParticleSystem.h"
class Scene {
private:
    struct Cloud { float x, y; float width, height; float speed; };
    struct Tree { float x, y; float trunkWidth, trunkHeight; float canopyRadius; };
    struct Star { float x, y; float size; };
    Renderer& renderer;
    StateManager& stateManager;
    std::vector<Cloud> clouds;
    std::vector<Tree> trees;
    std::vector<Star> stars;
    std::unique_ptr<ParticleSystem> particleSystem;
    float cameraX = 0.0f;
    void initializeClouds();
    void initializeTrees();
    void initializeStars();
    void updateClouds(float deltaTime);
    void drawSky();
    void drawStars();
    void drawCelestialObjects();
    void drawClouds();
    void drawGround();
    void drawBuildings();
    void drawTrees();
    void drawUI();
    Renderer::Color lerpColor(const Renderer::Color& a, const Renderer::Color& b, float t);
public:
    Scene(Renderer& renderer, StateManager& stateManager);
    void update(float deltaTime);
    void draw();
    void panCamera(float dx);
};
