#ifndef DRAWLEVEL_H
#define DRAWLEVEL_H

#include "level/maploader.h"
#include "core/entities.h"
#include <vector>

struct RenderAssets; // forward decl (vem de core/game_state.h)

bool isVisibleXZ(float objX, float objZ,
               float camX, float camZ,
               bool hasFwd, float fwdx, float fwdz);

void drawLevel(const MapLoader &map, float px, float pz, float dx, float dz, const RenderAssets &r, float time);

#endif