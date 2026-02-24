#include "level/level.h"
#include "core/config.h" 
#include <cstdio>

// Configurações básicas para spawn
static const float ENEMY_START_HP = 100.0f;

bool loadLevel(Level &lvl, const char *mapPath, float tileSize)
{
    // 1. Carrega o mapa de texto (paredes, chão)
    if (!lvl.map.load(mapPath))
    {
        std::printf("ERRO: nao foi possivel carregar o mapa: %s\n", mapPath);
        return false;
    }

    lvl.metrics = LevelMetrics::fromMap(lvl.map, tileSize);
    
    // Limpa entidades antigas se houver
    lvl.enemies.clear();
    lvl.items.clear();
    lvl.hasExit = false;

    const auto& data = lvl.map.data();
    int H = lvl.map.getHeight();

    for(int z = 0; z < H; z++)
    {
        const std::string& row = data[z];
        for(int x = 0; x < (int)row.size(); x++)
        {
            char c = row[x];
            float wx, wz;
            lvl.metrics.tileCenter(x, z, wx, wz);

            // --- Inimigos ---
            int enemyType = -1;

            if (c == 'J' || c == 'j') enemyType = 0;
            else if (c == 'T' || c == 't') enemyType = 1;
            else if (c == 'm') enemyType = 2; // 'M' is Market facade
            else if (c == 'G' || c == 'g') enemyType = 3;

            if (enemyType != -1)
            {
                Enemy e;
                e.type = enemyType;
                e.x = wx;
                e.z = wz;
                e.startX = wx; 
                e.startZ = wz;
                e.respawnTimer = 0.0f;
                e.hp = ENEMY_START_HP;
                e.state = STATE_IDLE;
                e.animFrame = 0;
                e.animTimer = 0;
                e.hurtTimer = 0.0f;
                e.attackCooldown = 0.0f;
                lvl.enemies.push_back(e);
            }
            else if (c == 'I' || c == 'h') // Item Health / kit
            {
                Item i;
                i.x = wx;
                i.z = wz;
                i.type = ITEM_HEALTH;
                i.active = true;
                i.respawnTimer = 0.0f;
                lvl.items.push_back(i);
            }
            else if (c == 'A' || c == 'a') // Ammo
            {
                Item i;
                i.x = wx;
                i.z = wz;
                i.type = ITEM_AMMO;
                i.active = true;
                lvl.items.push_back(i);
            }
            else if (c == 'E') // Exit / saída de fase
            {
                lvl.hasExit = true;
                lvl.exitX = wx;
                lvl.exitZ = wz;
            }
        }
    }

    return true;
}

void applySpawn(const Level &lvl, float &camX, float &camZ)
{
    lvl.metrics.spawnPos(lvl.map, camX, camZ);
}