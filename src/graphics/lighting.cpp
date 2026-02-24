#include "graphics/lighting.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>

void setupIndoorLightOnce()
{
    glEnable(GL_LIGHT1);
    GLfloat lampDiffuse[]  = {2.0f, 2.0f, 2.0f, 1.0f};
    GLfloat lampSpecular[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat lampAmbient[]  = {1.5f, 1.5f, 1.5f, 1.0f};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lampDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lampSpecular);
    glLightfv(GL_LIGHT1, GL_AMBIENT, lampAmbient);
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.6f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.06f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.02f);
    glDisable(GL_LIGHT1);
}

void setupSunLightOnce()
{
    glEnable(GL_LIGHT0);
    GLfloat sceneAmbient[] = {0.45f, 0.30f, 0.25f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, sceneAmbient);
    GLfloat sunDiffuse[] = {1.4f, 0.55f, 0.30f, 1.0f};
    GLfloat sunSpecular[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sunDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, sunSpecular);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void setSunDirectionEachFrame()
{
    GLfloat sunDir[] = {0.3f, 1.0f, 0.2f, 0.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, sunDir);
}

// Aplica iluminação distinta por fase
void applyLevelLighting(int level, float time)
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHT2);

    if (level == 1)
    {
        // Fase 1: Cidade — Clara e visível
        GLfloat amb[] = {0.4f, 0.4f, 0.4f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
        GLfloat diff[] = {0.8f, 0.8f, 0.8f, 1.0f};
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
    }
    else if (level == 2)
    {
        // Fase 2: Subterrâneo — Agora bem iluminado
        GLfloat amb[] = {0.35f, 0.4f, 0.35f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
        GLfloat diff[] = {0.7f, 0.8f, 0.7f, 1.0f};
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
        GLfloat dir[] = {0.0f, -1.0f, 0.0f, 0.0f};
        glLightfv(GL_LIGHT0, GL_POSITION, dir);
    }
    else if (level == 3)
    {
        // Fase 3: Inferno — Vermelho vibrante mas visível
        float pulse = 0.5f + 0.5f * std::sin(time * 1.5f);
        float r = 0.8f + 0.2f * pulse;
        GLfloat amb[] = {r * 0.6f, 0.3f, 0.3f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
        GLfloat diff[] = {r * 1.2f, 0.6f, 0.6f, 1.0f};
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
        GLfloat dir[] = {0.0f, 1.0f, 0.0f, 0.0f};
        glLightfv(GL_LIGHT0, GL_POSITION, dir);
    }
}
