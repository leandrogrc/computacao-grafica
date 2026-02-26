#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>
#include "graphics/drawlevel.h"
#include "core/game.h"
#include "level/levelmetrics.h"
#include "core/config.h"
#include "utils/utils.h"
#include <cstdio>

// =====================
// CONFIG / CONSTANTES
// =====================

// Config do grid
static const float TILE = GameConfig::TILE_SIZE;
static const float CEILING_H = GameConfig::CEILING_HEIGHT;
static const float WALL_H = GameConfig::WALL_HEIGHT;
static const float EPS_Y = 0.001f;   // evita z-fighting

static const GLfloat kAmbientOutdoor[] = {0.45f, 0.30f, 0.25f, 1.0f}; // quente (seu atual)
static const GLfloat kAmbientIndoor[] = {0.12f, 0.12f, 0.18f, 1.0f};  // frio/azulado

// ======================
// CONFIG ÚNICA DO CULLING (XZ)
// ======================
static float gCullHFovDeg = 170.0f;     // FOV horizontal do culling (cenário + entidades)
static float gCullNearTiles = 2.0f;     // dentro disso não faz culling angular
static float gCullMaxDistTiles = 20.0f; // 0 = sem limite; em tiles

// Retorna TRUE se deve renderizar o objeto no plano XZ (distância + cone de FOV)
// - Usa as configs globais gCull*
// - Usa forward já normalizado (fwdx,fwdz) e flag hasFwd
static inline bool isVisibleXZ(float objX, float objZ,
                               float camX, float camZ,
                               bool hasFwd, float fwdx, float fwdz)
{
    float vx = objX - camX;
    float vz = objZ - camZ;
    float distSq = vx * vx + vz * vz;

    // 0) Distância máxima (se habilitada)
    if (gCullMaxDistTiles > 0.0f)
    {
        float maxDist = gCullMaxDistTiles * TILE;
        float maxDistSq = maxDist * maxDist;
        if (distSq > maxDistSq)
            return false;
    }

    // 1) Dentro do near: não faz culling angular
    float nearDist = gCullNearTiles * TILE;
    float nearDistSq = nearDist * nearDist;
    if (distSq <= nearDistSq)
        return true;

    // 2) Sem forward válido: não faz culling angular
    if (!hasFwd)
        return true;

    // 3) Cone por FOV horizontal
    float cosHalf = std::cos(deg2rad(gCullHFovDeg * 0.5f));

    float invDist = 1.0f / std::sqrt(distSq);
    float nvx = vx * invDist;
    float nvz = vz * invDist;

    float dot = clampf(nvx * fwdx + nvz * fwdz, -1.0f, 1.0f);
    return dot >= cosHalf;
}

static void bindTexture0(GLuint tex)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
}



static void beginIndoor()
{
    glDisable(GL_LIGHT0);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, kAmbientIndoor);
    // Removed GL_LIGHT1 (indoor lamp) completely. 
}

static void endIndoor()
{
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, kAmbientOutdoor);
    glEnable(GL_LIGHT0);
}

static void desenhaQuadTeto(float x, float z, float tile, float tilesUV)
{
    float half = tile * 0.5f;

    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(x - half, CEILING_H, z - half);
    glTexCoord2f(tilesUV, 0.0f);
    glVertex3f(x + half, CEILING_H, z - half);
    glTexCoord2f(tilesUV, tilesUV);
    glVertex3f(x + half, CEILING_H, z + half);
    glTexCoord2f(0.0f, tilesUV);
    glVertex3f(x - half, CEILING_H, z + half);
    glEnd();
}

static void desenhaQuadChao(float x, float z, float tile, float tilesUV)
{
    float half = tile * 0.5f;

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(x - half, EPS_Y, z + half);
    glTexCoord2f(tilesUV, 0.0f);
    glVertex3f(x + half, EPS_Y, z + half);
    glTexCoord2f(tilesUV, tilesUV);
    glVertex3f(x + half, EPS_Y, z - half);
    glTexCoord2f(0.0f, tilesUV);
    glVertex3f(x - half, EPS_Y, z - half);
    glEnd();
}

static void desenhaTileChao(float x, float z, GLuint texChaoX, bool temTeto)
{
    glUseProgram(0);
    glColor3f(1, 1, 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texChaoX);

    desenhaQuadChao(x, z, TILE, 2.0f);

    if (temTeto)
    {
        glBindTexture(GL_TEXTURE_2D, texChaoX);
        desenhaQuadTeto(x, z, TILE, 2.0f);
    }
}

