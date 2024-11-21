#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
#include<cfloat>
#include<iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


// Camera position and orientation
float camX = 0.0f, camY = 2.0f, camZ = 10.0f;
float camYaw = 0.0f, camPitch = 0.0f;

// Player movement speed
const float movementSpeed = 0.1f;
float playerVelocityY = 0.0f;  // Vertical velocity of the player
bool isJumping = false;        // Whether the player is currently jumping
const float gravity = -0.01f;  // Simulated gravity
const float jumpForce = 0.2f;  // Initial jump velocity


float bulletStartX, bulletStartY, bulletStartZ;
float bulletEndX, bulletEndY, bulletEndZ;
bool bulletVisible = false;
int bulletTimer = 0; // Timer to hide the bullet line

//utils
int currentLevel = 1;               // Current level
int enemiesToSpawn = 5;             // Number of enemies to spawn in the current level
bool showLevelOverlay = true;       // Whether to show the level overlay
int levelOverlayTimer = 0;          // Timer for the level overlay

GLuint floorTexture; // Global variable to store the floor texture ID

// Enemy structure
const float minDistanceToPlayer = 5.0f; // Minimum distance enemies maintain from the player



struct Enemy {
    float x, y, z;  // Position
    float dx, dz;   // Direction
    float speed;    // Movement speed
    bool alive;     // Alive status
    int changeDirectionTimer; // Timer to change direction
};





std::vector<Enemy> enemies;
const int enemyCount = 5;
const float floorSize = 50.0f;

// Function prototypes
void setupCamera();
void drawFloor();
void drawCrosshair();
void drawEnemies();
void handleKeyboard(unsigned char key, int x, int y);
void handleMouse(int x, int y);
void updateEnemies();
void fire();

// Random number generator
float randomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

// Initialize enemies
/*
void generateEnemies() {
    srand(static_cast<unsigned int>(time(0)));
    for (int i = 0; i < enemyCount; ++i) {
        enemies.push_back({randomFloat(-floorSize / 2, floorSize / 2), 0.5f, randomFloat(-floorSize / 2, floorSize / 2), true});
    }
}
*/
void generateEnemies() {
    srand(static_cast<unsigned int>(time(0)));
    for (int i = 0; i < enemyCount; ++i) {
        enemies.push_back({
            randomFloat(-floorSize / 2, floorSize / 2), // Initial X position
            0.5f,                                       // Fixed Y position
            randomFloat(-floorSize / 2, floorSize / 2), // Initial Z position
            randomFloat(-1.0f, 1.0f),                   // Random initial X direction
            randomFloat(-1.0f, 1.0f),                   // Random initial Z direction
            randomFloat(0.02f, 0.05f),                  // Random speed
            true,                                       // Alive
            rand() % 120                                // Random timer to change direction
        });
    }
}
void setupCamera() {
    glLoadIdentity();
    float camXDir = camX + sin(camYaw);
    float camYDir = camY + sin(camPitch);
    float camZDir = camZ - cos(camYaw);
    gluLookAt(camX, camY, camZ, camXDir, camYDir, camZDir, 0.0f, 1.0f, 0.0f);
}

void drawFloor() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, floorTexture);

    glColor3f(1.0f, 1.0f, 1.0f); // Reset color to white for texture
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-floorSize / 2, 0.0f, -floorSize / 2);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(floorSize / 2, 0.0f, -floorSize / 2);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(floorSize / 2, 0.0f, floorSize / 2);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-floorSize / 2, 0.0f, floorSize / 2);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void drawCrosshair() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 600, 0, 600);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glColor3f(0.0f, 1.0f, 0.0f);
    glLineWidth((GLfloat)3.0f);
    glBegin(GL_LINES);
    glVertex2f(295, 300);
    glVertex2f(305, 300);
    glVertex2f(300, 295);
    glVertex2f(300, 305);
    glEnd();
    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void drawEnemies() {
    for (const auto& enemy : enemies) {
        if (enemy.alive) {
            glPushMatrix();
            glColor3f(1.0f, 0.0f, 0.0f);
            glTranslatef(enemy.x, enemy.y, enemy.z);
            glutSolidSphere(0.5f, 20, 20);
            glPopMatrix();
        }
    }
}
void drawLevelOverlay() {
    if (showLevelOverlay) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, 800, 0, 600);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_DEPTH_TEST); // Disable depth testing to draw on top
        glColor3f(1.0f, 1.0f, 0.0f); // White text

        // Render level text
        glRasterPos2i(350, 300); // Center of the screen
        std::string levelText = "Level " + std::to_string(currentLevel);
        for (char c : levelText) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }

        glEnable(GL_DEPTH_TEST);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }
}
void checkForNextLevel() {
    bool allEnemiesKilled = true;
    for (const auto& enemy : enemies) {
        if (enemy.alive) {
            allEnemiesKilled = false;
            break;
        }
    }

    if (allEnemiesKilled) {
        // Prepare for the next level
        currentLevel++;
        enemiesToSpawn += 3; // Increase enemies for the next level
        showLevelOverlay = true; // Show overlay
        levelOverlayTimer = 120; // Display overlay for 2 seconds (60 FPS * 2)

        // Clear existing enemies and spawn new ones
        enemies.clear();
        for (int i = 0; i < enemiesToSpawn; ++i) {
            enemies.push_back({
                randomFloat(-floorSize / 2, floorSize / 2), // Initial X position
                0.5f,                                       // Fixed Y position
                randomFloat(-floorSize / 2, floorSize / 2), // Initial Z position
                randomFloat(-1.0f, 1.0f),                   // Random initial X direction
                randomFloat(-1.0f, 1.0f),                   // Random initial Z direction
                randomFloat(0.02f, 0.05f),                  // Random speed
                true,                                       // Alive
                rand() % 120                                // Random timer to change direction
            });
        }
    }
}

