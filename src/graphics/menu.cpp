#include "graphics/menu.h"
#include "graphics/ui_text.h"
#include "graphics/shader.h"

#include "core/game.h"
#include "utils/assets.h"

#include <GL/glew.h>
#include <GL/glut.h>

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
    // então invertimos aqui como você fez no fundo do menu
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

static void begin2D(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

static void end2D()
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
        g_melt.offsets[i] = frand(1.0f, 2.0f); // offsets base (igual Godot-ish)

    g_melt.timer = 0.0f;
    g_melt.active = true;

    // capture após desenhar o menu
    meltCaptureFramebuffer(w, h);
}

void menuMeltRenderOverlay(int screenW, int screenH, float tempo)
{
    if (!g_drawMeltOverlay)
        return;

    // se não está ativo, overlay acabou
    if (!g_melt.active)
    {
        g_drawMeltOverlay = false;
        return;
    }

    // desenha por cima de qualquer render (jogo)
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    begin2D(screenW, screenH);

    meltUpdateFromTempo(tempo);

    // precisa do discard aparecer (buracos): não precisa blend,
    // mas se quiser alpha, habilite blend
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    meltRenderFullscreen(screenW, screenH);

    glDisable(GL_BLEND);

    end2D();
    glPopAttrib();
}

// funções públicas pro resto do jogo chamar
void menuMeltRequestStart()
{
    g_melt.requested = true;
    g_drawMeltOverlay = true;
}

bool menuMeltIsActive() { return g_melt.active; }

static void drawTexturedFullscreen(int w, int h, GLuint texHandle)
{
    if (texHandle == 0)
        return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texHandle);

    glColor4f(1, 1, 1, 1);
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
}

