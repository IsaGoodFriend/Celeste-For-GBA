#include "strawb.h"
#include "char.h"
#include "toolbox.h"
#include "particle.h"

#define RECHARGE_MASK 		0x0000FF00
#define RECHARGE_SHIFT				 8

#define RECHARGE_TIME			   120

int double_dash_index, bubble_sp_index;

void DASHCR_render(OBJ_ATTR* buffer, int camX, int camY, int* count, Actor *physics){

	int timer = (physics->flags & RECHARGE_MASK) >> RECHARGE_SHIFT;
	
	if (timer)
		return;
	
	int actor_id = physics->ID;
	
	int yPos = physics->y;
	
	signed char offset = (GAME_life << 1);
	yPos += FIXED_MULT(INT_ABS(offset), 0x800) - 0x250;
	
	obj_set_attr(buffer,
	ATTR0_SQUARE | ATTR0_Y(GetActorY(yPos, camY)),
	ATTR1_SIZE_16x16 | ATTR1_X(GetActorX(physics->x, camX)),
	ATTR2_PALBANK(1) | DASHCR_SP_OFFSET);
	
	++*count;
	
	if (current_chapter == LEVELIDX_WATER) {
		offset += 8;
		
		yPos = physics->y + FIXED_MULT(INT_ABS(offset), 0x800) - 0x250;
		
		obj_set_attr(buffer + 1,
		ATTR0_SQUARE | ATTR0_Y(GetActorY(yPos, camY)),
		ATTR1_SIZE_16x16 | ATTR1_X(GetActorX(physics->x, camX)),
		ATTR2_PALBANK(1) | bubble_sp_index);
		
		++*count;
		
	}
}

void DASHCR_playergrab(int id){
	PHYS_actors[id].flags |= (RECHARGE_TIME << RECHARGE_SHIFT);
	PHYS_actors[id].flags &= ~ACTOR_COLLIDABLE_MASK;
	
	int offX = ((PHYS_actors[id].x >> 4)) & 0xFFFF;
	int offY = ((PHYS_actors[id].y >> 4)) & 0xFFFF;
	offX += 0x40;
	
	int rng;
	
	int i;
	for (i = 0; i < 6; ++i){
		rng = RNG();
		int velX = (rng >> 3) & 0x1F;
		int velY = (rng >> 9) & 0xF;
		velX = (velX - 0xF) & 0xFF;
		velY = (velY - 0xF) & 0xFF;
		
		AddParticle(offX | 0x88000000 | (velX << 16),
					offY | 0xC0000000 | (1 << 24) | (velY << 16),
					16 | 0x100 | ((rng & 0x3) << 10));
	}
}
void DASHCR_update(Actor *physics){
	
	int timer = (physics->flags & RECHARGE_MASK) >> RECHARGE_SHIFT;
	
	if (timer)
	{
		--timer;
		physics->flags = (timer << RECHARGE_SHIFT) | (physics->flags & ~RECHARGE_MASK);
		
		if (!(physics->flags & RECHARGE_MASK))
		{
			physics->flags |= ACTOR_COLLIDABLE_MASK;
			
			int offX = ((physics->x >> 4)) & 0xFFFF;
			int offY = ((physics->y >> 4)) & 0xFFFF;
			offX += 0x40;
			
			int i;
			for (i = 0; i < 3; ++i){
				int velX = (((GAME_life + i) * 2153) >> 3) & 0x1F;
				int velY = (((GAME_life + i) * 2531) >> 3) & 0x1F;
				velY = (velY - 0xF) & 0xFF;
				velX = (velX - 0xF) & 0xFF;
				
				AddParticle(offX | 0x88000000 | (velX << 16),
							offY | 0xC0000000 | (1 << 24) | (velY << 16),
							16 | 0x100);
			}
		}
	}
}
void DASHCR_reset(Actor *physics){

	physics->flags &= ~RECHARGE_MASK;
	physics->flags |= ACTOR_COLLIDABLE_MASK;
}
