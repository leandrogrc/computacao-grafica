#include "graphics/menu.h"
#include "graphics/ui_text.h"
#include "graphics/shader.h"
#include "graphics/menu_melt.h"

#include "core/game.h"
#include "utils/assets.h"

#include <GL/glew.h>
#include <GL/glut.h>

#include <cmath>

// Lógica de Renderização de Tela derretendo "Melt" extraída para menu_melt.cpp

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

    // ---- 1. FUNDO DO MENU ----
    drawTexturedFullscreen(screenW, screenH, a.texMenuBG);

    // ---- 2. VINHETA (bordas escuras suaves) ----
    float bv = 160.0f; // largura da borda vertical
    float bh = 120.0f; // largura da borda horizontal
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);

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

    // ---- 3. LINHA DECORATIVA SUPERIOR E INFERIOR (Ciano) ----
    glLineWidth(2.0f);
    float lineY1 = (float)screenH * 0.72f;
    float lineY2 = (float)screenH * 0.32f;
    float margin = (float)screenW * 0.08f;
    float linePulse = 0.55f + 0.45f * std::sin(tempo * 2.0f);
    glColor4f(0.0f, 0.8f, 1.0f, linePulse); // Ciano
    glBegin(GL_LINES);
    glVertex2f(margin, lineY1);       glVertex2f(screenW - margin, lineY1);
    glVertex2f(margin, lineY2);       glVertex2f(screenW - margin, lineY2);
    glEnd();

    // ---- 4. TÍTULO DO JOGO (tela principal) ou título passado ----
    const std::string displayTitle = title.empty() ? "ABYSSAL PROTOCOL" : title;
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

    // glow suave atrás (magenta)
    drawTitle( 0,  0, 0.8f, 0.0f, 1.0f, 0.25f * glow, 16.0f);
    // sombra preta
    drawTitle( 8, -8, 0.0f, 0.0f, 0.0f, 0.9f,          6.0f);
    drawTitle( 5, -5, 0.0f, 0.0f, 0.0f, 0.8f,          6.0f);
    // contorno ciano
    drawTitle( 2, -2, 0.0f, 1.0f, 1.0f, 1.0f,          5.0f);
    // texto principal branco/azulado
    drawTitle( 0,  0, 0.9f, 0.95f, 1.0f, 1.0f,         3.5f);

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

    // Texto piscante magenta/ciano claro
    glLineWidth(3.0f);
    if ((int)(tempo * 2.5f) % 2 == 0)
        glColor4f(1.0f, 0.0f, 1.0f, 1.0f); // Magenta
    else
        glColor4f(0.8f, 1.0f, 1.0f, 1.0f); // Ciano claro
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

    // ---- 9. Disparo do efeito melt ----
    if (menuMeltCheckStart(screenW, screenH))
    {
        gameSetState(GameState::STORY_NARRATIVE);
    }

    glDisable(GL_BLEND);
    end2D();
    glPopAttrib();
}

void storyNarrativeRender(int screenW, int screenH, float tempo, GLuint bgTex)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);

    begin2D(screenW, screenH);

    // 1) Escurece bastante a arte de fundo com um blend preto
    glColor3f(0.2f, 0.2f, 0.2f);
    drawTexturedFullscreen(screenW, screenH, bgTex);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Vinheta extra escura
    glColor4f(0, 0, 0, 0.85f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(screenW, 0); glVertex2f(screenW, screenH); glVertex2f(0, screenH);
    glEnd();

    // 2) Textos de Narrativa
    std::string lines[] = {
        "O Protocolo falhou.",
        "Um nevoeiro abissal engoliu a cidade,",
        "trazendo horrores das profundezas.",
        "",
        "Voce e o ultimo sobrevivente do esquadrao.",
        "Colete 3 Cartoes de Acesso em cada setor para avancar.",
        "Alcance as profundezas e extermine o Boss final."
    };

    float scale = 0.25f;
    float startY = screenH - 180.0f;
    float lineSpacing = 45.0f;

    for (int i = 0; i < 7; ++i) {
        if (lines[i].empty()) continue;
        
        float w = uiStrokeTextWidthScaled(lines[i].c_str(), scale);
        float lineX = (screenW - w) / 2.0f;
        float lineY = startY - (i * lineSpacing);

        // Sombras vermelhas borradas e core text em branco/cinza
        glLineWidth(5.0f);
        glColor4f(0.6f, 0.0f, 0.0f, 0.7f);
        uiDrawStrokeText(lineX + 3, lineY - 3, lines[i].c_str(), scale);

        glLineWidth(2.5f);
        glColor4f(0.9f, 0.9f, 0.9f, 1.0f);
        uiDrawStrokeText(lineX, lineY, lines[i].c_str(), scale);
    }

    // 3) Push to continue blinking text
    float fPulse = 0.3f + 0.7f * std::abs(std::sin(tempo * 2.0f));
    const char* hint = "Pressione ENTER para Descer";
    float hw = uiStrokeTextWidthScaled(hint, 0.20f);
    
    glLineWidth(2.0f);
    glColor4f(1.0f, 1.0f, 1.0f, fPulse);
    uiDrawStrokeText((screenW - hw) / 2.0f, 80.0f, hint, 0.20f);

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
