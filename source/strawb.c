
#include "char.h"
#include "core.h"
#include "game_data.h"
#include "global_vars.h"
#include "graphics.h"
#include "save_handler.h"
#include "sprites.h"

#include "soundbank.h"
#include "soundbank_bin.h"
#include <maxmod.h>

#define STRAWB_FOLLOW_DIST 0x1200

#define KEY_COLL_MASK  0x00001000
#define STRAWBIDX_MASK 0x00FF0000

// Flag 0
#pragma region

#define SAVE_POSX_MASK 0x000000FF
#define SAVE_POSY_MASK 0x0000FF00
#define POSX_SHIFT	   0
#define POSY_SHIFT	   8

#define STRAWB_POSX(n) ((n[0] & SAVE_POSX_MASK) >> POSX_SHIFT)
#define STRAWB_POSY(n) ((n[0] & SAVE_POSY_MASK) >> POSY_SHIFT)

#define FOLLOWING_MASK 0x00010000
#define COLLECTED_MASK 0x00020000
#define FLY_AWAY_MASK  0x00040000
#define WINGED_MASK	   0x00080000
#define GOLDEN_MASK	   0x00100000
#define DASHLESS_MASK  0x00180000
#define CHESTED_MASK   0x00200000
#define CHEST_MASK	   0x00400000
#define PALETTE_MASK   0x00800000
#define PALETTE_SHIFT  23

#define STRAWB_OLD(n) ((n[0] & PALETTE_MASK) >> PALETTE_SHIFT)

#pragma endregion

// Flag 1
#pragma region

#define STRAWBERRY_ID_MASK 0x000000FF

#pragma endregion

int STRAWB_berry_sp, STRAWB_wing_sp;
int chestAnim;
int STRAWB_waitTime;
int STRAWB_palIndex;

// mm_sound_effect get = {
// 	{ SFX_GAME_STRAWBERRY_GET },
// 	0x400,
// 	0,
// 	255,
// 	128,
// };

void STRAWB_render(unsigned int index) {
	Entity* entity = &entities[index];

	draw(entity->x, entity->y, STRAWB_berry_sp, 0, 0, 1 + STRAWB_OLD(entity->flags));

	// int yPos = entity->y, xPos = entity->x;

	// int bufferOffset = 0;
	// int palette		 = STRAWB_N_PAL + ((entity->flags[0] & PALETTE_MASK) >> 28);
	// if (entity->flags[0] & CHESTED_MASK) {
	// 	chestAnim = 0;
	// } else {

	// 	if (!(entity->flags[0] & (COLLECTED_MASK | FOLLOWING_MASK))) {
	// 		if (STRAWB_waitTime > 0 && !TAS_ACTIVE) {
	// 			xPos += INT2FIXED((STRAWB_waitTime & 0x1) << 1);
	// 			yPos += INT2FIXED((STRAWB_waitTime & 0x2));
	// 		} else if (!(entity->flags[0] & FLY_AWAY_MASK)) {
	// 			int offset = ((GAME_life << 2) & 0xFF) - 0x7F;

	// 			if (entity->flags[0] & WINGED_MASK)
	// 				yPos += (FIXED_MULT(INT_ABS(offset), 0x1000) - 0x400) * !TAS_ACTIVE;
	// 			else
	// 				yPos += (FIXED_MULT(INT_ABS(offset), 0x800) - 0x200) * !TAS_ACTIVE;
	// 		}

	// 		if (entity->flags[0] & WINGED_MASK && !TAS_ACTIVE) {
	// 			obj_set_attr(buffer,
	// 						 ATTR0_SQUARE | ATTR0_Y(GetActorY(yPos, camY)),
	// 						 ATTR1_SIZE_16x16 | ATTR1_X(GetActorX(xPos - 0xC00, camX)) | ATTR1_FLIP(1),
	// 						 ATTR2_PALBANK(palette) | (STRAWB_SP_OFFSET + 4) | ATTR2_PRIO(CHAR_PRIORITY + 1));

	// 			obj_set_attr(buffer + 1,
	// 						 ATTR0_SQUARE | ATTR0_Y(GetActorY(yPos, camY)),
	// 						 ATTR1_SIZE_16x16 | ATTR1_X(GetActorX(xPos + 0xC00, camX)),
	// 						 ATTR2_PALBANK(palette) | (STRAWB_SP_OFFSET + 4) | ATTR2_PRIO(CHAR_PRIORITY + 1));

	// 			bufferOffset += 2;
	// 			*count += 2;
	// 		}
	// 	}

	// 	obj_set_attr(buffer + bufferOffset,
	// 				 ATTR0_SQUARE | ATTR0_Y(GetActorY(yPos, camY)),
	// 				 ATTR1_SIZE_16x16 | ATTR1_X(GetActorX(xPos, camX)),
	// 				 ATTR2_PALBANK(palette) | STRAWB_SP_OFFSET | ATTR2_PRIO((entity->flags[0] & (COLLECTED_MASK | FOLLOWING_MASK)) ? CHAR_PRIORITY : CHAR_PRIORITY + 1));

	// 	++*count;
	// 	++bufferOffset;
	// }

	// if (entity->flags[0] & CHEST_MASK && !(entity->flags[0] & (FLY_AWAY_MASK | FOLLOWING_MASK | COLLECTED_MASK))) {
	// 	if (chestAnim < 23)
	// 		++chestAnim;

	// 	memcpy(&tile_mem[4][CHEST_SP_OFFSET], &chest[(chestAnim & 0x3C) << 3], 128);

	// 	obj_set_attr(buffer + bufferOffset,
	// 				 ATTR0_SQUARE | ATTR0_Y(GetActorY(entity->y + 0x800, camY)),
	// 				 ATTR1_SIZE_16x16 | ATTR1_X(GetActorX(entity->x, camX)),
	// 				 ATTR2_PALBANK(CHEST_PAL) | CHEST_SP_OFFSET | ATTR2_PRIO(CHAR_PRIORITY + 1));

	// 	++*count;
	// }
}
int STRAWB_init(unsigned int index, unsigned char* data, unsigned char* is_loading) {

	Entity* entity = &entities[index];

	ENABLE_ENT_FLAG(DETECT, index);

	entity->flags[0] = FIXED2BLOCK(entity->x) | (FIXED2BLOCK(entity->y) << 8);

	entity->flags[1] = data[0];

	if (is_strawb_collected(entity->flags[1])) {
		entity->flags[0] |= PALETTE_MASK;
	}

	entity->width  = 16;
	entity->height = 16;

	return 0;
}

