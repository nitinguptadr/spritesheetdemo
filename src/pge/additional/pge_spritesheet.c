#include <pebble.h>
#include "pge_spritesheet.h"

#define TILE_NAME_MAX_SIZE 20
typedef struct {
  char tile_name[TILE_NAME_MAX_SIZE];
  uint32_t tile_local_id;
  uint32_t tile_png_offset;
  uint32_t tile_png_size;
} PGESpriteTableEntry;

typedef struct {
  uint32_t version;
  uint32_t filesize;
  uint32_t table_entries_size;
  uint32_t reserved;
} PGESpriteTableHeader;

typedef struct {
  PGESpriteTableHeader header;
  int resource_id;
  PGESpriteTableEntry *table_entries;
} PGESpriteTable;

PGESpriteSheet* pge_spritesheet_create(int resource_id, int num_sets) {
  PGESpriteSheet *this = calloc(1, sizeof(PGESpriteSheet));
  if (!this) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to allocate sprite sheet");
    goto cleanup;
  }

  // Allocate bitmap
  this->bitmap = gbitmap_create_with_resource(resource_id);
  if (!this->bitmap) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to create bitmap for spritesheet");
    goto cleanup;
  }

  this->sets = calloc(num_sets, sizeof(PGESpriteSet));
  if (!this->sets) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to sprite sets for sprite sheet");
    goto cleanup;
  }

  this->num_sets = num_sets;

  return this;

cleanup:
  if (this) {
    gbitmap_destroy(this->bitmap);
    if (this->sets) {
      free(this->sets);
    }
    free(this);
  }

  return NULL;
}

void pge_spritesheet_destroy(PGESpriteSheet *this) {
  if (!this) {
    return;
  }

  if (this->bitmap) {
    gbitmap_destroy(this->bitmap);
  }

  if (this->sets) {
    free(this->sets);
  }

  free(this);
}

uint32_t pge_spritesheet_add_set(PGESpriteSheet *spritesheet, GRect frame, GSize sprite_size, int16_t xspacing, int16_t yspacing) {
  if ((!spritesheet) || (!spritesheet->sets)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid params");
    return INVALID_SET_INDEX;
  }

  // Find next available slot
  uint32_t available_index = 0;
  for (; available_index < spritesheet->num_sets; available_index++) {
    // Frame size of 0 indicates uninitialized
    if (spritesheet->sets[available_index].num_sprites == 0) {
      // Setup slot
      spritesheet->sets[available_index].sprite_index = 0;
      spritesheet->sets[available_index].frame = frame;
      spritesheet->sets[available_index].sprite_size = sprite_size;
      spritesheet->sets[available_index].xspacing = xspacing;
      spritesheet->sets[available_index].yspacing = yspacing;

      // Each sprite takes up sprite_size + spacing - calculate how many sprites fit
      uint32_t num_sprites_in_row = 0;
      for (int16_t xoffset = 0; xoffset < frame.size.w; xoffset += sprite_size.w + xspacing) {
        if (xoffset + sprite_size.w <= frame.size.w) {
          num_sprites_in_row++;
        } else {
          break;
        }
      }
      spritesheet->sets[available_index].num_sprites_in_row = num_sprites_in_row;

      uint32_t num_sprites_in_col = 0;
      for (int16_t yoffset = 0; yoffset < frame.size.h; yoffset += sprite_size.h + yspacing) {
        if (yoffset + sprite_size.h <= frame.size.h) {
          num_sprites_in_col++;
        } else {
          break;
        }
      }
      spritesheet->sets[available_index].num_sprites_in_col = num_sprites_in_col;

      spritesheet->sets[available_index].num_sprites = num_sprites_in_row * num_sprites_in_col;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Add Sprite Set Index %ld, Num Sprites %ld", available_index, spritesheet->sets[available_index].num_sprites);
      break;
    }
  }

  if (available_index >= spritesheet->num_sets) {
    // Unable to find a slot
    available_index = INVALID_SET_INDEX;
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to find slot");
  }

  return available_index;  
}

uint32_t pge_spritesheet_get_sprite_index(PGESpriteSheet *spritesheet, uint32_t set_index) {
  if ((!spritesheet) || (!spritesheet->sets) || (set_index >= spritesheet->num_sets)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid params");
    return INVALID_SPRITE_INDEX;
  }

  return spritesheet->sets[set_index].sprite_index;
}

