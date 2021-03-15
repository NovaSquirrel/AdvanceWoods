#include <tonc.h>
#include "woods.h"

#include "SolidTiles_chr.h"
#include "Blocks_chr.h"
#include "Rocks_chr.h"
#include "Nova_chr.h"
#include "palettedata.h"

#define ROCKS_TILE_NUM 16
#define BLOCKS_TILE_NUM 24

#define BLOCKS_PAL_NUM1 0 
#define BLOCKS_PAL_NUM2 1
#define ROCKS_PAL_NUM 2

#define PLAYER_PAL_NUM 0

#define SB_NUM 31

struct player_state players[2];

void init_gameplay() {
	memcpy32(&tile_mem[0][0], SolidTiles_chrTiles, SolidTiles_chrTilesLen/4);
	memcpy32(&tile_mem[0][16], Rocks_chrTiles, Rocks_chrTilesLen/4);
	memcpy32(&tile_mem[0][24], Blocks_chrTiles, Blocks_chrTilesLen/4);
	memcpy32(&tile_mem_obj[0][0], Nova_chrTiles, Nova_chrTilesLen/4);

	pal_bg_mem[0] = RGB15(8, 8, 8);
	memcpy16(&pal_bg_bank[0][1], Block1PalData, Block1PalSize/2);
	memcpy16(&pal_bg_bank[1][1], Block2PalData, Block2PalSize/2);
	memcpy16(&pal_bg_bank[ROCKS_PAL_NUM][1], RocksPalData,  RocksPalSize/2);
	memcpy16(&pal_obj_bank[PLAYER_PAL_NUM][1], NovaPalData,  NovaPalSize/2);

	// Set up the tilemap
	REG_BG0CNT= BG_CBB(0) | BG_SBB(SB_NUM) | BG_4BPP | BG_REG_32x32;

	memset16(&se_mem[SB_NUM][0], 0, 1024/2); // Clear to zeros

	// Set up the frame on the sides
	for(int y=0; y<10;y++) {
		se_mat[SB_NUM][y*2+0][0] = SE_PALBANK(ROCKS_PAL_NUM)|(ROCKS_TILE_NUM+3);
		se_mat[SB_NUM][y*2+1][0] = SE_PALBANK(ROCKS_PAL_NUM)|(ROCKS_TILE_NUM+7);
		se_mat[SB_NUM][y*2+0][29] = SE_PALBANK(ROCKS_PAL_NUM)|(ROCKS_TILE_NUM+0);
		se_mat[SB_NUM][y*2+1][29] = SE_PALBANK(ROCKS_PAL_NUM)|(ROCKS_TILE_NUM+4);
		// Bar in the middle
		se_mat[SB_NUM][y*2+0][13] = SE_PALBANK(ROCKS_PAL_NUM)|(ROCKS_TILE_NUM+0);
		se_mat[SB_NUM][y*2+0][14] = SE_PALBANK(ROCKS_PAL_NUM)|(ROCKS_TILE_NUM+1);
		se_mat[SB_NUM][y*2+0][15] = SE_PALBANK(ROCKS_PAL_NUM)|(ROCKS_TILE_NUM+2);
		se_mat[SB_NUM][y*2+0][16] = SE_PALBANK(ROCKS_PAL_NUM)|(ROCKS_TILE_NUM+3);
		se_mat[SB_NUM][y*2+1][13] = SE_PALBANK(ROCKS_PAL_NUM)|(ROCKS_TILE_NUM+4);
		se_mat[SB_NUM][y*2+1][14] = SE_PALBANK(ROCKS_PAL_NUM)|(ROCKS_TILE_NUM+5);
		se_mat[SB_NUM][y*2+1][15] = SE_PALBANK(ROCKS_PAL_NUM)|(ROCKS_TILE_NUM+6);
		se_mat[SB_NUM][y*2+1][16] = SE_PALBANK(ROCKS_PAL_NUM)|(ROCKS_TILE_NUM+7);
	}

	REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ | DCNT_OBJ_1D;
}