void STRAWB_reset(unsigned int index) {
	Entity* entity = &entities[index];

	if (entity->flags[0] & COLLECTED_MASK)
		return;

	STRAWB_waitTime = -1;

	if (entity->flags[0] & FLY_AWAY_MASK) {
		ENABLE_ENT_FLAG(DETECT, index);
		entity->flags[0] &= ~FLY_AWAY_MASK;
	}

	int tempX = BLOCK2FIXED(STRAWB_POSX(entity->flags));
	int tempY = BLOCK2FIXED(STRAWB_POSY(entity->flags));
	entity->x = tempX;
	entity->y = tempY;
}
void STRAWB_playergrab(int id) {
	entities[id].flags[0] |= FOLLOWING_MASK;

	ENABLE_ENT_FLAG(PERSISTENT, id);
	DISABLE_ENT_FLAG(DETECT, id);
	entities[id].flags[0] &= ~FLY_AWAY_MASK;

	STRAWB_waitTime = -1;
}
void STRAWB_update(unsigned int index) {
	Entity* entity = &entities[index];

	if (entity->flags[0] & CHESTED_MASK)
		return;

	if ((entity->flags[0] & WINGED_MASK) && STRAWB_waitTime > 0) {
		--STRAWB_waitTime;
		if (STRAWB_waitTime == 0) {
			entity->vel_y = 0x100;
			entity->flags[0] |= FLY_AWAY_MASK;
		}
	}

	// If winged berry is frightened away
	if (entity->flags[0] & FLY_AWAY_MASK) {
		// Increase velocity, and move upwards
		entity->vel_y = SIGNED_MAX(entity->vel_y - 0x2C, -0x2A0);
		entity->y += entity->vel_y;
	}

	// If grabbed, but not collected, follow player
	else if (entity->flags[0] & FOLLOWING_MASK) {
		if (CHAR_ent->flags[0] & ISDEAD_MASK) // Return to original spot
		{
			entity->flags[0] &= ~FOLLOWING_MASK;
			DISABLE_ENT_FLAG(PERSISTENT, index);
			ENABLE_ENT_FLAG(DETECT, index);
			return;
		} else if ((CHAR_ent->flags[0] & ONGROUND_MASK && !(collide_rect(CHAR_ent->x, CHAR_ent->y, CHAR_ent->width, CHAR_ent->height, STRAWB_COLLISION))) || (entity->flags[0] & GOLDEN_MASK)) // Collect berry
		{
			// Disable following and persistance through levels.
			entity->flags[0] &= ~FOLLOWING_MASK;
			DISABLE_ENT_FLAG(PERSISTENT, index);
			entity->flags[0] |= COLLECTED_MASK;
			int id = entity->flags[1] & STRAWBERRY_ID_MASK;

			unload_entity(entity);

			// mmEffectEx(&get);
			//  GB_init_soundchip(0);

			++strawbs_colltemp;
			// MOAR BERRIES
			if (!is_strawb_collected(id)) {
				++strawbs_collected;
				collect_strawb(id);
				// if (!(entity->flags[0] & GOLDEN_MASK))
				// 	++STRAWB_levelCount;

				if ((entity->flags[0] & DASHLESS_MASK) == DASHLESS_MASK) {
					// WINGED_COLLECT();
				}
				if ((entity->flags[0] & DASHLESS_MASK) == GOLDEN_MASK) {
					// GOLD_COLLECT();
				}
			}

			//  Enable collected flag.
			// STRAWB_SAVESETCOLL(id, 1);
			// Fly up start velocity
			entity->vel_y = 0x240;
		}

		// previous position
		int prevX = CHAR_ent->x;
		int prevY = CHAR_ent->y;

		// get the distance between player and berry
		int diffX = entity->x - prevX, diffY = entity->y - prevY;
		int dist = INT_ABS(diffX) + INT_ABS(diffY); // FIXED_sqrt(FIXED_MULT(, diffX) + FIXED_MULT(diffY, diffY));
		dist	 = FIXED_DIV(STRAWB_FOLLOW_DIST, dist);

		// if farther away than needs be, put closer
		if (dist < 0x100) {
			entity->x = FIXED_LERP(entity->x, FIXED_MULT(diffX, dist) + prevX, 0x10);
			entity->y = FIXED_LERP(entity->y, FIXED_MULT(diffY, dist) + prevY, 0x10);
		}
	}
	// If collected, fly away
	else if (entity->flags[0] & COLLECTED_MASK) {
		entity->vel_y += -0x28;
		entity->y += entity->vel_y;
	}
	// All else, return to original spot
	else {
		int tempX = BLOCK2FIXED(STRAWB_POSX(entity->flags));
		int tempY = BLOCK2FIXED(STRAWB_POSY(entity->flags));
		entity->x = FIXED_LERP(entity->x, tempX, 0x20);
		entity->y = FIXED_LERP(entity->y, tempY, 0x20);
	}
}
void STRAWB_ondash(unsigned int index) {
	Entity* entity = &entities[index];

	if (entity->flags[0] & CHESTED_MASK)
		return;

	if ((entity->flags[0] & DASHLESS_MASK) == WINGED_MASK && !(entity->flags[0] & (FLY_AWAY_MASK | COLLECTED_MASK | FOLLOWING_MASK))) {
		if (STRAWB_waitTime == -1)
			STRAWB_waitTime = 20;
		else if (STRAWB_waitTime > 10)
			STRAWB_waitTime = 250;
	}
}

