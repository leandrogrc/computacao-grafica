#include "graphics/hud.h"
#include "graphics/ui_text.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <string>
#include <cstdlib>
#include <cmath>
#include "core/config.h"
#include "level/maploader.h"
#include "core/entities.h"
#include "core/game.h"

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

static void drawVignetteAndScanlines(int w, int h)
{
    begin2D(w, h);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 3. Scanlines e Vinheta
    glBegin(GL_QUADS);
    float pulse = 0.5f + 0.15f * std::sin(gameContext().time * 5.0f); // Use gameContext().time for animation
    
    // Cor base (vinheta preta)
    float rVig=0, gVig=0, bVig=0;
    // Se tiver power-up, mudamos o tom da "vinheta/visor"
    if (gameContext().player.berserkTimer > 0.0f) { rVig = 0.3f; }
    else if (gameContext().player.hasteTimer > 0.0f) { bVig = 0.3f; gVig = 0.2f; }

    // Top vignette with power-up tint
    glColor4f(rVig, gVig, bVig, 0.4f + pulse);
    glVertex2f(0, 0); glVertex2f((float)w, 0);
    glColor4f(0, 0, 0, 0);
    glVertex2f((float)w, h * 0.15f); glVertex2f(0, h * 0.15f);

    // Bottom vignette (original)
    glColor4f(0, 0, 0, 0);
    glVertex2f(0, h * 0.85f); glVertex2f((float)w, h * 0.85f);
    glColor4f(0, 0, 0, 0.4f);
    glVertex2f((float)w, (float)h); glVertex2f(0, (float)h);
    glEnd();

    // Subtle scanlines
    glColor4f(0, 0.5f, 1.0f, 0.05f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (int i = 0; i < h; i += 4)
    {
        glVertex2f(0, (float)i); glVertex2f((float)w, (float)i);
    }
    glEnd();

    glDisable(GL_BLEND);
    end2D();
}

static void drawHoloBrackets(int w, int h)
{
    begin2D(w, h);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float bSize = 40.0f;
    float thick = 3.0f;
    float pad = 20.0f;

    glColor4f(0.0f, 1.0f, 1.0f, 0.6f); // Cyan Holo
    glLineWidth(thick);

    // Top Left
    glBegin(GL_LINE_STRIP);
    glVertex2f(pad + bSize, h - pad);
    glVertex2f(pad, h - pad);
    glVertex2f(pad, h - (pad + bSize));
    glEnd();

    // Top Right
    glBegin(GL_LINE_STRIP);
    glVertex2f(w - (pad + bSize), h - pad);
    glVertex2f(w - pad, h - pad);
    glVertex2f(w - pad, h - (pad + bSize));
    glEnd();

    // Bottom Left
    glBegin(GL_LINE_STRIP);
    glVertex2f(pad, pad + bSize);
    glVertex2f(pad, pad);
    glVertex2f(pad + bSize, pad);
    glEnd();

    // Bottom Right
    glBegin(GL_LINE_STRIP);
    glVertex2f(w - pad, pad + bSize);
    glVertex2f(w - pad, pad);
    glVertex2f(w - (pad + bSize), pad);
    glEnd();

    glDisable(GL_BLEND);
    end2D();
}

static void drawCrosshair(int w, int h)
{
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    begin2D(w, h);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float cx = w / 2.0f;
    float cy = h / 2.0f;
    float size = 8.0f;

    // Outer circle shadow
    glColor4f(0, 0, 0, 0.3f);
    glPointSize(4.0f);
    glBegin(GL_POINTS);
    glVertex2f(cx, cy);
    glEnd();

    // Cross lines
    glColor4f(0.0f, 1.0f, 1.0f, 0.8f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(cx - size, cy); glVertex2f(cx - 2, cy);
    glVertex2f(cx + size, cy); glVertex2f(cx + 2, cy);
    glVertex2f(cx, cy - size); glVertex2f(cx, cy - 2);
    glVertex2f(cx, cy + size); glVertex2f(cx, cy + 2);
    glEnd();

    glDisable(GL_BLEND);
    end2D();
    glPopAttrib();
}

static void drawDamageOverlay(int w, int h, GLuint texDamage, float alpha)
{
    if (alpha <= 0.0f || texDamage == 0)
        return;

    begin2D(w, h);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, texDamage);
    glColor4f(1, 1, 1, alpha);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex2f(0, 0);
    glTexCoord2f(1, 1); glVertex2f((float)w, 0);
    glTexCoord2f(1, 0); glVertex2f((float)w, (float)h);
    glTexCoord2f(0, 0); glVertex2f(0, (float)h);
    glEnd();

    glDisable(GL_BLEND);

    end2D();
}

static void drawHealthOverlay(int w, int h, GLuint texHealth, float alpha)
{
    if (alpha <= 0.0f || texHealth == 0)
        return;

    begin2D(w, h);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, texHealth);
    glColor4f(1, 1, 1, alpha);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex2f(0, 0);
    glTexCoord2f(1, 1); glVertex2f((float)w, 0);
    glTexCoord2f(1, 0); glVertex2f((float)w, (float)h);
    glTexCoord2f(0, 0); glVertex2f(0, (float)h);
    glEnd();

    glDisable(GL_BLEND);

    end2D();
}