const SCR_ENTRY block_appearance[][4] = {
	{0, 0, 0, 0},
	{(BLOCKS_TILE_NUM+4*0+0)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*0+1)|SE_PALBANK(BLOCKS_PAL_NUM1),
	(BLOCKS_TILE_NUM+4*0+2)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*0+3)|SE_PALBANK(BLOCKS_PAL_NUM1)},
	{(BLOCKS_TILE_NUM+4*1+0)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*1+1)|SE_PALBANK(BLOCKS_PAL_NUM1),
	(BLOCKS_TILE_NUM+4*1+2)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*1+3)|SE_PALBANK(BLOCKS_PAL_NUM1)},
	{(BLOCKS_TILE_NUM+4*2+0)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*2+1)|SE_PALBANK(BLOCKS_PAL_NUM1),
	(BLOCKS_TILE_NUM+4*2+2)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*2+3)|SE_PALBANK(BLOCKS_PAL_NUM1)},
	{(BLOCKS_TILE_NUM+4*3+0)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*3+1)|SE_PALBANK(BLOCKS_PAL_NUM1),
	(BLOCKS_TILE_NUM+4*3+2)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*3+3)|SE_PALBANK(BLOCKS_PAL_NUM1)},
	{(BLOCKS_TILE_NUM+4*4+0)|SE_PALBANK(BLOCKS_PAL_NUM2), (BLOCKS_TILE_NUM+4*4+1)|SE_PALBANK(BLOCKS_PAL_NUM2),
	(BLOCKS_TILE_NUM+4*4+2)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*4+3)|SE_PALBANK(BLOCKS_PAL_NUM1)},
	{(BLOCKS_TILE_NUM+4*5+0)|SE_PALBANK(BLOCKS_PAL_NUM2), (BLOCKS_TILE_NUM+4*5+1)|SE_PALBANK(BLOCKS_PAL_NUM2),
	(BLOCKS_TILE_NUM+4*5+2)|SE_PALBANK(BLOCKS_PAL_NUM2), (BLOCKS_TILE_NUM+4*5+3)|SE_PALBANK(BLOCKS_PAL_NUM2)},
	{(BLOCKS_TILE_NUM+4*6+0)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*6+1)|SE_PALBANK(BLOCKS_PAL_NUM1),
	(BLOCKS_TILE_NUM+4*6+2)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*6+3)|SE_PALBANK(BLOCKS_PAL_NUM1)},
	{(BLOCKS_TILE_NUM+4*7+0)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*7+1)|SE_PALBANK(BLOCKS_PAL_NUM1),
	(BLOCKS_TILE_NUM+4*7+2)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*7+3)|SE_PALBANK(BLOCKS_PAL_NUM1)},
	{(BLOCKS_TILE_NUM+4*8+0)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*8+1)|SE_PALBANK(BLOCKS_PAL_NUM1),
	(BLOCKS_TILE_NUM+4*8+2)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*8+3)|SE_PALBANK(BLOCKS_PAL_NUM1)},
	{(BLOCKS_TILE_NUM+4*9+0)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*9+1)|SE_PALBANK(BLOCKS_PAL_NUM1),
	(BLOCKS_TILE_NUM+4*9+2)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*9+3)|SE_PALBANK(BLOCKS_PAL_NUM1)},
	{(BLOCKS_TILE_NUM+4*10+0)|SE_PALBANK(BLOCKS_PAL_NUM2), (BLOCKS_TILE_NUM+4*10+1)|SE_PALBANK(BLOCKS_PAL_NUM2),
	(BLOCKS_TILE_NUM+4*10+2)|SE_PALBANK(BLOCKS_PAL_NUM2), (BLOCKS_TILE_NUM+4*10+3)|SE_PALBANK(BLOCKS_PAL_NUM2)},
	{(BLOCKS_TILE_NUM+4*11+0)|SE_PALBANK(BLOCKS_PAL_NUM2), (BLOCKS_TILE_NUM+4*11+1)|SE_PALBANK(BLOCKS_PAL_NUM2),
	(BLOCKS_TILE_NUM+4*11+2)|SE_PALBANK(BLOCKS_PAL_NUM2), (BLOCKS_TILE_NUM+4*11+3)|SE_PALBANK(BLOCKS_PAL_NUM2)},
	{(BLOCKS_TILE_NUM+4*12+0)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*12+1)|SE_PALBANK(BLOCKS_PAL_NUM1),
	(BLOCKS_TILE_NUM+4*12+2)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*12+3)|SE_PALBANK(BLOCKS_PAL_NUM1)},
	{(BLOCKS_TILE_NUM+4*13+0)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*13+1)|SE_PALBANK(BLOCKS_PAL_NUM1),
	(BLOCKS_TILE_NUM+4*13+2)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*13+3)|SE_PALBANK(BLOCKS_PAL_NUM1)},
	{(BLOCKS_TILE_NUM+4*14+0)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*14+1)|SE_PALBANK(BLOCKS_PAL_NUM1),
	(BLOCKS_TILE_NUM+4*14+2)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*14+3)|SE_PALBANK(BLOCKS_PAL_NUM1)},
	{(BLOCKS_TILE_NUM+4*15+0)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*15+1)|SE_PALBANK(BLOCKS_PAL_NUM1),
	(BLOCKS_TILE_NUM+4*15+2)|SE_PALBANK(BLOCKS_PAL_NUM1), (BLOCKS_TILE_NUM+4*15+3)|SE_PALBANK(BLOCKS_PAL_NUM1)},
	{(BLOCKS_TILE_NUM+4*16+0)|SE_PALBANK(BLOCKS_PAL_NUM2), (BLOCKS_TILE_NUM+4*16+1)|SE_PALBANK(BLOCKS_PAL_NUM2),
	(BLOCKS_TILE_NUM+4*16+2)|SE_PALBANK(BLOCKS_PAL_NUM2), (BLOCKS_TILE_NUM+4*16+3)|SE_PALBANK(BLOCKS_PAL_NUM2)},
	{(BLOCKS_TILE_NUM+4*17+0)|SE_PALBANK(BLOCKS_PAL_NUM2), (BLOCKS_TILE_NUM+4*17+1)|SE_PALBANK(BLOCKS_PAL_NUM2),
	(BLOCKS_TILE_NUM+4*17+2)|SE_PALBANK(BLOCKS_PAL_NUM2), (BLOCKS_TILE_NUM+4*17+3)|SE_PALBANK(BLOCKS_PAL_NUM2)},
};

