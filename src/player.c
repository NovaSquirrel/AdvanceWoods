#include <tonc.h>
#include "woods.h"

IWRAM_CODE void random_seed(pcg32_random_t *state, uint64_t seed, uint64_t sequence) {
	pcg32_srandom_r(state, seed, sequence);
}

IWRAM_CODE uint32_t random_raw(struct player_state *p) {
	return pcg32_random_r(&p->random_state);
}

IWRAM_CODE uint32_t random_max(struct player_state *p, uint32_t bound) {
	return pcg32_boundedrand_r(&p->random_state, bound);
}

IWRAM_CODE uint32_t random_min_max(struct player_state *p, uint32_t min, uint32_t max) {
	return pcg32_boundedrand_r(&p->random_state, max-min)+min;
}

void init_player(struct player_state *p, uint64_t seed, uint64_t sequence) {
	memset32(p, 0, sizeof(struct player_state)/4);
	random_seed(&p->random_state, seed, sequence);

	p->player_x = 2;
	p->player_y = 4;

	// Create a random playfield
	for(int i=0; i<PLAYFIELD_W; i++) {
		for(int j=5; j<PLAYFIELD_H; j++) {
			int color;
			while(1) {
				color = (random_raw(p)&3)+BLOCK_RED;
				if(i > 1 && p->playfield[i-1][j].type == color && p->playfield[i-2][j].type == color)
					continue;
				if(j > 1 && p->playfield[i][j-1].type == color && p->playfield[i][j-2].type == color)
					continue;
				break;
			}
			p->playfield[i][j].type = color;
		}
	}
}

static int solid_xy(struct player_state *p, int x, int y) {
	if(x >= PLAYFIELD_W || x < 0 || y >= PLAYFIELD_H || y < 0) {
		return 1;
	}
	return p->playfield[x][y].type && (p->playfield[x][y].flags & BLOCK_CARRIED) == 0;
}

static int can_pickup_xy(struct player_state *p, int x, int y) {
	if(x >= PLAYFIELD_W || x < 0 || y >= PLAYFIELD_H || y < 0) {
		return 0;
	}
	return p->playfield[x][y].type;
}

void update_player(struct player_state *p) {
	p->key_new = ~p->key_last & p->key_down;
	p->key_last = p->key_down;

	if((p->key_down & (KEY_LEFT | KEY_RIGHT)) == 0) {
		p->key_repeat_timer = 0;
	} else if(p->key_repeat_timer < 60) {
		p->key_repeat_timer++;
	}

	int old_move = p->current_move;
	if(p->move_timer) {
		p->move_timer--;
		if(!p->move_timer) {
			p->current_move = 0;
		}
	}

	if(p->key_down & KEY_LEFT) {
		if(p->player_dir == 1 && !p->current_move && (p->key_repeat_timer > 8 || old_move == MOVE_DOWN) ) {
			if(!solid_xy(p, p->player_x-1, p->player_y)) {
				if(solid_xy(p, p->player_x-1, p->player_y+1) || solid_xy(p, p->player_x, p->player_y+1)) {
					p->buffered_move = MOVE_LEFT;
				}
			} else if(!solid_xy(p, p->player_x, p->player_y-1)){
				p->buffered_move = MOVE_UP;
			}
		}
		p->player_dir = 1;
	}

	if(p->key_down & KEY_RIGHT) {
		if(p->player_dir == 0 && !p->current_move && (p->key_repeat_timer > 8 || old_move == MOVE_DOWN) ) {
			if(!solid_xy(p, p->player_x+1, p->player_y)) {
				if(solid_xy(p, p->player_x+1, p->player_y+1) || solid_xy(p, p->player_x+1, p->player_y+1)) {
					p->buffered_move = MOVE_RIGHT;
				}
			} else if(!solid_xy(p, p->player_x, p->player_y-1)){
				p->buffered_move = MOVE_UP;
			}
		}
		p->player_dir = 0;
	}

	if(p->key_new & KEY_GET_ONE) {
		p->buffered_move = MOVE_PICK_UP_ONE;
	}
	if(p->key_new & KEY_GET_ALL) {
		p->buffered_move = MOVE_PICK_UP_ALL;
	}

	// Apply buffered move

	int grab_x = p->player_x + ((p->player_dir) ? -1 : 1);
	int grab_y = p->player_y;
	if(!can_pickup_xy(p, grab_x, grab_y)) {
		grab_y++;
	}

	if(!p->current_move && p->buffered_move) {
		p->current_move = p->buffered_move;
		switch(p->current_move) {
			case MOVE_LEFT:
				p->player_x--;
				break;
			case MOVE_RIGHT:
				p->player_x++;
				break;
			case MOVE_UP:
				p->player_y--;
				break;
			case MOVE_PICK_UP_ONE:
				if(!p->carrying && can_pickup_xy(p, grab_x, grab_y)) {
					p->playfield[p->player_x][p->player_y-1].type = p->playfield[grab_x][grab_y].type;
					p->playfield[p->player_x][p->player_y-1].flags = BLOCK_CARRIED;
					p->playfield[grab_x][grab_y].type = 0;
					p->carrying = 1;
				}
				break;
			case MOVE_PICK_UP_ALL:
				break;
		}
		if(p->current_move) {
			p->move_timer = 8;
		}
		p->buffered_move = 0;
	}

	if(!solid_xy(p, p->player_x, p->player_y+1) && !p->current_move) {
		p->player_y++;
		p->current_move = MOVE_DOWN;
		p->move_timer = 8;
	}
}
