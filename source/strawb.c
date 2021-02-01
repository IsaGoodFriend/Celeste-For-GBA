#include "strawb.h"
#include "char.h"
#include "game_data.h"
#include "sprites.h"

#include <maxmod.h>
#include "soundbank.h"
#include "soundbank_bin.h"

#define STRAWB_FOLLOW_DIST 0x1200

#define FOLLOWING_MASK 		0x00001000
#define COLLECTED_MASK 		0x00002000
#define FLY_AWAY_MASK		0x00008000
#define SAVE_POSX_MASK 		0x00FF0000
#define SAVE_POSY_MASK 		0xFF000000
#define POSX_SHIFT					16
#define POSY_SHIFT					24

#define WINGED_MASK			0x01000000
#define DASHLESS_MASK		0x03000000
#define GOLDEN_MASK			0x02000000
#define CHESTED_MASK		0x04000000
#define CHEST_MASK			0x08000000
#define PALETTE_MASK		0x10000000


#define KEY_COLL_MASK		0x00001000
#define STRAWBIDX_MASK		0x00FF0000

#define STRAWB_POSX(n) 		((n & SAVE_POSX_MASK) >> POSX_SHIFT)
#define STRAWB_POSY(n) 		((n & SAVE_POSY_MASK) >> POSY_SHIFT)

int chestAnim;
int STRAWB_waitTime;
int STRAWB_palIndex;

mm_sound_effect get = {
	{ SFX_GAME_STRAWBERRY_GET },
	0x400,
	0,
	255,
	128,
};

void STRAWB_render(OBJ_ATTR* buffer, int camX, int camY, int* count, Actor *physics){
	int yPos = physics->y, xPos = physics->x;
	
	int bufferOffset = 0;
	int palette = STRAWB_N_PAL + ((physics->ID & PALETTE_MASK) >> 28);
	if (physics->ID & CHESTED_MASK)
	{
		chestAnim = 0;
	}
	else{
	
		if (!(physics->flags & (COLLECTED_MASK | FOLLOWING_MASK))) {
			if (STRAWB_waitTime > 0 && !TAS_ACTIVE){
				xPos += INT2FIXED((STRAWB_waitTime & 0x1) << 1);
				yPos += INT2FIXED((STRAWB_waitTime & 0x2));
			}
			else if (!(physics->flags & FLY_AWAY_MASK)){
				int offset = ((GAME_life << 2) & 0xFF) - 0x7F;
				
				if (physics->ID & WINGED_MASK)
					yPos += (FIXED_MULT(INT_ABS(offset), 0x1000) - 0x400) * !TAS_ACTIVE;
				else
					yPos += (FIXED_MULT(INT_ABS(offset), 0x800) - 0x200) * !TAS_ACTIVE;
				
			}
			
			if (physics->ID & WINGED_MASK && !TAS_ACTIVE)
			{
				obj_set_attr(buffer,
				ATTR0_SQUARE | ATTR0_Y(GetActorY(yPos, camY)),
				ATTR1_SIZE_16x16 | ATTR1_X(GetActorX(xPos - 0xC00, camX)) | ATTR1_FLIP(1),
				ATTR2_PALBANK(palette) | (STRAWB_SP_OFFSET + 4) | ATTR2_PRIO(CHAR_PRIORITY + 1));
				
				obj_set_attr(buffer + 1,
				ATTR0_SQUARE | ATTR0_Y(GetActorY(yPos, camY)),
				ATTR1_SIZE_16x16 | ATTR1_X(GetActorX(xPos + 0xC00, camX)),
				ATTR2_PALBANK(palette) | (STRAWB_SP_OFFSET + 4) | ATTR2_PRIO(CHAR_PRIORITY + 1));
				
				bufferOffset += 2;
				*count += 2;
			}
		}
		
		obj_set_attr(buffer + bufferOffset,
		ATTR0_SQUARE | ATTR0_Y(GetActorY(yPos, camY)),
		ATTR1_SIZE_16x16 | ATTR1_X(GetActorX(xPos, camX)),
		ATTR2_PALBANK(palette) | STRAWB_SP_OFFSET | ATTR2_PRIO((physics->flags & (COLLECTED_MASK | FOLLOWING_MASK)) ? CHAR_PRIORITY : CHAR_PRIORITY + 1));
		
		++*count;
		++bufferOffset;
	}
	
	if (physics->ID & CHEST_MASK && !(physics->flags & (FLY_AWAY_MASK | FOLLOWING_MASK | COLLECTED_MASK)))
	{
		if (chestAnim < 23)
			++chestAnim;
		
		memcpy(&tile_mem[4][CHEST_SP_OFFSET], &chest[(chestAnim & 0x3C) << 3], 128);
		
		obj_set_attr(buffer + bufferOffset,
		ATTR0_SQUARE | ATTR0_Y(GetActorY(physics->y + 0x800, camY)),
		ATTR1_SIZE_16x16 | ATTR1_X(GetActorX(physics->x, camX)),
		ATTR2_PALBANK(CHEST_PAL) | CHEST_SP_OFFSET | ATTR2_PRIO(CHAR_PRIORITY + 1));
		
		++*count;
	}
}

