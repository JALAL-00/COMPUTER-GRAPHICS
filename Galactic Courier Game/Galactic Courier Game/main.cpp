// =================================================================================
// Galactic Courier: Space Delivery Under Fire (Version 1.1 - macOS Fixes)
//
// A 2D Space Game in a Single C++ File using OpenGL and GLUT
// Developed by an AI expert for demonstration purposes.
//
// How to Compile (macOS):
// g++ -o galactic_courier main.cpp -framework OpenGL -framework GLUT
//
// How to Compile (Linux/Windows with freeglut):
// g++ -o galactic_courier main.cpp -lGL -lGLU -lglut
//
// =================================================================================

// This definition silences warnings about using deprecated OpenGL/GLUT APIs on macOS
#define GL_SILENCE_DEPRECATION

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <ctime>

// For macOS, GLUT is included this way.
// For Windows/Linux, you might need <GL/glut.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// --- Constants & Configuration ---
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 900;
const float WORLD_WIDTH = 2400.0f; // Game world is larger than the window
const float WORLD_HEIGHT = 1800.0f;
const int FRAME_RATE = 60; // Target frames per second

// Game mechanics constants
const float PLAYER_ROTATION_SPEED = 4.0f;
const float PLAYER_THRUST = 0.15f;
const float PLAYER_BRAKE_FORCE = 0.98f;
const float MAX_PLAYER_SPEED = 8.0f;
const float PLAYER_ORBIT_LOCK_SPEED = 2.5f;
const int   INITIAL_PLAYER_HEALTH = 100;
const float SHIELD_DURATION = 3.0f; // in seconds
const float SHIELD_COOLDOWN = 10.0f;

const float MISSILE_SPEED = 5.0f;
const float MISSILE_TURN_RATE = 2.5f;
const int   MISSILE_DAMAGE = 25;
const int   ASTEROID_DAMAGE = 15;

const int NUM_STARS = 400;
const int NUM_INITIAL_ASTEROIDS = 10;
const float CARGO_DROP_SPEED = 6.0f;


// --- Enums for Game State ---
enum GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};


// --- Core Data Structures ---

// A simple 2D vector struct with basic operations
struct Vec2D {
    float x, y;

    Vec2D(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

    Vec2D operator+(const Vec2D& other) const { return Vec2D(x + other.x, y + other.y); }
    Vec2D operator-(const Vec2D& other) const { return Vec2D(x - other.x, y - other.y); }
    Vec2D operator*(float scalar) const { return Vec2D(x * scalar, y * scalar); }
    float length() const { return std::sqrt(x * x + y * y); }
    Vec2D normalize() const {
        float len = length();
        if (len > 0) return Vec2D(x / len, y / len);
        return Vec2D(0, 0);
    }
};

// Base class for all moving objects in the game
struct GameObject {
    Vec2D pos;
    Vec2D vel;
    float angle = 0.0f; // Degrees
    float radius = 1.0f;
    bool active = true;
};

// Player-specific data
struct Player : GameObject {
    int health;
    int score;
    bool hasCargo;
    int targetPlanetIndex;
    
    // Shield
    bool shieldActive;
    float shieldTimer;
    float shieldCooldownTimer;
    
