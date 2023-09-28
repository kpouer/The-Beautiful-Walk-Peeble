#include <pebble.h>
#pragma once


void initTextLayer(TextLayer *textLayer, const char * text, GColor fgColor, GColor bgColor,  GTextAlignment alignment, GFont font);

void readSettingsString(const uint32_t key, char * buffer, const char* def);
int readSettingsInt(const uint32_t key, int def);