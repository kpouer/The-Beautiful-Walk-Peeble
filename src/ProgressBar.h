#pragma once
#include <pebble.h>

struct ProgressBar
{
  uint32_t value;
  uint32_t total;
};

void resetProgressBar(struct ProgressBar *progressBar);

void drawHorizontalBar(GContext *ctx, struct ProgressBar *progressBar, GColor color, GColor bgColor, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t cornerRadius);