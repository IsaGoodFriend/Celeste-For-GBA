#include "strawb.h"
#include "char.h"
#include <tonc.h>
#include "particle.h"

void BG_TOGG_render(OBJ_ATTR* buffer, int camX, int camY, int* count, Actor *physics){
	
	int yPos = physics->y;
	
	int offset = ((GAME_life << 1) & 0xFF) - 0x7F;
	
	yPos += FIXED_MULT(INT_ABS(offset), 0x800) - 0x250;
	
	obj_set_attr(buffer,
	ATTR0_SQUARE | ATTR0_Y(GetActorY(yPos, camY)),
	ATTR1_SIZE_16x16 | ATTR1_X(GetActorX(physics->x, camX)),
	ATTR2_PALBANK(1) | DASHCR_SP_OFFSET);
	
	++*count;
}
