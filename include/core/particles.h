#ifndef PARTICLES_H
#define PARTICLES_H

/**
 * Sistema de Partículas Dinâmicas Químicas/Visuais em 3D.
 * Gerencia uma pool de Billboards para efeitos de Faísca de Parede e Sangue de Monstros.
 */

enum class ParticleType {
    BLOOD // Escorre vermelho
};

struct Particle {
    bool active;
    ParticleType type;

    float x, y, z;          // Posição no mundo
    float vx, vy, vz;       // Vetor de Velocidade (Gravidade atua em VY)
    
    float r, g, b, a;       // Cor (RGBA)
    float size;             // Tamanho escalar do billboard
    
    float life;             // Tempo de vida restante em MS
    float maxLife;          // Tempo de vida total (para calcular o decaimento linear de Alpha)
};

// --- Funções Globais do Sistema ---

// Inicializa a pool de partículas (zerando tudo)
void initParticles();

// Cria uma explosão/jorro de partículas num ponto exato do espaço
// count = quantidade gerada.
void spawnParticles(int count, float x, float y, float z, ParticleType type);

// Atualiza a gravidade, movimento e decadência Alpha das partículas ativas baseada em DeltaTime
void updateParticles(float dt);

// Função de renderização para ser chamada em drawWorld3D após os Monstros/Sprites.
void drawParticles(float camX, float camY, float camZ);

#endif // PARTICLES_H
