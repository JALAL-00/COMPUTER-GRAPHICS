#define GL_SILENCE_DEPRECATION
#include <cstdio>
#include <cmath>
#include <GLUT/glut.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <string>

// Include stb_image for texture loading
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define PI 3.14159265358979323846



const int NUM_MISSILES = 4;
const int NUM_TREES = 70;
const int NUM_GRASS_BLADES = 2000;
const int NUM_CLOUDS = 25;
const int TERRAIN_SIZE = 100;
const float TERRAIN_HEIGHT_SCALE = 5.0f;

// --- Structs & Enums ---

struct Vector3 {
    float x, y, z;
};

struct Particle {
    Vector3 position;
    Vector3 velocity;
    Vector3 color;
    float life;
    float size;
};

enum MissileState {
    IDLE,
    PRE_LAUNCH_SEQUENCE,
    IN_FLIGHT,
    STRIKE, // New state for impact
    EXPENDED // State after impact
};

struct Missile {
    MissileState state = IDLE;
    Vector3 position;
    Vector3 initialPosition;
    Vector3 targetPosition;
    Vector3 bezierControlPoint; // For arc trajectory
    float launchTime = 0.0f;
    float flightT = 0.0f; // 0.0 to 1.0 progress along trajectory
    float impactTime = 0.0f; // For explosion animation
    std::vector<Particle> smokeTrail;
};

enum CameraMode {
    ORBIT,
    SIDE_VIEW,
    TOP_DOWN_VIEW,
    FOLLOW_MISSILE
};

// --- Global State Variables ---

// Camera
CameraMode currentCameraMode = ORBIT;
float cameraAngleX = 20.0f, cameraAngleY = -45.0f;
float cameraDistance = 70.0f;
Vector3 cameraLookAt = {0, 5, 0};
int lastMouseX, lastMouseY;

// Scene & Simulation
Missile missiles[NUM_MISSILES];
Vector3 truckPositions[NUM_MISSILES];
Vector3 treePositions[NUM_TREES];
Vector3 grassPositions[NUM_GRASS_BLADES];
Vector3 cloudPositions[NUM_CLOUDS];
float terrainGrid[TERRAIN_SIZE + 1][TERRAIN_SIZE + 1];
GLuint cloudTextureID;

// Simulation State
bool launchSequenceActive = false;
bool isAborted = false;
bool isDay = true;
float simulationTime = 0.0f;
float lastFrameTime = 0.0f;
int nextMissileToLaunch = 0;

// Effects
bool ignitionFlashActive = false;
float ignitionFlashBrightness = 0.0f;
std::vector<Particle> groundSmoke;
std::vector<Particle> impactDebris;


// --- Function Prototypes ---
void init();
void display();
void reshape(int w, int h);
void update(int value);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);

// Drawing
void drawText(float x, float y, const std::string& text);
void drawGradientSky();

void drawTerrain();
void drawGrass();
void drawTruck(const Vector3& pos);
void drawTree(const Vector3& pos);
void drawMissile(const Missile& missile);
void drawParticles(const std::vector<Particle>& particles);
void drawImpact(const Missile& missile);

// Logic
void updateMissiles(float dt);
void updateParticles(std::vector<Particle>& particles, float dt, bool applyGravity);
void spawnParticle(std::vector<Particle>& particles, const Vector3& pos, const Vector3& baseVel, float spread, float lifetime, bool isGroundSmoke);
Vector3 calculateBezierPoint(float t, const Vector3& p0, const Vector3& p1, const Vector3& p2);
void resetSimulation();

// --- Main Function ---
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_ALPHA);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("🚀 Advanced Missile Launch & Strike Simulation v2.0");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}

