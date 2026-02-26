#include "core/player.h"
#include "core/game.h"
#include "core/camera.h"
#include "audio/audio_system.h"
#include "input/keystate.h"
#include <cmath>

constexpr int MAX_MAGAZINE = 12;

// Ajuste fino: raio do hitbox do inimigo no chão (mundo XZ)
static constexpr float HIT_RADIUS = 0.55f;

// Ajuste fino: alcance máximo do tiro no mundo
static constexpr float MAX_RANGE  = 17.0f;

static bool rayCircleIntersectXZ(
    float ox, float oz,     // origem do raio
    float dx, float dz,     // direção NORMALIZADA
    float cx, float cz,     // centro do inimigo
    float r,                // raio da hitbox
    float &outT             // distância ao longo do raio
)
{
    // vetor da origem até o centro
    float fx = cx - ox;
    float fz = cz - oz;

    // projeção no raio
    float proj = fx * dx + fz * dz;
    if (proj < 0.0f)
        return false; // inimigo atrás

    // ponto mais próximo no raio
    float px = ox + dx * proj;
    float pz = oz + dz * proj;

    // distância do centro ao ponto mais próximo
    float ex = cx - px;
    float ez = cz - pz;
    float d2 = ex * ex + ez * ez;

    float r2 = r * r;
    if (d2 > r2)
        return false; // não cruza o círculo

    // calcula ponto de entrada na circunferência (o mais próximo)
    float thc = std::sqrt(r2 - d2);
    float t0 = proj - thc; // entrada
    float t1 = proj + thc; // saída

    // se estiver dentro do círculo, t0 pode ser negativo
    outT = (t0 >= 0.0f) ? t0 : t1;
    return outT >= 0.0f;
}

void playerTryReload()
{
    auto &g = gameContext();
    auto &audio = gameAudio();
    auto &weapon = g.weapons[g.activeWeaponIdx];

    if (weapon.state != WeaponState::W_IDLE)
        return;
    if (g.player.currentAmmo >= MAX_MAGAZINE)
        return;
    if (g.player.reserveAmmo <= 0)
        return;

    weapon.state = WeaponState::W_RELOAD_1;
    weapon.timer = 0.50f;

    audioPlayReload(audio);
}

void playerTryAttack()
{
    auto &g = gameContext();
    auto &lvl = gameLevel();
    auto &audio = gameAudio();
    auto &weapon = g.weapons[g.activeWeaponIdx];

    if (weapon.state != WeaponState::W_IDLE)
        return;
    if (g.player.currentAmmo <= 0)
        return;
    if (keyShift)
        return;

    g.player.currentAmmo--;

    audioOnPlayerShot(audio);
    audioPlayShot(audio);

    weapon.state = WeaponState::W_FIRE_1;
    if (g.activeWeaponIdx == 0)
        weapon.timer = 0.08f;
    else
        weapon.timer = 0.05f; // Arma 2 é mais rápida

    // 1) raio sai do centro da visão do player (yaw)
    float radYaw = yaw * 3.14159f / 180.0f;
    float dirX = std::sin(radYaw);
    float dirZ = -std::cos(radYaw);

    // normaliza direção
    float len = std::sqrt(dirX * dirX + dirZ * dirZ);
    if (len <= 0.0f)
        return;
    dirX /= len;
    dirZ /= len;

    // 2) procura o inimigo mais próximo que o raio intersecta
    int bestIdx = -1;
    float bestT = MAX_RANGE;

    for (int i = 0; i < (int)lvl.enemies.size(); ++i)
    {
        auto &en = lvl.enemies[i];
        if (en.state == STATE_DEAD)
            continue;

        float tHit = 0.0f;
        if (rayCircleIntersectXZ(camX, camZ, dirX, dirZ, en.x, en.z, HIT_RADIUS, tHit))
        {
            if (tHit <= bestT)
            {
                bestT = tHit;
                bestIdx = i;
            }
        }
    }

    // 3) se acertou alguém, aplica dano; se não, não faz nada
    if (bestIdx >= 0)
    {
        auto &en = lvl.enemies[bestIdx];

        int damage = 30;
        if (g.player.berserkTimer > 0.0f) damage *= 2;

        en.hp -= damage;
        en.hurtTimer = 0.5f;

        if (en.hp <= 0)
        {
            en.state = STATE_DEAD;
            en.respawnTimer = 15.0f;

            Item drop;
            drop.x = en.x;
            drop.z = en.z;
            drop.active = true;
            drop.respawnTimer = 0.0f;

            // Logica de drop re-balanceada:
            // Prioridade: Municao (55%) > Vida (20%) > Arma (15%, 1x) > Cartao (10%, se precisar)
            int chance = std::rand() % 100;

            if (!g.hasWeapon[1] && chance < 10) // 10% chance unica de dar a arma
            {
                drop.type = ITEM_WEAPON2;
            }
            else if (chance >= 97) // 3% para Berserk
            {
                drop.type = ITEM_BERSERK;
            }
            else if (chance >= 94) // 3% para Haste
            {
                drop.type = ITEM_HASTE;
            }
            else if (g.player.cardsCollected < 3 && chance >= 85) // ~9% para cartao
            {
                drop.type = ITEM_CARD;
            }
            else if (chance >= 75 && g.player.health < 100) // ~10% para vida
            {
                drop.type = ITEM_HEALTH;
            }
            else // ~65%+ para municao (default)
            {
                drop.type = ITEM_AMMO;
            }

            lvl.items.push_back(drop);
        }
    }
}

