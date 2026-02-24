#pragma once
#include <GL/glew.h>

struct GameAssets
{
    // texturas
    GLuint texMenuBG = 0;
    GLuint texChao = 0;
    GLuint texParede = 0;
    GLuint texSangue = 0;
    GLuint texLava = 0;
    GLuint texChaoInterno = 0;
    GLuint texParedeInterna = 0;
    GLuint texTeto = 0;
    GLuint texEnemy = 0;
    GLuint texEnemyRage = 0;    // Viu o player
    GLuint texEnemyDamage = 0;  // Leva dano
    GLuint texHealthOverlay = 0; // Tela de cura
    GLuint texHealth = 0;
    GLuint texAmmo = 0;
    GLuint texGunDefault = 0;
    GLuint texGunFire1 = 0;
    GLuint texGunFire2 = 0;
    GLuint texGunReload1 = 0;
    GLuint texGunReload2 = 0;
    GLuint texGunSprint = 0;
    GLuint texDamage = 0;
    GLuint texGunHUD = 0;
    GLuint texGun2Default = 0;
    GLuint texGun2Fire1 = 0;
    GLuint texGun2Fire2 = 0;
    GLuint texHudTopBG = 0;
    GLuint texCard = 0;
    GLuint texHudFundo = 0;

    GLuint texEnemies[5]       = {0, 0, 0, 0, 0};
    GLuint texEnemiesRage[5]   = {0, 0, 0, 0, 0};
    GLuint texEnemiesDamage[5] = {0, 0, 0, 0, 0};


    // shaders
    GLuint progSangue = 0;
    GLuint progLava = 0;

    GLuint texSkydome = 0;

    // Texturas por fase (index 0=fase1, 1=fase2, 2=fase3)
    GLuint texChaoNivel[3]         = {0, 0, 0};
    GLuint texParedeNivel[3]       = {0, 0, 0};
    GLuint texChaoInternoNivel[3]  = {0, 0, 0};
    GLuint texParedeInterNivel[3]  = {0, 0, 0};
    GLuint texSkydomeNivel[3]      = {0, 0, 0};

    GLuint texWallMarket = 0;
    GLuint texWallHouse  = 0;
    GLuint texWallShop   = 0;
    GLuint texWallOffice = 0;
};

bool loadAssets(GameAssets &a);