// --- Initialization ---
void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE); // Important for scaled objects

    // Setup main directional light (Sun)
    GLfloat light_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat light_position[] = {1.0f, 1.0f, 0.5f, 0.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    // Setup point light for ignition flash (initially off)
    GLfloat flash_light_diffuse[] = {1.0f, 0.8f, 0.5f, 1.0f};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, flash_light_diffuse);
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.05);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.01);
    
    // Generate terrain
    for (int z = 0; z <= TERRAIN_SIZE; ++z) {
        for (int x = 0; x <= TERRAIN_SIZE; ++x) {
            float fx = (float)x / TERRAIN_SIZE * 2 * PI;
            float fz = (float)z / TERRAIN_SIZE * 2 * PI;
            terrainGrid[z][x] = TERRAIN_HEIGHT_SCALE * (sin(fx * 2) * cos(fz) + sin(fz * 2.5) * cos(fx * 0.5)) / 2.0f;
        }
    }

    srand((unsigned int)time(0));
    // Place trees, grass, and clouds
    for (int i = 0; i < NUM_TREES; ++i) {
        int tx = rand() % TERRAIN_SIZE;
        int tz = rand() % TERRAIN_SIZE;
        treePositions[i] = {(float)tx - TERRAIN_SIZE/2.0f, terrainGrid[tz][tx], (float)tz - TERRAIN_SIZE/2.0f};
    }
    for (int i = 0; i < NUM_GRASS_BLADES; ++i) {
        int tx = rand() % TERRAIN_SIZE;
        int tz = rand() % TERRAIN_SIZE;
        grassPositions[i] = {(float)tx - TERRAIN_SIZE/2.0f, terrainGrid[tz][tx], (float)tz - TERRAIN_SIZE/2.0f};
    }
    for (int i = 0; i < NUM_CLOUDS; ++i) {
        cloudPositions[i] = {
            (rand() % 200) - 100.0f,
            30.0f + (rand() % 2000) / 100.0f,
            (rand() % 200) - 100.0f
        };
    }
    
    // Load Cloud Texture
    int width, height, channels;
    unsigned char* data = stbi_load("cloud.png", &width, &height, &channels, 0);
    if (data) {
        glGenTextures(1, &cloudTextureID);
        glBindTexture(GL_TEXTURE_2D, cloudTextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    } else {
        std::cout << "Error: Could not load cloud.png. Make sure it's in the same directory." << std::endl;
    }


    resetSimulation();
    lastFrameTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

// --- GLUT Callbacks ---

void display() {
    // Set lighting based on day/night mode
    if (isDay) {
        GLfloat light_ambient[] = {0.4f, 0.4f, 0.4f, 1.0f};
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    } else {
        GLfloat light_ambient[] = {0.1f, 0.1f, 0.15f, 1.0f};
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // --- Background (Sky & Clouds) ---
    drawGradientSky();
    
    // --- 3D Scene ---
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1280.0 / 720.0, 1.0, 500.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Camera Setup
    Vector3 cameraPos;
    if (currentCameraMode == ORBIT) {
        cameraPos.x = cameraLookAt.x + cameraDistance * cos(cameraAngleX * PI / 180.0f) * sin(cameraAngleY * PI / 180.0f);
        cameraPos.y = cameraLookAt.y + cameraDistance * sin(cameraAngleX * PI / 180.0f);
        cameraPos.z = cameraLookAt.z + cameraDistance * cos(cameraAngleX * PI / 180.0f) * cos(cameraAngleY * PI / 180.0f);
        gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z, cameraLookAt.x, cameraLookAt.y, cameraLookAt.z, 0.0, 1.0, 0.0);
    } else if (currentCameraMode == SIDE_VIEW) {
        gluLookAt(-50, 20, 0, 0, 15, 0, 0, 1, 0);
    } else if (currentCameraMode == TOP_DOWN_VIEW) {
        gluLookAt(0, 100, 0.1, 0, 0, 0, 0, 1, 0);
    } else if (currentCameraMode == FOLLOW_MISSILE) {
        const Missile& m = missiles[0];
        gluLookAt(m.position.x - 15, m.position.y + 5, m.position.z, m.position.x, m.position.y, m.position.z, 0, 1, 0);
    }
    
    // Drawing Scene Objects
    glEnable(GL_LIGHTING);
    drawTerrain();
    drawGrass();
    
    for (int i = 0; i < NUM_TREES; ++i) drawTree(treePositions[i]);
    for (int i = 0; i < NUM_MISSILES; ++i) drawTruck(truckPositions[i]);
    for (int i = 0; i < NUM_MISSILES; ++i) drawMissile(missiles[i]);
    
    // Drawing Effects (with alpha blending and no depth writing)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);



    for (int i = 0; i < NUM_MISSILES; ++i) {
        if(missiles[i].state > PRE_LAUNCH_SEQUENCE) drawParticles(missiles[i].smokeTrail);
        if(missiles[i].state == STRIKE) drawImpact(missiles[i]);
    }
    drawParticles(groundSmoke);
    drawParticles(impactDebris);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    
    // --- HUD/UI Text ---
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1280, 0, 720);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // from: glColor3f(1.0, 1.0, 1.0);
    // to:
    glColor4f(1.0, 1.0, 1.0, 1.0);
    
    drawText(10, 700, "Advanced Missile Launch & Strike Simulation");
    std::string launch_status = "Status: STANDING BY. " + std::to_string(NUM_MISSILES - nextMissileToLaunch) + " missiles ready.";
    if(nextMissileToLaunch > 0) {
        launch_status = "Status: LAUNCH SEQUENCE ACTIVE.";
    }
    if(nextMissileToLaunch == NUM_MISSILES && missiles[NUM_MISSILES-1].state >= STRIKE) {
        launch_status = "Status: ALL MISSILES EXPENDED.";
    }
    drawText(10, 680, launch_status);
    drawText(10, 660, "[L] to Launch Next | [R] to Reset");
    
    if (isAborted) {
        glColor3f(1, 0, 0);
        drawText(10, 640, "LAUNCH ABORTED");
    } else if(launchSequenceActive) {
        bool prelaunch_found = false;
        for(int i=0; i<NUM_MISSILES; ++i) {
            if(missiles[i].state == PRE_LAUNCH_SEQUENCE) {
                prelaunch_found = true; break;
            }
        }
        if (prelaunch_found) drawText(10, 640, "[A] to Abort");
    }

    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW); glPopMatrix();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
}