void menuRender(int screenW, int screenH, float tempo,
                const std::string &title, const std::string &subTitle, const RenderAssets &a)
{
    (void)a; // background procedural, não usa textura

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);

    begin2D(screenW, screenH);

    // ---- 1. FUNDO PRETO ----
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glColor4f(0.03f, 0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f((float)screenW, 0);
    glVertex2f((float)screenW, (float)screenH);
    glVertex2f(0, (float)screenH);
    glEnd();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ---- 2. GLOW RADIAL CENTRAL (inferno) ----
    float cx = screenW * 0.5f;
    float cy = screenH * 0.5f;
    float pulse = 0.5f + 0.5f * std::sin(tempo * 1.4f);
    float r1 = screenW * 0.55f;
    float r2 = screenW * 0.9f;

    // inner glow (quente laranja-vermelho)
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.55f, 0.08f, 0.0f, 0.30f + 0.12f * pulse);
    glVertex2f(cx, cy);
    glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i <= 48; i++) {
        float ang = i * 2.0f * 3.14159f / 48.0f;
        glVertex2f(cx + std::cos(ang) * r1, cy + std::sin(ang) * r1);
    }
    glEnd();

    // outer halo (vermelho escuro)
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.30f, 0.0f, 0.0f, 0.14f + 0.08f * pulse);
    glVertex2f(cx, cy);
    glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i <= 48; i++) {
        float ang = i * 2.0f * 3.14159f / 48.0f;
        glVertex2f(cx + std::cos(ang) * r2, cy + std::sin(ang) * r2);
    }
    glEnd();

    // ---- 3. SCANLINES ANIMADAS (CRT retro) ----
    float scanOff = std::fmod(tempo * 25.0f, 4.0f);
    glColor4f(0.0f, 0.0f, 0.0f, 0.20f);
    glBegin(GL_QUADS);
    for (float y = -scanOff; y < screenH + 4.0f; y += 4.0f) {
        glVertex2f(0, y);
        glVertex2f((float)screenW, y);
        glVertex2f((float)screenW, y + 2.0f);
        glVertex2f(0, y + 2.0f);
    }
    glEnd();

    // ---- 4. VINHETA (bordas escuras) ----
    float bv = 160.0f; // largura da borda vertical
    float bh = 120.0f; // largura da borda horizontal
    glBegin(GL_QUADS);
    // topo
    glColor4f(0,0,0,0.75f); glVertex2f(0,(float)screenH); glVertex2f((float)screenW,(float)screenH);
    glColor4f(0,0,0,0);     glVertex2f((float)screenW,(float)screenH-bh); glVertex2f(0,(float)screenH-bh);
    // baixo
    glColor4f(0,0,0,0.75f); glVertex2f(0,0); glVertex2f((float)screenW,0);
    glColor4f(0,0,0,0);     glVertex2f((float)screenW,bh); glVertex2f(0,bh);
    // esquerda
    glColor4f(0,0,0,0.65f); glVertex2f(0,0); glVertex2f(0,(float)screenH);
    glColor4f(0,0,0,0);     glVertex2f(bv,(float)screenH); glVertex2f(bv,0);
    // direita
    glColor4f(0,0,0,0.65f); glVertex2f((float)screenW,0); glVertex2f((float)screenW,(float)screenH);
    glColor4f(0,0,0,0);     glVertex2f((float)screenW-bv,(float)screenH); glVertex2f((float)screenW-bv,0);
    glEnd();

    // ---- 5. LINHA DECORATIVA SUPERIOR E INFERIOR ----
    glLineWidth(2.0f);
    float lineY1 = (float)screenH * 0.72f;
    float lineY2 = (float)screenH * 0.32f;
    float margin = (float)screenW * 0.08f;
    float linePulse = 0.55f + 0.45f * std::sin(tempo * 2.0f);
    glColor4f(0.8f, 0.15f, 0.0f, linePulse);
    glBegin(GL_LINES);
    glVertex2f(margin, lineY1);       glVertex2f(screenW - margin, lineY1);
    glVertex2f(margin, lineY2);       glVertex2f(screenW - margin, lineY2);
    glEnd();

    // ---- 6. TÍTULO DO JOGO (tela principal) ou título passado ----
    const std::string displayTitle = title.empty() ? "DOOM-CG" : title;
    float scaleX = 1.4f;
    float scaleY = 1.4f;

    float rawW = 0.0f;
    for (char c : displayTitle)
        rawW += glutStrokeWidth(GLUT_STROKE_ROMAN, c);

    float titleW = rawW * scaleX;
    float xBase  = (screenW - titleW) / 2.0f;
    float yBase  = (screenH / 2.0f) + 60.0f;

    // camadas de sombra/glow:
    auto drawTitle = [&](float ox, float oy, float r, float g, float b, float a, float lw)
    {
        glLineWidth(lw);
        glColor4f(r, g, b, a);
        glPushMatrix();
        glTranslatef(xBase + ox, yBase + oy, 0);
        glScalef(scaleX, scaleY, 1);
        for (char c : displayTitle)
            glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
        glPopMatrix();
    };

    float glow = 0.7f + 0.3f * std::sin(tempo * 3.0f);

    // glow suave atrás (vermelho)
    drawTitle( 0,  0, 1.0f, 0.05f, 0.0f, 0.18f * glow, 16.0f);
    // sombra preta
    drawTitle( 8, -8, 0.0f, 0.0f,  0.0f, 0.9f,          6.0f);
    drawTitle( 5, -5, 0.0f, 0.0f,  0.0f, 0.8f,          6.0f);
    // contorno escarlate
    drawTitle( 2, -2, 0.6f, 0.0f,  0.0f, 1.0f,          5.0f);
    // texto principal branco-amarelado
    drawTitle( 0,  0, 1.0f, 0.92f, 0.70f, 1.0f,         3.5f);

    // ---- 7. SUBTÍTULO animado ----
    float scaleSub = 0.22f;

    float subW = 0.0f;
    for (char c : subTitle)
        subW += glutStrokeWidth(GLUT_STROKE_ROMAN, c);
    subW *= scaleSub;

    float xSub = (screenW - subW) / 2.0f;
    float ySub = (screenH / 2.0f) - 100.0f;

    // Contorno escuro do subtítulo
    glLineWidth(4.0f);
    glColor4f(0.0f, 0.0f, 0.0f, 0.9f);
    glPushMatrix();
    glTranslatef(xSub + 2.0f, ySub - 2.0f, 0);
    glScalef(scaleSub, scaleSub, 1);
    for (char c : subTitle)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
    glPopMatrix();

    // Texto piscante branco/amarelo
    glLineWidth(3.0f);
    if ((int)(tempo * 2.5f) % 2 == 0)
        glColor4f(1.0f, 0.96f, 0.40f, 1.0f);
    else
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(xSub, ySub, 0);
    glScalef(scaleSub, scaleSub, 1);
    for (char c : subTitle)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
    glPopMatrix();

    // ---- 8. VERSÃO / crédito pequeno ----
    glDisable(GL_BLEND);
    glEnable(GL_BLEND);
    {
        const char* ver = "CG  2025";
        float sv = 0.10f;
        float vw = 0.0f;
        for (const char* p = ver; *p; p++)
            vw += glutStrokeWidth(GLUT_STROKE_ROMAN, *p);
        vw *= sv;
        glLineWidth(1.5f);
        glColor4f(0.5f, 0.1f, 0.1f, 0.7f);
        glPushMatrix();
        glTranslatef(screenW - vw - 18.0f, 14.0f, 0);
        glScalef(sv, sv, 1);
        for (const char* p = ver; *p; p++)
            glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
        glPopMatrix();
    }

    // ---- 9. Disparo do efeito melt (igual ao original) ----
    if (g_melt.requested && !g_melt.active)
    {
        g_melt.requested = false;
        meltStartNow(screenW, screenH);
        g_melt.lastTempo = -1.0f;
        gameSetState(GameState::JOGANDO);
    }

    glDisable(GL_BLEND);
    end2D();
    glPopAttrib();
}


