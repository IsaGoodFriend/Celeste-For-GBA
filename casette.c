#include "casette.h"
#include "char.h"
#include "toolbox.h"

void CASS_render(OBJ_ATTR* buffer, int camX, int camY, int* count, Actor *physics){
	
	int yPos = physics->y + 0x200, xPos = physics->x + 0x200;
	
	obj_set_attr(buffer,
	ATTR0_SQUARE | ATTR0_Y(GetActorY(yPos, camY)),
	ATTR1_SIZE_16x16 | ATTR1_X(GetActorX(xPos, camX)),
	ATTR2_PALBANK(2) | CASSETTE_OFFSET | ATTR2_PRIO(CHAR_PRIORITY));
	
	++*count;
}
void CASS_update(int id) {
	
	if (id == ENT_CASSETTE)
		memcpy(&tile_mem[4][CASSETTE_OFFSET], &cassette[((GAME_life >> 3) & 0x7) << 5], SPRITE_16x16);
	else
		memcpy(&tile_mem[4][CASSETTE_OFFSET], &heart[((GAME_life >> 3) % 12) << 5], SPRITE_16x16);
}