void init_player(struct player_state *p, uint64_t seed, uint64_t sequence);
void update_player(struct player_state *p);

void run_gameplay() {
	init_gameplay();
	init_player(&players[0], 123, 456);

	while(1) {
		VBlankIntrWait();
		oam_copy(oam_mem, obj_buffer, 128);

		// Draw P1's playfield
		for(int x=0; x<PLAYFIELD_W; x++) {
			for(int y=0; y<PLAYFIELD_H; y++) {
				int tile = players[0].playfield[x][y].type;
				se_mat[SB_NUM][y*2+0][x*2+0+1] = block_appearance[tile][0];
				se_mat[SB_NUM][y*2+0][x*2+1+1] = block_appearance[tile][1];
				se_mat[SB_NUM][y*2+1][x*2+0+1] = block_appearance[tile][2];
				se_mat[SB_NUM][y*2+1][x*2+1+1] = block_appearance[tile][3];				
			}
		}

		// --------------------------------------
		int player_offset_x = 0, player_offset_y = 0;
		switch(players[0].current_move) {
			case MOVE_LEFT:
				player_offset_x = players[0].move_timer*2;
				break;
			case MOVE_RIGHT:
				player_offset_x = -players[0].move_timer*2;
				break;
			case MOVE_UP:
				player_offset_y = players[0].move_timer*2;
				break;
			case MOVE_DOWN:
				player_offset_y = -players[0].move_timer*2;
				break;
		}

		obj_buffer[0].attr0 = ATTR0_Y_MASK&(players[0].player_y*16-16+player_offset_y);
		obj_buffer[0].attr1 = (ATTR1_X_MASK&(players[0].player_x*16+8-8+player_offset_x)) | ATTR1_SIZE_32x32 | (players[0].player_dir?ATTR1_HFLIP:0);
		obj_buffer[0].attr2 = (players[0].move_timer&4)*4;
		obj_used = 1;

		players[0].key_down = ~REG_KEYINPUT;
		update_player(&players[0]);

		if(obj_used < 128)
			obj_hide_multi(&obj_buffer[obj_used], 128-obj_used);
		obj_used = 0;
	}
}