void update(int value) {
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float dt = currentTime - lastFrameTime;
    lastFrameTime = currentTime;

    if (launchSequenceActive && !isAborted) {
        simulationTime += dt;
        updateMissiles(dt);
    }
    
    // Update all particle systems
    for (int i = 0; i < NUM_MISSILES; ++i) updateParticles(missiles[i].smokeTrail, dt, false);
    updateParticles(groundSmoke, dt, false);
    updateParticles(impactDebris, dt, true); // Debris is affected by gravity
    
    // Update ignition flash effect
    if (ignitionFlashActive) {
        ignitionFlashBrightness -= 2.0 * dt;
        if(ignitionFlashBrightness <= 0.0) {
            ignitionFlashActive = false;
            ignitionFlashBrightness = 0.0;
            glDisable(GL_LIGHT1);
        } else {
            GLfloat flash_color[] = {ignitionFlashBrightness, ignitionFlashBrightness * 0.8f, ignitionFlashBrightness * 0.5f, 1.0f};
            glLightfv(GL_LIGHT1, GL_DIFFUSE, flash_color);
            glLightfv(GL_LIGHT1, GL_SPECULAR, flash_color);
        }
    }
    
    // Animate clouds
    for(auto& cloudPos : cloudPositions) {
        cloudPos.x += 1.5f * dt; // Simple wind effect
        if(cloudPos.x > 120) cloudPos.x = -120;
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: exit(0); break;
        case 'l': case 'L':
            if (nextMissileToLaunch < NUM_MISSILES && !isAborted) {
                missiles[nextMissileToLaunch].state = PRE_LAUNCH_SEQUENCE;
                missiles[nextMissileToLaunch].launchTime = simulationTime;
                nextMissileToLaunch++;
                launchSequenceActive = true;
            }
            break;
        case 'r': case 'R': resetSimulation(); break;
        case 'a': case 'A':
            if (launchSequenceActive && !isAborted) {
                bool canAbort = false;
                for(int i=0; i<NUM_MISSILES; ++i) {
                    if (missiles[i].state == PRE_LAUNCH_SEQUENCE) {
                        missiles[i].state = IDLE;
                        canAbort = true;
                    }
                }
                if(canAbort) isAborted = true;
            }
            break;
        case 'd': case 'D': isDay = !isDay; break;
        case '1': currentCameraMode = ORBIT; break;
        case '2': currentCameraMode = SIDE_VIEW; break;
        case '3': currentCameraMode = TOP_DOWN_VIEW; break;
        case '4':
            if (missiles[0].state >= IN_FLIGHT) currentCameraMode = FOLLOW_MISSILE;
            break;
    }
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        lastMouseX = x; lastMouseY = y;
    }
    if (currentCameraMode == ORBIT) {
        if (button == 3) cameraDistance = fmax(5.0f, cameraDistance - 2.0f);
        else if (button == 4) cameraDistance = fmin(200.0f, cameraDistance + 2.0f);
    }
}