    // Orbiting
    bool isOrbiting;
    int orbitingPlanetIdx;
    float orbitAngle;
};

// Planet data
struct Planet : GameObject {
    float orbitRadius;
    float orbitSpeed; // Degrees per second
    float gravityRadius;
    float r, g, b; // Color
};

// Missile data
struct Missile : GameObject {
    float lifetime;
};

// Asteroid data
struct Asteroid : GameObject {
    float rotationSpeed;
};

// Cargo pod data
struct CargoPod : GameObject {
    int targetPlanetIndex;
};

// Particle for effects
struct Particle {
    Vec2D pos;
    Vec2D vel;
    float lifetime;
    float r, g, b, a;
};


// --- Global Game State Variables ---
GameState gameState = MENU;
bool keys[256];
bool specialKeys[256];

Player player;
std::vector<Planet> planets;
std::vector<Missile> missiles;
std::vector<Asteroid> asteroids;
std::vector<CargoPod> cargoPods;
std::vector<Particle> particles;
std::vector<Vec2D> stars;

// Timers and counters
float gameTimer = 0.0f;
float missileSpawnTimer = 5.0f;
float missionTimer = 60.0f; // For timed missions
bool showMap = false;


// --- Forward Declarations of Functions ---

// Game Management
void initGame();
void resetGame();
void nextMission();

// Update Logic
void update(int value);
void updatePlayer(float dt);
void updatePlanets(float dt);
void updateMissiles(float dt);
void updateAsteroids(float dt);
void updateCargo(float dt);
void updateParticles(float dt);
void checkCollisions();
void spawnMissile();
void spawnParticleEffect(Vec2D pos, int count, float speed, Vec2D color, float lifetime);

// Rendering
void display();
void reshape(int w, int h);
void drawText(float x, float y, const std::string& text, void* font = GLUT_BITMAP_HELVETICA_18);
void drawCircle(float cx, float cy, float r, int num_segments, bool filled = true);
void drawPlayer();
void drawPlanets();
void drawMissiles();
void drawAsteroids();
void drawCargo();
void drawStarfield();
void drawHUD();
void drawMap();
void drawParticles();
void drawUI();

// Input Callbacks
void keyboard(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void specialKeysFunc(int key, int x, int y);
void specialKeysUpFunc(int key, int x, int y);

// --- main() Function ---
int main(int argc, char** argv) {
    // FIX: Cast time(0) to unsigned int to prevent compiler warning on some systems.
    srand((unsigned int)time(0));

    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Galactic Courier: Space Delivery Under Fire");

    // Register callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeysFunc);
    glutSpecialUpFunc(specialKeysUpFunc);
    
    // This timer function drives the main game loop
    glutTimerFunc(1000 / FRAME_RATE, update, 0);

    initGame();

    // Enter the GLUT event processing loop
    glutMainLoop();
    return 0;
}


// --- Game Management Functions ---

void initGame() {
    // Initialize starfield once
    for (int i = 0; i < NUM_STARS; ++i) {
        stars.push_back(Vec2D(
            (rand() % (int)WORLD_WIDTH) - WORLD_WIDTH / 2.0f,
            (rand() % (int)WORLD_HEIGHT) - WORLD_HEIGHT / 2.0f
        ));
    }
    resetGame();
}

void resetGame() {
    gameState = MENU; // Start at the menu
    
    // Clear all dynamic objects
    planets.clear();
    missiles.clear();
    asteroids.clear();
    cargoPods.clear();
    particles.clear();
    
    // Initialize Player
    player.pos = Vec2D(0, 0);
    player.vel = Vec2D(0, 0);
    player.angle = 90.0f; // Pointing up
    player.radius = 15.0f;
    player.health = INITIAL_PLAYER_HEALTH;
    player.score = 0;
    player.hasCargo = false;
    player.shieldActive = false;
    player.shieldTimer = 0.0f;
    player.shieldCooldownTimer = 0.0f;
    player.isOrbiting = false;

    // FIX: Initialize planets explicitly to avoid ambiguous aggregate initialization errors.
    // This is a more robust way to create complex objects.
    Planet sun;
    sun.pos = {0, 0}; sun.vel = {0, 0}; sun.angle = 0; sun.radius = 80.0f; sun.active = true;
    sun.orbitRadius = 0; sun.orbitSpeed = 0; sun.gravityRadius = 200.0f;
    sun.r = 0.9f; sun.g = 0.9f; sun.b = 0.2f;
    planets.push_back(sun);

    Planet earthLike;
    earthLike.pos = {0, 0}; earthLike.vel = {0, 0}; earthLike.angle = 0; earthLike.radius = 30.0f; earthLike.active = true;
    earthLike.orbitRadius = 400.0f; earthLike.orbitSpeed = 20.0f; earthLike.gravityRadius = 100.0f;
    earthLike.r = 0.2f; earthLike.g = 0.8f; earthLike.b = 0.4f;
    planets.push_back(earthLike);
    
    Planet marsLike;
    marsLike.pos = {0, 0}; marsLike.vel = {0, 0}; marsLike.angle = 0; marsLike.radius = 45.0f; marsLike.active = true;
    marsLike.orbitRadius = 750.0f; marsLike.orbitSpeed = 12.0f; marsLike.gravityRadius = 150.0f;
    marsLike.r = 0.8f; marsLike.g = 0.3f; marsLike.b = 0.2f;
    planets.push_back(marsLike);

    Planet purplePlanet;
    purplePlanet.pos = {0, 0}; purplePlanet.vel = {0, 0}; purplePlanet.angle = 0; purplePlanet.radius = 20.0f; purplePlanet.active = true;
    purplePlanet.orbitRadius = 250.0f; purplePlanet.orbitSpeed = -35.0f; purplePlanet.gravityRadius = 80.0f;
    purplePlanet.r = 0.5f; purplePlanet.g = 0.5f; purplePlanet.b = 0.6f;
    planets.push_back(purplePlanet);

    // Initialize Asteroids
    for (int i = 0; i < NUM_INITIAL_ASTEROIDS; ++i) {
        Asteroid a;
        a.pos = Vec2D( (rand() % (int)WORLD_WIDTH) - WORLD_WIDTH / 2.0f, (rand() % (int)WORLD_HEIGHT) - WORLD_HEIGHT / 2.0f);
        a.vel = Vec2D( (rand() % 100 - 50) / 50.0f, (rand() % 100 - 50) / 50.0f);
        a.radius = 10.0f + (rand() % 15);
        a.angle = rand() % 360;
        a.rotationSpeed = (rand() % 100 - 50) / 20.0f;
        asteroids.push_back(a);
    }

    // Reset timers
    gameTimer = 0.0f;
    missileSpawnTimer = 8.0f;
    showMap = false;
    
    // Initialize key states
    for(int i = 0; i < 256; ++i) {
        keys[i] = false;
        specialKeys[i] = false;
    }
}

void nextMission() {
    if (planets.size() < 2) return;
    
    player.hasCargo = true;
    
    // Assign a random target planet (not the sun, index 0)
    player.targetPlanetIndex = 1 + (rand() % (planets.size() - 1));
    missionTimer = 60.0f; // 60 seconds for delivery
    
    // Give a hint
    std::cout << "New Mission: Deliver cargo to Planet " << player.targetPlanetIndex << std::endl;
}

// --- Update Logic Functions ---

void update(int value) {
    // Schedule the next update
    glutTimerFunc(1000 / FRAME_RATE, update, 0);

    float dt = 1.0f / FRAME_RATE;
    gameTimer += dt;

    if (gameState == PLAYING) {
        updatePlayer(dt);
        updatePlanets(dt);
        updateMissiles(dt);
        updateAsteroids(dt);
        updateCargo(dt);
        updateParticles(dt);
        checkCollisions();

        // Spawn new missiles periodically
        missileSpawnTimer -= dt;
        if (missileSpawnTimer <= 0) {
            spawnMissile();
            missileSpawnTimer = 10.0f - (player.score / 500.0f); // Difficulty increases with score
            if (missileSpawnTimer < 3.0f) missileSpawnTimer = 3.0f;
        }

        // Mission logic
        if (player.hasCargo) {
            missionTimer -= dt;
            if (missionTimer <= 0) {
                player.hasCargo = false; // Failed delivery
                player.health -= 10; // Penalty
            }
        } else {
            // If no cargo, start a new mission
            nextMission();
        }
        
        if (player.health <= 0) {
            gameState = GAME_OVER;
            // FIX: Pass a Vec2D {r, g} for color, not {r, g, b}
            spawnParticleEffect(player.pos, 200, 5.0f, {1.0f, 0.5f}, 1.5f);
        }
    }
    else if (gameState == GAME_OVER) {
        updateParticles(dt); // Keep explosion animating
    }

    // Redraw the screen
    glutPostRedisplay();
}


void updatePlayer(float dt) {
    if (player.health <= 0) return;

    // --- Shield Logic ---
    if (player.shieldActive) {
        player.shieldTimer -= dt;
        if (player.shieldTimer <= 0) {
            player.shieldActive = false;
        }
    } else if (player.shieldCooldownTimer > 0) {
        player.shieldCooldownTimer -= dt;
    }

    if (keys['s'] || keys['S']) {
        if (!player.shieldActive && player.shieldCooldownTimer <= 0) {
            player.shieldActive = true;
            player.shieldTimer = SHIELD_DURATION;
            player.shieldCooldownTimer = SHIELD_COOLDOWN;
        }
    }

    // --- Orbiting Logic ---
    if (player.isOrbiting) {
        Planet& p = planets[player.orbitingPlanetIdx];
        float orbitDist = player.pos.y; // We stored the orbit distance in pos.y when locking
        player.orbitAngle += (PLAYER_ORBIT_LOCK_SPEED * 360.0f / (2 * M_PI * orbitDist)) * dt;
        player.pos.x = p.pos.x + orbitDist * cos(player.orbitAngle * M_PI / 180.0f);
        player.pos.y = p.pos.y + orbitDist * sin(player.orbitAngle * M_PI / 180.0f);
        
        // Face tangential to orbit
        player.angle = player.orbitAngle + 90;

        // Thrust to break orbit (slingshot)
        if (specialKeys[GLUT_KEY_UP]) {
            player.isOrbiting = false;
            float rad = player.angle * M_PI / 180.0f;
            player.vel = Vec2D(cos(rad), sin(rad)) * MAX_PLAYER_SPEED * 0.8f; // Slingshot boost
        }
        return; // Skip normal movement
    }

    // --- Standard Movement ---
    if (specialKeys[GLUT_KEY_LEFT]) player.angle += PLAYER_ROTATION_SPEED;
    if (specialKeys[GLUT_KEY_RIGHT]) player.angle -= PLAYER_ROTATION_SPEED;

    if (specialKeys[GLUT_KEY_UP]) {
        float rad = player.angle * M_PI / 180.0f;
        player.vel.x += cos(rad) * PLAYER_THRUST;
        player.vel.y += sin(rad) * PLAYER_THRUST;

        // Thrust particles
        Vec2D thrustDir = { (float)-cos(rad), (float)-sin(rad) };
        Vec2D particlePos = player.pos + thrustDir * player.radius;
        // FIX: Pass a Vec2D {r, g} for color, not {r, g, b}
        spawnParticleEffect(particlePos, 2, 2.0f, {1.0f, 0.7f}, 0.5f);
    }
    
    if (specialKeys[GLUT_KEY_DOWN]) {
        player.vel = player.vel * PLAYER_BRAKE_FORCE;
    }

    // Cap speed
    if (player.vel.length() > MAX_PLAYER_SPEED) {
        player.vel = player.vel.normalize() * MAX_PLAYER_SPEED;
    }

    // Update position
    player.pos = player.pos + player.vel;

    // Inertial drift (slight slowdown over time)
    player.vel = player.vel * 0.998f;

    // --- Check for Orbit Lock ---
    for (int i = 0; i < planets.size(); ++i) {
        float dist = (player.pos - planets[i].pos).length();
        if (dist < planets[i].gravityRadius && dist > planets[i].radius) {
            // Lock into orbit
            player.isOrbiting = true;
            player.orbitingPlanetIdx = i;
            player.vel = {0,0}; // Stop free movement
            // Set initial orbit angle and distance
            Vec2D dir_to_planet = player.pos - planets[i].pos;
            player.orbitAngle = atan2(dir_to_planet.y, dir_to_planet.x) * 180.0f / M_PI;
            player.pos.y = dir_to_planet.length(); // Store orbit distance here temporarily
            break;
        }
    }
}


void updatePlanets(float dt) {
    for (int i = 1; i < planets.size(); ++i) { // Skip sun at index 0
        Planet& p = planets[i];
        p.angle += p.orbitSpeed * dt;
        p.pos.x = p.orbitRadius * cos(p.angle * M_PI / 180.0f);
        p.pos.y = p.orbitRadius * sin(p.angle * M_PI / 180.0f);
    }
}

void updateMissiles(float dt) {
    for (auto& m : missiles) {
        if (!m.active) continue;

        m.lifetime -= dt;
        if (m.lifetime <= 0) {
            m.active = false;
            // FIX: Pass a Vec2D {r, g} for color, not {r, g, b}
            spawnParticleEffect(m.pos, 30, 2.0f, {0.5f, 0.5f}, 0.8f); // Fizzle out
            continue;
        }

        // Homing logic
        Vec2D dirToPlayer = (player.pos - m.pos).normalize();
        float targetAngle = atan2(dirToPlayer.y, dirToPlayer.x) * 180.0f / M_PI;
        
        // Normalize angles to handle wrapping
        while (targetAngle - m.angle > 180) targetAngle -= 360;
        while (targetAngle - m.angle < -180) targetAngle += 360;

        // Smoothly turn towards the target angle
        if (m.angle < targetAngle) {
            m.angle += MISSILE_TURN_RATE;
            if (m.angle > targetAngle) m.angle = targetAngle;
        } else if (m.angle > targetAngle) {
            m.angle -= MISSILE_TURN_RATE;
            if (m.angle < targetAngle) m.angle = targetAngle;
        }
        
        // Update velocity based on new angle
        float rad = m.angle * M_PI / 180.0f;
        m.vel = Vec2D(cos(rad), sin(rad)) * MISSILE_SPEED;
        m.pos = m.pos + m.vel;

        // Missile trail particles
        // FIX: Pass a Vec2D {r, g} for color, not {r, g, b}
        spawnParticleEffect(m.pos, 1, 1.0f, {1.0f, 0.2f}, 0.6f);
    }
    // Remove inactive missiles
    missiles.erase(std::remove_if(missiles.begin(), missiles.end(), [](const Missile& m){ return !m.active; }), missiles.end());
}

void updateAsteroids(float dt) {
    for (auto& a : asteroids) {
        a.pos = a.pos + a.vel;
        a.angle += a.rotationSpeed;

        // World wrapping
        if (a.pos.x > WORLD_WIDTH / 2) a.pos.x = -WORLD_WIDTH / 2;
        if (a.pos.x < -WORLD_WIDTH / 2) a.pos.x = WORLD_WIDTH / 2;
        if (a.pos.y > WORLD_HEIGHT / 2) a.pos.y = -WORLD_HEIGHT / 2;
        if (a.pos.y < -WORLD_HEIGHT / 2) a.pos.y = WORLD_HEIGHT / 2;
    }
}

void updateCargo(float dt) {
    for (auto& pod : cargoPods) {
        if (!pod.active) continue;

        // Simple homing towards target planet
        Vec2D targetPos = planets[pod.targetPlanetIndex].pos;
        Vec2D dir = (targetPos - pod.pos).normalize();
        pod.vel = dir * CARGO_DROP_SPEED;
        pod.pos = pod.pos + pod.vel;
        pod.angle = atan2(dir.y, dir.x) * 180.0 / M_PI;
    }
}

void updateParticles(float dt) {
    for (auto& p : particles) {
        p.pos = p.pos + p.vel * dt;
        p.lifetime -= dt;
        p.a = p.lifetime; // Fade out
    }
    // Remove dead particles
    particles.erase(std::remove_if(particles.begin(), particles.end(), [](const Particle& p){ return p.lifetime <= 0; }), particles.end());
}


void checkCollisions() {
    if (player.health <= 0) return;
    
    // Player vs Missiles
    for (auto& m : missiles) {
        if (!m.active) continue;
        float dist = (player.pos - m.pos).length();
        if (dist < player.radius + m.radius) {
            m.active = false;
            if (player.shieldActive) {
                player.score += 5; // Score for blocking
                // FIX: Pass a Vec2D {r, g} for color, not {r, g, b}
                spawnParticleEffect(player.pos, 100, 3.0f, {0.2f, 0.8f}, 1.0f); // Shield impact
            } else {
                player.health -= MISSILE_DAMAGE;
                // FIX: Pass a Vec2D {r, g} for color, not {r, g, b}
                spawnParticleEffect(m.pos, 100, 4.0f, {1.0f, 0.5f}, 1.0f);
            }
        }
    }

    // Player vs Asteroids
    for (auto& a : asteroids) {
        float dist = (player.pos - a.pos).length();
        if (dist < player.radius + a.radius) {
            if (!player.shieldActive) {
                player.health -= ASTEROID_DAMAGE;
                player.vel = player.vel * -0.5f; // Bounce off
            }
            // Move asteroid to avoid constant collision
            a.pos.x += (rand() % 200 - 100);
            a.pos.y += (rand() % 200 - 100);
            // FIX: Pass a Vec2D {r, g} for color, not {r, g, b}
            spawnParticleEffect(player.pos, 50, 2.0f, {0.6f, 0.6f}, 0.8f);
        }
    }

    // Cargo vs Planets
    for (auto& pod : cargoPods) {
        if (!pod.active) continue;
        Planet& targetPlanet = planets[pod.targetPlanetIndex];
        float dist = (pod.pos - targetPlanet.pos).length();
        if (dist < targetPlanet.radius) {
            pod.active = false;
            if (pod.targetPlanetIndex == player.targetPlanetIndex) {
                // Successful delivery
                player.score += (int)(missionTimer * 10); // Bonus for speed
                player.hasCargo = false;
                // FIX: Pass a Vec2D {r, g} for color, not {r, g, b}
                spawnParticleEffect(pod.pos, 150, 2.0f, {0.1f, 1.0f}, 1.5f);
            } else {
                // Wrong planet
                player.score -= 50;
                // FIX: Pass a Vec2D {r, g} for color, not {r, g, b}
                spawnParticleEffect(pod.pos, 100, 2.0f, {1.0f, 0.1f}, 1.5f);
            }
        }
    }
    cargoPods.erase(std::remove_if(cargoPods.begin(), cargoPods.end(), [](const CargoPod& p){ return !p.active; }), cargoPods.end());
}


void spawnMissile() {
    Missile m;
    // Spawn off-screen
    float angle = (rand() % 360) * M_PI / 180.0f;
    m.pos = player.pos + Vec2D(cos(angle), sin(angle)) * (WINDOW_WIDTH / 1.5f);
    m.radius = 8.0f;
    m.lifetime = 15.0f; // 15 seconds to live
    m.angle = (rand() % 360);
    missiles.push_back(m);
}

void spawnParticleEffect(Vec2D pos, int count, float speed, Vec2D color, float lifetime) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.pos = pos;
        float angle = (rand() % 360) * M_PI / 180.0f;
        float p_speed = speed * ( (rand() % 100) / 100.0f + 0.5f );
        p.vel = { cos(angle) * p_speed, sin(angle) * p_speed };
        p.r = color.x; p.g = color.y; p.b = 0.0f; // Blue is set to 0, using x for red and y for green
        p.a = 1.0f;
        p.lifetime = lifetime * ( (rand() % 100) / 100.0f + 0.5f );
        particles.push_back(p);
    }
}


