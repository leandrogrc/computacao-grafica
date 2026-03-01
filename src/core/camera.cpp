#include "core/camera.h"

float camX = 0.0f;
float camY = 1.5f;
float camZ = 25.0f;

float yaw = 0.0f;
float pitch = 0.0f;

/**
 * Altera forçadamente a posição cardinal X,Y,Z e os ângulos vetoriais Pitch e Yaw da 
 * Câmera do Jogador. Muito utilizado pelo Sistema de Níveis para Spawnar o player num mapa.
 */
void setCamera(float x, float y, float z, float newYaw, float newPitch)
{
    camX = x;
    camY = y;
    camZ = z;
    yaw = newYaw;
    pitch = newPitch;
}
