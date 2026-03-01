#include "level/level.h"
#include "core/config.h" 
#include "core/game.h"
#include <cstdio>

// Configurações básicas para spawn
static const float ENEMY_START_HP = 100.0f;

/**
 * Função de Parser principal que lê o arquivo de texto (ex: mapa1.txt), gera as métricas de offset
 * e converte cada caractere do mapa num bloco 3D sólido ou numa entidade (Inimigo, Boss, Saída).
 * Também ajusta a dificuldade multiplicando o HP do monstro baseado no índice do nível atual.
 */
bool loadLevel(Level &lvl, const char *mapPath, float tileSize, int levelIndex)
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
        const auto& row = data[z];
        for(int x = 0; x < (int)row.size(); x++)
        {
            char c = row[x];
            float wx, wz;
            lvl.metrics.tileCenter(x, z, wx, wz);

            // --- Inimigos ---
            int enemyType = -1;
            bool isBoss = false;

            if (c == 'J' || c == 'j') enemyType = 0;
            else if (c == 'T' || c == 't') enemyType = 1;
            else if (c == 'm') enemyType = 2; // 'M' is Market facade
            else if (c == 'B' || c == 'b') { enemyType = 4; isBoss = true; }
            else if (c == 'G' || c == 'g') enemyType = 3;

            if (enemyType != -1)
            {
                Enemy e;
                e.type = enemyType;
                e.isBoss = isBoss;
                e.x = wx;
                e.z = wz;
                e.startX = wx; 
                e.startZ = wz;
                e.respawnTimer = 0.0f;
                // Multiplicador de HP baseado na Fase:
                // Fase 1 = 1x
                // Fase 2 = 1.3x
                // Fase 3 = 1.6x
                float hpMultiplier = 1.0f + ((float)(levelIndex - 1) * 0.3f);
                e.hp = (isBoss ? 500.0f : ENEMY_START_HP) * hpMultiplier;
                e.state = STATE_IDLE;
                e.animFrame = 0;
                e.animTimer = 0;
                e.hurtTimer = 0.0f;
                e.attackCooldown = 0.0f;
                lvl.enemies.push_back(e);
            }
            else if (c == 'I' || c == 'h') // Item Health / kit
            {
                // Removido: Itens servem apenas como drops agora
            }
            else if (c == 'A' || c == 'a') // Ammo
            {
                // Removido: Itens servem apenas como drops agora
            }
            else if (c == 'C' || c == 'c') // Item Card
            {
                // Removido: Cartões agora são apenas dropados a cada 5 mortes
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

/**
 * Procura pelo caractere 'P' ou 'p' no mapa carregado para ancorar as coordenadas iniciais 
 * da câmera do jogador (Spawn Point X e Z).
 */
void applySpawn(const Level &lvl, float &camX, float &camZ)
{
    lvl.metrics.spawnPos(lvl.map, camX, camZ);
}