void handleKeyboard(unsigned char key, int x, int y) {
    float speed = movementSpeed;
    switch (key) {
        case 'w': // Move forward
            camX += speed * sin(camYaw);
            camZ -= speed * cos(camYaw);
            break;
        case 's': // Move backward
            camX -= speed * sin(camYaw);
            camZ += speed * cos(camYaw);
            break;
        case 'a': // Strafe left
            camX -= speed * cos(camYaw);
            camZ -= speed * sin(camYaw);
            break;
        case 'd': // Strafe right
            camX += speed * cos(camYaw);
            camZ += speed * sin(camYaw);
            break;
        case 'f': // Fire
            fire();
            break;
        case ' ': // Jump
            if (!isJumping) {
                playerVelocityY = jumpForce; // Apply upward velocity
                isJumping = true;           // Prevent double jump
            }
            break;
    }
    glutPostRedisplay();
}



void handleMouse(int x, int y) {
    static bool warpPointer = false;
    if (warpPointer) {
        warpPointer = false;
        return;
    }

    int centerX = 400, centerY = 300;
    int dx = x - centerX;
    int dy = y - centerY;

    camYaw += dx * 0.002f;
    camPitch -= dy * 0.002f;

    if (camPitch > 1.5f) camPitch = 1.5f;
    if (camPitch < -1.5f) camPitch = -1.5f;

    warpPointer = true;
    glutWarpPointer(centerX, centerY);
    glutPostRedisplay();
}
void drawBulletLine() {
    if (bulletVisible) {
        glPushMatrix(); // Save the current matrix state
        glColor3f(1.0f, 1.0f, 0.0f); // Yellow bullet line
        glBegin(GL_LINES);
        glVertex3f(bulletStartX, bulletStartY, bulletStartZ);
        glVertex3f(bulletEndX, bulletEndY, bulletEndZ);
        glEnd();
        glPopMatrix(); // Restore the previous matrix state
    }
}


void fire() {
    // Direction vector of the camera
    float camXDir = sin(camYaw) * cos(camPitch);
    float camYDir = sin(camPitch);
    float camZDir = -cos(camYaw) * cos(camPitch);

    // Set the bullet line positions
    bulletStartX = camX;
    bulletStartY = camY;
    bulletStartZ = camZ;

    // Extend the line far into the distance
    bulletEndX = camX + camXDir * 100.0f;
    bulletEndY = camY + camYDir * 100.0f;
    bulletEndZ = camZ + camZDir * 100.0f;

    bulletVisible = true;
    bulletTimer = 3;  // Show the bullet line for a short duration

     // Debugging log
    //std::cout<<"Bullet Start: "<<bulletStartX<<" "<<bulletStartY<<" "<<bulletStartZ<<"\n";
    //std::cout<<"Bullet End: "<<bulletEndX<<" "<<bulletEndY<<" "<<bulletEndZ<<"\n";

    Enemy* closestEnemy = nullptr;
    float closestDistance = FLT_MAX;

    for (auto& enemy : enemies) {
        if (!enemy.alive) continue;

        // Vector from camera to enemy
        float dx = enemy.x - camX;
        float dy = enemy.y - camY;
        float dz = enemy.z - camZ;

        // Project vector onto the camera's direction
        float dotProduct = dx * camXDir + dy * camYDir + dz * camZDir;

        // Ensure the enemy is in front of the player
        if (dotProduct <= 0) continue;

        // Calculate perpendicular distance from the ray to the enemy
        float distanceSquared = dx * dx + dy * dy + dz * dz - dotProduct * dotProduct;

        // Assume enemies are spheres with a radius of 0.5 units
        if (distanceSquared <= 0.25f) { // 0.5^2 = 0.25
            float distance = sqrt(dx * dx + dy * dy + dz * dz);
            if (distance < closestDistance) {
                closestDistance = distance;
                closestEnemy = &enemy;
            }
        }
    }

    // If a closest enemy was found, mark it as dead
    if (closestEnemy) {
        closestEnemy->alive = false;
    }
}


