#pragma once

#include "core/game_enums.h"
#include <GL/glew.h>
#include <vector>

class MapLoader;
struct Enemy;
struct Item;


struct HudTextures
{
    GLuint texHudFundo = 0;
    GLuint texHudTopBG = 0;
    GLuint texGunHUD = 0;

    GLuint texGunDefault = 0;
    GLuint texGunFire1 = 0;
    GLuint texGunFire2 = 0;
    GLuint texGunReload1 = 0;
    GLuint texGunReload2 = 0;
    GLuint texGunSprint = 0;
    GLuint texGun2Default = 0;
    GLuint texGun2Fire1 = 0;
    GLuint texGun2Fire2 = 0;

    GLuint texDamage = 0;
    GLuint texHealthOverlay = 0;
};

struct HudState
{
    int playerHealth = 100;
    int currentAmmo = 0;
    int reserveAmmo = 0;

    float damageAlpha = 0.0f;
    float healthAlpha = 0.0f;

    int cardsCollected = 0;

    float berserkTimer = 0.0f;
    float hasteTimer = 0.0f;

    WeaponState weaponState = WeaponState::W_IDLE;
    int activeWeaponIdx = 0;

    float px = 0;
    float pz = 0;
    float pyaw = 0;

    bool isSprinting = false;
};

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
    bool showDoomBar);