void STRAWB_reset(Actor *physics){
	if (physics->flags & COLLECTED_MASK)
		return;
	
	STRAWB_waitTime = -1;
	
	if (physics->flags & FLY_AWAY_MASK){
		physics->flags |= ACTOR_COLLIDABLE_MASK;
		physics->flags &= ~FLY_AWAY_MASK;
	}
		
	int tempX = BLOCK2FIXED(STRAWB_POSX(physics->flags));
	int tempY = BLOCK2FIXED(STRAWB_POSY(physics->flags));
	physics->x = tempX;
	physics->y = tempY;
}
void STRAWB_playergrab(int id){
	PHYS_actors[id].flags |= FOLLOWING_MASK;
	PHYS_actors[id].flags |= ACTOR_PERSIST_MASK;
	PHYS_actors[id].flags &= ~ACTOR_COLLIDABLE_MASK;
	PHYS_actors[id].flags &= ~FLY_AWAY_MASK;
	
	STRAWB_waitTime = -1;
}
void STRAWB_update(Actor *physics){
	if (physics->ID & CHESTED_MASK)
		return;
	
	if ((physics->ID & WINGED_MASK) && STRAWB_waitTime > 0)
	{
		--STRAWB_waitTime;
		if (STRAWB_waitTime == 0)
		{
			physics->velY = 0x100;
			physics->flags |= FLY_AWAY_MASK;
		}
	}
	
	// If winged berry is frightened away
	if (physics->flags & FLY_AWAY_MASK){
		// Increase velocity, and move upwards
		physics->velY = SIGNED_MAX(physics->velY - 0x2C, -0x2A0);
		physics->y += physics->velY;
	}
	
	// If grabbed, but not collected, follow player
	else if (physics->flags & FOLLOWING_MASK) {
		if (PHYS_actors[0].flags & ISDEAD_MASK) // Return to original spot
		{
			physics->flags &= ~FOLLOWING_MASK;
			physics->flags &= ~ACTOR_PERSIST_MASK;
			physics->flags |= ACTOR_COLLIDABLE_MASK;
			return;
		}
		else if ((PHYS_actors[0].flags & ONGROUND_MASK && !IN_BACKGROUND && !(collide_rect(PHYS_actors[0].x, PHYS_actors[0].y, NORMAL_HITBOX_W, NORMAL_HITBOX_H) & COLLISION_STRAWB)) || (physics->ID & GOLDEN_MASK)) // Collect berry
		{
			// Disable following and persistance through levels.
			physics->flags &= ~FOLLOWING_MASK;
			physics->flags &= ~ACTOR_PERSIST_MASK;
			physics->flags |= COLLECTED_MASK;
			int id = (physics->ID>>16) & 0xFF;
			
			
			mmEffectEx(&get);
			//GB_init_soundchip(0);
			
			++STRAWB_tempCount;
			// MOAR BERRIES
			if (!STRAWB_SAVEISCOLL(id)){
				++STRAWB_count;
				if (!(physics->ID & GOLDEN_MASK))
					++STRAWB_levelCount;
				
				if ((physics->ID & DASHLESS_MASK) == DASHLESS_MASK){
					WINGED_COLLECT();
				}
				if ((physics->ID & DASHLESS_MASK) == GOLDEN_MASK){
					GOLD_COLLECT();
				}
			}
			
			//  Enable collected flag.
			STRAWB_SAVESETCOLL(id, 1);
			// Fly up start velocity
			physics->velY = 0x240;
			
		}
		
		// previous position
		int prevX = PHYS_actors[0].x;
		int prevY = PHYS_actors[0].y;
		
		// get the distance between player and berry
		int diffX = physics->x - prevX, diffY = physics->y - prevY;
		int dist = INT_ABS(diffX) + INT_ABS(diffY);//FIXED_sqrt(FIXED_MULT(, diffX) + FIXED_MULT(diffY, diffY));
		dist = FIXED_DIV(STRAWB_FOLLOW_DIST, dist);
		
		// if farther away than needs be, put closer
		if (dist < 0x100){
			physics->x = FIXED_LERP(physics->x, FIXED_MULT(diffX, dist) + prevX, 0x10);
			physics->y = FIXED_LERP(physics->y, FIXED_MULT(diffY, dist) + prevY, 0x10);
		}
	}
	// If collected, fly away
	else if (physics->flags & COLLECTED_MASK){
		physics->velY += -0x28;
		physics->y += physics->velY;
	}
	// All else, return to original spot
	else{
		int tempX = BLOCK2FIXED(STRAWB_POSX(physics->flags));
		int tempY = BLOCK2FIXED(STRAWB_POSY(physics->flags));
		physics->x = FIXED_LERP(physics->x, tempX, 0x20);
		physics->y = FIXED_LERP(physics->y, tempY, 0x20);
		
	}
}
void STRAWB_ondash(Actor *physics){
	
	if (physics->ID & CHESTED_MASK)
		return;
	
	if ((physics->ID & DASHLESS_MASK) == WINGED_MASK && !(physics->flags & (FLY_AWAY_MASK | COLLECTED_MASK | FOLLOWING_MASK))){
		if (STRAWB_waitTime == -1)
			STRAWB_waitTime = 20;
		else if (STRAWB_waitTime > 10)
			STRAWB_waitTime = 250;
	}
}

