#include "graphics/draw_entities.h"
#include "graphics/drawlevel.h"
#include "core/game.h"
#include "utils/utils.h"
#include "core/game_state.h" // RenderAssets
#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>

/**
 * Desenha uma geometria retangular (Quad) texturizada virada para a câmera (Billboard).
 * É a base para colocar Inimigos, Itens e a vegetação/elementos 2D no mundo 3D simulado.
 */
void drawSprite(float x, float z, float w, float h, GLuint tex, float camX, float camZ)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);

    glBindTexture(GL_TEXTURE_2D, tex);
    glColor3f(1, 1, 1);

    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    float ddx = camX - x;
    float ddz = camZ - z;
    float angle = std::atan2(ddx, ddz) * 180.0f / 3.14159f;

    glRotatef(angle, 0.0f, 1.0f, 0.0f);

    float hw = w * 0.5f;

    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);

    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-hw, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(hw, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(hw, h, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-hw, h, 0.0f);
    glEnd();

    glPopMatrix();

    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
}

/**
 * Renderiza os feixes de ataque disparados por inimigos. 
 * Implementa um Additive Blending para criar um efeito incandescente "Neon" e usa cruzamentos 
 * de polígonos para simular volume ao invés de ser só uma folha de papel.
 */
void drawProjectiles(const std::vector<Projectile>& projectiles, float camX, float camZ, float time)
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for glow
    glDisable(GL_TEXTURE_2D);

    for (const auto& p : projectiles)
    {
        if (!p.active) continue;

        glPushMatrix();
        glTranslatef(p.x, 1.2f, p.z); // Altura do peito

        float pulse = 0.5f + 0.5f * std::sin(time * 10.0f);
        glColor4f(1.0f, 0.2f + 0.5f*pulse, 0.1f, 0.8f);

        // Billboard para garantir visibilidade de qualquer angulo
        float ddx = camX - p.x;
        float ddz = camZ - p.z;
        float angle = std::atan2(ddx, ddz) * 180.0f / 3.14159f;
        glRotatef(angle, 0.0f, 1.0f, 0.0f);

        float size = 0.18f;
        glBegin(GL_QUADS);
        // Quad principal billboarded
        glVertex3f(-size, -size, 0); glVertex3f(size, -size, 0);
        glVertex3f(size, size, 0); glVertex3f(-size, size, 0);
        // Cruzamento para volume
        glVertex3f(0, -size, -size); glVertex3f(0, -size, size);
        glVertex3f(0, size, size); glVertex3f(0, size, -size);
        glEnd();

        glPopMatrix();
    }

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

/**
 * Lógica principal para desenhar todas as criaturas e utilitários que habitam o cenário atual.
 * 
 * Itera pela lista de Itens verificando se estão visíveis (Viewing Frustum simples).
 * Depois itera pelos inimigos e escolhe qual frame de animação carregar na Sprite principal 
 * (Caminhando, Correndo ou Levando Dano), escalonando-os caso sejam o Chefão Final.
 */
void drawEntities(const std::vector<Enemy> &enemies, const std::vector<Item> &items,
                  float camX, float camZ, float dx, float dz, const RenderAssets &r)
{
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);

    float fwdx, fwdz;
    bool hasFwd = getForwardXZ(dx, dz, fwdx, fwdz);

    // --- ITENS ---
    for (const auto &item : items)
    {
        if (!item.active)
            continue;

        if (!isVisibleXZ(item.x, item.z, camX, camZ, hasFwd, fwdx, fwdz))
            continue;

        GLuint tex = 0;
        float rCol=1, gCol=1, bCol=1;

        if (item.type == ITEM_HEALTH) tex = r.texHealth;
        else if (item.type == ITEM_AMMO) tex = r.texAmmo;
        else if (item.type == ITEM_CARD) tex = r.texCard;
        else if (item.type == ITEM_BERSERK) tex = r.texBerserk;
        else if (item.type == ITEM_HASTE) tex = r.texHaste;
        else if (item.type == ITEM_WEAPON2) { tex = r.texGun2Default; rCol=0.4f; gCol=0.6f; bCol=1.0f; } // Blue tint

        if (tex != 0)
        {
            glColor3f(rCol, gCol, bCol);
            drawSprite(item.x, item.z, 0.4f, 0.4f, tex, camX, camZ);
            glColor3f(1, 1, 1);
        }
    }
    // --- INIMIGOS ---
    for (const auto &en : enemies)
    {
        if (en.state == STATE_DEAD)
            continue;

        if (!isVisibleXZ(en.x, en.z, camX, camZ, hasFwd, fwdx, fwdz))
            continue;

        int t = (en.type < 0 || en.type > 4) ? 0 : en.type;

        GLuint currentTex;
        if (en.hurtTimer > 0.0f)
            currentTex = r.texEnemiesDamage[t];
        else if (en.state == STATE_CHASE || en.state == STATE_ATTACK)
            currentTex = r.texEnemiesRage[t];
        else
            currentTex = r.texEnemies[t];

        float w = 2.5f;
        float h = 2.5f;

        if (en.isBoss) {
            w *= 1.8f;
            h *= 1.8f;
        }

        drawSprite(en.x, en.z, w, h, currentTex, camX, camZ);
    }

    // --- PROJÉTEIS ---
    drawProjectiles(gameLevel().projectiles, camX, camZ, gameContext().time);

    glDisable(GL_ALPHA_TEST);
}
