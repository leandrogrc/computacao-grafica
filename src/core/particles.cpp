#include "core/particles.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <cstdlib>

static const int MAX_PARTICLES = 300;
static std::vector<Particle> gParticles;

// Retorna float aleatório entre min e max
static float randF(float min, float max) {
    float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    return min + r * (max - min);
}

void initParticles() {
    gParticles.clear();
    gParticles.resize(MAX_PARTICLES);
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        gParticles[i].active = false;
    }
}

void spawnParticles(int count, float x, float y, float z, ParticleType type) {
    int spawned = 0;
    for (int i = 0; i < MAX_PARTICLES && spawned < count; ++i) {
        if (!gParticles[i].active) {
            Particle& p = gParticles[i];
            p.active = true;
            p.type = type;
            
            p.x = x;
            p.y = y;
            p.z = z;
            
            p.life = randF(0.4f, 0.8f);
            p.maxLife = p.life;
            p.size = randF(0.02f, 0.05f); // Particulas miúdas

            // Cone point-blank 360 burst direction
            float angle = randF(0.0f, 3.1415f * 2.0f);
            float speedXZ = randF(1.0f, 3.0f);
            
            p.vx = std::cos(angle) * speedXZ;
            p.vz = std::sin(angle) * speedXZ;
            
            // Sangue "espirra" mais reto pros lados e cai com o peso
            p.vy = randF(0.0f, 1.5f); 
            p.r = randF(0.6f, 1.0f); // Tons vermelhos
            p.g = 0.0f;
            p.b = 0.0f;

            p.a = 1.0f;
            
            spawned++;
        }
    }
}

void updateParticles(float dt) {
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        Particle& p = gParticles[i];
        if (!p.active) continue;

        p.life -= dt;
        if (p.life <= 0.0f) {
            p.active = false;
            continue;
        }

        p.x += p.vx * dt;
        p.z += p.vz * dt;
        p.y += p.vy * dt;

        // Gravidade densa puxando sangue pro chão
        p.vy -= 9.8f * dt;
        // Para no chão (+- eps_y)
        if (p.y < 0.01f) {
            p.y = 0.01f;
            p.vy = 0.0f;
            p.vx = 0.0f;
            p.vz = 0.0f;
        }

        // Fade out
        p.a = p.life / p.maxLife;
    }
}

void drawParticles(float camX, float camY, float camZ) {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Depth read-only (não obscurecer umas nas outras artificialmente)
    glDepthMask(GL_FALSE);  

    for (int i = 0; i < MAX_PARTICLES; ++i) {
        const Particle& p = gParticles[i];
        if (!p.active) continue;

        glPushMatrix();
        glTranslatef(p.x, p.y, p.z);
        
        // Billboarding individual simples (Vira para os eixos centrais da câmera)
        float ddx = camX - p.x;
        float ddz = camZ - p.z;
        float angle = std::atan2(ddx, ddz) * 180.0f / 3.14159f;
        glRotatef(angle, 0.0f, 1.0f, 0.0f);

        glColor4f(p.r, p.g, p.b, p.a);
        
        float hw = p.size;
        
        glBegin(GL_QUADS);
        glNormal3f(0, 0, 1);
        glVertex3f(-hw, -hw, 0.0f);
        glVertex3f( hw, -hw, 0.0f);
        glVertex3f( hw,  hw, 0.0f);
        glVertex3f(-hw,  hw, 0.0f);
        glEnd();

        glPopMatrix();
    }
    
    glDepthMask(GL_TRUE); // Reinstitui o Z-Buffer check normal
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
}
