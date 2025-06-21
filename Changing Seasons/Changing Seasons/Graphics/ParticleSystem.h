//
//  ParticleSystem.h
//  Changing Seasons
//
//  Created by JALAL on 6/14/25.
//

#pragma once
#include <vector>
#include "Renderer.h"
struct Particle {
    float x, y;
    float vx, vy;
    float lifetime;
    Renderer::Color color;
    float size;
};
enum class ParticleEffect {
    NONE,
    SNOW,
    RAIN,
    LEAVES
};
class ParticleSystem {
private:
    std::vector<Particle> particlePool;
    Renderer& renderer;
    ParticleEffect currentEffect = ParticleEffect::NONE;
    void respawnParticle(Particle& particle, ParticleEffect effect);
public:
    ParticleSystem(int maxParticles, Renderer& renderer);
    void setEffect(ParticleEffect newEffect);
    void update(float deltaTime);
    void draw();
};
