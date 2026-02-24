// #include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "core/game_enums.h"
#include "core/game_state.h"

#include "core/game.h"

#include "level/level.h"

#include "core/camera.h"
#include "input/input.h"
#include "input/keystate.h"

#include "graphics/drawlevel.h"
#include "graphics/skybox.h"
#include "graphics/hud.h"
#include "graphics/menu.h"
#include "graphics/lighting.h"

#include "core/movement.h"
#include "core/player.h"
#include "core/entities.h"

#include "audio/audio_system.h"

#include "utils/assets.h"
#include "core/config.h"

#include "core/window.h"

#include <GL/glew.h>
#include <GL/glut.h>

static HudTextures gHudTex;
static GameContext g;

constexpr int MAX_MAGAZINE = 12;

// --- Assets / Level ---
static GameAssets gAssets;
Level gLevel;
static AudioSystem gAudioSys;

GameContext &gameContext() { return g; }

AudioSystem &gameAudio() { return gAudioSys; }

Level &gameLevel() { return gLevel; }

GameState gameGetState() { return g.state; }

void gameSetState(GameState s) { g.state = s; }

void gameTogglePause()
{
    if (g.state == GameState::JOGANDO)
        g.state = GameState::PAUSADO;
    else if (g.state == GameState::PAUSADO)
        g.state = GameState::JOGANDO;
}

// --- Carrega uma fase pelo número (1, 2 ou 3) ---
bool gameLoadLevel(int levelNum)
{
    static const char* mapPaths[] = {
        nullptr,
        "maps/map1.txt",
        "maps/map2.txt",
        "maps/map3.txt"
    };
    if (levelNum < 1 || levelNum > 3) return false;

    g.currentLevel = levelNum;
    
    // Para e fecha o sistema de áudio atual de forma completa
    audioShutdown(gAudioSys);

    if (!loadLevel(gLevel, mapPaths[levelNum], GameConfig::TILE_SIZE))
        return false;

    applySpawn(gLevel, camX, camZ);
    camY = GameConfig::PLAYER_EYE_Y;

    audioInit(gAudioSys, gLevel);

    // Reseta munição e arma mas mantém vida
    g.player.currentAmmo = MAX_MAGAZINE;
    g.player.reserveAmmo = 25;
    g.player.cardsCollected = 0; // Novo: Reseta objetivo
    g.player.berserkTimer = 0.0f;
    g.player.hasteTimer = 0.0f;
    gLevel.projectiles.clear();
    for (int i=0; i<2; ++i) g.weapons[i] = WeaponAnim{};
    g.activeWeaponIdx = 0;
    g.hasWeapon[0] = true; 
    g.hasWeapon[1] = false;
    g.time = 0.0f;

    g.state = GameState::JOGANDO;
    return true;
}

