#include <pebble.h>
#include "PebbleUI.h"

void initTextLayer(TextLayer *textLayer, const char * text, GColor fgColor, GColor bgColor,  GTextAlignment alignment, GFont font)
{
  text_layer_set_background_color(textLayer, bgColor);
  text_layer_set_text_color(textLayer, fgColor);
  text_layer_set_text(textLayer, text);
  text_layer_set_text_alignment(textLayer, alignment);
  text_layer_set_font(textLayer, font);
}

void readSettingsString(const uint32_t key, char * buffer, const char* def)
{
  if (persist_exists(key))
    persist_read_string(key, buffer, sizeof(buffer));
  else
    strcpy(buffer, def);
}

int readSettingsInt(const uint32_t key, int def)
{
  if (persist_exists(key))
    return persist_read_int(key);
  return def;
}