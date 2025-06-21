//
//  ParticleSystem.cpp
//  Changing Seasons
//
//  Created by JALAL on 6/14/25.
//

#include "ParticleSystem.h"
#include <random>
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
ParticleSystem::ParticleSystem(int maxParticles, Renderer& renderer) : renderer(renderer) {
    particlePool.resize(maxParticles);
}
void ParticleSystem::setEffect(ParticleEffect newEffect) {
    if (currentEffect == newEffect) return;
    currentEffect = newEffect;
    if (currentEffect != ParticleEffect::NONE) {
        for (auto& p : particlePool) {
            respawnParticle(p, currentEffect);
            p.lifetime = (rand() / (float)RAND_MAX) * 5.0f;
        }
    }
}
void ParticleSystem::update(float deltaTime) {
    if (currentEffect == ParticleEffect::NONE) return;
    for (auto& p : particlePool) {
        if (p.lifetime <= 0.0f) {
            respawnParticle(p, currentEffect);
        }
        p.lifetime -= deltaTime;
        p.x += p.vx * deltaTime;
        p.y += p.vy * deltaTime;
    }
}
void ParticleSystem::draw() {
    if (currentEffect == ParticleEffect::NONE) return;
    for (const auto& p : particlePool) {
        if (p.lifetime > 0.0f) {
            if (currentEffect == ParticleEffect::RAIN) {
                renderer.drawRect(p.x, p.y, p.size, p.size * 5.0f, p.color);
            } else {
                renderer.drawRect(p.x, p.y, p.size, p.size, p.color);
            }
        }
    }
}
void ParticleSystem::respawnParticle(Particle& particle, ParticleEffect effect) {
    switch (effect) {
        case ParticleEffect::SNOW:
            particle.x = rand() % WINDOW_WIDTH;
            particle.y = WINDOW_HEIGHT + 10;
            particle.vx = (rand() % 50) - 25;
            particle.vy = -(rand() % 50) - 50;
            particle.lifetime = 5.0f + (rand() / (float)RAND_MAX) * 5.0f;
            particle.color = {1.0f, 1.0f, 1.0f};
            particle.size = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
            break;
        case ParticleEffect::RAIN:
            particle.x = rand() % WINDOW_WIDTH;
            particle.y = WINDOW_HEIGHT + 10;
            particle.vx = 0;
            particle.vy = -(rand() % 150) - 300;
            particle.lifetime = 1.5f + (rand() / (float)RAND_MAX) * 1.0f;
            particle.color = {0.9f, 0.9f, 1.0f};
            particle.size = 1.5f;
            break;
        case ParticleEffect::LEAVES:
            particle.x = rand() % WINDOW_WIDTH;
            particle.y = WINDOW_HEIGHT + 10;
            particle.vx = (rand() % 100) - 50;
            particle.vy = -(rand() % 40) - 20;
            particle.lifetime = 6.0f + (rand() / (float)RAND_MAX) * 6.0f;
            switch (rand() % 3) {
                case 0: particle.color = {0.8f, 0.2f, 0.1f}; break;
                case 1: particle.color = {0.9f, 0.5f, 0.1f}; break;
                case 2: particle.color = {0.9f, 0.8f, 0.2f}; break;
            }
            particle.size = 5.0f + (rand() / (float)RAND_MAX) * 3.0f;
            break;
        case ParticleEffect::NONE:
            particle.lifetime = 0.0f;
            break;
    }
}
