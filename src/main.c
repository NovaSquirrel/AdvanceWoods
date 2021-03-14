#include <tonc.h>
#include <string.h>
#include "SolidTiles_chr.h"
#include "Blocks_chr.h"
#include "palettedata.h"

void upload_bg_palette(int palette, const void *data, size_t size) {
	memcpy16(&pal_bg_mem[16*palette]+1, data, size);
}

int main(void) {
	memcpy32(&tile_mem[0][0], SolidTiles_chrTiles, SolidTiles_chrTilesLen);
	memcpy32(&tile_mem[0][16], Blocks_chrTiles, Blocks_chrTilesLen);
	pal_bg_mem[0] = RGB15(8, 8, 8);
	upload_bg_palette(0, Block1PalData, Block1PalSize);
	upload_bg_palette(1, Block2PalData, Block2PalSize);

	// Set up the tilemap
	REG_BG0CNT= BG_CBB(0) | BG_SBB(31) | BG_4BPP | BG_REG_32x32;

	memset16(&se_mem[31][0], 0, 1024); // Clear to zeros

	for(int y=0; y<6; y++) {
		for(int x=0; x<6; x++) {
			int tile = (x+y)&3;
			se_mat[31][y*2+0][x*2+0] = 16+tile*4+0;
			se_mat[31][y*2+0][x*2+1] = 16+tile*4+1;
			se_mat[31][y*2+1][x*2+0] = 16+tile*4+2;
			se_mat[31][y*2+1][x*2+1] = 16+tile*4+3;
		}
	}
	REG_DISPCNT = DCNT_MODE0 | DCNT_BG0;

/*	
	tte_init_chr4c_default(0, BG_CBB(0) | BG_SBB(31));
	tte_set_pos(92, 68);
	tte_write("Hello World!");
*/

	irq_init(NULL);
	irq_enable(II_VBLANK);
	
	while (1) {
		VBlankIntrWait();
	}
	
}
