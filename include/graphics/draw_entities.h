#pragma once

#include "core/entities.h"
#include <vector>

struct RenderAssets;

void drawSprite(float x, float z, float w, float h, unsigned int tex, float camX, float camZ);
void drawProjectiles(const std::vector<Projectile>& projectiles, float camX, float camZ, float time);

void drawEntities(const std::vector<Enemy> &enemies,
                  const std::vector<Item> &items,
                  float camX, float camZ, float dx, float dz,
                  const RenderAssets &r);