// ----- KEY -----


void KEY_render(OBJ_ATTR* buffer, int camX, int camY, int* count, Actor *physics){
	
	if (physics->flags & KEY_COLL_MASK)
		return;
	
	obj_set_attr(buffer,
	ATTR0_SQUARE | ATTR0_Y(GetActorY(physics->y, camY)),
	ATTR1_SIZE_16x16 | ATTR1_X(GetActorX(physics->x, camX)),
	ATTR2_PALBANK(CHEST_PAL) | KEY_SP_OFFSET);
	
	++*count;
}
// Reset key if strawb has not been collected
void KEY_reset(Actor *physics){
	int index = (physics->flags & STRAWBIDX_MASK) >> 16;
	
	if (!(PHYS_actors[index].flags & COLLECTED_MASK))
	{
		PHYS_actors[index].flags &= ~ACTOR_COLLIDABLE_MASK;
		PHYS_actors[index].ID |= CHESTED_MASK;
		physics->flags &= ~KEY_COLL_MASK;
		physics->flags |= ACTOR_COLLIDABLE_MASK;
	}
}
void KEY_playergrab(int id) {
	
	// Collect key and don't collide
	PHYS_actors[id].flags |= KEY_COLL_MASK;
	PHYS_actors[id].flags &= ~ACTOR_COLLIDABLE_MASK;
	
	int index = (PHYS_actors[id].flags & STRAWBIDX_MASK) >> 16;
	
	if (!(PHYS_actors[index].flags & COLLECTED_MASK))
	{
		PHYS_actors[index].ID &= ~CHESTED_MASK;
		PHYS_actors[index].flags |= ACTOR_COLLIDABLE_MASK;
	}
}