// --- Rendering Functions ---

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Camera follows player
    glTranslatef(-player.pos.x + WINDOW_WIDTH / 2.0f, -player.pos.y + WINDOW_HEIGHT / 2.0f, 0.0f);

    drawStarfield();
    drawPlanets();
    drawAsteroids();
    drawCargo();
    if (player.health > 0) drawPlayer();
    drawMissiles();
    drawParticles();

    // Switch to screen-space for UI
    drawUI();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW);
}

void drawUI() {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    if (showMap) {
        drawMap();
    } else {
        drawHUD();
    }
    
    // Draw game state messages
    if (gameState == MENU) {
        std::string title = "GALACTIC COURIER";
        std::string subtitle = "Press 'Enter' to Start";
        drawText(WINDOW_WIDTH/2 - 120, WINDOW_HEIGHT/2 + 20, title, GLUT_BITMAP_TIMES_ROMAN_24);
        drawText(WINDOW_WIDTH/2 - 90, WINDOW_HEIGHT/2 - 20, subtitle);
    } else if (gameState == PAUSED) {
        std::string text = "PAUSED";
        drawText(WINDOW_WIDTH/2 - 40, WINDOW_HEIGHT/2, text, GLUT_BITMAP_TIMES_ROMAN_24);
    } else if (gameState == GAME_OVER) {
        std::string text = "GAME OVER";
        std::string score_text = "Final Score: " + std::to_string(player.score);
        std::string restart_text = "Press 'R' to Restart";
        drawText(WINDOW_WIDTH/2 - 60, WINDOW_HEIGHT/2 + 20, text, GLUT_BITMAP_TIMES_ROMAN_24);
        drawText(WINDOW_WIDTH/2 - 80, WINDOW_HEIGHT/2 - 20, score_text);
        drawText(WINDOW_WIDTH/2 - 80, WINDOW_HEIGHT/2 - 50, restart_text);
    }

    glPopMatrix();
}

