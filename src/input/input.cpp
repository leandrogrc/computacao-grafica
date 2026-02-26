#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include "input/input.h"
#include "input/keystate.h"
#include "core/window.h"
#include "graphics/menu.h"
#include "core/game.h"
#include "core/player.h"

void keyboard(unsigned char key, int, int)
{
    const GameState state = gameGetState();
    // ESC sai do jogo imediatamente em qualquer tela
    if (key == 27)
        std::exit(0);

    // --- MENU INICIAL ---
    if (state == GameState::MENU_INICIAL)
    {
        if (key == 13)
        {
            if (!menuMeltIsActive()) // evita recomeçar toda hora
                menuMeltRequestStart();
        }
        return;
    }

    // --- GAME OVER ---
    if (state == GameState::GAME_OVER)
    {
        if (key == 13)
        {
            gameReset();
            gameLoadLevel(1);
        }
        return;
    }

    // --- LEVEL CLEAR ---
    if (state == GameState::LEVEL_CLEAR)
    {
        if (key == 13)
        {
            auto& ctx = gameContext();
            gameLoadLevel(ctx.currentLevel + 1);
        }
        return;
    }

    // --- VITORIA ---
    if (state == GameState::VITORIA)
    {
        if (key == 13)
        {
            gameReset();
            gameLoadLevel(1);
        }
        return;
    }

    // --- PAUSE ---
    if (state == GameState::PAUSADO)
    {
        if (key == 'p' || key == 'P')
        {
            gameSetState(GameState::JOGANDO);
        }
        return;
    }

    // --- JOGANDO ---
    if (state == GameState::JOGANDO)
    {
        if (key == 'p' || key == 'P')
        {
            gameSetState(GameState::PAUSADO);
            // Para o movimento ao pausar
            keyW = keyA = keyS = keyD = keyShift = false;
            return;
        }

        // Atualiza estado do Shift
        keyShift = (glutGetModifiers() & GLUT_ACTIVE_SHIFT);

        // Controles de Jogo (WASD + R)
        switch (key)
        {
        case 'w':
        case 'W':
            keyW = true;
            break;
        case 's':
        case 'S':
            keyS = true;
            break;
        case 'a':
        case 'A':
            keyA = true;
            break;
        case 'd':
        case 'D':
            keyD = true;
            break;
        case 'r':
        case 'R':
            playerTryReload();
            break;
        case 'g':
        case 'G':
            keyG = true;
            break;
        case '1':
            playerSwitchWeapon(0);
            break;
        case '2':
            playerSwitchWeapon(1);
            break;
        }
    }
}

void keyboardUp(unsigned char key, int, int)
{
    keyShift = (glutGetModifiers() & GLUT_ACTIVE_SHIFT);

    switch (key)
    {
    case 'w':
    case 'W':
        keyW = false;
        break;
    case 's':
    case 'S':
        keyS = false;
        break;
    case 'a':
    case 'A':
        keyA = false;
        break;
    case 'd':
    case 'D':
        keyD = false;
        break;
    case 'g':
    case 'G':
        keyG = false;
        break;
    }

    if ((key == 13 || key == '\r') && (glutGetModifiers() & GLUT_ACTIVE_ALT))
    {
        altFullScreen();
    }
}
void mouseClick(int button, int state, int /*x*/, int /*y*/)
{
    keyShift = (glutGetModifiers() & GLUT_ACTIVE_SHIFT);

    // Se apertou o botão esquerdo
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        playerTryAttack();
    }
}
