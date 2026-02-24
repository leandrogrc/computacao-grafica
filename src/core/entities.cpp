#include "core/entities.h"
#include "core/game.h"
#include "core/camera.h"
#include "audio/audio_system.h"
#include <cmath>

bool isWalkable(float x, float z)
{
    auto& lvl = gameLevel();
    float tile = lvl.metrics.tile;
    float offX = lvl.metrics.offsetX;
    float offZ = lvl.metrics.offsetZ;

    int tx = (int)((x - offX) / tile);
    int tz = (int)((z - offZ) / tile);

    const auto& data = lvl.map.data();

    if (tz < 0 || tz >= (int)data.size()) return false;
    if (tx < 0 || tx >= (int)data[tz].size()) return false;

    char c = data[tz][tx];
    if (c == '1' || c == '2' || c == 'M' || c == 'H' || c == 'S' || c == 'O') return false;
    // Entidades (h, a, k, j, t, m, g, E, 9) sao chao caminhavel
    return true;
}

static void spawnEnemyProjectile(float x, float z, float dx, float dz)
{
    float len = std::sqrt(dx*dx + dz*dz);
    if (len < 0.001f) return;
    
    Projectile p;
    p.x = x;
    p.z = z;
    p.dx = dx / len;
    p.dz = dz / len;
    p.speed = 8.0f;
    p.active = true;
    p.fromEnemy = true;
    
    gameLevel().projectiles.push_back(p);
}

void updateEntities(float dt)
{
    auto& g = gameContext();
    auto& lvl = gameLevel();
    auto& audio = gameAudio();

    for (auto& en : lvl.enemies)
    {
        if (en.state == STATE_DEAD)
        {
            en.respawnTimer -= dt;
            if (en.respawnTimer <= 0.0f)
            {
                en.state = STATE_IDLE;
                en.hp = 100;
                en.x = en.startX;
                en.z = en.startZ;
                en.hurtTimer = 0.0f;
            }
            continue;
        }

        if (en.hurtTimer > 0.0f) en.hurtTimer -= dt;

        float dx = camX - en.x;
        float dz = camZ - en.z;
        float dist = std::sqrt(dx * dx + dz * dz);

        switch (en.state)
        {
        case STATE_IDLE:
            if (dist < ENEMY_VIEW_DIST) en.state = STATE_CHASE;
            en.attackCooldown = 0.0f;
            break;

        case STATE_CHASE:
            if (dist < ENEMY_ATTACK_DIST)
            {
                en.state = STATE_ATTACK;
                en.attackCooldown = 0.5f;
            }
            else if (dist > ENEMY_VIEW_DIST * 1.5f)
            {
                en.state = STATE_IDLE;
            }
            else
            {
                float dirX = dx / dist;
                float dirZ = dz / dist;

                float moveStep = ENEMY_SPEED * dt;

                float nextX = en.x + dirX * moveStep;
                if (isWalkable(nextX, en.z)) en.x = nextX;

                float nextZ = en.z + dirZ * moveStep;
                if (isWalkable(en.x, nextZ)) en.z = nextZ;

                // NOVO: Chance de atirar projétil se estiver longe
                if (dist > ENEMY_ATTACK_DIST * 2.0f && dist < ENEMY_VIEW_DIST)
                {
                    en.attackCooldown -= dt;
                    if (en.attackCooldown <= 0.0f)
                    {
                        spawnEnemyProjectile(en.x, en.z, dx, dz);
                        en.attackCooldown = 2.0f + (std::rand() % 20) * 0.1f; // 2~4 seg
                    }
                }
            }
            break;

        case STATE_ATTACK:
            if (dist > ENEMY_ATTACK_DIST)
            {
                en.state = STATE_CHASE;
            }
            else
            {
                en.attackCooldown -= dt;
                if (en.attackCooldown <= 0.0f)
                {
                    g.player.health -= 10;
                    en.attackCooldown = 1.0f;
                    g.player.damageAlpha = 1.0f;
                    audioPlayHurt(audio);
                }
            }
            break;

        default:
            break;
        }
    }

    for (auto& item : lvl.items)
    {
        if (!item.active)
        {
            item.respawnTimer -= dt;
            if (item.respawnTimer <= 0.0f) item.active = true;
            continue;
        }

        float dx = camX - item.x;
        float dz = camZ - item.z;

        if (dx * dx + dz * dz < 1.0f)
        {
            item.active = false;

            if (item.type == ITEM_HEALTH)
            {
                item.respawnTimer = 15.0f;
                g.player.health += 50;
                if (g.player.health > 100) g.player.health = 100;
                g.player.healthAlpha = 1.0f;
            }
            else if (item.type == ITEM_AMMO)
            {
                item.respawnTimer = 999999.0f;
                g.player.reserveAmmo = 20;
            }
            else if (item.type == ITEM_CARD)
            {
                item.respawnTimer = 1e9f; // Nao respawna
                g.player.cardsCollected++;
            }
            else if (item.type == ITEM_BERSERK)
            {
                item.respawnTimer = 30.0f;
                g.player.berserkTimer = 10.0f;
            }
            else if (item.type == ITEM_HASTE)
            {
                item.respawnTimer = 30.0f;
                g.player.hasteTimer = 10.0f;
            }
        }
    }

    // 3. Atualizar Projéteis
    for (auto& p : lvl.projectiles)
    {
        if (!p.active) continue;

        p.x += p.dx * p.speed * dt;
        p.z += p.dz * p.speed * dt;

        // Colisão com parede
        if (!isWalkable(p.x, p.z))
        {
            p.active = false;
            continue;
        }

        // Colisão com player
        if (p.fromEnemy)
        {
            float pdx = p.x - camX;
            float pdz = p.z - camZ;
            if (pdx*pdx + pdz*pdz < 0.5f)
            {
                p.active = false;
                g.player.health -= 15;
                g.player.damageAlpha = 1.0f;
                audioPlayHurt(audio);
            }
        }
    }

    // 4. Decay de Timers
    if (g.player.berserkTimer > 0.0f) g.player.berserkTimer -= dt;
    if (g.player.hasteTimer > 0.0f) g.player.hasteTimer -= dt;
}
