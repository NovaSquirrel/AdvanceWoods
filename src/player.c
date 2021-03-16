#include <tonc.h>
#include "woods.h"

#define block_is_bomb(x) (x >= BOMB_RED && x <= DIAMOND_GRAY)
#define block_is_explosion(x) (x >= EXPLOSION_RED && x <= EXPLOSION_GRAY)

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
	p->playfield[0][0].type = BOMB_RED;
	p->playfield[1][0].type = BOMB_GREEN;
	p->playfield[3][0].type = BOMB_BLUE;
	p->playfield[4][0].type = BOMB_YELLOW;
	p->playfield[0][1].type = BOMB_RED;
	p->playfield[1][1].type = BOMB_GREEN;
	p->playfield[3][1].type = BOMB_BLUE;
	p->playfield[4][1].type = BOMB_YELLOW;
}

void check_for_floating(struct player_state *p) {
	// Start blocks falling if needed
	for(int x=0; x<PLAYFIELD_W; x++) {
		for(int y=0; y<PLAYFIELD_H-1; y++) {
			if(p->playfield[x][y].type && !p->playfield[x][y+1].type) {
				p->board_state = BOARD_BLOCKS_FALLING;
				return;
			}
		}
	}
}

int xy_in_range(int x, int y) {
	return x < PLAYFIELD_W && x >= 0 && y < PLAYFIELD_H && y >= 0;
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

void make_blocks_fall(struct player_state *p) {
	int any_fell = 0;
	for(int x=0; x<PLAYFIELD_W; x++) {
		for(int y=PLAYFIELD_H-1; y>0; y--) {
			if(!p->playfield[x][y].type && p->playfield[x][y-1].type && !p->playfield[x][y-1].flags) {
				p->playfield[x][y].type = p->playfield[x][y-1].type;
				p->playfield[x][y-1].type = 0;
				any_fell = 1;
			}
		}
	}
	if(!any_fell)
		p->board_state = BOARD_IDLE;
}

int check_if_can_offset_stack(struct player_state *p, int x, int y, int new_x, int offset_y) {
	// Can check about stuff going out of bounds horizontally once
	if(new_x >= PLAYFIELD_W || new_x < 0)
		return 0;

	while(y >= 0) {
		// Can't put stuff out of bounds
		 if(y+offset_y < 0)
			return 0;
		// Also can't put stuff in other stuff
		if(p->playfield[new_x][y+offset_y].type)
			return 0;
		y--;
		if(!p->playfield[x][y].type) // Reached the end of a stack
			return 1;
	}
	return 1;
}

void shift_carried_down(struct player_state *p) {
	for(int y=PLAYFIELD_H-2; y>=0; y--) {
		if(!(p->playfield[p->player_x][y].flags & BLOCK_CARRIED))
			continue;
		p->playfield[p->player_x][y+1] = p->playfield[p->player_x][y];
		p->playfield[p->player_x][y].type = 0;
		p->playfield[p->player_x][y].flags = 0;
	}
}

void move_stack(struct player_state *p, int old_x, int old_y, int new_x, int new_y, int new_flags) {
	for(int i=0; i<PLAYFIELD_H; i++) {
		if(old_y - i < 0)
			break;
		if(!p->playfield[old_x][old_y-i].type)
			break;
		p->playfield[new_x][new_y-i].type = p->playfield[old_x][old_y-i].type;
		p->playfield[new_x][new_y-i].flags = new_flags;
		p->playfield[old_x][old_y-i].type = 0;
		p->playfield[old_x][old_y-i].flags = 0;
	}
}

const int dir_x[8] = {1, 1, 0, -1, -1, -1, 0, 1};
const int dir_y[8] = {0, 1, 1, 1, 0, -1, -1, -1};

void update_player(struct player_state *p) {
	if((p->key_down & (KEY_LEFT | KEY_RIGHT)) == 0 ||
		(p->key_down & (KEY_LEFT | KEY_RIGHT)) != (p->key_last & (KEY_LEFT | KEY_RIGHT))
	) {
		p->key_repeat_timer = 0;
	} else if(p->key_repeat_timer < 60) {
		p->key_repeat_timer++;
	}
	p->key_new = ~p->key_last & p->key_down;
	p->key_last = p->key_down;

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
				if( (solid_xy(p, p->player_x-1, p->player_y+1) && old_move == MOVE_UP)
				|| solid_xy(p, p->player_x, p->player_y+1)) {
					p->buffered_move = MOVE_LEFT;
				}
			} else if(!solid_xy(p, p->player_x, p->player_y-1) && !p->carrying){
				p->buffered_move = MOVE_UP;
			}
		}
		p->player_dir = 1;
	}

	if(p->key_down & KEY_RIGHT) {
		if(p->player_dir == 0 && !p->current_move && (p->key_repeat_timer > 8 || old_move == MOVE_DOWN) ) {
			if(!solid_xy(p, p->player_x+1, p->player_y)) {
				if( (solid_xy(p, p->player_x+1, p->player_y+1) && old_move == MOVE_UP)
				|| solid_xy(p, p->player_x, p->player_y+1)) {
					p->buffered_move = MOVE_RIGHT;
				}
			} else if(!solid_xy(p, p->player_x, p->player_y-1) && !p->carrying){
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
	if(p->key_new & KEY_UP) {
		p->buffered_move = MOVE_TOP_OF_STACK;
	}

	switch(p->board_state) {
		case BOARD_EXPLODING:
			p->state_timer--;
			if(!p->state_timer) {
				p->board_state = BOARD_IDLE;
				for(int x=0; x<PLAYFIELD_W; x++) {
					for(int y=0; y<PLAYFIELD_H; y++) {
						if(block_is_explosion(p->playfield[x][y].type)) {
							p->playfield[x][y].type = 0;
						}
					}
				}
				check_for_floating(p);
			}
			break;
		case BOARD_BLOCKS_FALLING:
			make_blocks_fall(p);
			break;
		default:
			break;
	}

	// Apply buffered move

	int grab_x = p->player_x + ((p->player_dir) ? -1 : 1);
	int grab_y = p->player_y;
	if(!can_pickup_xy(p, grab_x, grab_y)) {
		grab_y++;
	}

	if(!p->current_move && p->buffered_move && p->board_state == BOARD_IDLE) {
		p->current_move = p->buffered_move;
		switch(p->current_move) {
			case MOVE_LEFT:
				if(p->carrying) { // Offset carried blocks left
					for(int y=0; y<PLAYFIELD_H; y++) {
						if(!(p->playfield[p->player_x][y].flags & BLOCK_CARRIED))
							continue;
						if(p->playfield[p->player_x-1][y].type) {
							p->playfield[p->player_x][y].flags = 0; // No longer carried
						} else {
							p->playfield[p->player_x-1][y] = p->playfield[p->player_x][y];
							p->playfield[p->player_x][y].type = 0;
							p->playfield[p->player_x][y].flags = 0;
						}
					}
				}
				p->player_x--;
				break;
			case MOVE_RIGHT:
				if(p->carrying) { // Offset carried blocks right
					for(int y=0; y<PLAYFIELD_H; y++) {
						if(!(p->playfield[p->player_x][y].flags & BLOCK_CARRIED))
							continue;
						if(p->playfield[p->player_x+1][y].type) {
							p->playfield[p->player_x][y].flags = 0; // No longer carried
						} else {
							p->playfield[p->player_x+1][y] = p->playfield[p->player_x][y];
							p->playfield[p->player_x][y].type = 0;
							p->playfield[p->player_x][y].flags = 0;
						}
					}
				}
				p->player_x++;
				break;
			case MOVE_UP:
				p->player_y--;
				break;
			case MOVE_PICK_UP_ONE:
				if(!p->carrying && can_pickup_xy(p, grab_x, grab_y) && !p->playfield[p->player_x][p->player_y-1].type) {
					p->playfield[p->player_x][p->player_y-1].type = p->playfield[grab_x][grab_y].type;
					p->playfield[p->player_x][p->player_y-1].flags = BLOCK_CARRIED;
					p->playfield[grab_x][grab_y].type = 0;
					p->carrying = 1;
				} else if(p->carrying) {
					int target_y;
					if(!p->playfield[grab_x][p->player_y].type) {
						target_y = p->player_y;
					} else if(!p->playfield[grab_x][p->player_y-1].type)  {
						target_y = p->player_y - 1;
					} else if(!p->playfield[grab_x][p->player_y-2].type) {
						target_y = p->player_y - 2;
					} else {
						break;
					}

					p->playfield[grab_x][target_y].type = p->playfield[p->player_x][p->player_y-1].type;
					p->playfield[p->player_x][p->player_y-1].type = 0; 
					p->playfield[p->player_x][p->player_y-1].flags = 0; 
					shift_carried_down(p);
					if(!p->playfield[p->player_x][p->player_y-1].type)
						p->carrying = 0;
				}
				break;
			case MOVE_PICK_UP_ALL:
				if(!p->carrying && can_pickup_xy(p, grab_x, grab_y) && check_if_can_offset_stack(p, grab_x, grab_y, p->player_x, -1)) {
					move_stack(p, grab_x, grab_y, p->player_x, p->player_y-1, BLOCK_CARRIED);
					p->carrying = 1;
				} else if(p->carrying) {
					int target_y;
					if(check_if_can_offset_stack(p, p->player_x, p->player_y-1, grab_x, 1)) {
						target_y = p->player_y;
					} else if(check_if_can_offset_stack(p, p->player_x, p->player_y-1, grab_x, 0)) {
						target_y = p->player_y - 1;
					} else if(check_if_can_offset_stack(p, p->player_x, p->player_y-1, grab_x, -1)) {
						target_y = p->player_y - 2;
					} else {
						break;
					}
					move_stack(p, p->player_x, p->player_y-1, grab_x, target_y, 0);
					p->carrying = 0;
				}
				break;
			case MOVE_TOP_OF_STACK:
				if(!p->carrying)
					break;
				move_stack(p, p->player_x, p->player_y-1, p->player_x, p->player_y, 0);
				while(p->playfield[p->player_x][p->player_y].type)
					p->player_y--;
				p->carrying = 0;
				break;
		}
		if(p->current_move) {
			p->move_timer = 8;
		}
		p->buffered_move = 0;
	}

	// Start player falling
	if(!solid_xy(p, p->player_x, p->player_y+1) && !p->current_move) {
		if(p->carrying) { // Offset carried blocks down
			shift_carried_down(p);
		}
		p->player_y++;
		p->current_move = MOVE_DOWN;
		p->move_timer = 8;
	}

	if(p->board_state == BOARD_IDLE) {
		check_for_floating(p);

		// Otherwise look for matches
		if(p->board_state == BOARD_IDLE) {
			for(int x=0; x<PLAYFIELD_W; x++) {
				for(int y=0; y<PLAYFIELD_H; y++) {
					if(!block_is_bomb(p->playfield[x][y].type))
						continue;
					int this_color = block_color[p->playfield[x][y].type];
					int count_in_direction[8] = {0, 0, 0, 0, 0, 0, 0, 0};
					for(int dir = 0; dir<8; dir++) {
						int count = 0;
						int nx = x + dir_x[dir];
						int ny = y + dir_y[dir];
						while(xy_in_range(nx, ny) && this_color == block_color[p->playfield[nx][ny].type]) {
							nx += dir_x[dir];
							ny += dir_y[dir];
							count++;
						}
						count_in_direction[dir] = count;
					}

					// Is it enough?
					int any_exploded = 0;
					for(int dir = 0; dir<4; dir++) {
						if(count_in_direction[dir] + count_in_direction[dir+4] >= 2) {
							any_exploded = 1;
							for(int i = 0, nx=x, ny=y; i<=count_in_direction[dir]; i++, nx+=dir_x[dir], ny+=dir_y[dir]) {
								p->playfield[nx][ny].type = EXPLOSION_RED + this_color;
								p->playfield[nx][ny].flags = 0;
							}
							for(int i = 0, nx=x, ny=y; i<=count_in_direction[dir+4]; i++, nx+=dir_x[dir+4], ny+=dir_y[dir+4]) {
								p->playfield[nx][ny].type = EXPLOSION_RED + this_color;
								p->playfield[nx][ny].flags = 0;
							}
						}
					}
					if(any_exploded) {
						p->playfield[x][y].type = EXPLOSION_RED + this_color;
						p->playfield[x][y].flags = 0;
						p->board_state = BOARD_EXPLODING;
						p->state_timer = 8;
					}
				}
			}
		}
	}

}
