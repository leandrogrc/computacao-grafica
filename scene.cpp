#include <GL/glut.h>
#include <math.h>
#include "scene.h"

// ==========================================
// 1. DEFINIÇÃO DE VARIÁVEIS GLOBAIS
// ==========================================

// Variável da animação da roda
float anguloRoda = 0.0f;

// Variáveis da Janela
int janelaW = 1920;
int janelaH = 1080;
int centerX = janelaW / 2;
int centerY = janelaH / 2;
bool fullScreen = false;

// Variáveis da Câmera
float camX = 0.0f;
float camY = 1.5f;
float camZ = 25.0f;
float yaw = 0.0f;
float pitch = 0.0f;

// Controle do Mouse
bool ignoreWarp = false;
bool firstMouse = true;

// Teclas
bool keyW = false;
bool keyA = false;
bool keyS = false;
bool keyD = false;

// ==========================================
// 2. FUNÇÕES DE GERENCIAMENTO DE JANELA
// ==========================================

void atualizaCentroJanela(int w, int h)
{
    centerX = w / 2;
    centerY = h / 2;
}

void altFullScreen()
{
    firstMouse = true;

    if (!fullScreen)
    {
        glutFullScreen();
        fullScreen = true;
    }
    else
    {
        glutReshapeWindow(janelaW, janelaH);
        fullScreen = false;
    }
}

// ==========================================
// 3. FUNÇÕES DE DESENHO DA BICICLETA
// ==========================================