void motion(int x, int y) {
    if (currentCameraMode == ORBIT) {
        cameraAngleY += (x - lastMouseX);
        cameraAngleX += (y - lastMouseY);
        lastMouseX = x; lastMouseY = y;
        if (cameraAngleX > 89.0f) cameraAngleX = 89.0f;
        if (cameraAngleX < -89.0f) cameraAngleX = -89.0f;
    }
}

// --- Drawing Functions ---

void drawText(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);
    for (char c : text) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
}

// Replace your old drawGradientSky function with this one

void drawGradientSky() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glBegin(GL_QUADS);
    Vector3 topColor = isDay ? Vector3{0.4f, 0.6f, 1.0f} : Vector3{0.05f, 0.05f, 0.1f};
    Vector3 bottomColor = isDay ? Vector3{0.7f, 0.8f, 1.0f} : Vector3{0.1f, 0.1f, 0.25f};
    
    // --- THIS IS THE FIX ---
    // Use glColor4f to set the alpha to 1.0 (fully opaque)
    glColor4f(topColor.x, topColor.y, topColor.z, 1.0f);
    glVertex2f(-1.0, 1.0);
    glVertex2f(1.0, 1.0);
    glColor4f(bottomColor.x, bottomColor.y, bottomColor.z, 1.0f);
    glVertex2f(1.0, -1.0);
    glVertex2f(-1.0, -1.0);
    glEnd();
    
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// Replace your entire existing drawClouds function with this one.



void drawTerrain() {
    glColor3f(0.4f, 0.5f, 0.2f);
    float halfSize = TERRAIN_SIZE / 2.0f;

    for (int z = 0; z < TERRAIN_SIZE; ++z) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int x = 0; x <= TERRAIN_SIZE; ++x) {
            Vector3 p1 = {(float)x - halfSize, terrainGrid[z][x], (float)z - halfSize};
            Vector3 p2 = {(float)x - halfSize, terrainGrid[z + 1][x], (float)z + 1 - halfSize};
            
            // Basic normal calculation for smooth lighting
            float L = terrainGrid[z][x > 0 ? x - 1 : x];
            float R = terrainGrid[z][x < TERRAIN_SIZE ? x + 1 : x];
            float D = terrainGrid[z > 0 ? z - 1 : z][x];
            float U = terrainGrid[z < TERRAIN_SIZE ? z + 1 : z][x];
            Vector3 n = {L - R, 2.0f, D - U};
            glNormal3f(n.x, n.y, n.z);
            
            glVertex3f(p1.x, p1.y, p1.z);
            glVertex3f(p2.x, p2.y, p2.z);
        }
        glEnd();
    }
}