void drawText(float x, float y, const std::string& text, void* font) {
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}

void drawCircle(float cx, float cy, float r, int num_segments, bool filled) {
    glBegin(filled ? GL_POLYGON : GL_LINE_LOOP);
    for (int i = 0; i < num_segments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(num_segments);
        float x = r * cosf(theta);
        float y = r * sinf(theta);
        glVertex2f(x + cx, y + cy);
    }
    glEnd();
}

void drawPlayer() {
    glPushMatrix();
    glTranslatef(player.pos.x, player.pos.y, 0);
    glRotatef(player.angle - 90, 0, 0, 1); // Adjust because 0 degrees is right, but we want up

    // Ship body (yellow triangle)
    glColor3f(1.0, 0.8, 0.0);
    glBegin(GL_TRIANGLES);
    glVertex2f(0, player.radius);
    glVertex2f(-player.radius * 0.6f, -player.radius * 0.6f);
    glVertex2f(player.radius * 0.6f, -player.radius * 0.6f);
    glEnd();

    // Cockpit
    glColor3f(0.2, 0.6, 0.8);
    glBegin(GL_TRIANGLES);
    glVertex2f(0, player.radius * 0.7f);
    glVertex2f(-player.radius * 0.3f, 0);
    glVertex2f(player.radius * 0.3f, 0);
    glEnd();
    
    // Thrust animation
    if (specialKeys[GLUT_KEY_UP] && gameState == PLAYING) {
        glColor3f(1.0, 0.5, 0.0);
        glBegin(GL_TRIANGLES);
        glVertex2f(0, -player.radius * 0.6f);
        glVertex2f(-player.radius * 0.4f, -player.radius);
        glVertex2f(player.radius * 0.4f, -player.radius * 1.5f);
        glEnd();
    }
    glPopMatrix();
    
    // Shield visual
    if (player.shieldActive) {
        float alpha = player.shieldTimer / SHIELD_DURATION;
        glColor4f(0.3f, 0.8f, 1.0f, 0.2f + alpha * 0.3f);
        drawCircle(player.pos.x, player.pos.y, player.radius + 10.0f, 40);
    }
}

