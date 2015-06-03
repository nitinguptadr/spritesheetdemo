#include <pebble.h>
#include "pge/pge.h"
#include "pge/additional/pge_spritesheet.h"
#include "pge/additional/pge_tilesheet.h"

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
PGESprite* mario_large;
PGESpriteTableHandle sth;
uint32_t mario_index = 0;
bool anim_forward = true;

PGESprite* bush1;
PGESprite* bush2;
PGESprite* bush3;
PGESprite* cloud;

PGETileSheetHandle s_tilesheet_handle;
GSize s_tilesheet_size;

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

static int16_t ground_position = 16;

typedef enum {
  JUMP_STATE_NONE,
  JUMP_STATE_UP,
  JUMP_STATE_TOP,
  JUMP_STATE_DOWN
} JumpState;

static JumpState jump_state = JUMP_STATE_NONE;
#define INITIAL_MARIO_POSITION (GPoint(40, 168-32-32))
static GPoint mario_position = {40, 168-32-32};
static GPoint bush_position;
static GPoint cloud_position;
static uint32_t top_count = 0;

void logic() {
}

//bush - 11, 8 and 12, 8

void draw(GContext *ctx) {
  if (auto_increment) {
    ground_position -= 4;
    if (ground_position == 0) {
      ground_position = 16;
    }
    bush_position.x -= 4;
    if (bush_position.x == -40) {
      bush_position.x = 144;
    }

    cloud_position.x -= 4;
    if (cloud_position.x == -96) {
      cloud_position.x = 144;
    }
  }

  if (auto_increment) {
    if (jump_state == JUMP_STATE_NONE) {
      if (anim_forward) {
        mario_index++;
        if (mario_index == 4) {
          anim_forward = !anim_forward;
        }
      } else {
        mario_index--;
        if (mario_index == 2) {
          anim_forward = !anim_forward;
        }
      }
    }
  } else if (jump_state == JUMP_STATE_NONE) {
    mario_index = 1;
  }

  if (jump_state == JUMP_STATE_UP) {
      mario_position.y -= 4;
      if (mario_position.y == INITIAL_MARIO_POSITION.y - 40) {
        jump_state = JUMP_STATE_TOP;
      }
  } else if (jump_state == JUMP_STATE_TOP) {
    if (top_count++ > 4) {
      top_count = 0;
      jump_state = JUMP_STATE_DOWN;
    }
  } else if (jump_state == JUMP_STATE_DOWN) {
    mario_position.y += 4;
    if (mario_position.y == INITIAL_MARIO_POSITION.y) {
      jump_state = JUMP_STATE_NONE;
      mario_index = 3;
      anim_forward = true;
    }
  }

  pge_sprite_set_position(mario_large, mario_position);
  pge_spritesheet_set_anim_frame(mario_large, sth, "mario_large", mario_index);

#ifdef PBL_PLATFORM_BASALT
  graphics_context_set_fill_color(ctx, GColorVividCerulean);
  graphics_fill_rect(ctx, GRect(0, 0, 144, 168), 0, GCornerNone);
#endif

  // Draw each sprite
  graphics_context_set_compositing_mode(ctx, GCompOpSet);

  GPoint draw_bush_position = bush_position;
  pge_sprite_set_position(bush1, draw_bush_position);
  pge_sprite_draw(bush1, ctx);

  draw_bush_position.x += 16;
  pge_sprite_set_position(bush2, draw_bush_position);
  pge_sprite_draw(bush2, ctx);

  draw_bush_position.x += 16;
  pge_sprite_set_position(bush3, draw_bush_position);
  pge_sprite_draw(bush3, ctx);


  GPoint draw_cloud_position = cloud_position;
  pge_sprite_set_position(cloud, draw_cloud_position);
  pge_sprite_draw(cloud, ctx);
  draw_cloud_position.x += 36;
  draw_cloud_position.y += 24;
  pge_sprite_set_position(cloud, draw_cloud_position);
  pge_sprite_draw(cloud, ctx);

  pge_sprite_draw(mario_large, ctx);

  pge_tilesheet_draw_grid(ctx, s_tilesheet_handle, GRect(0, 0, s_tilesheet_size.w, s_tilesheet_size.h), GPoint((ground_position - 16), 168-32), GSize(16, 16));
}

// Optional, can be NULL if only using pge_get_button_state()
void click(int button_id, bool long_click) {
  if ((button_id == BUTTON_ID_UP) && (jump_state == JUMP_STATE_NONE)) {
    mario_index = 15;
    jump_state = JUMP_STATE_UP;
    anim_forward = false;
  } else if (button_id == BUTTON_ID_DOWN) {
  } else if (button_id == BUTTON_ID_SELECT) {
    auto_increment = !auto_increment;
  }
}

void pge_init() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Begin game");
  s_window = pge_begin(GColorBlack, logic, draw, click);
  pge_set_framerate(20);
  s_spritesheet = pge_spritesheet_create(RESOURCE_ID_MARIOSPRITESHEET, NUM_MARIO_SPRITESETS);

  if (s_spritesheet) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Created spritesheet");
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to create spritesheet");
  }

  sth = pge_spritesheet_load_table(RESOURCE_ID_MARIOSPRITESHEET_TILESETS);
  s_tilesheet_handle = pge_tilesheet_create(RESOURCE_ID_MARIOSPRITESHEET_TILESHEET0, sth);
  if (!s_tilesheet_handle) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "ERROR: Unable to create tilesheet");
  }
  mario_index = 2;
  mario_large = pge_spritesheet_create_sprite(sth, "mario_large", mario_index, INITIAL_MARIO_POSITION);
  anim_forward = true;

  bush_position = GPoint(80, INITIAL_MARIO_POSITION.y + 16);
  bush1 = pge_spritesheet_create_sprite(sth, "mariotiles", 9*33 + 14 - 2, bush_position);
  bush_position.x += 16;
  bush2 = pge_spritesheet_create_sprite(sth, "mariotiles", 9*33 + 14 - 1, bush_position);
  bush_position.x += 16;
  bush3 = pge_spritesheet_create_sprite(sth, "mariotiles", 9*33 + 14, bush_position);

  cloud_position = GPoint(20, 10);
  cloud = pge_spritesheet_create_sprite(sth, "cloud", 1, cloud_position);

  s_tilesheet_size = pge_tilesheet_get_tilesheet_size(s_tilesheet_handle);
}

void pge_deinit() {
  pge_spritesheet_destroy(s_spritesheet);

  // Destroy all game resources
  pge_finish();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "End game");
}