// --- Desenha parede FACE POR FACE ---
static void desenhaParedePorFace(float x, float z, GLuint texParedeX, int f)
{
    float half = TILE * 0.5f;

    glUseProgram(0);
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_2D, texParedeX);

    float tilesX = 1.0f;
    float tilesY = WALL_H * 0.5f; // Mantém proporção (1 repeat a cada 2 unidades)

    glBegin(GL_QUADS);

    switch (f)
    {
    case 0: // z+ (Frente)
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(x - half, 0.0f, z + half);
        glTexCoord2f(tilesX, 0.0f);
        glVertex3f(x + half, 0.0f, z + half);
        glTexCoord2f(tilesX, tilesY);
        glVertex3f(x + half, WALL_H, z + half);
        glTexCoord2f(0.0f, tilesY);
        glVertex3f(x - half, WALL_H, z + half);
        break;

    case 1: // z- (Trás)
        glNormal3f(0.0f, 0.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(x + half, 0.0f, z - half);
        glTexCoord2f(tilesX, 0.0f);
        glVertex3f(x - half, 0.0f, z - half);
        glTexCoord2f(tilesX, tilesY);
        glVertex3f(x - half, WALL_H, z - half);
        glTexCoord2f(0.0f, tilesY);
        glVertex3f(x + half, WALL_H, z - half);
        break;

    case 2: // x+ (Direita)
        glNormal3f(1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(x + half, 0.0f, z + half);
        glTexCoord2f(tilesX, 0.0f);
        glVertex3f(x + half, 0.0f, z - half);
        glTexCoord2f(tilesX, tilesY);
        glVertex3f(x + half, WALL_H, z - half);
        glTexCoord2f(0.0f, tilesY);
        glVertex3f(x + half, WALL_H, z + half);
        break;

    case 3: // x- (Esquerda)
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(x - half, 0.0f, z - half);
        glTexCoord2f(tilesX, 0.0f);
        glVertex3f(x - half, 0.0f, z + half);
        glTexCoord2f(tilesX, tilesY);
        glVertex3f(x - half, WALL_H, z + half);
        glTexCoord2f(0.0f, tilesY);
        glVertex3f(x - half, WALL_H, z - half);
        break;
    }
    glEnd();
}

// Wrapper para desenhar o cubo todo (parede outdoor)
static void desenhaParedeCuboCompleto(float x, float z, GLuint texParedeX)
{
    desenhaParedePorFace(x, z, texParedeX, 0);
    desenhaParedePorFace(x, z, texParedeX, 1);
    desenhaParedePorFace(x, z, texParedeX, 2);
    desenhaParedePorFace(x, z, texParedeX, 3);

    float half = TILE * 0.5f;
    glBindTexture(GL_TEXTURE_2D, texParedeX);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(x - half, WALL_H, z + half);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(x + half, WALL_H, z + half);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(x + half, WALL_H, z - half);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(x - half, WALL_H, z - half);
    glEnd();
}

static void desenhaTileLava(float x, float z, const RenderAssets &r, float time)
{
    glUseProgram(r.progLava);

    GLint locTime = glGetUniformLocation(r.progLava, "uTime");
    GLint locStr = glGetUniformLocation(r.progLava, "uStrength");
    GLint locScr = glGetUniformLocation(r.progLava, "uScroll");
    GLint locHeat = glGetUniformLocation(r.progLava, "uHeat");
    GLint locTex = glGetUniformLocation(r.progLava, "uTexture");

    glUniform1f(locTime, time);
    glUniform1f(locStr, 1.0f);
    glUniform2f(locScr, 0.1f, 0.0f);
    glUniform1f(locHeat, 0.6f);

    bindTexture0(r.texLava);
    glUniform1i(locTex, 0);

    glColor3f(1, 1, 1);
    desenhaQuadChao(x, z, TILE, 2.0f);

    glUseProgram(0);
}

static void desenhaTileSangue(float x, float z, const RenderAssets &r, float time)
{
    glUseProgram(r.progSangue);

    GLint locTime = glGetUniformLocation(r.progSangue, "uTime");
    GLint locStr = glGetUniformLocation(r.progSangue, "uStrength");
    GLint locSpd = glGetUniformLocation(r.progSangue, "uSpeed");
    GLint locTex = glGetUniformLocation(r.progSangue, "uTexture");

    glUniform1f(locTime, time);
    glUniform1f(locStr, 1.0f);
    glUniform2f(locSpd, 2.0f, 1.3f);

    bindTexture0(r.texSangue);
    glUniform1i(locTex, 0);

    glColor3f(1, 1, 1);
    desenhaQuadChao(x, z, TILE, 2.0f);

    glUseProgram(0);
}

// --- Checa vizinhos ---
static char getTileAt(const MapLoader &map, int tx, int tz)
{
    const auto &data = map.data();
    const int H = map.getHeight();

    if (tz < 0 || tz >= H)
        return '0';
    if (tx < 0 || tx >= (int)data[tz].size())
        return '0';

    return data[tz][tx];
}

static void drawFace(float wx, float wz, int face, char neighbor, GLuint texParedeInternaX)
{
    bool outside = (neighbor == '0' || neighbor == 'L' || neighbor == 'B');

    if (outside)
    {
        glDisable(GL_LIGHT1);
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, kAmbientOutdoor);
        glEnable(GL_LIGHT0);

        desenhaParedePorFace(wx, wz, texParedeInternaX, face);
    }
    else if (neighbor != '2')
    {
        beginIndoor();
        desenhaParedePorFace(wx, wz, texParedeInternaX, face);
        endIndoor();
    }
}