void drawPlanets() {
    for (size_t i = 0; i < planets.size(); ++i) {
        const auto& p = planets[i];
        // Draw orbit path (don't draw for the sun at index 0)
        if (i > 0) {
            glColor4f(1.0, 1.0, 1.0, 0.15);
            drawCircle(0, 0, p.orbitRadius, 100, false);
        }
        
        // Draw gravity well
        glColor4f(p.r, p.g, p.b, 0.1);
        drawCircle(p.pos.x, p.pos.y, p.gravityRadius, 50, true);

        // Draw planet body
        glColor3f(p.r, p.g, p.b);
        drawCircle(p.pos.x, p.pos.y, p.radius, 50);
        
        // Indicate target planet
        if (player.hasCargo && i == player.targetPlanetIndex) {
            glColor3f(1.0, 1.0, 0.0);
            drawCircle(p.pos.x, p.pos.y, p.radius + 10, 40, false);
        }
    }
}

void drawMissiles() {
    for (const auto& m : missiles) {
        glPushMatrix();
        glTranslatef(m.pos.x, m.pos.y, 0);
        glRotatef(m.angle - 90, 0, 0, 1);
        
        // Missile Body
        glColor3f(0.8, 0.8, 0.8);
        glBegin(GL_QUADS);
        glVertex2f(-m.radius*0.25f, m.radius);
        glVertex2f(m.radius*0.25f, m.radius);
        glVertex2f(m.radius*0.25f, -m.radius);
        glVertex2f(-m.radius*0.25f, -m.radius);
        glEnd();
        
        // Nose Cone (red)
        glColor3f(1.0, 0.2, 0.2);
        glBegin(GL_TRIANGLES);
        glVertex2f(0, m.radius + 5);
        glVertex2f(-m.radius*0.25f, m.radius);
        glVertex2f(m.radius*0.25f, m.radius);
        glEnd();
        glPopMatrix();
    }
}

