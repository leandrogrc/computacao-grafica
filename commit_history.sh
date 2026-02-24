#!/bin/bash
git reset

# 1. Ajustes de ambiente e mapas (Spawn, etc)
git add maps/map2.txt src/level/maploader.cpp
git commit -m "fix: corrigir ponto de spawn do jogador no mapa 2"

# 2. Remoção da lanterna e iluminação global
git add src/graphics/lighting.cpp src/core/game.cpp src/input/input.cpp include/graphics/lighting.h include/core/game_state.h
git commit -m "feat: remover lanterna e clarear iluminacao ambiente do jogo"

# 3. Assets da nova arma e Script de background
git add assets/ remove_bg.py
git commit -m "feat: adicionar texturas do Lightning Cannon e script de chroma key"

# 4. Implementação da nova arma e HUD de arma
git add src/core/player.cpp src/core/entities.cpp src/graphics/drawlevel.cpp
git commit -m "feat: implementar suporte logico e visual para arma secundaria e rebalancear drops"

# 5. Redesign do HUD 
git add src/graphics/hud.cpp src/graphics/ui_text.cpp
git commit -m "style: redesenhar HUD inferior com background, cores e icones OpenGL"

# 6. O resto (main.cpp e outros ajustes menores de assets)
git add .
git commit -m "chore: adicionar map3 e refatoracoes menores de renderizacao"

