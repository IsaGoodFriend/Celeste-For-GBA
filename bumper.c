#include "bumper.h"
#include "char.h"
#include "toolbox.h"

int BUMPER_index, BUMPER_pal;

void BUMPER_render(OBJ_ATTR* buffer, int camX, int camY, int* count, Actor *physics){
	int yPos = physics->y + 0x200,
		xPos = physics->x + 0x200;
	
	
	obj_set_attr(buffer,
	ATTR0_SQUARE | ATTR0_Y(GetActorY(yPos, camY)),
	ATTR1_SIZE_16x16 | ATTR1_X(GetActorX(xPos, camX)),
	ATTR2_PALBANK(BUMPER_pal) | BUMPER_index);
	
	obj_set_attr(buffer + 1,
	ATTR0_SQUARE | ATTR0_Y(GetActorY(yPos, camY)) | ATTR0_TALL,
	ATTR1_SIZE_8x16 | ATTR1_X(GetActorX(xPos - 0x800, camX)),
	ATTR2_PALBANK(BUMPER_pal) | (BUMPER_index + 4));
	
	obj_set_attr(buffer + 2,
	ATTR0_SQUARE | ATTR0_Y(GetActorY(yPos, camY)) | ATTR0_TALL,
	ATTR1_SIZE_8x16 | ATTR1_X(GetActorX(xPos + 0x1000, camX)) | ATTR1_HFLIP,
	ATTR2_PALBANK(BUMPER_pal) | (BUMPER_index + 4));
	
	obj_set_attr(buffer + 3,
	ATTR0_SQUARE | ATTR0_Y(GetActorY(yPos - 0x800, camY)) | ATTR0_WIDE,
	ATTR1_SIZE_8x16 | ATTR1_X(GetActorX(xPos, camX)),
	ATTR2_PALBANK(BUMPER_pal) | (BUMPER_index + 6));
	
	obj_set_attr(buffer + 4,
	ATTR0_SQUARE | ATTR0_Y(GetActorY(yPos + 0x1000, camY)) | ATTR0_WIDE,
	ATTR1_SIZE_8x16 | ATTR1_X(GetActorX(xPos, camX)) | ATTR1_VFLIP,
	ATTR2_PALBANK(BUMPER_pal) | (BUMPER_index + 6));
	
	*count += 5;
}
void BUMPER_playerhit(int id){
	PHYS_actors[id].velY = 0x100;
}
void BUMPER_update(Actor *physics){
	physics->velX = 0x100;
	char xNew = (physics->flags & 0xFF00) >> 8;
	char yNew = (physics->flags & 0xFF0000) >> 16;
	
	
}
