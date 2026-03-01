#include "graphics/menu_melt.h"
#include "graphics/shader.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <cstdlib>
#include <cmath>

struct MeltState
{
    bool requested = false;
    bool active = false;
    float timer = 0.0f;
    float speed = 0.5f;
    int xRes = 120;
    float maxOffset = 2.0f;

    GLuint prog = 0;
    GLuint texCapture = 0;
    int capW = 0, capH = 0;

    float offsets[256] = {0};

    float lastTempo = -1.0f;
};

static bool g_drawMeltOverlay = false;
static MeltState g_melt;

static float frand(float a, float b)
{
    return a + (b - a) * (float)std::rand() / (float)RAND_MAX;
}

static void meltEnsureShaderLoaded()
{
    if (g_melt.prog != 0)
        return;
    g_melt.prog = criaShader("shaders/melt.vert", "shaders/melt.frag");
}

static void meltEnsureCaptureTex(int w, int h)
{
    if (g_melt.texCapture == 0)
        glGenTextures(1, &g_melt.texCapture);

    if (g_melt.capW == w && g_melt.capH == h)
        return;

    g_melt.capW = w;
    g_melt.capH = h;

    glBindTexture(GL_TEXTURE_2D, g_melt.texCapture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void meltCaptureFramebuffer(int w, int h)
{
    meltEnsureCaptureTex(w, h);

    glBindTexture(GL_TEXTURE_2D, g_melt.texCapture);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, w, h);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void meltUpdateFromTempo(float tempo)
{
    if (g_melt.lastTempo < 0.0f)
        g_melt.lastTempo = tempo;

    float dt = tempo - g_melt.lastTempo;
    g_melt.lastTempo = tempo;

    if (dt < 0.0f)
        dt = 0.0f;
    if (dt > 0.1f)
        dt = 0.1f;

    if (!g_melt.active)
        return;

    g_melt.timer += dt * g_melt.speed;

    if (g_melt.timer > 1.2f)
        g_melt.active = false;
}

static void meltRenderFullscreen(int w, int h)
{
    if (!g_melt.active)
        return;
    if (g_melt.texCapture == 0)
        return;
    if (g_melt.prog == 0)
        return;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);

    glUseProgram(g_melt.prog);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_melt.texCapture);

    glUniform1i(glGetUniformLocation(g_melt.prog, "uTex"), 0);
    glUniform1f(glGetUniformLocation(g_melt.prog, "uTimer"), g_melt.timer);
    glUniform1i(glGetUniformLocation(g_melt.prog, "uXRes"), g_melt.xRes);
    glUniform1f(glGetUniformLocation(g_melt.prog, "uMaxOffset"), g_melt.maxOffset);

    GLint locOff = glGetUniformLocation(g_melt.prog, "uOffsets");
    glUniform1fv(locOff, g_melt.xRes, g_melt.offsets);

    glEnable(GL_TEXTURE_2D);
    glColor4f(1, 1, 1, 1);

    // IMPORTANT: captura via glCopyTexSubImage2D costuma vir invertida em Y
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1);
    glVertex2f(0, 0);
    glTexCoord2f(1, 1);
    glVertex2f((float)w, 0);
    glTexCoord2f(1, 0);
    glVertex2f((float)w, (float)h);
    glTexCoord2f(0, 0);
    glVertex2f(0, (float)h);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    glUseProgram(0);
}

static void begin2D_Melt(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

static void end2D_Melt()
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

static void meltStartNow(int w, int h)
{
    meltEnsureShaderLoaded();

    if (g_melt.xRes > 256)
        g_melt.xRes = 256;
    if (g_melt.xRes < 1)
        g_melt.xRes = 1;

    for (int i = 0; i < g_melt.xRes; ++i)
        g_melt.offsets[i] = frand(1.0f, 2.0f); // offsets base

    g_melt.timer = 0.0f;
    g_melt.active = true;

    meltCaptureFramebuffer(w, h);
}

void menuMeltRenderOverlay(int screenW, int screenH, float tempo)
{
    if (!g_drawMeltOverlay)
        return;

    if (!g_melt.active)
    {
        g_drawMeltOverlay = false;
        return;
    }

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    begin2D_Melt(screenW, screenH);

    meltUpdateFromTempo(tempo);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    meltRenderFullscreen(screenW, screenH);

    glDisable(GL_BLEND);

    end2D_Melt();
    glPopAttrib();
}

void menuMeltRequestStart()
{
    g_melt.requested = true;
    g_drawMeltOverlay = true;
}

bool menuMeltIsActive() { return g_melt.active; }

bool menuMeltCheckStart(int screenW, int screenH) {
    if (g_melt.requested && !g_melt.active) {
        g_melt.requested = false;
        meltStartNow(screenW, screenH);
        g_melt.lastTempo = -1.0f;
        return true; 
    }
    return false;
}
