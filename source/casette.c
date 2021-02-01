#include <tonc.h>
#include "sprites.h"

#include "casette.h"
#include "char.h"

void CASS_render(OBJ_ATTR* buffer, int camX, int camY, int* count, Actor *physics){
	
	if (physics->flags & ACTOR_COLLIDABLE_MASK) {
		
		if (ACTOR_ENTITY_TYPE(physics->ID) == ENT_CASSETTE)
			memcpy(&tile_mem[4][CASSETTE_OFFSET], &cassette[((GAME_life >> 3) & 0x7) << 5], SPRITE_16x16);
		else
			memcpy(&tile_mem[4][CASSETTE_OFFSET], &heart[((GAME_life >> 3) % 12) << 5], SPRITE_16x16);
		
		int yPos = physics->y, xPos = physics->x;
		
		obj_set_attr(buffer,
		ATTR0_SQUARE | ATTR0_Y(GetActorY(yPos, camY)),
		ATTR1_SIZE_16x16 | ATTR1_X(GetActorX(xPos, camX)),
		ATTR2_PALBANK(2) | CASSETTE_OFFSET | ATTR2_PRIO(CHAR_PRIORITY));
		
		++*count;
	}
}
void CASS_update(int id) {
	
	if (ACTOR_ENTITY_TYPE(PHYS_actors[id].ID) != ENT_CASSETTE) {
		
		if (deathAnimTimer && !(PHYS_actors[id].flags & ACTOR_COLLIDABLE_MASK)){
			PHYS_actors[id].flags |= ACTOR_COLLIDABLE_MASK;
		}
		else if (!(PHYS_actors[id].flags & ACTOR_COLLIDABLE_MASK) && KEY_DOWN_NOW(KEY_A)) {
			HEART_COLLECT(((levelFlags & LEVELFLAG_BSIDE) > 0));
			STRAWB_SETTEMPCOLL(HEART_TEMP);
			
			
			if (levelFlags & LEVELFLAG_BSIDE) {
				START_FADE();
				nextGamestate = GS_END_LEVEL;
			}
			else {
				PHYS_actors[id].flags &= ~ACTOR_ACTIVE_MASK;
			}
		}
	}
}
