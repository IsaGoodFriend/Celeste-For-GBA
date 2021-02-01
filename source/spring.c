#include "spring.h"
#include "char.h"
#include <tonc.h>

#define SPRING_TIMER 		0x0000F000
int SPRING_PAL;

void SPRING_render(OBJ_ATTR* buffer, int camX, int camY, int* count, Actor *physics){
	
	if (physics->flags & SPRING_TIMER)
	{
		// Remove one from timer
		physics->flags = (((physics->flags & SPRING_TIMER) - 1) & SPRING_TIMER) | (physics->flags & (~SPRING_TIMER));
		
		// Render spring up
		obj_set_attr(buffer,
		ATTR0_WIDE | ATTR0_Y(GetActorY(physics->y, camY)),
		ATTR1_SIZE_16x8 | ATTR1_X(GetActorX(physics->x, camX)),
		ATTR2_PALBANK(SPRING_PAL) | (SPRING_U_OFFSET));
	}
	else{
		obj_set_attr(buffer,
		ATTR0_WIDE | ATTR0_Y(GetActorY(physics->y, camY)),
		ATTR1_SIZE_16x8 | ATTR1_X(GetActorX(physics->x, camX)),
		ATTR2_PALBANK(SPRING_PAL) | (SPRING_D_OFFSET));
		
	}
	++(*count);
}