void pge_spritesheet_set_sprite_index(PGESpriteSheet *spritesheet, uint32_t set_index, uint32_t sprite_index) {
  if ((!spritesheet) || (!spritesheet->sets) || (set_index >= spritesheet->num_sets)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid params");
    return;
  }

  PGESpriteSet *spriteset = &spritesheet->sets[set_index];
  if (sprite_index >= spriteset->num_sprites) {
    return;
  }

  spriteset->sprite_index = sprite_index;
}

GPoint pge_spritesheet_get_sprite_position(PGESpriteSheet *spritesheet, uint32_t set_index) {
  if ((!spritesheet) || (!spritesheet->sets) || (set_index >= spritesheet->num_sets)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid params");
    return GPointZero;
  }

  return spritesheet->sets[set_index].position;
}

void pge_spritesheet_set_sprite_position(PGESpriteSheet *spritesheet, uint32_t set_index, GPoint position) {
  if ((!spritesheet) || (!spritesheet->sets) || (set_index >= spritesheet->num_sets)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid params");
    return;
  }
  spritesheet->sets[set_index].position = position;
}

static GRect prv_get_sprite_frame(PGESpriteSet *spriteset) {
  uint32_t row = spriteset->sprite_index / spriteset->num_sprites_in_row;
  uint32_t col = spriteset->sprite_index % spriteset->num_sprites_in_row;

  // Sprite frame will be a relative offset from the origin of the main sprite sheet bitmap
  // spriteset->frame.origin is the offset of the spriteset
  GRect sprite_frame = GRect(spriteset->frame.origin.x, spriteset->frame.origin.y, spriteset->sprite_size.w, spriteset->sprite_size.h);
  sprite_frame.origin.x += col * (spriteset->sprite_size.w + spriteset->xspacing);
  sprite_frame.origin.y += row * (spriteset->sprite_size.h + spriteset->yspacing);

  return sprite_frame;
}


GRect pge_spritesheet_get_sprite_bounds(PGESpriteSheet *spritesheet, uint32_t set_index) {
  if ((!spritesheet) || (!spritesheet->sets) || (set_index >= spritesheet->num_sets)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid params");
    return GRectZero;
  }

  return GRect(spritesheet->sets[set_index].position.x,
               spritesheet->sets[set_index].position.y,
               spritesheet->sets[set_index].sprite_size.w,
               spritesheet->sets[set_index].sprite_size.h);
}

void pge_spritesheet_draw(GContext *ctx, PGESpriteSheet *spritesheet, uint32_t set_index) {
  if ((!ctx) || (!spritesheet) || (!spritesheet->sets) || (set_index >= spritesheet->num_sets)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid params");
    return;
  }

  PGESpriteSet *spriteset = &spritesheet->sets[set_index];

  // Get rectangle for sub bitmap within the sprite sheet based on current index within spriteset
  GRect sub_bitmap_frame = prv_get_sprite_frame(spriteset);

  // Create a sub bitmap out of the main sprite sheet
  GBitmap *sub_bitmap = gbitmap_create_as_sub_bitmap(spritesheet->bitmap, sub_bitmap_frame);

  // Draw sprite at appropriate position
  GRect sprite_frame = GRect(spriteset->position.x, spriteset->position.y, sub_bitmap_frame.size.w, sub_bitmap_frame.size.h);
  graphics_draw_bitmap_in_rect(ctx, sub_bitmap, sprite_frame);

  // Cleanup
  gbitmap_destroy(sub_bitmap);
}

uint32_t pge_spritesheet_get_num_sprites(PGESpriteSheet *spritesheet, uint32_t set_index) {
  if ((!spritesheet) || (!spritesheet->sets) || (set_index >= spritesheet->num_sets)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid params");
    return 0;
  }

  return spritesheet->sets[set_index].num_sprites;
}

