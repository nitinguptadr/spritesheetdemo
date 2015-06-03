#pragma once
 
#include <pebble.h>
#include "pge_sprite.h"
#include "pge_spritesheet.h"

typedef uint32_t PGETileSheetHandle;

PGETileSheetHandle pge_tilesheet_create(int resource_id, PGESpriteTableHandle sprite_table_handle);

void pge_tilesheet_destroy(PGETileSheetHandle handle);

void pge_tilesheet_draw_tile(GContext *ctx, PGETileSheetHandle handle, GPoint coordinate, GPoint position);

void pge_tilesheet_draw_grid(GContext *ctx, PGETileSheetHandle handle, GRect box, GPoint position, GSize spacing);

GSize pge_tilesheet_get_tilesheet_size(PGETileSheetHandle handle);


