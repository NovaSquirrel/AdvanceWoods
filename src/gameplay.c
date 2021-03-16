#include <tonc.h>
#include "woods.h"

#include "SolidTiles_chr.h"
#include "Blocks_chr.h"
#include "Rocks_chr.h"
#include "Nova_chr.h"
#include "palettedata.h"

#define ROCKS_TILE_NUM 16
#define BLOCKS_TILE_NUM 24
#define BLOCKS_OBJ_TILE_NUM 32

#define BLOCKS_PAL_NUM1 0 
#define BLOCKS_PAL_NUM2 1
#define BLOCKS_PAL_NUM3 2
#define BLOCKS_PAL_NUM4 3
#define BLOCKS_PAL_NUM5 4
#define BLOCKS_PAL_NUM6 5
#define ROCKS_PAL_NUM 6

#define PLAYER_PAL_NUM 6

#define SB_NUM 31

struct player_state players[2];

void init_gameplay() {
	memcpy32(&tile_mem[0][0], SolidTiles_chrTiles, SolidTiles_chrTilesLen/4);
	memcpy32(&tile_mem[0][ROCKS_TILE_NUM], Rocks_chrTiles, Rocks_chrTilesLen/4);
	memcpy32(&tile_mem[0][BLOCKS_TILE_NUM], Blocks_chrTiles, Blocks_chrTilesLen/4);
	memcpy32(&tile_mem_obj[0][0], Nova_chrTiles, Nova_chrTilesLen/4);
	memcpy32(&tile_mem_obj[0][BLOCKS_OBJ_TILE_NUM], Blocks_chrTiles, Blocks_chrTilesLen/4);

	pal_bg_mem[0] = RGB15(8, 8, 8);
	memcpy16(&pal_bg_bank[BLOCKS_PAL_NUM1][1], BlockRedPalData, BlockRedPalSize/2);
	memcpy16(&pal_bg_bank[BLOCKS_PAL_NUM2][1], BlockYellowPalData, BlockYellowPalSize/2);
	memcpy16(&pal_bg_bank[BLOCKS_PAL_NUM3][1], BlockGreenPalData, BlockGreenPalSize/2);
	memcpy16(&pal_bg_bank[BLOCKS_PAL_NUM4][1], BlockBluePalData, BlockBluePalSize/2);
	memcpy16(&pal_bg_bank[BLOCKS_PAL_NUM5][1], BlockBrownPalData, BlockBrownPalSize/2);
	memcpy16(&pal_bg_bank[BLOCKS_PAL_NUM6][1], BlockGrayPalData, BlockGrayPalSize/2);
	memcpy16(&pal_bg_bank[ROCKS_PAL_NUM][1], RocksPalData,  RocksPalSize/2);

	memcpy16(&pal_obj_bank[BLOCKS_PAL_NUM1][1], BlockRedPalData, BlockRedPalSize/2);
	memcpy16(&pal_obj_bank[BLOCKS_PAL_NUM2][1], BlockYellowPalData, BlockYellowPalSize/2);
	memcpy16(&pal_obj_bank[BLOCKS_PAL_NUM3][1], BlockGreenPalData, BlockGreenPalSize/2);
	memcpy16(&pal_obj_bank[BLOCKS_PAL_NUM4][1], BlockBluePalData, BlockBluePalSize/2);
	memcpy16(&pal_obj_bank[BLOCKS_PAL_NUM5][1], BlockBrownPalData, BlockBrownPalSize/2);
	memcpy16(&pal_obj_bank[BLOCKS_PAL_NUM6][1], BlockGrayPalData, BlockGrayPalSize/2);
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

#define BLOCK_APPEARANCE_DEFINITION(tilenum, palette) {(BLOCKS_TILE_NUM+4*tilenum+0)|SE_PALBANK(palette), (BLOCKS_TILE_NUM+4*tilenum+1)|SE_PALBANK(palette),\
	(BLOCKS_TILE_NUM+4*tilenum+2)|SE_PALBANK(palette), (BLOCKS_TILE_NUM+4*tilenum+3)|SE_PALBANK(palette)},
