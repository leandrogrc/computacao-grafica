#include "graphics/lighting.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>

void setupIndoorLightOnce()
{
}

/**
 * Configuração global das luzes que representam o ambiente diurno ou global (Sol).
 * Chamada apenas uma vez na inicialização ou transição longa.
 */
void setupSunLightOnce()
{
    glEnable(GL_LIGHT0);
    // Escuridão Global - Muito pouca luz ambiente para tudo não ficar com pitch black absoluto.
    GLfloat sceneAmbient[] = {0.05f, 0.05f, 0.08f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, sceneAmbient);
    
    // Sem luz do Sol difusa para o clima "Alan Wake" prevalecer.
    GLfloat sunDiffuse[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat sunSpecular[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sunDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, sunSpecular);
    
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

/**
 * Usada para posicionar a luz global (se estivesse ativa) a cada frame.
 * Atualmente as cores difusas estão desligadas para manter o ambiente noturno/tenso.
 */
void setSunDirectionEachFrame()
{
    // A luz direcional zero global (Sol) foi inutilizada pelas cores difusas "zeradas" em setup.
    // Direção apontada apenas para evitar possíveis artefatos no math da GL.
    GLfloat sunDir[] = {0.0f, 1.0f, 0.0f, 0.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, sunDir);
}

/**
 * Aplica parâmetros de iluminação globais específicos para o ambiente de cada nível.
 * Controla também a coloração misteriosa e pulsações, criando o clima e atmosfera 
 * tensa da sobrevivência para as 3 diferentes fases (cidade escura, caverna e inferno).
 *
 * @param level Número da fase atual (1, 2 ou 3)
 * @param time Tempo global do jogo (usado no cálculo do seno para pulsos nas luzes)
 */
void applyLevelLighting(int level, float time)
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0); // Luz fraca da "Lua/Estrelas" (Apenas ambiente)
    glDisable(GL_LIGHT2);

    // Em todos os níveis, a luz difusa será nula e a ambiente absurdamente baixa
    // Forçando o jogador a depender 100% do novo Spotlight da Lanterna (GL_LIGHT1).

    if (level == 1)
    {
        // Fase 1: Cidade Obscura
        GLfloat amb[] = {0.08f, 0.08f, 0.12f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
        GLfloat diff[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
    }
    else if (level == 2)
    {
        // Fase 2: Subterrâneo - Breu Quase Absoluto
        GLfloat amb[] = {0.02f, 0.02f, 0.02f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
        GLfloat diff[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
        GLfloat dir[] = {0.0f, -1.0f, 0.0f, 0.0f};
        glLightfv(GL_LIGHT0, GL_POSITION, dir);
    }
    else if (level == 3)
    {
        // Fase 3: Inferno - Escuridão opressora levemente avermelhada
        float pulse = 0.5f + 0.5f * std::sin(time * 0.5f);
        GLfloat amb[] = {0.05f + 0.03f * pulse, 0.01f, 0.01f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
        GLfloat diff[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
        GLfloat dir[] = {0.0f, -1.0f, 0.0f, 0.0f};
        glLightfv(GL_LIGHT0, GL_POSITION, dir);
    }
}

/**
 * Configura e envia as propriedades cônicas (Spotlight) simulando uma Lanterna que
 * acompanha o Jogador. Transforma essa luz em uma "Halógena" branca focalizada que 
 * recorta a escuridão absoluta e exige o olhar do jogador.
 *
 * NOTA: Esta função PRECISA ser chamada APÓS `gluLookAt` para que as coordenadas do mundo 
 * se convertam em coordenadas da câmera de forma precisa.
 *
 * @param camX Posição em X da câmera do jogador
 * @param camY Posição vertical da câmera do jogador (levemente afundada simula segurar a lanterna)
 * @param camZ Posição em Z da câmera do jogador
 * @param dirX Vetor de olhar para o eixo X
 * @param dirY Vetor de olhar para o eixo Y
 * @param dirZ Vetor de olhar para o eixo Z
 */
void applyPlayerFlashlight(float camX, float camY, float camZ, float dirX, float dirY, float dirZ)
{
    glEnable(GL_LIGHT1);

    // 1. Cor Forte e Branca (Halógena fria)
    GLfloat lightAmbient[]  = {0.0f, 0.0f, 0.0f, 1.0f}; // Spotlight nao emite luz global pra cantos escuros
    GLfloat lightDiffuse[]  = {1.5f, 1.4f, 1.3f, 1.0f}; 
    GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};

    glLightfv(GL_LIGHT1, GL_AMBIENT,  lightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  lightDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);

    // 2. Posição (Onde a câmera está) - w = 1.0f pq é Positional (ponto local e nao infinito)
    GLfloat lightPos[] = {camX, camY, camZ, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos);

    // 3. O Cone do Sporlight
    GLfloat spotDir[] = {dirX, dirY, dirZ};
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDir);
    
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 25.0f); // Ângulo de corte da lanterna (25 graus pros lados)
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 15.0f); // Foco. Quanto maior, mais concentrada a borda da poça de luz.

    // 4. Atenuação (Decair distanciamento)
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION,  1.0f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION,    0.025f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.001f);
}
