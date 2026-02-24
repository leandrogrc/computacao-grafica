// core/game_state.h
#pragma once
#include "core/game_enums.h"
#include <GL/glew.h>

struct PlayerState
{
    int health = 100;
    float damageAlpha = 0.0f;
    float healthAlpha = 0.0f;

    int currentAmmo = 12;
    int reserveAmmo = 25;
    int cardsCollected = 0;

    float berserkTimer = 0.0f;
    float hasteTimer = 0.0f;
};

struct WeaponAnim
{
    WeaponState state = WeaponState::W_IDLE;
    float timer = 0.0f;
};

struct RenderAssets
{
    GLuint texChao = 0, texParede = 0, texSangue = 0, texLava = 0;
    GLuint texChaoInterno = 0, texParedeInterna = 0, texTeto = 0, texSkydome = 0, texMenuBG = 0;
    // GLuint texSkydome=0;

    GLuint texEnemies[5] = {0};
    GLuint texEnemiesRage[5] = {0};
    GLuint texEnemiesDamage[5] = {0};

    GLuint texHealth = 0;
    GLuint texAmmo = 0;

    GLuint progSangue = 0;
    GLuint progLava = 0;


    GLuint texWallMarket = 0;
    GLuint texWallHouse  = 0;
    GLuint texWallShop   = 0;
    GLuint texWallOffice = 0;
    GLuint texHudTopBG   = 0;
    GLuint texCard       = 0;
    GLuint texGun2Default = 0;
};

struct GameContext
{
    GameState state = GameState::MENU_INICIAL;
    PlayerState player;
    WeaponAnim weapons[2];
    bool hasWeapon[2] = {true, false}; // Começa com a arma 1 (Shotgun)
    int activeWeaponIdx = 0;
    float time = 0.0f;
    int currentLevel = 1;

    RenderAssets r;
};