// --- INIT ---
bool gameInit(const char *mapPath)
{
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);

    setupSunLightOnce();
    setupIndoorLightOnce();

    if (!loadAssets(gAssets))
        return false;

    g.r.texChao = gAssets.texChao;
    g.r.texParede = gAssets.texParede;
    g.r.texSangue = gAssets.texSangue;
    g.r.texLava = gAssets.texLava;
    g.r.texChaoInterno = gAssets.texChaoInterno;
    g.r.texParedeInterna = gAssets.texParedeInterna;
    g.r.texTeto = gAssets.texTeto;

    g.r.texSkydome = gAssets.texSkydome;
    g.r.texMenuBG = gAssets.texMenuBG;

    gHudTex.texHudFundo = gAssets.texHudFundo;
    gHudTex.texHudTopBG = gAssets.texHudTopBG;
    gHudTex.texGunHUD = gAssets.texGunHUD;

    gHudTex.texGunDefault = gAssets.texGunDefault;
    gHudTex.texGunFire1 = gAssets.texGunFire1;
    gHudTex.texGunFire2 = gAssets.texGunFire2;
    gHudTex.texGunReload1 = gAssets.texGunReload1;
    gHudTex.texGunReload2 = gAssets.texGunReload2;
    gHudTex.texGunSprint = gAssets.texGunSprint;
    gHudTex.texGun2Default = gAssets.texGun2Default;
    gHudTex.texGun2Fire1 = gAssets.texGun2Fire1;
    gHudTex.texGun2Fire2 = gAssets.texGun2Fire2;
    gHudTex.texDamage = gAssets.texDamage;
    gHudTex.texHealthOverlay = gAssets.texHealthOverlay;

    for (int i = 0; i < 5; i++)
    {
        g.r.texEnemies[i] = gAssets.texEnemies[i];
        g.r.texEnemiesRage[i] = gAssets.texEnemiesRage[i];
        g.r.texEnemiesDamage[i] = gAssets.texEnemiesDamage[i];
    }

    g.r.texHealth = gAssets.texHealth;
    g.r.texAmmo = gAssets.texAmmo;

    g.r.progSangue = gAssets.progSangue;
    g.r.progLava = gAssets.progLava;


    g.r.texWallMarket = gAssets.texWallMarket;
    g.r.texWallHouse  = gAssets.texWallHouse;
    g.r.texWallShop   = gAssets.texWallShop;
    g.r.texWallOffice = gAssets.texWallOffice;
    g.r.texHudTopBG   = gAssets.texHudTopBG;
    g.r.texCard       = gAssets.texCard;
    g.r.texGun2Default = gAssets.texGun2Default;

    if (!loadLevel(gLevel, mapPath, GameConfig::TILE_SIZE))
        return false;

    applySpawn(gLevel, camX, camZ);
    camY = GameConfig::PLAYER_EYE_Y;

    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutPassiveMotionFunc(mouseMotion);
    glutSetCursor(GLUT_CURSOR_NONE);

    // Audio init + ambient + enemy sources
    audioInit(gAudioSys, gLevel);

    g.currentLevel = 1;
    g.state = GameState::MENU_INICIAL;
    g.time = 0.0f;
    g.player = PlayerState{};
    for (int i=0; i<2; ++i) g.weapons[i] = WeaponAnim{};
    g.hasWeapon[0] = true;
    g.hasWeapon[1] = false;
    g.activeWeaponIdx = 0;

    return true;
}

// Reinicia o jogo
void gameReset()
{
    g.player.health = 100;
    g.player.currentAmmo = 12;
    g.player.reserveAmmo = 25;

    g.player.damageAlpha = 0.0f;
    g.player.healthAlpha = 0.0f;

    for (int i = 0; i < 2; ++i)
    {
        g.weapons[i].state = WeaponState::W_IDLE;
        g.weapons[i].timer = 0.0f;
    }
    g.hasWeapon[0] = true;
    g.hasWeapon[1] = false;
    g.activeWeaponIdx = 0;

    // Respawna o jogador
    applySpawn(gLevel, camX, camZ);
}

void gameUpdate(float dt)
{
    g.time += dt;

    // SE NÃO ESTIVER JOGANDO, NÃO RODA A LÓGICA DO JOGO
    if (g.state != GameState::JOGANDO)
        return;

    atualizaMovimento();

    AudioListener L;
    L.pos = {camX, camY, camZ};
    {
        float ry = yaw * 3.14159f / 180.0f;
        float rp = pitch * 3.14159f / 180.0f;
        L.forward = {cosf(rp) * sinf(ry), sinf(rp), -cosf(rp) * cosf(ry)};
    }
    L.up = {0.0f, 1.0f, 0.0f};
    L.vel = {0.0f, 0.0f, 0.0f};

    bool moving = (keyW || keyA || keyS || keyD);
    audioUpdate(gAudioSys, gLevel, L, dt, moving, g.player.health);

    if (g.player.damageAlpha > 0.0f)
    {
        g.player.damageAlpha -= dt * 0.5f;
        if (g.player.damageAlpha < 0.0f)
            g.player.damageAlpha = 0.0f;
    }
    if (g.player.healthAlpha > 0.0f)
    {
        g.player.healthAlpha -= dt * 0.9f;
        if (g.player.healthAlpha < 0.0f)
            g.player.healthAlpha = 0.0f;
    }

    updateEntities(dt);
    if (keyG) playerTryGrabWeapon();
    updateWeaponAnim(dt);

    // CHECAGEM DE GAME OVER
    if (g.player.health <= 0)
    {
        g.state = GameState::GAME_OVER;
        g.player.damageAlpha = 1.0f;
        return;
    }

    // CHECAGEM DE SAÍDA DE FASE
    if (gLevel.hasExit)
    {
        float dx = camX - gLevel.exitX;
        float dz = camZ - gLevel.exitZ;
        // Só permite sair se coletou as 3 cartas
        if (dx*dx + dz*dz < 2.5f && g.player.cardsCollected >= 3)
        {
            if (g.currentLevel < 3)
                g.state = GameState::LEVEL_CLEAR;
            else
                g.state = GameState::VITORIA;
        }
    }
}