void drawGrass() {
    glDisable(GL_LIGHTING);
    glColor3f(0.3f, 0.6f, 0.1f);
    glBegin(GL_LINES);
    for (int i = 0; i < NUM_GRASS_BLADES; ++i) {
        glVertex3f(grassPositions[i].x, grassPositions[i].y, grassPositions[i].z);
        glVertex3f(grassPositions[i].x, grassPositions[i].y + 0.4f, grassPositions[i].z);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}


void drawTruck(const Vector3& pos) {
    glPushMatrix();
    glTranslatef(pos.x, pos.y, pos.z);
    
    // Main Body
    glColor3f(0.2f, 0.3f, 0.2f);
    glPushMatrix();
    glTranslatef(0, 0.5f, 0);
    glScalef(6.0f, 1.0f, 2.8f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Cab
    glColor3f(0.25f, 0.35f, 0.25f);
    glPushMatrix();
    glTranslatef(-2.0f, 1.5f, 0.0f);
    glScalef(2.0f, 1.5f, 2.5f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Wheels
    glColor3f(0.1f, 0.1f, 0.1f);
    GLUquadric* wheel = gluNewQuadric();
    for(int i=-1; i<=1; i+=2) { // Left/Right
        glPushMatrix();
        glTranslatef(1.5f, 0.25f, 1.4f * i);
        gluCylinder(wheel, 0.5, 0.5, 0.5, 12, 1);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(-1.5f, 0.25f, 1.4f * i);
        gluCylinder(wheel, 0.5, 0.5, 0.5, 12, 1);
        glPopMatrix();
    }
    gluDeleteQuadric(wheel);

    // Launch Pad Base (the part that tilts)
    glColor3f(0.4f, 0.4f, 0.4f);
    glPushMatrix();
    glTranslatef(1.0f, 1.25f, 0.0f);
    glRotatef(75, 1, 0, 0); // Rotates up
    glScalef(5.0f, 2.0f, 0.5f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPopMatrix();
}

// In your C++ file, replace the old drawTree function with this one.

void drawTree(const Vector3& pos) {
    glPushMatrix();
    glTranslatef(pos.x, pos.y, pos.z);

    // Trunk
    glColor3f(0.5f, 0.35f, 0.05f);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    GLUquadric* trunk = gluNewQuadric();
    // --- SCALED DOWN VALUES ---
    gluCylinder(trunk, 0.2, 0.15, 2.5, 8, 1); // Height is now 2.5
    // A branch
    glTranslatef(0,0,1.0);
    glRotatef(45, 0, 1, 0);
    gluCylinder(trunk, 0.08, 0.04, 1.2, 6, 1); // Smaller branch
    gluDeleteQuadric(trunk);
    glPopMatrix();

    // Leaves (as a cluster of spheres)
    glColor3f(0.1f, 0.5f, 0.1f);
    // --- SCALED DOWN VALUES ---
    glTranslatef(0, 2.8, 0); // Y-offset lowered
    glutSolidSphere(1.0, 8, 6); // Radius is now 1.0
    glTranslatef(0.3, -0.3, 0.3);
    glutSolidSphere(0.8, 8, 6); // Radius is now 0.8
    glTranslatef(-0.6, 0, -0.6);
    glutSolidSphere(0.9, 8, 6); // Radius is now 0.9
    
    glPopMatrix();
}

void drawMissile(const Missile& missile) {
    if (missile.state == IDLE || missile.state == EXPENDED) return;

    glPushMatrix();
    glTranslatef(missile.position.x, missile.position.y, missile.position.z);
    
    // Orient missile to follow its trajectory path
    if (missile.state == IN_FLIGHT) {
        Vector3 nextPos = calculateBezierPoint(fmin(1.0f, missile.flightT + 0.01f), missile.initialPosition, missile.bezierControlPoint, missile.targetPosition);
        Vector3 dir = {nextPos.x - missile.position.x, nextPos.y - missile.position.y, nextPos.z - missile.position.z};
        float len = sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
        if (len > 0.001f) {
            dir.x /= len; dir.y /= len; dir.z /= len;
            float angle = acos(dir.y) * 180.0 / PI; // Angle from vertical (Y-axis)
            Vector3 axis = {-dir.z, 0, dir.x}; // Cross product with (0,1,0)
            glRotatef(angle, axis.x, axis.y, axis.z);
        }
    } else {
        glRotatef(-90, 1, 0, 0); // Pointing up
    }
    

    // Body
    glColor3f(0.8f, 0.8f, 0.85f);
    GLUquadric* body = gluNewQuadric();
    gluCylinder(body, 0.5, 0.5, 4.0, 16, 1);
    gluDeleteQuadric(body);

    // Nose cone
    glColor3f(0.9f, 0.2f, 0.2f);
    glPushMatrix();
    glTranslatef(0, 0, 4.0);
    glutSolidCone(0.5, 1.5, 16, 1);
    glPopMatrix();
    
    // Fins
    glColor3f(0.6f, 0.6f, 0.65f);
    for(int i=0; i<4; ++i) {
        glPushMatrix();
        glRotatef(i * 90, 0, 0, 1);
        glTranslatef(0.5f, 0, 0.5f);
        glScalef(0.5f, 0.05f, 1.0f);
        glutSolidCube(1.0);
        glPopMatrix();
    }


    // Flame effect
    if (missile.state == IN_FLIGHT) {
        glDisable(GL_LIGHTING);
        float flameLength = 2.0f + (rand() % 100) / 100.0f * 1.5f;
        glColor3f(1.0f, 0.6f, 0.0f);
        glPushMatrix(); glTranslatef(0, 0, -flameLength); glutSolidCone(0.4, flameLength, 12, 1); glPopMatrix();
        glColor3f(1.0f, 1.0f, 0.5f);
        glPushMatrix(); glTranslatef(0, 0, -flameLength*0.6f); glutSolidCone(0.2, flameLength*0.6f, 12, 1); glPopMatrix();
        glEnable(GL_LIGHTING);
    }
    
    glPopMatrix();
}

void drawParticles(const std::vector<Particle>& particles) {
    glDisable(GL_LIGHTING);
    glBegin(GL_POINTS);
    for (const auto& p : particles) {
        glPointSize(p.size);
        glColor4f(p.color.x, p.color.y, p.color.z, p.life * 0.5f);
        glVertex3f(p.position.x, p.position.y, p.position.z);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void drawImpact(const Missile& missile) {
    glDisable(GL_LIGHTING);
    float t = missile.impactTime; // time since impact
    
    // Expanding shockwave sphere
    float radius = t * 30.0f;
    float alpha = fmax(0.0f, 1.0f - t);
    glColor4f(1.0, 0.8, 0.5, alpha * 0.5f);
    glPushMatrix();
    glTranslatef(missile.targetPosition.x, missile.targetPosition.y + radius * 0.5f, missile.targetPosition.z);
    glutSolidSphere(radius, 16, 16);
    glPopMatrix();

    // Scorched earth decal
    glColor4f(0.1, 0.1, 0.1, alpha);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(missile.targetPosition.x, missile.targetPosition.y + 0.1f, missile.targetPosition.z);
    for(int i=0; i<=360; i+=30) {
        float angle = i * PI / 180.0f;
        glVertex3f(missile.targetPosition.x + cos(angle)*5.0f, missile.targetPosition.y + 0.1f, missile.targetPosition.z + sin(angle)*5.0f);
    }
    glEnd();

    glEnable(GL_LIGHTING);
}


// --- Simulation and Update Logic ---

void updateMissiles(float dt) {
    for (int i = 0; i < NUM_MISSILES; ++i) {
        Missile& m = missiles[i];
        if (m.state == EXPENDED) continue;
        
        float timeSinceLaunch = simulationTime - m.launchTime;

        switch (m.state) {
            case PRE_LAUNCH_SEQUENCE:
                 if (timeSinceLaunch >= 2.0f) { // 2 second countdown
                     m.state = IN_FLIGHT;
                     ignitionFlashActive = true;
                     ignitionFlashBrightness = 1.0;
                     glEnable(GL_LIGHT1);
                     GLfloat pos[] = {m.initialPosition.x, m.initialPosition.y, m.initialPosition.z, 1.0f};
                     glLightfv(GL_LIGHT1, GL_POSITION, pos);
                     for(int k=0; k<100; ++k) spawnParticle(groundSmoke, m.initialPosition, {0,0,0}, 2.0, 4.0, true);
                 }
                 break;

            case IN_FLIGHT:
                 m.flightT += dt * 0.25f; // Speed of missile along arc
                 m.position = calculateBezierPoint(m.flightT, m.initialPosition, m.bezierControlPoint, m.targetPosition);
                 
                 if ((rand() % 2) == 0) {
                     spawnParticle(m.smokeTrail, m.position, {0, -2.0f, 0}, 0.5, 3.0, false);
                 }

                 if (m.flightT >= 1.0f) {
                     m.state = STRIKE;
                     m.impactTime = 0.0f;
                     // Spawn impact debris
                     for(int k=0; k<200; ++k) spawnParticle(impactDebris, m.targetPosition, {0, 10.0f, 0}, 15.0, 3.0, false);
                 }
                 break;
            
            case STRIKE:
                m.impactTime += dt;
                if (m.impactTime > 1.0f) {
                    m.state = EXPENDED;
                }
                break;
            
            case IDLE:
            case EXPENDED:
                break; // Do nothing
        }
    }
}

void updateParticles(std::vector<Particle>& particles, float dt, bool applyGravity) {
    for (auto it = particles.begin(); it != particles.end(); ) {
        it->life -= dt;
        if (it->life <= 0) {
            it = particles.erase(it);
        } else {
            it->position.x += it->velocity.x * dt;
            it->position.y += it->velocity.y * dt;
            it->position.z += it->velocity.z * dt;
            if (applyGravity) {
                it->velocity.y -= 9.8f * dt; // Simple gravity
            }
            
            // Fade to grey for smoke
            it->color.x += (0.5f - it->color.x) * dt * 0.5f;
            it->color.y = it->color.x;
            it->color.z = it->color.x;
            
            ++it;
        }
    }
}

void spawnParticle(std::vector<Particle>& particles, const Vector3& pos, const Vector3& baseVel, float spread, float lifetime, bool isGroundSmoke) {
    Particle p;
    p.position = pos;
    p.velocity.x = baseVel.x + (rand() % 200 - 100) / 100.0f * spread;
    p.velocity.y = baseVel.y + (isGroundSmoke ? (rand() % 100) / 100.0f * spread : (rand() % 200 - 100) / 100.0f * spread);
    p.velocity.z = baseVel.z + (rand() % 200 - 100) / 100.0f * spread;
    p.life = lifetime * (0.5f + (rand() % 100) / 100.0f * 0.5f);
    p.size = isGroundSmoke ? 8.0f : 5.0f;
    // Impact debris is fiery
    if(&particles == &impactDebris) {
        p.color = {1.0f, 0.8f, 0.2f};
    } else {
        p.color = {0.8f, 0.8f, 0.8f};
    }
    particles.push_back(p);
}


Vector3 calculateBezierPoint(float t, const Vector3& p0, const Vector3& p1, const Vector3& p2) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;

    Vector3 p;
    p.x = uu * p0.x + 2 * u * t * p1.x + tt * p2.x;
    p.y = uu * p0.y + 2 * u * t * p1.y + tt * p2.y;
    p.z = uu * p0.z + 2 * u * t * p1.z + tt * p2.z;
    return p;
}


void resetSimulation() {
    launchSequenceActive = false;
    isAborted = false;
    simulationTime = 0.0f;
    nextMissileToLaunch = 0;
    
    float spacing = 15.0f;
    float start_x = -spacing * (NUM_MISSILES - 1) / 2.0f;
    for (int i = 0; i < NUM_MISSILES; ++i) {
        float x = start_x + i * spacing;
        float z = -20;
        int tx = (int)(x + TERRAIN_SIZE/2.0f);
        int tz = (int)(z + TERRAIN_SIZE/2.0f);
        float y = terrainGrid[tz][tx];
        
        truckPositions[i] = {x, y, z};
        
        missiles[i] = Missile(); // Reset missile
        missiles[i].initialPosition = {x, y + 2.0f, z};
        missiles[i].position = missiles[i].initialPosition;
        
        // Assign a target and calculate the trajectory control point
        float target_x = (rand() % 40) - 20;
        float target_z = 30 + (rand() % 20);
        int ttx = (int)(target_x + TERRAIN_SIZE/2.0f);
        int ttz = (int)(target_z + TERRAIN_SIZE/2.0f);
        float target_y = terrainGrid[ttz][ttx];
        missiles[i].targetPosition = {target_x, target_y, target_z};

        missiles[i].bezierControlPoint = {
            (missiles[i].initialPosition.x + target_x) / 2.0f,
            100.0f, // Apex height of the trajectory
            (missiles[i].initialPosition.z + target_z) / 2.0f
        };
    }
    
    groundSmoke.clear();
    impactDebris.clear();
    currentCameraMode = ORBIT;
}