static void drawWeaponHUD(int w, int h, const HudTextures& tex, const HudState& s)
{
    WeaponState ws = s.weaponState;
    GLuint currentTex = 0;

    if (s.activeWeaponIdx == 0) // Shotgun
    {
        currentTex = tex.texGunDefault;
        if (ws == WeaponState::W_FIRE_1 || ws == WeaponState::W_RETURN) currentTex = tex.texGunFire1;
        else if (ws == WeaponState::W_FIRE_2) currentTex = tex.texGunFire2;
        else if (ws == WeaponState::W_RELOAD_1 || ws == WeaponState::W_RELOAD_3) currentTex = tex.texGunReload1;
        else if (ws == WeaponState::W_RELOAD_2) currentTex = tex.texGunReload2;
    }
    else // Plasma Rifle
    {
        currentTex = tex.texGun2Default;
        if (ws == WeaponState::W_FIRE_1 || ws == WeaponState::W_RETURN) currentTex = tex.texGun2Fire1;
        else if (ws == WeaponState::W_FIRE_2) currentTex = tex.texGun2Fire2;
        // Se no futuro houver reload para plasma, adicionar aqui
    }

    if (currentTex == 0)
        return;

    begin2D(w, h);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, currentTex);
    glColor4f(1, 1, 1, 1);

    float gunH = h * 0.55f;
    float gunW = gunH;
    float centerX = w / 2.0f;
    float baseY = 0.0f;

    glPushMatrix();
    if (s.isSprinting && s.activeWeaponIdx == 0 && tex.texGunSprint != 0)
    {
        // Bind sprint texture BEFORE drawing
        glBindTexture(GL_TEXTURE_2D, tex.texGunSprint);
        // Posição de corrida (arma abaixada e para o lado)
        glTranslatef(centerX + 140.0f, baseY + 60.0f, 0);
        // Balanço de corrida sutil
        float swayX = 15.0f * std::sin(gameContext().time * 10.0f);
        float swayY = 8.0f * std::cos(gameContext().time * 20.0f);
        glTranslatef(swayX, swayY, 0);
    }
    else
    {
        glTranslatef(centerX, baseY, 0);
        if (ws != WeaponState::W_IDLE)
        {
            glTranslatef((float)(std::rand() % 10 - 5), -20.0f, 0);
        }
    }

    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex2f(-gunW / 2.0f, 0);
    glTexCoord2f(1, 1); glVertex2f(gunW / 2.0f, 0);
    glTexCoord2f(1, 0); glVertex2f(gunW / 2.0f, gunH);
    glTexCoord2f(0, 0); glVertex2f(-gunW / 2.0f, gunH);
    glEnd();
    glPopMatrix();

    glDisable(GL_BLEND);

    end2D();
}