PGESpriteTableHandle pge_spritesheet_load_table(int resource_id) {
  ResHandle rh = resource_get_handle(resource_id);

  // Create Sprite Table
  PGESpriteTableHandle sprite_table_handle = 0;
  PGESpriteTable *sprite_table = malloc(sizeof(PGESpriteTable));
  if (!sprite_table) {
    goto cleanup;
  }

  sprite_table->resource_id = resource_id;
  sprite_table->table_entries = NULL;
  // Load the table header
  size_t header_size = sizeof(PGESpriteTableHeader);
  if (resource_load_byte_range(rh, 0, (uint8_t*)sprite_table, header_size) != header_size) {
    goto cleanup;
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded sprite table header %ld, %ld, %ld", sprite_table->header.version, sprite_table->header.filesize, sprite_table->header.table_entries_size);

  // Load the table entries
  uint32_t table_entries_size = sprite_table->header.table_entries_size;
  sprite_table->table_entries = malloc(table_entries_size);
  if (!sprite_table->table_entries) {
    goto cleanup;
  }
  if (resource_load_byte_range(rh, header_size, (uint8_t*)&sprite_table->table_entries[0], table_entries_size) != table_entries_size) {
    goto cleanup;
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded sprite table entries %ld %ld %ld", sprite_table->table_entries[0].tile_local_id, sprite_table->table_entries[0].tile_png_offset, sprite_table->table_entries[0].tile_png_size);

  sprite_table_handle = (uint32_t)sprite_table;
  goto done;

cleanup:
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Error while loading sprite table");
  if (sprite_table) {
    if (sprite_table->table_entries) {
      free(sprite_table->table_entries);
    }
    free(sprite_table);
  }

done:
  return sprite_table_handle;
}

static PGESpriteTableEntry* prv_find_table_entry(PGESpriteTableHandle handle, char *tile_name, uint32_t tile_local_id) {
  PGESpriteTableEntry *table_entry = NULL;
  PGESpriteTable *sprite_table = (PGESpriteTable *)handle;
  uint32_t num_entries = sprite_table->header.table_entries_size / sizeof(PGESpriteTableEntry);

  for (uint32_t index = 0; index < num_entries; index++) {
    if ((strncmp(sprite_table->table_entries[index].tile_name, tile_name, TILE_NAME_MAX_SIZE) == 0) &&
        sprite_table->table_entries[index].tile_local_id == tile_local_id) {
      table_entry = &sprite_table->table_entries[index];
      break;
    }
  }

  return table_entry;
}

PGESprite* pge_spritesheet_create_sprite(PGESpriteTableHandle handle, char *tile_name, uint32_t tile_local_id, GPoint position) {
  PGESprite *sprite = NULL;
#ifdef PBL_PLATFORM_BASALT
  PGESpriteTable *sprite_table = (PGESpriteTable *)handle;
  PGESpriteTableEntry *table_entry = prv_find_table_entry(handle, tile_name, tile_local_id);
  if (table_entry) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Found table entry %s %ld %ld %ld", table_entry->tile_name, table_entry->tile_local_id, table_entry->tile_png_offset, table_entry->tile_png_size);
    uint8_t *png_data = malloc(table_entry->tile_png_size);
    uint32_t file_offset = sizeof(PGESpriteTableHeader) + sprite_table->header.table_entries_size + table_entry->tile_png_offset;
    ResHandle rh = resource_get_handle(sprite_table->resource_id);
    if (png_data && (resource_load_byte_range(rh, file_offset, (uint8_t*)png_data, table_entry->tile_png_size) == table_entry->tile_png_size)) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Creating sprite: %s, id: %ld, offset: %ld, size: %ld", table_entry->tile_name, table_entry->tile_local_id, table_entry->tile_png_offset, table_entry->tile_png_size);
      sprite = pge_sprite_create_from_png_data(position, png_data, table_entry->tile_png_size);
      free(png_data);
    } else if (png_data) {
      free(png_data);
    }
  }
#endif
  return sprite;
}

void pge_spritesheet_set_anim_frame(PGESprite *this, PGESpriteTableHandle handle, char *tile_name, uint32_t tile_local_id) {
#ifdef PBL_PLATFORM_BASALT
  // Destroy existing bitmap
  gbitmap_destroy(this->bitmap);
  this->bitmap = NULL;

  // Find corresponding sprite table entry
  PGESpriteTable *sprite_table = (PGESpriteTable *)handle;
  PGESpriteTableEntry *table_entry = prv_find_table_entry(handle, tile_name, tile_local_id);
  if (table_entry) {
    // Load up PNG data and create GBitmap
    uint8_t *png_data = malloc(table_entry->tile_png_size);
    uint32_t file_offset = sizeof(PGESpriteTableHeader) + sprite_table->header.table_entries_size + table_entry->tile_png_offset;
    ResHandle rh = resource_get_handle(sprite_table->resource_id);
    if (png_data && resource_load_byte_range(rh, file_offset, (uint8_t*)png_data, table_entry->tile_png_size) == table_entry->tile_png_size) {
      this->bitmap = gbitmap_create_from_png_data(png_data, table_entry->tile_png_size);
      free(png_data);
    } else if (png_data) {
      free(png_data);
    }
  }
#endif
}

