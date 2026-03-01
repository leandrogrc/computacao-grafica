#pragma once

#include "level/maploader.h"
#include "core/entities.h"
#include "graphics/hud.h" // Precisa acessar propriedades do struct HudState
#include <vector>

void drawMiniMap(int w, int h, const HudState& s, const MapLoader& map, const std::vector<Enemy>& enemies, const std::vector<Item>& items);
