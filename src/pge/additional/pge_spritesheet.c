#include "pge_spritesheet.h"

PGESpriteSheet* pge_spritesheet_create(int resource_id, int num_sets) {
  PGESpriteSheet *this = calloc(1, sizeof(PGESpriteSheet));
  if (!this) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to allocate sprite sheet");
    return NULL;
  }

  // Allocate bitmap
  this->bitmap = gbitmap_create_with_resource(resource_id);
  if (!this->bitmap) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to create bitmap for spritesheet");
    free(this);
    return NULL;
  }

  this->sets = calloc(num_sets, sizeof(PGESpriteSet));
  if (!this->sets) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to sprite sets for sprite sheet");
    gbitmap_destroy(this->bitmap);
    free(this);
  }

  this->num_sets = num_sets;

  GRect bounds = gbitmap_get_bounds(this->bitmap);

  return this;
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