void drawAsteroids() {
    for (const auto& a : asteroids) {
        glPushMatrix();
        glTranslatef(a.pos.x, a.pos.y, 0);
        glRotatef(a.angle, 0, 0, 1);
        
        glColor3f(0.5, 0.4, 0.3);
        // Draw an irregular polygon for the asteroid
        glBegin(GL_POLYGON);
        glVertex2f(-a.radius*0.8f, a.radius);
        glVertex2f(a.radius, a.radius*0.7f);
        glVertex2f(a.radius*0.9f, -a.radius*0.6f);
        glVertex2f(0, -a.radius);
        glVertex2f(-a.radius, -a.radius*0.5f);
        glEnd();
        glPopMatrix();
    }
}

void drawCargo() {
     for (const auto& pod : cargoPods) {
        glPushMatrix();
        glTranslatef(pod.pos.x, pod.pos.y, 0);
        glRotatef(pod.angle - 90, 0, 0, 1);
        
        glColor3f(0.2, 1.0, 0.3); // Bright green
        glBegin(GL_QUADS);
        glVertex2f(-5, 5);
        glVertex2f(5, 5);
        glVertex2f(5, -5);
        glVertex2f(-5, -5);
        glEnd();
        glPopMatrix();
    }
}

void drawStarfield() {
    glPointSize(1.5);
    glBegin(GL_POINTS);
    for (const auto& star : stars) {
        float intensity = 0.5f + (rand() % 50) / 100.0f;
        glColor3f(intensity, intensity, intensity);
        glVertex2f(star.x, star.y);
    }
    glEnd();
}

