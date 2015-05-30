#pragma once
 
#include <pebble.h>
#include "pge/additional/pge_sprite.h"

// Sprite Set - This is a collection of sprites images for a given set. For example, if you have an 
// animated sprite that is split into 16 sprites, then that set of 16 sprites can be grouped as one
// PGESpriteSet so long as they are arranged consecutively in the sprite sheet. The sprites must be
// arranged in a linear or grid fashion. Sprites are indexed from left to right, one row at a time
// starting from 0. e.g. If there are 4 sprites per row and 4 rows of sprites, then sprite index 5
// corresponds to the second row, second column.
typedef struct {
  uint32_t sprite_index;        // Index within this sprite set of current sprite to draw, Default is 0
  GRect frame;                  // Offset and size of the sprite set within the source GBitmap image
  int16_t xspacing;             // Horizontal pixel spacing between individual sprites within a set, Default is 0
  int16_t yspacing;             // Vertical pixel spacing between individual sprites within a set, Default is 0
  uint32_t num_sprites;         // Total number of sprites within sprite set - calculated based on frame and sprite_size
  uint32_t num_sprites_in_row;  // Number of sprites within a row of the sprite set
  uint32_t num_sprites_in_col;  // Number of sprites within a column of the sprite set
  GPoint position;              // Point on screen where to draw the sprite
  GSize sprite_size;            // Size of an individual sprite within this sprite set
} PGESpriteSet;

// Sprite Sheet
// This stores the pointer to the GBitmap as well as a pointer to the array of PGESpriteSet that is in the sprite sheet.
// This serves as the main point of access to all sprite sets within a sprite sheet.
typedef struct {
  GBitmap *bitmap;      // Pointer to the GBitmap image for a given sprite sheet
  uint32_t num_sets;    // Number of sprite sets
  PGESpriteSet *sets;   // Array of PGESpriteSet
} PGESpriteSheet;

#define INVALID_SET_INDEX ~(0)
#define INVALID_SPRITE_INDEX ~(0)

//! Creates an empty sprite sheet from a given resource image
//! @param resource_id Resource id of the sprite sheet image to load
//! @param num_sets Number of PGESpriteSet to create
//! @return Pointer to the created PGESpriteSheet
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
uint32_t pge_spritesheet_add_set(PGESpriteSheet *spritesheet, GRect frame, GSize sprite_size,
                                 int16_t xspacing, int16_t yspacing);

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

//! Gets the current position of the sprite of the PGESpriteSet pointed to by set_index
//! @param spritesheet Pointer to the PGESpriteSheet
//! @param set_index Index of the slot for the PGESpriteSet
//! @return The current position on screen of the PGESpriteSet
GPoint pge_spritesheet_get_sprite_position(PGESpriteSheet *spritesheet, uint32_t set_index);

//! Sets the position of where to draw the sprite of the PGESpriteSet pointed to by set_index
//! @param spritesheet Pointer to the PGESpriteSheet
//! @param set_index Index of the slot for the PGESpriteSet to update
//! @param position The GPoint of where the sprite will be drawn when pge_spritesheet_draw is called
void pge_spritesheet_set_sprite_position(PGESpriteSheet *spritesheet, uint32_t set_index, GPoint position);

//! Gets the bounds (position on screen and size) of the sprite of the PGESpriteSet pointed to by set_index
//! @param spritesheet Pointer to the PGESpriteSheet
//! @param set_index Index of the slot for the PGESpriteSet
//! @return The bounds of the sprite where origin is set to the current position and size is based on the size
//!         calculated when the PGESpriteSet was created
GRect pge_spritesheet_get_sprite_bounds(PGESpriteSheet *spritesheet, uint32_t set_index);

//! Draws the sprite for a given PGESpriteSet based on the current sprite_index
//! @param spritesheet Pointer to the PGESpriteSheet
//! @param set_index Index of the slot for the PGESpriteSet to draw
void pge_spritesheet_draw(GContext *ctx, PGESpriteSheet *spritesheet, uint32_t set_index);

//! Returns the number of sprites in a given PGESpriteSet pointed to by set_index
//! @param spritesheet Pointer to the PGESpriteSheet
//! @param set_index Index of the PGESpriteSet to update
//! @return The number of sprites in a given PGESpriteSet pointed to by set_index
uint32_t pge_spritesheet_get_num_sprites(PGESpriteSheet *spritesheet, uint32_t set_index);