const SCR_ENTRY block_appearance[][4] = {
	{0, 0, 0, 0},
	BLOCK_APPEARANCE_DEFINITION(0, BLOCKS_PAL_NUM1)
	BLOCK_APPEARANCE_DEFINITION(1, BLOCKS_PAL_NUM2) // Block
	BLOCK_APPEARANCE_DEFINITION(2, BLOCKS_PAL_NUM3)
	BLOCK_APPEARANCE_DEFINITION(3, BLOCKS_PAL_NUM4)
	BLOCK_APPEARANCE_DEFINITION(4, BLOCKS_PAL_NUM5)
	BLOCK_APPEARANCE_DEFINITION(5, BLOCKS_PAL_NUM6)
	BLOCK_APPEARANCE_DEFINITION(6, BLOCKS_PAL_NUM1) // Bomb
	BLOCK_APPEARANCE_DEFINITION(7, BLOCKS_PAL_NUM2)
	BLOCK_APPEARANCE_DEFINITION(8, BLOCKS_PAL_NUM3)
	BLOCK_APPEARANCE_DEFINITION(9, BLOCKS_PAL_NUM4)
	BLOCK_APPEARANCE_DEFINITION(10, BLOCKS_PAL_NUM5)
	BLOCK_APPEARANCE_DEFINITION(11, BLOCKS_PAL_NUM6)
	BLOCK_APPEARANCE_DEFINITION(12, BLOCKS_PAL_NUM1) // Diamond
	BLOCK_APPEARANCE_DEFINITION(13, BLOCKS_PAL_NUM2)
	BLOCK_APPEARANCE_DEFINITION(14, BLOCKS_PAL_NUM3)
	BLOCK_APPEARANCE_DEFINITION(15, BLOCKS_PAL_NUM4)
	BLOCK_APPEARANCE_DEFINITION(16, BLOCKS_PAL_NUM5)
	BLOCK_APPEARANCE_DEFINITION(17, BLOCKS_PAL_NUM6)
	BLOCK_APPEARANCE_DEFINITION(18, BLOCKS_PAL_NUM1) // Explosions
	BLOCK_APPEARANCE_DEFINITION(18, BLOCKS_PAL_NUM2)
	BLOCK_APPEARANCE_DEFINITION(18, BLOCKS_PAL_NUM3)
	BLOCK_APPEARANCE_DEFINITION(18, BLOCKS_PAL_NUM4)
	BLOCK_APPEARANCE_DEFINITION(18, BLOCKS_PAL_NUM5)
	BLOCK_APPEARANCE_DEFINITION(18, BLOCKS_PAL_NUM6)
};

const int block_color[] = {
	-1,
	0,1,2,3,4,5,
	0,1,2,3,4,5,
	0,1,2,3,4,5,
	0,1,2,3,4,5,
};

void init_player(struct player_state *p, uint64_t seed, uint64_t sequence);
void update_player(struct player_state *p);

void run_gameplay() {
	init_gameplay();
	init_player(&players[0], 123, 456);

	while(1) {
		VBlankIntrWait();
		oam_copy(oam_mem, obj_buffer, 128);

		// Draw P1's playfield into the screenblock
		for(int x=0; x<PLAYFIELD_W; x++) {
			for(int y=0; y<PLAYFIELD_H; y++) {
				if(players[0].playfield[x][y].flags & BLOCK_CARRIED)
					continue;
				int tile = players[0].playfield[x][y].type;
				se_mat[SB_NUM][y*2+0][x*2+0+1] = block_appearance[tile][0];
				se_mat[SB_NUM][y*2+0][x*2+1+1] = block_appearance[tile][1];
				se_mat[SB_NUM][y*2+1][x*2+0+1] = block_appearance[tile][2];
				se_mat[SB_NUM][y*2+1][x*2+1+1] = block_appearance[tile][3];				
			}
		}

		// --------------------------------------
		// Get the X and Y offset, used for both players and sprites
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

		// Draw player first
		obj_buffer[0].attr0 = ATTR0_Y_MASK&(players[0].player_y*16-16+player_offset_y);
		obj_buffer[0].attr1 = (ATTR1_X_MASK&(players[0].player_x*16+8-8+player_offset_x)) | ATTR1_SIZE_32x32 | (players[0].player_dir?ATTR1_HFLIP:0);
		obj_buffer[0].attr2 = ATTR2_PALBANK(PLAYER_PAL_NUM)|((players[0].move_timer&4)*4);
		obj_used = 1;

		// Draw carried blocks as sprites
		for(int x=0; x<PLAYFIELD_W; x++) {
			for(int y=0; y<PLAYFIELD_H; y++) {
				if((players[0].playfield[x][y].flags & BLOCK_CARRIED) == 0)
					continue;
				int tile = players[0].playfield[x][y].type;
				obj_buffer[obj_used].attr0 = ATTR0_Y_MASK&(y*16+player_offset_y);
				obj_buffer[obj_used].attr1 = (ATTR1_X_MASK&(x*16+8+player_offset_x)) | ATTR1_SIZE_16x16;
				obj_buffer[obj_used].attr2 = ATTR2_PALBANK(block_color[tile])|(tile*4+BLOCKS_OBJ_TILE_NUM-4);
				obj_used++;
			}
		}

		// Run actual game logic
		players[0].key_down = ~REG_KEYINPUT;
		update_player(&players[0]);

		if(obj_used < 128)
			obj_hide_multi(&obj_buffer[obj_used], 128-obj_used);
		obj_used = 0;
	}
}
