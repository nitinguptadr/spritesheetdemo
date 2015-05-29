#pragma once
 
#include <pebble.h>

// Sprite Sheet
typedef struct {
  uint32_t sprite_index;  // Index within this sprite set of current sprite to draw, Default is 0
  uint32_t num_sprites;    // Total number of sprites within sprite set - calculated based on frame and sprite_size
  GRect frame;            // Offset and size of the sprite set within the source GBitmap image
  GSize sprite_size;      // Size of an individual sprite within this sprite set
  int16_t xspacing;       // Optional: horizontal pixel spacing between individual sprites within a set, Default is 0
  int16_t yspacing;       // Optional: vertical pixel spacing between individual sprites within a set, Default is 0
  GPoint position;        // Point on screen where to draw the sprite
  uint32_t num_sprites_in_row;
  uint32_t num_sprites_in_col;
} PGESpriteSet;

// Sprite Sheet
typedef struct {
  GBitmap *bitmap;     // Pointer to the GBitmap image for a given sprite sheet
  uint32_t num_sets;   // Number of sprite sets
  PGESpriteSet *sets;  // Array of PGESpriteSet
} PGESpriteSheet;

#define INVALID_SET_INDEX ~(0)
#define INVALID_SPRITE_INDEX ~(0)

//! Creates an empty sprite sheet from a given resource image
//! @param resource_id Resource id of the sprite sheet image to load
//! @param num_sets Number of PGESpriteSet to create
//! @return Pointer to the created sprite sheet
PGESpriteSheet* pge_spritesheet_create(int resource_id, int num_sets);

//! Destroys and frees the memory used by the PGESpriteSheet
//! @param spritesheet Pointer to the PGESpriteSheet to destroy
void pge_spritesheet_destroy(PGESpriteSheet *spritesheet);

//! Adds a new PGESpriteSet to the given PGESpriteSheet.
//! @param spritesheet Pointer to the PGESpriteSheet to add a sprite set to 
//! @param frame Offset and size in pixels of the new PGESpriteSet relative to the origin of the GBitmap for the given PGESpriteSheet 
//! @param xspacing Horizontal pixel spacing between individual sprites within the PGESpriteSet
//! @param yspacing Vertical pixel spacing between individual sprites within the PGESpriteSet
//! @return Index of the slot allocated for the PGESpriteSet (i.e. set_index); INVALID_SET_INDEX if unable to add
uint32_t pge_spritesheet_add_set(PGESpriteSheet *spritesheet, GRect frame, GSize sprite_size, int16_t xspacing, int16_t yspacing);

//! Gets the current sprite index of the PGESpriteSet pointed to by set_index
//! @param spritesheet Pointer to the PGESpriteSheet
//! @param set_index Index of the slot for the PGESpriteSet
//! @return The current sprite index for the given PGESpriteSet
uint32_t pge_spritesheet_get_sprite_index(PGESpriteSheet *spritesheet, uint32_t set_index);

//! Sets the current sprite index for the given PGESpriteSet pointed to by set_index. Has no affect if an invalid parameters passed.
//! @param spritesheet Pointer to the PGESpriteSheet
//! @param set_index Index of the slot for the PGESpriteSet
//! @param sprite_index The index to update for the PGESpriteSet
void pge_spritesheet_set_sprite_index(PGESpriteSheet *spritesheet, uint32_t set_index, uint32_t sprite_index);

//! Sets the position of where to draw the sprite of the PGESpriteSet pointed to by set_index
//! @param spritesheet Pointer to the PGESpriteSheet
//! @param set_index   Index of the PGESpriteSet to update
//! @param position    The GPoint of where the sprite will be drawn when pge_spritesheet_draw is called
void pge_spritesheet_set_sprite_position(PGESpriteSheet *spritesheet, uint32_t set_index, GPoint position);

//! Draws the sprite for a given PGESpriteSet based on the current sprite_index
//! @param spritesheet Pointer to the PGESpriteSheet
//! @param set_index   Index of the PGESpriteSet to draw
void pge_spritesheet_draw(GContext *ctx, PGESpriteSheet *spritesheet, uint32_t set_index);

//! Returns the number of sprites in a given PGESpriteSet pointed to by set_index
//! @param spritesheet Pointer to the PGESpriteSheet
//! @param set_index   Index of the PGESpriteSet to update
//! @return The number of sprites in a given PGESpriteSet pointed to by set_index
uint32_t pge_spritesheet_get_num_sprites(PGESpriteSheet *spritesheet, uint32_t set_index);