void desenhaRoda()
{
    glPushMatrix();
    // Gira a roda baseada na variável global atualizada no timer
    glRotatef(anguloRoda, 0.0f, 0.0f, 1.0f);

    glColor3f(0.1f, 0.1f, 0.1f); // Pneu escuro
    glutSolidTorus(0.05, 0.3, 10, 20); 

    // Raios da roda
    glColor3f(0.7f, 0.7f, 0.7f); // Prata
    glPushMatrix();
    glScalef(0.5f, 0.02f, 0.02f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glScalef(0.02f, 0.5f, 0.02f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPopMatrix();
}

void desenhaBicicleta()
{
    // Rodas
    glPushMatrix(); glTranslatef(-0.5f, 0.0f, 0.0f); desenhaRoda(); glPopMatrix();
    glPushMatrix(); glTranslatef(0.5f, 0.0f, 0.0f); desenhaRoda(); glPopMatrix();

    // Quadro (Amarelo)
    glColor3f(1.0f, 1.0f, 0.0f); 

    // Barra inferior
    glPushMatrix(); glScalef(1.0f, 0.05f, 0.05f); glutSolidCube(1.0f); glPopMatrix();

    // Barra vertical
    glPushMatrix(); glTranslatef(-0.2f, 0.25f, 0.0f); glRotatef(-15.0f, 0.0f, 0.0f, 1.0f); 
    glScalef(0.05f, 0.6f, 0.05f); glutSolidCube(1.0f); glPopMatrix();

    // Barra diagonal
    glPushMatrix(); glTranslatef(0.15f, 0.25f, 0.0f); glRotatef(45.0f, 0.0f, 0.0f, 1.0f); 
    glScalef(0.05f, 0.7f, 0.05f); glutSolidCube(1.0f); glPopMatrix();

    // Guidão
    glColor3f(0.6f, 0.6f, 0.6f);
    glPushMatrix(); glTranslatef(0.4f, 0.5f, 0.0f); glRotatef(90.0f, 1.0f, 0.0f, 0.0f); 
    glScalef(0.05f, 0.6f, 0.05f); glutSolidCube(1.0f); glPopMatrix();

    // Selim
    glColor3f(0.2f, 0.2f, 0.2f);
    glPushMatrix(); glTranslatef(-0.25f, 0.55f, 0.0f); glScalef(0.2f, 0.05f, 0.15f); 
    glutSolidCube(1.0f); glPopMatrix();
}

// ==========================================
// 4. FUNÇÕES DE DESENHO DO CENÁRIO
// ==========================================

#define NUM_TORRES 5
#define RAIO 10.0f

static void desenhaLosango(float altura)
{
    float h = altura / 2.0f;
    float s = altura / 3.0f;
    float claro[3] = {0.3f, 1.0f, 0.3f};
    float escuro[3] = {0.0f, 0.6f, 0.0f};

    glBegin(GL_TRIANGLES);
    glColor3fv(claro);  glVertex3f(0.0f, h, 0.0f); glVertex3f(-s, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, s);
    glColor3fv(escuro); glVertex3f(0.0f, h, 0.0f); glVertex3f(0.0f, 0.0f, s); glVertex3f(s, 0.0f, 0.0f);
    glColor3fv(claro);  glVertex3f(0.0f, h, 0.0f); glVertex3f(s, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, -s);
    glColor3fv(escuro); glVertex3f(0.0f, h, 0.0f); glVertex3f(0.0f, 0.0f, -s); glVertex3f(-s, 0.0f, 0.0f);
    glColor3fv(claro);  glVertex3f(0.0f, -h, 0.0f); glVertex3f(0.0f, 0.0f, s); glVertex3f(-s, 0.0f, 0.0f);
    glColor3fv(escuro); glVertex3f(0.0f, -h, 0.0f); glVertex3f(s, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, s);
    glColor3fv(claro);  glVertex3f(0.0f, -h, 0.0f); glVertex3f(0.0f, 0.0f, -s); glVertex3f(s, 0.0f, 0.0f);
    glColor3fv(escuro); glVertex3f(0.0f, -h, 0.0f); glVertex3f(-s, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, -s);
    glEnd();
}

void desenhaChao()
{
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(-80.0f, 0.0f, -80.0f);
    glVertex3f(80.0f, 0.0f, -80.0f);
    glVertex3f(80.0f, 0.0f, 80.0f);
    glVertex3f(-80.0f, 0.0f, 80.0f);
    glEnd();
}

void desenhaTorresELosangos()
{
    float alturaTorre = 2.5f;
    float w = 0.7f;
    float ang0 = -M_PI / 2.0f;
    float passo = 2.0f * M_PI / NUM_TORRES;

    for (int i = 0; i < NUM_TORRES; i++)
    {
        float ang = ang0 + passo * i;
        float x = RAIO * cosf(ang);
        float z = RAIO * sinf(ang);

        glPushMatrix();
        glTranslatef(x, 0.0f, z);

        // Torre
        glPushMatrix();
        glColor3f(0.6f, 0.0f, 0.8f);
        glTranslatef(0.0f, alturaTorre / 2.0f, 0.0f);
        glScalef(w, alturaTorre, w);
        glutSolidCube(1.0f);
        glPopMatrix();

        // Losango
        glPushMatrix();
        glTranslatef(0.0f, alturaTorre + 1.2f, 0.0f);
        glRotatef(anguloPiramide, 0.0f, 1.0f, 0.0f);
        desenhaLosango(1.5f);
        glPopMatrix();

        // >>> A BICICLETA NA PRIMEIRA TORRE <<<
        if (i == 0) 
        {
            glPushMatrix();
            glTranslatef(0.8f, 0.3f, 0.5f);     // Posição ao lado da torre
            glRotatef(90.0f, 0.0f, 1.0f, 0.0f); // Alinha com a torre
            glRotatef(-15.0f, 1.0f, 0.0f, 0.0f); // Inclina ("Escorada")
            desenhaBicicleta();
            glPopMatrix();
        }

        glPopMatrix();
    }
}

void desenhaPiramideDegraus()
{
    float alturaDegrau = 0.5f;
    float tamanhoBase = 6.0f;
    float reducao = 0.65f;

    glPushMatrix();

    float roxo1[3] = {0.55f, 0.00f, 0.75f};
    float roxo2[3] = {0.65f, 0.10f, 0.85f};
    float roxo3[3] = {0.75f, 0.20f, 0.95f};

    // Degraus
    glColor3fv(roxo1); glPushMatrix(); glTranslatef(0.0f, alturaDegrau/2, 0.0f); glScalef(tamanhoBase, alturaDegrau, tamanhoBase); glutSolidCube(1.0f); glPopMatrix();
    glColor3fv(roxo2); glPushMatrix(); glTranslatef(0.0f, alturaDegrau * 1.5f, 0.0f); glScalef(tamanhoBase*reducao, alturaDegrau, tamanhoBase*reducao); glutSolidCube(1.0f); glPopMatrix();
    glColor3fv(roxo3); glPushMatrix(); glTranslatef(0.0f, alturaDegrau * 2.5f, 0.0f); glScalef(tamanhoBase*reducao*reducao, alturaDegrau, tamanhoBase*reducao*reducao); glutSolidCube(1.0f); glPopMatrix();

    // Esfera Cortada
    float topoDegrausY = 3.0f * alturaDegrau;
    float raioEsfera = 3.0f;

    glPushMatrix();
    glTranslatef(0.0f, topoDegrausY + raioEsfera, 0.0f);
    glRotatef(anguloEsfera, 1.0f, 1.5f, 0.0f);

    double eqCima[4] = {0, -1, 0, 0};
    double eqBaixo[4] = {0, 1, 0, 0};

    glColor3f(0.40f, 0.00f, 0.00f);
    glClipPlane(GL_CLIP_PLANE0, eqCima); glEnable(GL_CLIP_PLANE0);
    glutSolidSphere(raioEsfera, 40, 40);
    glDisable(GL_CLIP_PLANE0);

    glColor3f(0.80f, 0.00f, 0.00f);
    glClipPlane(GL_CLIP_PLANE1, eqBaixo); glEnable(GL_CLIP_PLANE1);
    glutSolidSphere(raioEsfera, 40, 40);
    glDisable(GL_CLIP_PLANE1);

    glPopMatrix();
    glPopMatrix();
}