#pragma once

#include "common.h"

int GetScreenBrightnessConfig();
bool isSpaceShown();
bool isJapaneseClockUsed();
char* GetFontPathConfig();

void SetFontPathConfig(const char* path);

bool SaveConfig();
bool LoadConfig();
void ConfigMenu();