void pauseMenuRender(int screenW, int screenH, float tempo)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);

    begin2D(screenW, screenH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 1) filtro avermelhado (em vez de preto)
    glColor4f(0.35f, 0.00f, 0.00f, 0.35f); // vermelho escuro transparente
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f((float)screenW, 0);
    glVertex2f((float)screenW, (float)screenH);
    glVertex2f(0, (float)screenH);
    glEnd();

    // 2) vinheta (bordas mais escuras)
    float m = 70.0f; // largura da borda
    glBegin(GL_QUADS);

    // topo
    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
    glVertex2f(0, (float)screenH);
    glVertex2f((float)screenW, (float)screenH);
    glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
    glVertex2f((float)screenW, (float)screenH - m);
    glVertex2f(0, (float)screenH - m);

    // baixo
    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
    glVertex2f(0, 0);
    glVertex2f((float)screenW, 0);
    glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
    glVertex2f((float)screenW, m);
    glVertex2f(0, m);

    // esquerda
    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
    glVertex2f(0, 0);
    glVertex2f(0, (float)screenH);
    glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
    glVertex2f(m, (float)screenH);
    glVertex2f(m, 0);

    // direita
    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
    glVertex2f((float)screenW, 0);
    glVertex2f((float)screenW, (float)screenH);
    glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
    glVertex2f((float)screenW - m, (float)screenH);
    glVertex2f((float)screenW - m, 0);

    glEnd();

    // 3) título
    const char *t = "PAUSADO";
    float scale = 0.6f;
    float w = uiStrokeTextWidthScaled(t, scale);
    float x = (screenW - w) / 2.0f;
    float y = (screenH / 2.0f) + 20.0f;

    // glow pulsante vermelho atrás
    float pulse = 0.15f + 0.10f * (0.5f + 0.5f * std::sin(tempo * 6.0f)); // 0.15..0.25
    glLineWidth(8.0f);
    glColor4f(1.0f, 0.15f, 0.15f, pulse);
    uiDrawStrokeText(x + 2, y - 2, t, scale);
    uiDrawStrokeText(x - 2, y + 2, t, scale);

    // contorno preto forte
    glLineWidth(6.0f);
    glColor3f(0, 0, 0);
    uiDrawStrokeText(x + 3, y - 3, t, scale);

    // texto principal branco
    glLineWidth(4.0f);
    glColor3f(1, 1, 1);
    uiDrawStrokeText(x, y, t, scale);

    // 4) subtítulo
    const char *sub = "Pressione P para Voltar";
    float scaleSub = 0.22f;
    float wSub = uiStrokeTextWidthScaled(sub, scaleSub);
    float xSub = (screenW - wSub) / 2.0f;
    float ySub = (screenH / 2.0f) - 60.0f;

    if ((int)(tempo * 3) % 2 == 0)
        glColor3f(1, 1, 0);
    else
        glColor3f(1, 1, 1);

    glLineWidth(3.0f);
    uiDrawStrokeText(xSub, ySub, sub, scaleSub);

    glDisable(GL_BLEND);

    end2D();
    glPopAttrib();
}
