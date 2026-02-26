#include "utils/assets.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include <cstdio>

bool loadAssets(GameAssets &a)
{
    a.texMenuBG = carregaTextura("assets/menu_bg.png");

    a.texChao = carregaTextura("assets/levels/floor1.png");
    a.texParede = carregaTextura("assets/levels/wall1.png");
    a.texSangue = carregaTextura("assets/016.png");
    a.texLava = carregaTextura("assets/179.png");
    a.texChaoInterno = carregaTextura("assets/100.png");
    a.texParedeInterna = carregaTextura("assets/060.png");
    a.texTeto = carregaTextura("assets/081.png");

    a.progSangue = criaShader("shaders/blood.vert", "shaders/blood.frag");
    a.progLava = criaShader("shaders/lava.vert", "shaders/lava.frag");
    a.texMenuBG = carregaTextura("assets/menu_bg.png");

    // --- Texturas por fase ---
    // Fase 1: Cidade Apocalíptica
    a.texChaoNivel[0]        = carregaTextura("assets/levels/floor1.png");
    a.texParedeNivel[0]      = carregaTextura("assets/levels/wall1.png");
    a.texChaoInternoNivel[0] = carregaTextura("assets/levels/floor1.png");
    a.texParedeInterNivel[0] = carregaTextura("assets/levels/wall1.png");
    a.texSkydomeNivel[0]     = carregaTextura("assets/levels/sky1.png");
    // Fase 2: Esgoto / Subterrâneo
    a.texChaoNivel[1]        = carregaTextura("assets/levels/floor2.png");
    a.texParedeNivel[1]      = carregaTextura("assets/levels/wall2.png");
    a.texChaoInternoNivel[1] = carregaTextura("assets/levels/floor2.png");
    a.texParedeInterNivel[1] = carregaTextura("assets/levels/wall2.png");
    a.texSkydomeNivel[1]     = carregaTextura("assets/levels/sky2.png");
    // Fase 3: Inferno
    a.texChaoNivel[2]        = carregaTextura("assets/levels/floor3.png");
    a.texParedeNivel[2]      = carregaTextura("assets/levels/wall3.png");
    a.texChaoInternoNivel[2] = carregaTextura("assets/levels/floor3.png");
    a.texParedeInterNivel[2] = carregaTextura("assets/levels/wall3.png");
    a.texSkydomeNivel[2]     = carregaTextura("assets/levels/sky3.png");

    // --- INIMIGO 0 ('J') ---
    a.texEnemies[0] = carregaTextura("assets/enemies/enemy.png");
    a.texEnemiesRage[0] = carregaTextura("assets/enemies/enemyRage.png");
    a.texEnemiesDamage[0] = carregaTextura("assets/enemies/enemyRageDamage.png");

    // --- INIMIGO 1 ('T') ---
    a.texEnemies[1] = carregaTextura("assets/enemies/enemy2.png");
    a.texEnemiesRage[1] = carregaTextura("assets/enemies/enemyRage2.png");
    a.texEnemiesDamage[1] = carregaTextura("assets/enemies/enemyRageDamage2.png");

    // --- INIMIGO 2 ('M') ---
    a.texEnemies[2] = carregaTextura("assets/enemies/enemy3.png");
    a.texEnemiesRage[2] = carregaTextura("assets/enemies/enemyRage3.png");
    a.texEnemiesDamage[2] = carregaTextura("assets/enemies/enemyRageDamage3.png");

    // --- INIMIGO 3 ('K') ---
    a.texEnemies[3] = carregaTextura("assets/enemies/enemy4.png");
    a.texEnemiesRage[3] = carregaTextura("assets/enemies/enemyRage4.png");
    a.texEnemiesDamage[3] = carregaTextura("assets/enemies/enemyRageDamage4.png");
    // --- INIMIGO 4 ('G') ---
    a.texEnemies[4] = carregaTextura("assets/enemies/enemy5.png");
    a.texEnemiesRage[4] = carregaTextura("assets/enemies/enemyRage5.png");
    a.texEnemiesDamage[4] = carregaTextura("assets/enemies/enemyRageDamage5.png");

    a.texHealthOverlay = carregaTextura("assets/heal.png");
    a.texGunDefault = carregaTextura("assets/gun_default.png");
    a.texGunFire1 = carregaTextura("assets/gun_fire1.png");
    a.texGunFire2 = carregaTextura("assets/gun_fire2.png");
    a.texGunReload1 = carregaTextura("assets/gun_reload1.png");
    a.texGunReload2 = carregaTextura("assets/gun_reload2.png");
    a.texGunSprint = carregaTextura("assets/gun_sprint.png");
    a.texDamage = carregaTextura("assets/damage.png");
    a.texGun2Default = carregaTextura("assets/plasma_idle.png");
    a.texGun2Fire1 = carregaTextura("assets/plasma_fire1.png");
    a.texGun2Fire2 = carregaTextura("assets/plasma_fire2.png");
    a.texGun2Reload1 = carregaTextura("assets/plasma_reload.png");

    a.texHealth = carregaTextura("assets/health.png");
    a.texAmmo = carregaTextura("assets/066.png");
    a.texBerserk = carregaTextura("assets/item_berserk.png");
    a.texHaste = carregaTextura("assets/item_haste.png");

    a.texSkydome = carregaTextura("assets/levels/sky1.png");

    a.texGunHUD = carregaTextura("assets/Shotgun.png");
    a.texHudFundo = carregaTextura("assets/088.png");


    a.texWallMarket = carregaTextura("assets/levels/wall_market.png");
    a.texWallHouse  = carregaTextura("assets/levels/wall_house.png");
    a.texWallShop   = carregaTextura("assets/levels/wall_shop.png");
    a.texWallOffice = carregaTextura("assets/levels/wall_office.png");
    a.texHudTopBG   = carregaTextura("assets/hud_top_bg.png");
    a.texCard       = carregaTextura("assets/item_card.png");

    if (!a.texChao || !a.texParede || !a.texSangue || !a.texLava || !a.progSangue ||
        !a.progLava || !a.texHealth || !a.texGunDefault || !a.texGunFire1 ||
        !a.texGunFire2 || !a.texSkydome || !a.texGunReload1 || !a.texGunReload2 ||
        !a.texDamage || !a.texAmmo || !a.texHealthOverlay || !a.texEnemies[0] ||
        !a.texEnemiesRage[0] || !a.texEnemiesDamage[0] || !a.texEnemies[1] ||
        !a.texEnemiesRage[1] || !a.texEnemiesDamage[1] || !a.texEnemies[2] ||
        !a.texEnemiesRage[2] || !a.texEnemiesDamage[2] || !a.texGunHUD || !a.texHudFundo || !a.texMenuBG ||
        !a.texGun2Default || !a.texGun2Fire1 || !a.texGun2Fire2 || !a.texGun2Reload1)
    {
        std::printf("ERRO: falha ao carregar algum asset (textura/shader).\n");
        return false;
    }
    return true;
}