// Função auxiliar para desenhar o mundo 3D (Inimigos, Mapa, Céu)
void drawWorld3D()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    // Iluminação por fase
    applyLevelLighting(g.currentLevel, g.time);
    if (g.currentLevel == 1)
        setSunDirectionEachFrame();

    // ---- Troca de texturas por fase ----
    {
        int ni = g.currentLevel - 1; // 0,1,2
        if (ni >= 0 && ni < 3 && gAssets.texChaoNivel[ni])
        {
            g.r.texChao          = gAssets.texChaoNivel[ni];
            g.r.texParede        = gAssets.texParedeNivel[ni];
            g.r.texChaoInterno   = gAssets.texChaoInternoNivel[ni];
            g.r.texParedeInterna = gAssets.texParedeInterNivel[ni];
            g.r.texSkydome       = gAssets.texSkydomeNivel[ni];
        }
    }

    // Configuração da Câmera
    float radYaw = yaw * 3.14159265f / 180.0f;
    float radPitch = pitch * 3.14159265f / 180.0f;
    float dirX = cosf(radPitch) * sinf(radYaw);
    float dirY = sinf(radPitch);
    float dirZ = -cosf(radPitch) * cosf(radYaw);

    gluLookAt(camX, camY, camZ, camX + dirX, camY + dirY, camZ + dirZ, 0.0f, 1.0f, 0.0f);

    drawSkydome(camX, camY, camZ, g.r);
    drawLevel(gLevel.map, camX, camZ, dirX, dirZ, g.r, g.time);
    drawEntities(gLevel.enemies, gLevel.items, camX, camZ, dirX, dirZ, g.r);
}

void gameRender()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    HudState hs;
    hs.playerHealth = g.player.health;
    hs.currentAmmo = g.player.currentAmmo;
    hs.reserveAmmo = g.player.reserveAmmo;
    hs.damageAlpha = g.player.damageAlpha;
    hs.healthAlpha = g.player.healthAlpha;
    hs.cardsCollected = g.player.cardsCollected;
    hs.berserkTimer = g.player.berserkTimer;
    hs.hasteTimer = g.player.hasteTimer;
    hs.weaponState = g.weapons[g.activeWeaponIdx].state;
    hs.activeWeaponIdx = g.activeWeaponIdx;
    hs.px = camX;
    hs.pz = camZ;
    hs.pyaw = yaw;
    hs.isSprinting = keyShift && (keyW || keyA || keyS || keyD);

    if (g.state == GameState::MENU_INICIAL)
    {
        menuRender(janelaW, janelaH, g.time, "", "Pressione ENTER para Jogar", g.r);
    }
    else if (g.state == GameState::GAME_OVER)
    {
        drawWorld3D();
        menuRender(janelaW, janelaH, g.time, "GAME OVER", "Pressione ENTER para Reiniciar", g.r);
    }
    else if (g.state == GameState::LEVEL_CLEAR)
    {
        drawWorld3D();
        char msg[64];
        std::snprintf(msg, sizeof(msg), "FASE %d COMPLETA!", g.currentLevel);
        menuRender(janelaW, janelaH, g.time, msg, "Pressione ENTER para Continuar", g.r);
    }
    else if (g.state == GameState::VITORIA)
    {
        drawWorld3D();
        menuRender(janelaW, janelaH, g.time, "VITORIA!", "Pressione ENTER para Jogar Novamente", g.r);
    }
    else if (g.state == GameState::PAUSADO)
    {
        drawWorld3D();
        hudRenderAll(janelaW, janelaH, gHudTex, hs, gLevel.map, gLevel.enemies, gLevel.items, true, true, true);
        pauseMenuRender(janelaW, janelaH, g.time);
    }
    else // JOGANDO
    {
        drawWorld3D();
        hudRenderAll(janelaW, janelaH, gHudTex, hs, gLevel.map, gLevel.enemies, gLevel.items, true, true, true);
        menuMeltRenderOverlay(janelaW, janelaH, g.time);
    }

    glutSwapBuffers();
}