/*
void updateEnemies() {
    for (auto& enemy : enemies) {
        if (!enemy.alive) continue;

        enemy.x += randomFloat(-0.05f, 0.05f);
        enemy.z += randomFloat(-0.05f, 0.05f);

        // Keep enemies within bounds
        if (enemy.x < -floorSize / 2) enemy.x = -floorSize / 2;
        if (enemy.x > floorSize / 2) enemy.x = floorSize / 2;
        if (enemy.z < -floorSize / 2) enemy.z = -floorSize / 2;
        if (enemy.z > floorSize / 2) enemy.z = floorSize / 2;
    }
    glutPostRedisplay();
}
*/
void updateEnemies() {
    for (auto& enemy : enemies) {
        if (!enemy.alive) continue;

        // Calculate distance to the player
        float dx = camX - enemy.x;
        float dz = camZ - enemy.z;
        float distanceToPlayer = sqrt(dx * dx + dz * dz);

        // If enemy is too close to the player, move away
        if (distanceToPlayer < minDistanceToPlayer) {
            // Move away from the player
            float length = sqrt(dx * dx + dz * dz);
            if (length > 0.0f) {
                enemy.dx = -dx / length; // Reverse direction away from the player
                enemy.dz = -dz / length;
            }
        } else {
            // Regular random movement or chasing logic
            if (rand() % 100 < 5) { // 5% chance to chase the player
                float length = sqrt(dx * dx + dz * dz);
                if (length > 0.0f) {
                    enemy.dx = dx / length; // Move toward the player
                    enemy.dz = dz / length;
                }
            }
        }

        // Update position based on direction and speed
        enemy.x += enemy.dx * enemy.speed;
        enemy.z += enemy.dz * enemy.speed;

        // Keep enemies within bounds and adjust direction if they hit boundaries
        if (enemy.x < -floorSize / 2 || enemy.x > floorSize / 2) {
            enemy.dx = -enemy.dx; // Reverse direction
        }
        if (enemy.z < -floorSize / 2 || enemy.z > floorSize / 2) {
            enemy.dz = -enemy.dz; // Reverse direction
        }

        // Occasionally change direction randomly
        if (--enemy.changeDirectionTimer <= 0) {
            enemy.dx = randomFloat(-1.0f, 1.0f);
            enemy.dz = randomFloat(-1.0f, 1.0f);
            enemy.changeDirectionTimer = rand() % 120 + 60; // New timer (1-2 seconds)
        }
    }
    glutPostRedisplay();
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    setupCamera();         // Camera reflects updated `camY`
    drawFloor();
    drawEnemies();
    drawCrosshair();
    drawBulletLine();
    drawLevelOverlay();    // Draw level overlay if active
    glutSwapBuffers();
}




void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / height, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}
void timer(int value) {
    // Handle player jumping mechanics
    if (isJumping) {
        camY += playerVelocityY;         // Update vertical position
        playerVelocityY += gravity;      // Apply gravity to velocity

        // Check for landing
        if (camY <= 2.0f) {              // Ground level is 2.0
            camY = 2.0f;                 // Reset to ground level
            playerVelocityY = 0.0f;      // Reset velocity
            isJumping = false;           // Allow jumping again
        }
    }

    // Handle level overlay timer
    if (showLevelOverlay) {
        if (--levelOverlayTimer <= 0) {
            showLevelOverlay = false; // Hide overlay after timer ends
        }
    }

    // Update game objects
    updateEnemies();
    checkForNextLevel();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // 60 FPS
}


// load texture

GLuint loadTexture(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        exit(1);
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);


    stbi_image_free(data);

    return textureID;
}
void loadTextures() {
    floorTexture = loadTexture("floor.jpg"); // Replace with your texture file
}


//lighting
void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightPos[] = { 0.0f, 10.0f, 0.0f, 1.0f }; // Position of light
    GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    glEnable(GL_COLOR_MATERIAL); // Allow objects to reflect light
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("FPS Interactive World");

    glEnable(GL_DEPTH_TEST);

    // Setup game features
    loadTextures();
    setupLighting();


    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutPassiveMotionFunc(handleMouse);
    glutKeyboardFunc(handleKeyboard);
    glutTimerFunc(0, timer, 0);

    glutSetCursor(GLUT_CURSOR_NONE); // Hide cursor
    generateEnemies();
    glClearColor(0.2f,0.2f,0.2f, 1.0f);
    glutMainLoop();
    return 0;
}