void drawHUD() {
    // Score
    drawText(20, WINDOW_HEIGHT - 30, "SCORE: " + std::to_string(player.score));
    
    // Health Bar
    drawText(20, WINDOW_HEIGHT - 60, "HEALTH:");
    glColor3f(1.0, 0.0, 0.0);
    glRectf(100, WINDOW_HEIGHT - 65, 100 + 150, WINDOW_HEIGHT - 45);
    glColor3f(0.0, 1.0, 0.0);
    glRectf(100, WINDOW_HEIGHT - 65, 100 + (150 * (float)player.health / INITIAL_PLAYER_HEALTH), WINDOW_HEIGHT - 45);
    
    // Mission Timer
    if (player.hasCargo) {
        drawText(WINDOW_WIDTH - 200, WINDOW_HEIGHT - 30, "DELIVERY TIMER: " + std::to_string((int)missionTimer));
        drawText(WINDOW_WIDTH - 200, WINDOW_HEIGHT - 60, "TARGET: Planet " + std::to_string(player.targetPlanetIndex));
    } else {
        drawText(WINDOW_WIDTH - 200, WINDOW_HEIGHT - 30, "Awaiting new mission...");
    }

    // Shield Status
    if (player.shieldCooldownTimer > 0) {
        glColor3f(0.8, 0.8, 0.8);
        drawText(20, 30, "SHIELD RECHARGING: " + std::to_string((int)player.shieldCooldownTimer));
    } else {
        glColor3f(0.2, 0.8, 1.0);
        drawText(20, 30, "SHIELD READY (S)");
    }
    
    // Missile Alert
    for (const auto& m : missiles) {
        Vec2D diff = m.pos - player.pos;
        if (diff.length() < 600) {
            glColor3f(1.0, 0.2, 0.0);
            drawText(WINDOW_WIDTH/2 - 80, 50, "!! MISSILE INBOUND !!", GLUT_BITMAP_HELVETICA_18);
            break;
        }
    }
}

