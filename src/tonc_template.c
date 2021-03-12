// Include all tonc functions, better to only include files you're using
#include <tonc.h>
#include <stdint.h>

int main(void) {
	// Wait for vsync
	vid_vsync();

	// Bitmap mode 3, enable BG2 (bitmap layer)
	REG_DISPCNT = DCNT_MODE3 | DCNT_BG2;

	// Plot RGB points
	m3_plot(119, 80, RGB15(31, 0, 0)); // Red
	m3_plot(120, 80, RGB15(0, 31, 0)); // Green
	m3_plot(121, 80, RGB15(0, 0, 31)); // Blue

	for(int i=0; i<32; i++) {
		m3_hline(0, i, 239, RGB15(0, 31-i, 0));
		m3_hline(0, 159-i, 239, RGB15(0, 31-i, 0));
	}

	irq_init(NULL);
	irq_add(II_VBLANK, NULL);

	int x = 240/2;
	int y = 160/2;
	int x_positions[32];
	int y_positions[32];
	for(int i=0; i<32; i++) {
		x_positions[i] = x;
		y_positions[i] = y;
	}
	int write_index = 0;

	while(1) {
		VBlankIntrWait();

		uint16_t keys = ~REG_KEYINPUT;
		if(keys & (1<<KI_LEFT))
			x--;
		if(keys & (1<<KI_RIGHT))
			x++;
		if(keys & (1<<KI_UP))
			y--;
		if(keys & (1<<KI_DOWN))
			y++;

		x_positions[write_index] = x;
		y_positions[write_index] = y;

		for(int level=0; level<32; level++) {
			int i = (write_index-31+level)&31;
			m3_rect(x_positions[i], y_positions[i], x_positions[i]+7, y_positions[i]+7, RGB15(0, level, 0));
		}

		write_index = (write_index+1)&31;
	}

	return 0;
}
