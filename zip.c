#include "zip.h"
#include "char.h"
#include "toolbox.h"
#include "particle.h"

#define FLAG_GRABBED	0x00000001	

int ZIP_index, ZIP_pal;

void ZIP_render(OBJ_ATTR* buffer, int camX, int camY, int* count, Actor *physics){
	int yPos = physics->y - 0xD00,
		xPos = physics->x;
	
	obj_set_attr(buffer,
	ATTR0_WIDE | ATTR0_Y(GetActorY(yPos, camY)),
	ATTR1_SIZE_16x8 | ATTR1_X(GetActorX(xPos, camX)),
	ATTR2_PALBANK(ZIP_pal) | ZIP_index | ATTR2_PRIO(0));
	
	obj_set_attr(buffer + 1,
	ATTR0_WIDE | ATTR0_Y(GetActorY(yPos, camY) + 8),
	ATTR1_SIZE_16x8 | ATTR1_X(GetActorX(xPos, camX)),
	ATTR2_PALBANK(ZIP_pal) | (ZIP_index + 2) | ATTR2_PRIO(2));
	
	*count += 2;
}
void ZIP_grab(int _id){
	PHYS_actors[_id].flags |=  FLAG_GRABBED;
	PHYS_actors[_id].velX = 0;
}
void ZIP_ungrab(int _id){
	PHYS_actors[_id].flags &= ~FLAG_GRABBED;
	PHYS_actors[_id].flags |= ACTOR_COLLIDABLE_MASK;
}
void ZIP_reset(Actor *physics){
	physics->x = BLOCK2FIXED(((physics->flags & ZIP_OG_XPOS) >> ZIP_OG_XPOS_SHIFT) + ((physics->flags & ZIP_OFFSET) >> ZIP_OFFSET_SHIFT));
	physics->velX = 0;
	physics->flags &= ~FLAG_GRABBED;
}
void ZIP_update(Actor *physics){
	
	if (GAME_freeze)
		return;
	
	
	if (collide_rect(physics->x, physics->y, physics->width, physics->height) & COLLISION_WALL) {
		if (physics->velX == 0){
			physics->velX = 0x80;
		}
		else if (INT_ABS(physics->velX) < 0x80){
			physics->velX = INT_SIGN(physics->velX) * 0x80;
		}
	}
	else {
		if (INT_ABS(physics->velX) < 0x60){
			physics->velX = 0;
		}
	}
	int velX = physics->velX;
	
	if (!(physics->flags & FLAG_GRABBED)){
		physics->x += physics->velX;
	}
	else
		velX = CHAR_phys->velX;
	
	if ((INT_ABS(velX) > 0x80 && !(GAME_life & 0xF)) || (INT_ABS(velX) > 0x300 && !(GAME_life & 0x3)) || (INT_ABS(velX) > 0x180 && !(GAME_life & 0x7))){
	
		int rng = RNG();
		
		int offX = ((physics->x >> 4)) & 0xFFFF;
		int offY = ((physics->y >> 4)) & 0xFFFF;
		offY -= 0xD0;
		offX += 0x40;
		
		velX = (rng) & 0x7;
		
		AddParticle(offX | 0x66000000,
					offY | 0xC0000000 | (ZIP_pal << 24) | ((0xF0 + velX) << 16),
					12 | 0x100 | ((rng & 0x1) << 10));
	}
	
	int left = ((physics->flags & ZIP_OG_XPOS) >> ZIP_OG_XPOS_SHIFT),
		right = left + ((physics->flags & ZIP_LENGTH) >> ZIP_LENGTH_SHIFT);
	
	if (physics->x < BLOCK2FIXED(left))
	{
		if (physics->flags & FLAG_GRABBED){
			if (PHYS_actors[0].velX < 0){
				CHAR_stop_zipline();
				PHYS_actors[0].velX = 0;
			}
			PHYS_actors[0].x = BLOCK2FIXED(left) + ZIP_GRAB_OFFSET;
		}
		else {
			physics->velX = 0;
		}
		physics->x = BLOCK2FIXED(left);
	}
	if (FIXED2BLOCK(physics->x) >= right)
	{
		if (physics->flags & FLAG_GRABBED){
			if (PHYS_actors[0].velX > 0){
				CHAR_stop_zipline();
				PHYS_actors[0].velX = 0;
			}
			PHYS_actors[0].x = BLOCK2FIXED(right) + ZIP_GRAB_OFFSET;
		}
		else {
			physics->velX = 0;
		}
		physics->x = BLOCK2FIXED(right);
	}
}
