#include "graphics/hud_minimap.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>
#include "core/config.h"

// Função auxiliar para preparar o 2D (copiada pois é "static" no hud.cpp)
static void begin2D_MiniMap(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

static void end2D_MiniMap()
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
}

void drawMiniMap(int w, int h, const HudState& s, const MapLoader& map, const std::vector<Enemy>& enemies, const std::vector<Item>& items)
{
    begin2D_MiniMap(w, h);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float radarR = 75.0f;
    float margin = 25.0f;
    float radarX = w - radarR - margin;
    float radarY = h - radarR - margin - 80.0f; // Posicionado à direita, abaixo do topo

    // 1. Fundo circular (Holográfico)
    glColor4f(0.0f, 0.1f, 0.2f, 0.5f);
    glBegin(GL_POLYGON);
    for(int i=0; i<32; i++) {
        float a = i * 2.0f * 3.14159f / 32;
        glVertex2f(radarX + std::cos(a)*radarR, radarY + std::sin(a)*radarR);
    }
    glEnd();

    // 2. Borda
    glColor4f(0.0f, 1.0f, 1.0f, 0.4f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    for(int i=0; i<32; i++) {
        float a = i * 2.0f * 3.14159f / 32;
        glVertex2f(radarX + std::cos(a)*radarR, radarY + std::sin(a)*radarR);
    }
    glEnd();

    // 3. Desenho de Paredes e Entidades
    float TILE_WORLD = GameConfig::TILE_SIZE;
    int MW = map.getWidth();
    int MH = map.getHeight();
    float offX = -(MW * TILE_WORLD) * 0.5f;
    float offZ = -(MH * TILE_WORLD) * 0.5f;

    float ryaw = s.pyaw * 3.14159f / 180.0f;
    float cosA = std::cos(-ryaw);
    float sinA = std::sin(-ryaw);
    float zoom = 6.0f; // pixels por unidade de mundo

    // Clip e Desenha Pontos
    auto drawDot = [&](float wx, float wz, float r, float g, float b, float a, float size) {
        float dx = (wx - s.px) * zoom;
        float dz = (wz - s.pz) * zoom;
        
        // Rotação 2D
        float rx = dx * cosA - dz * sinA;
        float rz = dx * sinA + dz * cosA;

        float dist = std::sqrt(rx*rx + rz*rz);
        if (dist < radarR - 2.0f) {
            glPointSize(size);
            glColor4f(r, g, b, a);
            glBegin(GL_POINTS);
            glVertex2f(radarX + rx, radarY - rz);
            glEnd();
        }
    };

    // Paredes
    float wallS = zoom * 0.8f;
    for (int tz = 0; tz < MH; tz++) {
        for (int tx = 0; tx < MW; tx++) {
            char c = map.data()[tz][tx];
            bool isWall = (c == '1' || c == '2' || c == '3' || c == 'M' || c == 'H' || c == 'S' || c == 'O');
            if (isWall) {
                float wx = offX + (tx + 0.5f) * TILE_WORLD;
                float wz = offZ + (tz + 0.5f) * TILE_WORLD;
                
                float dx = (wx - s.px) * zoom;
                float dz = (wz - s.pz) * zoom;

                // Rotação 2D
                float rx = dx * cosA - dz * sinA;
                float rz = dx * sinA + dz * cosA;

                if (std::sqrt(rx*rx + rz*rz) < radarR - wallS) {
                    glColor4f(0.0f, 0.6f, 1.0f, 0.25f);
                    glBegin(GL_QUADS);
                    glVertex2f(radarX + rx - wallS/2, radarY - rz - wallS/2);
                    glVertex2f(radarX + rx + wallS/2, radarY - rz - wallS/2);
                    glVertex2f(radarX + rx + wallS/2, radarY - rz + wallS/2);
                    glVertex2f(radarX + rx - wallS/2, radarY - rz + wallS/2);
                    glEnd();
                }
            }
        }
    }

    // Entidades
    for (const auto& en : enemies) {
        if (en.state != STATE_DEAD)
            drawDot(en.x, en.z, 1.0f, 0.2f, 0.2f, 0.9f, 4.0f);
    }
    for (const auto& it : items) {
        if (it.active && it.type == ITEM_CARD)
            drawDot(it.x, it.z, 0.2f, 1.0f, 0.2f, 1.0f, 5.0f);
    }

    // 4. Jogador (Seta central - SEMPRE PARA CIMA)
    float pSize = 7.0f;
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(radarX, radarY + pSize); // Topo
    glVertex2f(radarX - pSize*0.6f, radarY - pSize*0.4f); // Base Esq
    glVertex2f(radarX + pSize*0.6f, radarY - pSize*0.4f); // Base Dir
    glEnd();

    glDisable(GL_BLEND);
    end2D_MiniMap();
}