static void drawBottomHUD(int w, int h, const HudState& s)
{
    begin2D(w, h);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float hudH   = 90.0f;
    float hudY   = 0.0f;
    float centerY = hudY + hudH * 0.5f;

    // --- Full-width background panel ---
    glColor4f(0.02f, 0.04f, 0.10f, 0.96f);
    glBegin(GL_QUADS);
    glVertex2f(0, hudY);         glVertex2f((float)w, hudY);
    glVertex2f((float)w, hudY + hudH); glVertex2f(0, hudY + hudH);
    glEnd();

    // --- Top separator line ---
    glColor4f(0.0f, 0.9f, 1.0f, 0.85f);
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    glVertex2f(0, hudY + hudH); glVertex2f((float)w, hudY + hudH);
    glEnd();

    // === LEFT: HEALTH ===
    float margin = 28.0f;
    float hx = margin + 20.0f;
    float hy = centerY;

    // Medical cross icon (red)
    float cs = 14.0f;  // arm half-length
    float cw = 5.0f;   // arm half-width
    glColor4f(1.0f, 0.15f, 0.15f, 1.0f);
    // Vertical arm
    glBegin(GL_QUADS);
    glVertex2f(hx - cw, hy - cs); glVertex2f(hx + cw, hy - cs);
    glVertex2f(hx + cw, hy + cs); glVertex2f(hx - cw, hy + cs);
    glEnd();
    // Horizontal arm
    glBegin(GL_QUADS);
    glVertex2f(hx - cs, hy - cw); glVertex2f(hx + cs, hy - cw);
    glVertex2f(hx + cs, hy + cw); glVertex2f(hx - cs, hy + cw);
    glEnd();
    // Bright outline
    glColor4f(1.0f, 0.5f, 0.5f, 0.8f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(hx - cw, hy - cs); glVertex2f(hx + cw, hy - cs);
    glVertex2f(hx + cw, hy - cw); glVertex2f(hx + cs, hy - cw);
    glVertex2f(hx + cs, hy + cw); glVertex2f(hx + cw, hy + cw);
    glVertex2f(hx + cw, hy + cs); glVertex2f(hx - cw, hy + cs);
    glVertex2f(hx - cw, hy + cw); glVertex2f(hx - cs, hy + cw);
    glVertex2f(hx - cs, hy - cw); glVertex2f(hx - cw, hy - cw);
    glEnd();

    // Health bar
    float barX = hx + cs + 12.0f;
    float barW = 170.0f;
    float barH = 20.0f;
    float barY = centerY - barH * 0.5f;

    glColor4f(0.15f, 0.0f, 0.0f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(barX, barY); glVertex2f(barX + barW, barY);
    glVertex2f(barX + barW, barY + barH); glVertex2f(barX, barY + barH);
    glEnd();

    float hpPct = std::max(0.0f, std::min(1.0f, (float)s.playerHealth / 100.0f));
    if (hpPct > 0.5f)       glColor4f(0.1f, 1.0f, 0.4f, 1.0f);
    else if (hpPct > 0.25f) glColor4f(1.0f, 0.75f, 0.0f, 1.0f);
    else                    glColor4f(1.0f, 0.1f, 0.1f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(barX, barY); glVertex2f(barX + barW * hpPct, barY);
    glVertex2f(barX + barW * hpPct, barY + barH); glVertex2f(barX, barY + barH);
    glEnd();

    glColor4f(0.0f, 1.0f, 1.0f, 0.6f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(barX, barY); glVertex2f(barX + barW, barY);
    glVertex2f(barX + barW, barY + barH); glVertex2f(barX, barY + barH);
    glEnd();

    // Health value
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    std::string hpStr = std::to_string(std::max(0, s.playerHealth)) + "%";
    uiDrawStrokeText(barX + barW + 8.0f, barY, hpStr.c_str(), 0.22f);

    // === RIGHT: AMMO ===
    float ax = (float)w - margin;

    // Diamond bullet icon (gold)
    std::string ammoRes = "/" + std::to_string(s.reserveAmmo);
    std::string ammoNow = std::to_string(s.currentAmmo);
    float resW  = uiStrokeTextWidthScaled(ammoRes.c_str(), 0.18f);
    float nowW  = uiStrokeTextWidthScaled(ammoNow.c_str(), 0.42f);
    float ammoTotalW = nowW + 4.0f + resW;

    // Right-align ammo at ax
    float ammoStartX = ax - ammoTotalW;
    float diamondX   = ammoStartX - 28.0f;
    float ds = 14.0f; // diamond half-size

    glColor4f(1.0f, 0.85f, 0.0f, 1.0f);
    glBegin(GL_POLYGON);
    glVertex2f(diamondX,        centerY + ds * 0.55f);  // top
    glVertex2f(diamondX + ds * 0.4f, centerY);           // right
    glVertex2f(diamondX,        centerY - ds * 0.55f);  // bottom
    glVertex2f(diamondX - ds * 0.4f, centerY);           // left
    glEnd();
    // Outline
    glColor4f(1.0f, 1.0f, 0.3f, 0.7f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(diamondX,        centerY + ds * 0.55f);
    glVertex2f(diamondX + ds * 0.4f, centerY);
    glVertex2f(diamondX,        centerY - ds * 0.55f);
    glVertex2f(diamondX - ds * 0.4f, centerY);
    glEnd();

    // Ammo values
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    uiDrawStrokeText(ammoStartX, centerY - 12.0f, ammoNow.c_str(), 0.42f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // White
    uiDrawStrokeText(ammoStartX + nowW + 4.0f, centerY - 6.0f, ammoRes.c_str(), 0.18f);

    // === Center: Thin vertical dividers ===
    glColor4f(0.0f, 0.8f, 1.0f, 0.2f);
    glLineWidth(1.0f);
    float divX1 = (float)w * 0.35f;
    float divX2 = (float)w * 0.65f;
    glBegin(GL_LINES);
    glVertex2f(divX1, hudY + 10); glVertex2f(divX1, hudY + hudH - 10);
    glVertex2f(divX2, hudY + 10); glVertex2f(divX2, hudY + hudH - 10);
    glEnd();

    glDisable(GL_BLEND);
    end2D();
}

static void drawObjectiveHUD(int w, int h, int collected)
{
    begin2D(w, h);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    float textScale = 0.155f;
    const char* objText = "OBJETIVO: CARTOES DE DADOS";
    float objW  = uiStrokeTextWidthScaled(objText, textScale);

    // Use the max expected counter text to avoid resizing on collection
    float cntW  = uiStrokeTextWidthScaled("[3/3]", textScale);

    float icSize = 8.0f;
    float iconArea = icSize * 0.6f + icSize + 10.0f; // diamond + gap

    float padding = 24.0f;
    float panelW = iconArea + objW + 12.0f + cntW + padding;
    float panelH = 36.0f;
    float x = (w - panelW) / 2.0f;
    float y = h - panelH - 20.0f;

    // Panel background
    glColor4f(0.0f, 0.05f, 0.12f, 0.92f);
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x + panelW, y);
    glVertex2f(x + panelW, y + panelH); glVertex2f(x, y + panelH);
    glEnd();

    // Border glow
    glColor4f(0.0f, 1.0f, 1.0f, 0.6f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y); glVertex2f(x + panelW, y);
    glVertex2f(x + panelW, y + panelH); glVertex2f(x, y + panelH);
    glEnd();

    // Diamond icon on the left
    float icX = x + 20.0f;
    float icY = y + panelH * 0.5f;
    glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
    glBegin(GL_POLYGON);
    glVertex2f(icX, icY + icSize); glVertex2f(icX + icSize * 0.6f, icY);
    glVertex2f(icX, icY - icSize); glVertex2f(icX - icSize * 0.6f, icY);
    glEnd();

    // Objective text
    float textX = icX + icSize * 0.6f + 10.0f;
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    uiDrawStrokeText(textX, y + 9, objText, textScale);

    std::string counts = "[" + std::to_string(collected) + "/3]";
    if (collected >= 3) glColor4f(0.2f, 1.0f, 0.2f, 1.0f);
    else                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    uiDrawStrokeText(textX + objW + 12.0f, y + 9, counts.c_str(), textScale);

    glDisable(GL_BLEND);
    end2D();
}

static void drawMiniMap(int w, int h, const HudState& s, const MapLoader& map, const std::vector<Enemy>& enemies, const std::vector<Item>& items)
{
    begin2D(w, h);
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
    end2D();
}

static void drawPowerupStatus(int w, int h, const HudState& s)
{
    if (s.berserkTimer <= 0.0f && s.hasteTimer <= 0.0f) return;

    begin2D(w, h);
    glEnable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    
    float x = 25.0f;
    float y = h - 220.0f;
    float barW = 120.0f;
    float barH = 5.0f;

    auto drawBar = [&](const char* label, float timer, float r, float g, float b) {
        if (timer <= 0.0f) return;
        
        float progress = timer / 10.0f; // Max 10s
        if (progress > 1.0f) progress = 1.0f;

        // Label
        glColor4f(r, g, b, 1.0f);
        uiDrawStrokeText(x, y + 10, label, 0.1f);

        // BG Bar
        glColor4f(0, 0, 0, 0.5f);
        glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + barW, y);
        glVertex2f(x + barW, y + barH); glVertex2f(x, y + barH);
        glEnd();

        // Fill Bar
        glColor4f(r, g, b, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + barW * progress, y);
        glVertex2f(x + barW * progress, y + barH); glVertex2f(x, y + barH);
        glEnd();

        y -= 25.0f;
    };

    drawBar("BERSERK", s.berserkTimer, 1.0f, 0.2f, 0.1f);
    drawBar("HASTE", s.hasteTimer, 0.1f, 1.0f, 1.0f);

    glDisable(GL_BLEND);
    end2D();
}

void hudRenderAll(
    int screenW,
    int screenH,
    const HudTextures& tex,
    const HudState& state,
    const MapLoader& map,
    const std::vector<Enemy>& enemies,
    const std::vector<Item>& items,
    bool showCrosshair,
    bool showWeapon,
    bool showDoomBar)
{
    // 1. Environmental Effects (Visor)
    drawVignetteAndScanlines(screenW, screenH);
    drawHoloBrackets(screenW, screenH);

    // 2. Weapon
    if (showWeapon) drawWeaponHUD(screenW, screenH, tex, state);
    
    // 3. Main HUD (Stats)
    if (showDoomBar) drawBottomHUD(screenW, screenH, state);
    
    // 4. Objective
    drawObjectiveHUD(screenW, screenH, state.cardsCollected);

    // 5. Mini-mapa
    drawMiniMap(screenW, screenH, state, map, enemies, items);

    // 6. Power-up Status
    drawPowerupStatus(screenW, screenH, state);

    // 7. Crosshair
    if (showCrosshair) drawCrosshair(screenW, screenH);

    // 7. Overlays
    drawDamageOverlay(screenW, screenH, tex.texDamage, state.damageAlpha);
    drawHealthOverlay(screenW, screenH, tex.texHealthOverlay, state.healthAlpha);
}
