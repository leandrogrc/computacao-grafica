# 🦇 Abyssal Protocol (Computação Gráfica Project)

Um projeto desenvolvido utilizando **OpenGL** (pipeline fixo + GLSL 1.20) construído em C++, demonstrando a aplicação de técnicas complexas de rasterização 3D, culling, física simples, processamento de áudio 3D e Shaders num formato de jogo de tiro "Doom-Like" de Sobrevivência e Terror.

## 🎥 Demonstração de Gameplay
> *Aqui você pode colar o link do YouTube / GIF da sua versão final do projeto!*

---

## 💻 Dependências de Instalação (Linux)

Para conseguir compilar e abrir **Abyssal Protocol**, seu SO Debian/Ubuntu precisará ter:
- `g++` (Compilador de C++ Moderno)
- `make` (Automação de Build)
- `freeglut3-dev` (Gerenciador de Janelas OpenGL)
- `libglew-dev` (Para lidar com Shaders GLSL modernos)
- `libopenal-dev` (Para a arquitetura de Áudio Espacial)

Sinta-se à vontade para instalar todas no terminal:
```bash
sudo apt install g++ make freeglut3-dev libglew-dev libglu1-mesa-dev libopenal-dev
```

*(O carregador de Texturas `stb_image.h` já vem embutido na pasta `include` do projeto).*

---

## 🚀 Compilando e Rodando

O projeto usa **Make** modular. Todas as rotinas já foram construídas num arquivo `Makefile` limpo.
Na raiz do projeto onde os arquivos de código se encontram, rode:

```bash
make clean && make -j4
make run
```
Esse comando vai invocar o linker e ler todos os arquivos de `src/`, montar os objetos matemáticos em `build/` e explodir a fúria do jogo aberto na sua frente através do executável padrão principal.

---

## ⌨️ Controles do Jogador

A navegação ocorre com teclado num paradigma tradicional FPS de primeira pessoa, usando _Strafe_ e Câmera fluida pelo Mouse Control.

| Tecla / Botão | Ação no Jogo |
|-------|------|
| **W, A, S, D** | Caminhar nas 4 direções cardeais pelo labirinto |
| **Mouse / Touchpad** | Girar Direção da Cabeça (Yaw / Pitch) |
| **Clique Esquerdo** | Atirar com a Arma Primária Equipada |
| **R** | Recarregar munição da arma atual |
| **G** | Coletar armas poderosas espalhadas no chão |
| **1, 2** | Trocar de armamento (Escopeta / Rifle Plasma) |
| **ALT + Enter** | Alterna Dinamicamente entre Janela de Dev e Tela Cheia! |
| **P** | Abre o Menu de Pausa para respiro |
| **ESC** | Sai abruptamente do jogo fechando o terminal |

---

## 🗺️ O Engine de Mapas Textuais (`.txt`)

O projeto usa um **Tile-Based Parser**. Isso significa que as construções tridimensionais (Paredes, Luzes, Inimigos) são decididas pelo caractere tipado no `.txt` das pastas de mapas (`maps/`).

### 🧩 Como ler o código de Fase

- O arquivo precisa ter um tamanho de linhas/colunas retangular. Exemplo `10x10`.
- Os blocos que representam barreiras intransponíveis: 
  - `1`: Parede com Textura Padrão (Laboratório)
  - `2`: Parede Interativa que só projeta Textura em faces coladas no Vazio `0`.
  - `M`, `H`, `S`, `O`: Paredes de Market, House, Shop e Office.
- Os pisos:
  - `0`: Chão normal da rua coberto por paralelepípedos e sem teto (Outdoor).
  - `3`: Chão e Teto Internos de instalação.
  - `L` e `B`: Telhas texturizadas animadas de Lava e Sangue pelo fragment shader.
- Spawn do Jogador: Onde começar: `P` (ou `p`).
- Saída do Nível: A localização estática do Portal é declarada com o `E` (Exit).

### 👹 Inimigos, Spawn e Combate

Não existem itens de coleta como vida (`H`) ou munição no `.txt`. Nosso jogo utiliza um prático **Sistema Dinâmico de Drops In-Game**. Toda vez que um inimigo morre, ele libera prêmios no chão que podem incluir Cartões, Berserks, Municação extra e Cura!

Basta declarar as criaturas diretamente na grade pra que a IA deles desperte:
- `J/j`, `T/t`, `m`, `G/g`: Referem-se aos monstros das sprites e IA predatórias (Zumbis, Imps e Mutantes).  
- `B/b`: **O Chefão Final!** Uma classe separada multiplicada em tamanho e vida e requerida obrigatoriamente morta na última fase para o triunfo do jogador.

---

## 📚 Tecnologias Gráficas Usadas

- **RayCircle Intersection:** Colisão hitscan em tempo real do tiro matemático desferido da base do nariz do jogador (pitch e yaw angle) atravessando fisicamente as caixas limitadoras 2D dos monstros no círculo.
- **Frustum Culling Angular:** Função `isVisibleXZ` limitando renderizações ocultadas (para escalar o rendimento das janelas com incontáveis blocos de mapas).
- **Billboarding Mosaico:** Desenho de Quads projetados frente a frente à lente da matriz LookAt da Câmera baseada no plano `arctan2` de visualização global para manter Sprites planas.
- **Particle Systems 3D:** Fumaça translúcida, física Alpha na morte e jatos direcionais com gravidade real no sangue inimigo atingido - testados desabilitando de forma seletiva a `DepthMask` por loop. 
- **Ambient Listener no OpenAL:** Interpolação sonora dinâmica (Atenuação de Ganho de Fonte com Distância Radial). Escute gemidos arrepiantes de longe.

> 🛠️ **Status**: *Concluído com Excelência!* 🛠️
