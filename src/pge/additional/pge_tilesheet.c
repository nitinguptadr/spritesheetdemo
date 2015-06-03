#include <pebble.h>
#include "pge_tilesheet.h"
#include "pge_spritesheet.h"

#define INVALID_GLOBAL_TILE_ID 0 // This equates to not drawing anything in the tile map

typedef struct {
  uint32_t version;
  uint32_t filesize;  // Size of the tilesheet data file
  uint32_t width;     // Number of tiles
  uint32_t height;    // Number of tiles
} PGETileSheetHeader;

typedef struct {
  PGETileSheetHeader header;                // Header of the tile sheet data table
  int resource_id;                          // Resource ID of tile sheet data file
  PGESpriteTableHandle sprite_table_handle; // Pointer to corresponding sprite sheet
  uint32_t *tile_global_ids;                // Pointer to an array of global tile ids
                                            // Tiles are organized in a 1D array sequentially based on position in a 2D grid
                                            // i.e. Left to right (width number of tiles) and top to bottom (i.e. height number of tiles)
                                            // tile_global_ids[0] = grid position [0,0]
                                            // tile_global_ids[width - 1] = grid position [width - 1,0]
                                            // tile_global_ids[width] = grid position [0,1]
                                            // tile_global_ids[width + 1] = grid position [1,1]
                                            // tile_global_ids[width + 2] = grid position [2,1]
                                            // tile_global_ids[(width * height) - 1] = grid position [width - 1, height - 1]

} PGETileSheet;

PGETileSheetHandle pge_tilesheet_create(int resource_id, PGESpriteTableHandle sprite_table_handle) {
  PGETileSheetHandle handle = 0;
  PGETileSheet *this = calloc(1, sizeof(PGETileSheet));
  if (!this) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to allocate tile sheet");
    goto cleanup;
  }

  size_t header_size = sizeof(PGETileSheetHeader);
  ResHandle rh = resource_get_handle(resource_id);
  if (resource_load_byte_range(rh, 0, (uint8_t*)&this->header, header_size) != header_size) {
    goto cleanup;
  }

  uint32_t size_of_global_ids = this->header.filesize - sizeof(PGETileSheetHeader);
  this->tile_global_ids = malloc(size_of_global_ids);
  if (!this->tile_global_ids) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to allocate array for tile sheet global ids");
    goto cleanup;
  }

  if (resource_load_byte_range(rh, sizeof(PGETileSheetHeader), (uint8_t*)&this->tile_global_ids[0], size_of_global_ids) != size_of_global_ids) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to load tile sheet global ids");
    goto cleanup;
  }

  this->resource_id = resource_id;
  this->sprite_table_handle = sprite_table_handle;
  handle = (uint32_t) this;
  goto done;

cleanup:
  if (this) {
    if (this->tile_global_ids) {
      free(this->tile_global_ids);
    }
    free(this);
  }

done:
  return handle;
}

void pge_tilesheet_destroy(PGETileSheetHandle handle) {
  PGETileSheet *this = (PGETileSheet *)handle;
  if (this) {
    if (this->tile_global_ids) {
      free(this->tile_global_ids);
    }
    free(this);
  }
}

static PGESprite *s_tile_sprite = NULL;
static PGETileSheetHandle prev_tile_handle = 0;
static uint32_t prev_tile_global_id = 0;
void pge_tilesheet_draw_tile(GContext *ctx, PGETileSheetHandle handle, GPoint coordinate, GPoint position) {
  if (!handle) {
    return;
  }
  PGETileSheet *this = (PGETileSheet *)handle;
  // Get global ID for the tile to draw
  uint32_t index = (coordinate.y * this->header.width) + coordinate.x;
  if (this->tile_global_ids[index] == INVALID_GLOBAL_TILE_ID) {
    return;
  }

  if ((s_tile_sprite == NULL) || (prev_tile_handle != handle) || (this->tile_global_ids[index] != prev_tile_global_id)) {
    if (s_tile_sprite) {
      pge_sprite_destroy(s_tile_sprite);
    }

    // Create a temporary sprite to draw
    s_tile_sprite = pge_spritesheet_create_sprite_gid(this->sprite_table_handle, this->tile_global_ids[index], position);

    prev_tile_handle = handle;
    prev_tile_global_id = this->tile_global_ids[index];
  } else {
    pge_sprite_set_position(s_tile_sprite, position);
  }
 
  if (!s_tile_sprite) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to create sprite with global id %ld at index %ld", this->tile_global_ids[index], index);
    return;
  }

  // Draw and then destroy the sprite
  pge_sprite_draw(s_tile_sprite, ctx);
}

void pge_tilesheet_draw_grid(GContext *ctx, PGETileSheetHandle handle, GRect box, GPoint position, GSize spacing) {
  if (!handle) {
    return;
  }
  for (uint16_t y = box.origin.y; y < box.origin.y + box.size.h; y++) {
    for (uint16_t x = box.origin.x; x < box.origin.x + box.size.w; x++) {
      GPoint coordinate = GPoint(x, y);
      GPoint draw_position = position;
      draw_position.x += (x - box.origin.x) * spacing.w;
      draw_position.y += (y - box.origin.y) * spacing.h;
      pge_tilesheet_draw_tile(ctx, handle, coordinate, draw_position);
    }
  }
}

GSize pge_tilesheet_get_tilesheet_size(PGETileSheetHandle handle) {
  if (!handle) {
    return GSizeZero;
  }
  PGETileSheet *this = (PGETileSheet *)handle;
  return GSize(this->header.width, this->header.height);
}