void updateWeaponAnim(float dt)
{
    auto &g = gameContext();
    auto &audio = gameAudio();

    for (int i = 0; i < 2; ++i)
    {
        auto &weapon = g.weapons[i];
        if (weapon.state == WeaponState::W_IDLE)
            continue;

        weapon.timer -= dt;
        if (weapon.timer > 0.0f)
            continue;

        if (weapon.state == WeaponState::W_FIRE_1)
        {
            weapon.state = WeaponState::W_FIRE_2;
            weapon.timer = (i == 0) ? 0.12f : 0.08f;
        }
        else if (weapon.state == WeaponState::W_FIRE_2)
        {
            if (i == 0) // Só a Shotgun (arma 0) faz o pump
            {
                weapon.state = WeaponState::W_PUMP;
                weapon.timer = AudioTuning::PUMP_TIME;
                audioPlayPumpClick(audio);
            }
            else
            {
                weapon.state = WeaponState::W_IDLE;
                weapon.timer = 0.0f;
            }
        }
        else if (weapon.state == WeaponState::W_PUMP)
        {
            weapon.state = WeaponState::W_IDLE;
            weapon.timer = 0.0f;
        }
        else if (weapon.state == WeaponState::W_RELOAD_1)
        {
            weapon.state = WeaponState::W_RELOAD_2;
            weapon.timer = 0.85f;
        }
        else if (weapon.state == WeaponState::W_RELOAD_2)
        {
            weapon.state = WeaponState::W_RELOAD_3;
            weapon.timer = 0.25f;
        }
        else if (weapon.state == WeaponState::W_RELOAD_3)
        {
            weapon.state = WeaponState::W_IDLE;
            weapon.timer = 0.0f;

            int needed = MAX_MAGAZINE - g.player.currentAmmo;
            if (needed > g.player.reserveAmmo)
                needed = g.player.reserveAmmo;

            g.player.currentAmmo += needed;
            g.player.reserveAmmo -= needed;
        }
        else if (weapon.state == WeaponState::W_RETURN)
        {
            weapon.state = WeaponState::W_IDLE;
            weapon.timer = 0.0f;
        }
    }
}

void playerSwitchWeapon(int idx)
{
    auto &g = gameContext();
    if (idx < 0 || idx >= 2) return;
    if (!g.hasWeapon[idx]) return;
    if (g.activeWeaponIdx == idx) return;

    // Se estiver no meio de uma animação, talvez devesse bloquear? 
    // Por enquanto troca direto se idle
    if (g.weapons[g.activeWeaponIdx].state != WeaponState::W_IDLE) return;

    g.activeWeaponIdx = idx;
}

void playerTryGrabWeapon()
{
    auto &g = gameContext();
    auto &lvl = gameLevel();

    float grabDist = 1.5f;

    for (auto &it : lvl.items)
    {
        if (!it.active || it.type != ITEM_WEAPON2) continue;

        float dx = it.x - camX;
        float dz = it.z - camZ;
        float dist = std::sqrt(dx * dx + dz * dz);

        if (dist <= grabDist)
        {
            it.active = false;
            g.hasWeapon[1] = true;
            g.activeWeaponIdx = 1; // Troca automaticamente ao pegar
            std::printf("Arma secundaria obtida!\n");
            return;
        }
    }
}
