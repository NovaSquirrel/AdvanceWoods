#include <tonc.h>

OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *const obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;
int obj_used;

void run_gameplay();

int main(void) {
	oam_init(obj_buffer, 128);

	irq_init(NULL);
	irq_enable(II_VBLANK);
	
	run_gameplay();
}
