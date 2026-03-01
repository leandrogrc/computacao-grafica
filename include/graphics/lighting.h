#pragma once
void setupIndoorLightOnce();
void setupSunLightOnce();
void setSunDirectionEachFrame();
void applyLevelLighting(int level, float time);
void applyPlayerFlashlight(float camX, float camY, float camZ, float dirX, float dirY, float dirZ);
