#ifndef WOODS_HEADER
#define WOODS_HEADER

#include <stdint.h>
#include "pcg_variants.h"

#define PLAYFIELD_W 6
#define PLAYFIELD_H 10

enum {
	BLOCK_CARRIED = 1
};

struct block {
	uint8_t type;
	uint8_t flags;
};

enum BLOCK_TYPE {
	BLOCK_EMPTY,
	BLOCK_RED,
	BLOCK_YELLOW,
	BLOCK_GREEN,
	BLOCK_BLUE,
	BLOCK_BROWN,
	BLOCK_GRAY,
	BOMB_RED,
	BOMB_YELLOW,
	BOMB_GREEN,
	BOMB_BLUE,
	BOMB_BROWN,
	BOMB_GRAY,
	DIAMOND_RED,
	DIAMOND_YELLOW,
	DIAMOND_GREEN,
	DIAMOND_BLUE,
	DIAMOND_BROWN,
	DIAMOND_GRAY
};

enum board_state {
	BOARD_STARTUP,
	BOARD_IDLE,
	BOARD_WALKING,
	BOARD_EXPLODING,
};

enum player_move {
	MOVE_NONE,
	MOVE_LEFT,
	MOVE_RIGHT,
	MOVE_UP,
	MOVE_DOWN,
	MOVE_PICK_UP_ONE,
	MOVE_PICK_UP_ALL,
	MOVE_PUT_DOWN,
};

#define KEY_GET_ONE    KEY_B
#define KEY_GET_ALL    KEY_A
#define KEY_KICK_LEFT  KEY_L
#define KEY_KICK_RIGHT KEY_R

ALIGN4 struct player_state {
	pcg32_random_t random_state;
	int frame, animation_frame;
	int player_x, player_y, player_dir;
	unsigned int key_down, key_last, key_new;

	uint_fast8_t key_repeat_timer;

	int carrying;
	int current_move;
	int buffered_move;
	int move_timer;

	struct block playfield[PLAYFIELD_W][PLAYFIELD_H];
};

extern OBJ_ATTR obj_buffer[128];         // Mirror
extern OBJ_AFFINE *const obj_aff_buffer;
extern int obj_used;                     // Number of sprites used so far

IWRAM_CODE void random_seed(pcg32_random_t *state, uint64_t seed, uint64_t sequence);
IWRAM_CODE uint32_t random_raw(struct player_state *p);
IWRAM_CODE uint32_t random_max(struct player_state *p, uint32_t bound);
IWRAM_CODE uint32_t random_min_max(struct player_state *p, uint32_t min, uint32_t max);

#endif
