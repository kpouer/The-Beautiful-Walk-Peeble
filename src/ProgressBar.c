#include <pebble.h>
#include "ProgressBar.h"

void resetProgressBar(struct ProgressBar *progressBar)
{
  progressBar->value = 0;
  progressBar->total = 0;
}

void drawHorizontalBar(GContext *ctx, struct ProgressBar *progressBar, GColor color, GColor bgColor, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t cornerRadius)
{
  if (progressBar->total)
  {
    graphics_context_set_fill_color(ctx, bgColor);
    graphics_fill_rect(ctx, GRect(x, y, w, h), cornerRadius, GCornersAll);
    if (progressBar->value)
    {
      graphics_context_set_fill_color(ctx, color);
      graphics_fill_rect(ctx, GRect(x, y, w * progressBar->value / progressBar->total, h), cornerRadius, GCornersLeft);
    }
  }  
}
