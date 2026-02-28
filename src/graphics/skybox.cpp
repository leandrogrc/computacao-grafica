#include "graphics/skybox.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>
#include "core/game_state.h"

// Limpando funcoes antigas nao utilizadas

void drawSkydome(float camX, float camY, float camZ, const RenderAssets& r)
{
    const float R = 200.0f; // Tamanho da caixa do céu

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_TEXTURE_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE); // O céu fica no fundo de tudo
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, r.texSkydome);
    
    // Configuração para evitar bordas feias entre as faces
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glPushMatrix();
    glTranslatef(camX, camY, camZ); // O céu segue o jogador

    glBegin(GL_QUADS);

    // Mapeamos a textura panorâmica nas 4 paredes do cubo (0 a 1 em U)
    // Frente (Z negativo) - U: 0.00 a 0.25
    glTexCoord2f(0.25f, 0.0f); glVertex3f( R,  R, -R);
    glTexCoord2f(0.00f, 0.0f); glVertex3f(-R,  R, -R);
    glTexCoord2f(0.00f, 1.0f); glVertex3f(-R, -R, -R);
    glTexCoord2f(0.25f, 1.0f); glVertex3f( R, -R, -R);

    // Direita (X positivo) - U: 0.25 a 0.50
    glTexCoord2f(0.50f, 0.0f); glVertex3f( R,  R,  R);
    glTexCoord2f(0.25f, 0.0f); glVertex3f( R,  R, -R);
    glTexCoord2f(0.25f, 1.0f); glVertex3f( R, -R, -R);
    glTexCoord2f(0.50f, 1.0f); glVertex3f( R, -R,  R);

    // Trás (Z positivo) - U: 0.50 a 0.75
    glTexCoord2f(0.75f, 0.0f); glVertex3f(-R,  R,  R);
    glTexCoord2f(0.50f, 0.0f); glVertex3f( R,  R,  R);
    glTexCoord2f(0.50f, 1.0f); glVertex3f( R, -R,  R);
    glTexCoord2f(0.75f, 1.0f); glVertex3f(-R, -R,  R);

    // Esquerda (X negativo) - U: 0.75 a 1.00
    glTexCoord2f(1.00f, 0.0f); glVertex3f(-R,  R, -R);
    glTexCoord2f(0.75f, 0.0f); glVertex3f(-R,  R,  R);
    glTexCoord2f(0.75f, 1.0f); glVertex3f(-R, -R,  R);
    glTexCoord2f(1.00f, 1.0f); glVertex3f(-R, -R, -R);

    // Teto (Topo) - Esticamos a borda superior da imagem
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-R,  R, -R);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( R,  R, -R);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( R,  R,  R);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-R,  R,  R);

    // Chão (Fundo) - Esticamos a borda inferior
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-R, -R,  R);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( R, -R,  R);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( R, -R, -R);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-R, -R, -R);

    glEnd();

    glPopMatrix();
    
    glDepthMask(GL_TRUE);
    glPopAttrib();
}