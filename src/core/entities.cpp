#include "core/entities.h"
#include "core/game.h"
#include "core/camera.h"
#include "audio/audio_system.h"
#include <cmath>

/**
 * Testa se uma coordenada tridimensional (X, Z) do mundo de jogo cai dentro de um bloco sólido 
 * (parede, estante, máquina) ou num corredor onde o player/inimigos podem atravessar.
 * Ajuda a impedir que os monstros entrem atráves de paredes (Clipping).
 */
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

/**
 * Gira o tiro do inimigo na direção do jogador de forma estática (Projectile Instancing).
 * É disparado geralmente por inimigos tipo ranged ou quando o player está muito distante 
 * e o inimigo tentou "prever" onde o alvo estaria.
 */
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

/**
 * Função Core que itera em 'delta time' todas as variáveis físicas das listas dinâmicas de 
 * inimigos (Npcs), Itens espalhados pelo chão e os Projéteis voando pelo ar.
 * Aplica lógica de IA: (Idle -> Chase -> Attack) para os Monstros;
 * Física de Coleta circular para os itens: cura vida, municão, cartões;
 * Movimenta os feixes de projétil ou destrói quando encontram paredes ou acertam o jogador.
 */
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
            if (dist < ENEMY_VIEW_DIST || en.isBoss) en.state = STATE_CHASE;
            en.attackCooldown = 0.0f;
            break;

        case STATE_CHASE:
            if (dist < ENEMY_ATTACK_DIST)
            {
                en.state = STATE_ATTACK;
                en.attackCooldown = 0.5f;
            }
            else if (dist > ENEMY_VIEW_DIST * 1.5f && !en.isBoss)
            {
                en.state = STATE_IDLE;
            }
            else
            {
                float dirX = dx / dist;
                float dirZ = dz / dist;

                // Mapa 1 = 1x, Mapa 2 = 1.15x, Mapa 3 = 1.30x
                float speedMultiplier = 1.0f + ((float)(g.currentLevel - 1) * 0.15f);
                float moveStep = (ENEMY_SPEED * speedMultiplier) * dt;

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
                    // Fase 1: 10 de dano, 1.0s cooldown
                    // Fase 2: 15 de dano, 0.85s cooldown
                    // Fase 3: 20 de dano, 0.70s cooldown
                    int baseDamage = 10;
                    float dmgMultiplier = 1.0f + ((float)(g.currentLevel - 1) * 0.5f);
                    float speedBonus = ((float)(g.currentLevel - 1) * 0.15f);

                    g.player.health -= (int)(baseDamage * dmgMultiplier);
                    en.attackCooldown = 1.0f - speedBonus; 
                    
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
            continue;

        float dx = camX - item.x;
        float dz = camZ - item.z;

        if (dx * dx + dz * dz < 1.0f)
        {
            if (item.type == ITEM_CARD && g.player.cardsCollected >= 3) {
                continue; // Limite atingido, não coleta
            }

            item.active = false;

            if (item.type == ITEM_HEALTH)
            {
                g.player.health += 50;
                if (g.player.health > 100) g.player.health = 100;
                g.player.healthAlpha = 1.0f;
            }
            else if (item.type == ITEM_AMMO)
            {
                g.player.reserveAmmo += 12; // Ganha +1 pente
                if (g.player.reserveAmmo > 60) g.player.reserveAmmo = 60; // Limite máximo de munição reserva
            }
            else if (item.type == ITEM_CARD)
            {
                g.player.cardsCollected++;
            }
            else if (item.type == ITEM_BERSERK)
            {
                g.player.berserkTimer = 10.0f;
            }
            else if (item.type == ITEM_HASTE)
            {
                g.player.hasteTimer = 10.0f;
            }
            else if (item.type == ITEM_WEAPON2)
            {
                if (!g.hasWeapon[1]) {
                    g.hasWeapon[1] = true;
                    g.activeWeaponIdx = 1;
                    std::printf("Arma secundaria obtida!\n");
                }
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
