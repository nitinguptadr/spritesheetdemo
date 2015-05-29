#include <pebble.h>
#include "pge/pge.h"
#include "pge/additional/pge_spritesheet.h"

#define NUM_MARIO_SPRITESETS 6
#define INDEX_BIG_MARIO      0
#define INDEX_SMALL_MARIO    1
#define INDEX_OTHER_MARIO    2
#define INDEX_SMALL_MARIO2   3

#define MIN(a, b) (a > b ? b : a)

Window *s_window;
PGESpriteSheet *s_spritesheet;
uint32_t s_mario_spritesets[NUM_MARIO_SPRITESETS];
uint32_t s_num_sprites[NUM_MARIO_SPRITESETS];
bool auto_increment = false;

// Increment or decrement the sprite_index for a given PGESpriteSet pointed to by set_index.
// Wrap around if reached 0 or the number of sprites in a set.
static void update_index(bool increment, uint32_t set_index) {
  uint32_t sprite_index = pge_spritesheet_get_sprite_index(s_spritesheet, set_index);
  if (increment) {
    if (sprite_index + 1 >= s_num_sprites[set_index]) {
      pge_spritesheet_set_sprite_index(s_spritesheet, set_index, 0);
    } else {
      pge_spritesheet_set_sprite_index(s_spritesheet, set_index, sprite_index + 1);
    }
  } else {
    if (sprite_index ==  0) {
      pge_spritesheet_set_sprite_index(s_spritesheet, set_index, s_num_sprites[set_index] - 1);
    } else {
      pge_spritesheet_set_sprite_index(s_spritesheet, set_index, sprite_index - 1);
    }
  }
}

void logic() {
}

void draw(GContext *ctx) {
  if (auto_increment) {
    // Automatically increment the sprite index for each set
    update_index(true, INDEX_BIG_MARIO);
    update_index(true, INDEX_SMALL_MARIO);
    update_index(true, INDEX_OTHER_MARIO);
    update_index(true, INDEX_SMALL_MARIO2);
  }

  // Draw each sprite
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  pge_spritesheet_draw(ctx, s_spritesheet, INDEX_BIG_MARIO);
  pge_spritesheet_draw(ctx, s_spritesheet, INDEX_SMALL_MARIO);
  pge_spritesheet_draw(ctx, s_spritesheet, INDEX_OTHER_MARIO);
  pge_spritesheet_draw(ctx, s_spritesheet, INDEX_SMALL_MARIO2);
}

// Optional, can be NULL if only using pge_get_button_state()
void click(int button_id, bool long_click) {
  if (button_id == BUTTON_ID_UP) {
    // Increment the sprite index for each set
    update_index(true, INDEX_BIG_MARIO);
    update_index(true, INDEX_SMALL_MARIO);
    update_index(true, INDEX_OTHER_MARIO);
    update_index(true, INDEX_SMALL_MARIO2);
  } else if (button_id == BUTTON_ID_DOWN) {
    // Decrement the sprite index for each set
    update_index(false, INDEX_BIG_MARIO);
    update_index(false, INDEX_SMALL_MARIO);
    update_index(false, INDEX_OTHER_MARIO);
    update_index(false, INDEX_SMALL_MARIO2);
  } else if (button_id == BUTTON_ID_SELECT) {
    auto_increment = !auto_increment;
  }
}

void pge_init() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Begin game");
  s_window = pge_begin(GColorBlack, logic, draw, click);
  pge_set_framerate(8);
  s_spritesheet = pge_spritesheet_create(RESOURCE_ID_MARIOSPRITESHEET, NUM_MARIO_SPRITESETS);

  if (s_spritesheet) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Created spritesheet");
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to create spritesheet");
  }

  pge_spritesheet_add_set(s_spritesheet, GRect(80, 0, 336, 32), GSize(16, 32), 0, 0);
  s_num_sprites[INDEX_BIG_MARIO] = pge_spritesheet_get_num_sprites(s_spritesheet, INDEX_BIG_MARIO);
  pge_spritesheet_set_sprite_position(s_spritesheet, INDEX_BIG_MARIO, GPoint(40, 10));

  pge_spritesheet_add_set(s_spritesheet, GRect(80, 32, 224, 16), GSize(16, 16), 0, 0);
  s_num_sprites[INDEX_SMALL_MARIO] = pge_spritesheet_get_num_sprites(s_spritesheet, INDEX_SMALL_MARIO);
  pge_spritesheet_set_sprite_position(s_spritesheet, INDEX_SMALL_MARIO, GPoint(40, 100));

  // Demonstrate vertical spacing and horizontal spacing
  // Note image will be cut off horizontally since there is no actual spacing between sprites - this simulates the spacing
  pge_spritesheet_add_set(s_spritesheet, GRect(80, 48, 336, 64+16), GSize(12, 32), 4, 16);
  s_num_sprites[INDEX_OTHER_MARIO] = pge_spritesheet_get_num_sprites(s_spritesheet, INDEX_OTHER_MARIO);
  pge_spritesheet_set_sprite_position(s_spritesheet, INDEX_OTHER_MARIO, GPoint(40, 130));

  // Demonstrate horizontal spacing
  // Note image will be cut off horizontally since there is no actual spacing between sprites - this simulates the spacing
  pge_spritesheet_add_set(s_spritesheet, GRect(80, 128, 224, 16), GSize(8, 16), 8, 0);
  s_num_sprites[INDEX_SMALL_MARIO2] = pge_spritesheet_get_num_sprites(s_spritesheet, INDEX_SMALL_MARIO2);
  pge_spritesheet_set_sprite_position(s_spritesheet, INDEX_SMALL_MARIO2, GPoint(100, 130));
}

void pge_deinit() {
  pge_spritesheet_destroy(s_spritesheet);

  // Destroy all game resources
  pge_finish();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "End game");
}