void drawMap() {
    // Semi-transparent background for the map
    glColor4f(0.0, 0.0, 0.0, 0.8);
    glRectf(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    float map_scale = 0.15f;
    
    glPushMatrix();
    // Center the map view
    glTranslatef(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f, 0);
    glScalef(map_scale, map_scale, 1.0);

    // Draw map elements (scaled down)
    // Planets
    for(const auto& p : planets) {
        glColor3f(p.r, p.g, p.b);
        drawCircle(p.pos.x, p.pos.y, p.radius, 20);
    }
    // Player
    glColor3f(1.0, 1.0, 0.0);
    drawCircle(player.pos.x, player.pos.y, player.radius * 3, 10); // Make player icon bigger
    // Missiles
    glColor3f(1.0, 0.0, 0.0);
    for(const auto& m : missiles) {
        drawCircle(m.pos.x, m.pos.y, m.radius * 3, 8);
    }

    glPopMatrix();
    
    drawText(WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT - 40, "SYSTEM MAP", GLUT_BITMAP_HELVETICA_18);
    drawText(20, 20, "Press 'Tab' to close map");
}

void drawParticles() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(2.0);
    glBegin(GL_POINTS);
    for (const auto& p : particles) {
        glColor4f(p.r, p.g, p.b, p.a);
        glVertex2f(p.pos.x, p.pos.y);
    }
    glEnd();
    glDisable(GL_BLEND);
}


// --- Input Callback Functions ---

void keyboard(unsigned char key, int x, int y) {
    keys[key] = true;
    switch (key) {
        case 27: // ESC key
            exit(0);
            break;
        case 13: // Enter key
            if (gameState == MENU) {
                gameState = PLAYING;
                nextMission();
            }
            break;
        case 'p':
        case 'P':
            if (gameState == PLAYING) gameState = PAUSED;
            else if (gameState == PAUSED) gameState = PLAYING;
            break;
        case 'r':
        case 'R':
            if (gameState == GAME_OVER) {
                resetGame();
            }
            break;
        case ' ': // Spacebar for dropping cargo
            if (gameState == PLAYING && player.hasCargo) {
                CargoPod pod;
                pod.pos = player.pos;
                pod.targetPlanetIndex = player.targetPlanetIndex;
                pod.radius = 5.0f;
                cargoPods.push_back(pod);
                player.hasCargo = false; // Cargo is away!
            }
            break;
        case '\t': // Tab key
            if (gameState == PLAYING || gameState == PAUSED) {
                showMap = !showMap;
            }
            break;
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    keys[key] = false;
}

void specialKeysFunc(int key, int x, int y) {
    specialKeys[key] = true;
}

void specialKeysUpFunc(int key, int x, int y) {
    specialKeys[key] = false;
}