void drawLevel(const MapLoader &map, float px, float pz, float dx, float dz, const RenderAssets &r, float time)
{
    const auto &data = map.data();
    const int H = map.getHeight();

    LevelMetrics m = LevelMetrics::fromMap(map, TILE);

    float fwdx, fwdz;
    bool hasFwd = getForwardXZ(dx, dz, fwdx, fwdz);

    for (int z = 0; z < H; z++)
    {
        for (int x = 0; x < (int)data[z].size(); x++)
        {
            float wx, wz;
            m.tileCenter(x, z, wx, wz);

            // CULLING ÚNICO (cenário)
            if (!isVisibleXZ(wx, wz, px, pz, hasFwd, fwdx, fwdz))
                continue;

            char c = data[z][x];

            bool isEntity = (c == 'J' || c == 'T' || c == 'G' || c == 'F' || c == 'I' ||
                             c == 'j' || c == 't' || c == 'm' || c == 'g' ||
                             c == 'h' || c == 'a' || c == 'k' || c == 'w' || c == 'W');

            if (isEntity)
            {
                char viz1 = getTileAt(map, x + 1, z);
                char viz2 = getTileAt(map, x - 1, z);
                char viz3 = getTileAt(map, x, z + 1);
                char viz4 = getTileAt(map, x, z - 1);

                bool isIndoor = (viz1 == '3' || viz1 == '2' ||
                                 viz2 == '3' || viz2 == '2' ||
                                 viz3 == '3' || viz3 == '2' ||
                                 viz4 == '3' || viz4 == '2');

                if (isIndoor)
                {
                    beginIndoor();
                    desenhaTileChao(wx, wz, r.texChaoInterno, true);
                    endIndoor();
                }
                else
                {
                    desenhaTileChao(wx, wz, r.texChao, false);
                }
            }
            else if (c == '0')
            {
                desenhaTileChao(wx, wz, r.texChao, false);
            }
            else if (c == '3')
            {
                beginIndoor();
                desenhaTileChao(wx, wz, r.texChaoInterno, true);
                endIndoor();
            }
            else if (c == '1')
            {
                desenhaParedeCuboCompleto(wx, wz, r.texParede);
            }
            else if (c == 'M')
            {
                desenhaParedeCuboCompleto(wx, wz, r.texWallMarket);
            }
            else if (c == 'H')
            {
                desenhaParedeCuboCompleto(wx, wz, r.texWallHouse);
            }
            else if (c == 'S')
            {
                desenhaParedeCuboCompleto(wx, wz, r.texWallShop);
            }
            else if (c == 'O')
            {
                desenhaParedeCuboCompleto(wx, wz, r.texWallOffice);
            }
            else if (c == '2')
            {
                char vizFrente = getTileAt(map, x, z + 1);
                char vizTras = getTileAt(map, x, z - 1);
                char vizDireita = getTileAt(map, x + 1, z);
                char vizEsq = getTileAt(map, x - 1, z);

                drawFace(wx, wz, 0, vizFrente, r.texParedeInterna);
                drawFace(wx, wz, 1, vizTras, r.texParedeInterna);
                drawFace(wx, wz, 2, vizDireita, r.texParedeInterna);
                drawFace(wx, wz, 3, vizEsq, r.texParedeInterna);
            }
            else if (c == 'L')
            {
                desenhaTileLava(wx, wz, r, time);
            }
            else if (c == 'B')
            {
                desenhaTileSangue(wx, wz, r, time);
            }
            else if (c == 'E')
            {
                // --- PORTAL DE SAÍDA ---
                float pulse  = 0.5f + 0.5f * std::sin(time * 3.0f);
                float pulse2 = 0.5f + 0.5f * std::sin(time * 3.0f + 1.57f);
                float half   = TILE * 0.5f;

                glUseProgram(0);
                glDisable(GL_LIGHTING);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                // 1) Piso brilhante pulsante (verde → amarelo)
                glColor4f(0.2f + 0.4f * pulse2, 0.9f, 0.2f, 1.0f);
                glDisable(GL_TEXTURE_2D);
                glBegin(GL_QUADS);
                glNormal3f(0, 1, 0);
                glVertex3f(wx - half, EPS_Y, wz + half);
                glVertex3f(wx + half, EPS_Y, wz + half);
                glVertex3f(wx + half, EPS_Y, wz - half);
                glVertex3f(wx - half, EPS_Y, wz - half);
                glEnd();

                // 2) Cruz de vigas verticais de luz (4 pilares translúcidos)
                float beamH  = WALL_H * 1.1f;
                float beamW  = 0.18f;
                float alpha  = 0.35f + 0.25f * pulse;

                // Ângulo de rotação lento para os pilares
                float rot = time * 45.0f; // graus/s
                float cosR = std::cos(rot * 3.14159f / 180.0f);
                float sinR = std::sin(rot * 3.14159f / 180.0f);

                float offsets[4][2] = {
                    { half * 0.55f,  0.0f},
                    {-half * 0.55f,  0.0f},
                    { 0.0f,  half * 0.55f},
                    { 0.0f, -half * 0.55f}
                };

                for (int i = 0; i < 4; i++)
                {
                    float ox = offsets[i][0] * cosR - offsets[i][1] * sinR;
                    float oz = offsets[i][0] * sinR + offsets[i][1] * cosR;
                    float bx = wx + ox;
                    float bz = wz + oz;

                    glColor4f(0.3f, 1.0f, 0.3f, alpha);
                    glBegin(GL_QUADS);
                    glVertex3f(bx - beamW, 0.0f,   bz);
                    glVertex3f(bx + beamW, 0.0f,   bz);
                    glVertex3f(bx + beamW, beamH,  bz);
                    glVertex3f(bx - beamW, beamH,  bz);
                    glEnd();

                    glBegin(GL_QUADS);
                    glVertex3f(bx, 0.0f,   bz - beamW);
                    glVertex3f(bx, 0.0f,   bz + beamW);
                    glVertex3f(bx, beamH,  bz + beamW);
                    glVertex3f(bx, beamH,  bz - beamW);
                    glEnd();
                }

                // 3) Halo no chão (anel translúcido)
                int   segs    = 24;
                float ringR   = half * 0.75f;
                float ringAlpha = 0.5f + 0.3f * pulse;
                glColor4f(0.5f, 1.0f, 0.2f, ringAlpha);
                glLineWidth(3.0f);
                glBegin(GL_LINE_LOOP);
                for (int i = 0; i < segs; i++)
                {
                    float ang = i * 2.0f * 3.14159f / segs + time;
                    glVertex3f(wx + std::cos(ang) * ringR, EPS_Y * 2, wz + std::sin(ang) * ringR);
                }
                glEnd();

                glEnable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);
                glEnable(GL_LIGHTING);
            }
        }
    }
}

static void drawSprite(float x, float z, float w, float h, GLuint tex, float camX, float camZ)
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

static void drawProjectiles(const std::vector<Projectile>& projectiles, float camX, float camZ, float time)
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

// Desenha inimigos e itens
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

        drawSprite(en.x, en.z, 2.5f, 2.5f, currentTex, camX, camZ);
    }

    // --- PROJÉTEIS ---
    drawProjectiles(gameLevel().projectiles, camX, camZ, gameContext().time);

    glDisable(GL_ALPHA_TEST);
}