// ----- KEY -----

// void KEY_render(OBJ_ATTR* buffer, int camX, int camY, int* count, Actor* physics) {

// 	if (entity->flags[0] & KEY_COLL_MASK)
// 		return;

// 	obj_set_attr(buffer,
// 				 ATTR0_SQUARE | ATTR0_Y(GetActorY(entity->y, camY)),
// 				 ATTR1_SIZE_16x16 | ATTR1_X(GetActorX(entity->x, camX)),
// 				 ATTR2_PALBANK(CHEST_PAL) | KEY_SP_OFFSET);

// 	++*count;
// }
// // Reset key if strawb has not been collected
// void KEY_reset(Actor* physics) {
// 	int index = (entity->flags[0] & STRAWBIDX_MASK) >> 16;

// 	if (!(entities[index].flags[0] & COLLECTED_MASK)) {
// 		entities[index].flags[0] &= ~ACTOR_COLLIDABLE_MASK;
// 		entities[index].ID |= CHESTED_MASK;
// 		entity->flags[0] &= ~KEY_COLL_MASK;
// 		entity->flags[0] |= ACTOR_COLLIDABLE_MASK;
// 	}
// }
// void KEY_playergrab(int id) {

// 	// Collect key and don't collide
// 	entities[id].flags[0] |= KEY_COLL_MASK;
// 	entities[id].flags[0] &= ~ACTOR_COLLIDABLE_MASK;

// 	int index = (entities[id].flags[0] & STRAWBIDX_MASK) >> 16;

// 	if (!(entities[index].flags[0] & COLLECTED_MASK)) {
// 		entities[index].ID &= ~CHESTED_MASK;
// 		entities[index].flags[0] |= ACTOR_COLLIDABLE_MASK;
// 	}
// }
