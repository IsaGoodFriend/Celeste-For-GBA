#include "game_data.h"
#include "sprites.h"
#include <maxmod.h>

#include "soundbank.h"
#include "soundbank_bin.h"

#include "char.h"

#include "physics.h"
#include "strawb.h"
#include "dashcrystal.h"
#include "particle.h"

#define HAIR_DIST 0x280

#define WALL_HITBOX_W 3
#define WALL_HITBOX_H 11
#define DUCK_DIFF 5

#define SPRING_hold_time 4
#define SPRING_bounce -0x4C0


// Character specific masks
#define EMPTY_FLAG_ASDF			0x0001

#define DASH_HORZ_MASK 			0x0006
#define DASH_VERT_MASK 			0x0018
#define DASH_DIR_MASK 			0x001E

#define DUCKING_MASK 			0x0080
#define DUCKING_SHIFT			7

#define TIRED_MASK				0x0100

#define DASH_REFILL_MASK		0x0800
#define GROUND_JUMP_MASK		0x1000
#define SLIDING					0x2000

#define ON_GROUND (CHAR_phys->flags & ONGROUND_MASK)
#define CAN_JUMP (coyoteTimer && (!CHALLENGE_MODE || current_chapter != LEVELIDX_CORE))

#define DASH_UP_FLAGS 0x18
#define DASH_DOWN_FLAGS 0x10
#define IS_DASH_DIR(f) (!((CHAR_flags & DASH_DIR_MASK) ^ f))
#define IS_DUCKING ((CHAR_flags & DUCKING_MASK) > 0)
#define IS_TIRED (stamina <= F_ClimbTiredThreshold)

#define CAN_REFILL_DASH 			(CHAR_flags & DASH_REFILL_MASK)

#define FACING_LEFT(n) (n & ACTOR_FLIPX_MASK ? 1 : -1)
#define SET_FACING(n, v) (n = (n & ~ACTOR_FLIPX_MASK) | (v < 0 ? 1 : 0))

#define ANIM_IDLE 		0
#define ANIM_DUCK 		4
#define ANIM_LOOK		5
#define ANIM_WALK 		6

#define ANIM_JUMP 		12
#define ANIM_AIRMID		13
#define ANIM_FALL 		14

#define ANIM_CLIMB 		15

#define ANIM_DASH 		19
#define ANIM_DEATH		20

// +1 for y Vel not zero
// +1 for x Vel zero
// +2 for y Vel pos
#define ANIM_DDASH_L 	21
#define ANIM_DDASH_UL 	22
#define ANIM_DDASH_U 	23
#define ANIM_DDASH_DL 	24
#define ANIM_DDASH_D	25

#define ANIM_ZIPGRAB	26
#define ANIM_ZIPGRAB_F	27
#define ANIM_ZIPGRAB_B	28

#define ANIM_VIEW		29


#define ANIM_HITBOX		30
#define ANIM_HITSMALL	31

unsigned int CHAR_flags = TWO_DASH_MASK | TOTAL_DASH2_MASK | DASH_REFILL_MASK;
unsigned int CHAR_state, CHAR_dashTimer, staminaBlink;
unsigned int anim_state, anim_timer, anim_last;

void (*updateMethod)(void);

mm_sound_effect dash = {
	{ SFX_CHAR_MAD_DASH_RED_LEFT },
	0x400,
	0,
	255,
	128,
};
mm_sound_effect dashDouble = {
	{ SFX_CHAR_MAD_DASH_PINK_LEFT },
	0x400,
	0,
	255,
	128,
};

mm_sound_effect jump = {
	{ SFX_CHAR_MAD_JUMP },
	0x400,
	0,
	255,
	128,
};
mm_sound_effect jumpSuper = {
	{ SFX_CHAR_MAD_JUMP_SUPER },
	0x400,
	0,
	255,
	128,
};
mm_sound_effect jumpWallBounce = {
	{ SFX_CHAR_MAD_JUMP_SUPERWALL },
	0x400,
	0,
	255,
	128,
};

mm_sound_effect death = {
	{ SFX_CHAR_MAD_DEATH },
	0x400,
	0,
	255,
	128,
};
mm_sound_effect respawn = {
	{ SFX_CHAR_MAD_REVIVE },
	0x400,
	0,
	255,
	128,
};

int horz;
int vert;

int saveX, saveY;

int forceMoveX, varJumpSpeed, lastClimbMove, climbNoMoveTimer, stamina = F_ClimbMaxStamina, wallSpeedRetained, climbJumpSave;

Actor *CHAR_phys;

int hairPoints[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	
COLOR hairColor[6];// = {0x7A8D, 0x54E4, 0x08B3, 0x084B, 0x621E, 0x30D8};
COLOR skinStaminaColor[16];

// timers
unsigned int varJumpTimer, zipJumpBuffer, jumpBufferTimer, dashBufferTimer, grabBufferTimer, forceMoveXTimer, coyoteTimer, speedRetentionTimer, deathAnimTimer, dreamDashStayTimer;
int grabbed_entity;

int speedVert, speedHorz;
int CHAR_PRIORITY;
int CHAR_umbrellaOffset;

void CHAR_updateAnim(){
	if (HEART_FREEZE)
		return;
	
	if (levelFlags & LEVELFLAG_TAS) {
		anim_state = ANIM_HITBOX + IS_DUCKING;
		return;
	}
	
	switch (CHAR_state * (deathAnimTimer == 0)) {
		case ST_DREAMDASH: {
			anim_state = 0;
			anim_state += CHAR_phys->velY != 0;
			anim_state += CHAR_phys->velX == 0;
			anim_state += (CHAR_phys->velY > 0) << 1;
			
			anim_state += ANIM_DDASH_L;
		
		
		break; }
		//
		case ST_CLIMB: {
			anim_state = ANIM_CLIMB;
			anim_timer = (anim_timer + 2) * (CHAR_phys->velY < 0 && !IS_FADING);
			anim_timer &= 0x3F;
			
			anim_state += anim_timer >> 4;
			
		break; }
		//
		case ST_VIEWER: {
			anim_state = ANIM_VIEW;
			
		break; }
		//
		case ST_ZIPLINE: {
			anim_state = ANIM_ZIPGRAB;
			anim_state += horz && !IS_TIRED;
			anim_state += FACING_LEFT(CHAR_phys->flags) == horz && !IS_TIRED;
			
		break; }
		//
		case ST_NORMAL: {
			
			if (ON_GROUND) {
				anim_state  = ANIM_DUCK * (vert == 1);
				anim_state += ANIM_LOOK * (vert == -1);
				anim_state += (ANIM_WALK - anim_state) * (CHAR_phys->velX != 0);
				
				anim_timer = (anim_timer + 1) * (anim_state == ANIM_IDLE || anim_state == ANIM_WALK);
				anim_timer += anim_state == ANIM_WALK && !IS_FADING;
				anim_timer %= 0x40 + ((anim_state == ANIM_WALK) * 0x20);
				anim_state += anim_timer >> 4;
			} else {
				anim_state = ANIM_JUMP;
				anim_state += CHAR_phys->velY > -0xC0;
				anim_state += CHAR_phys->velY > 0x40;
				
				anim_state += (ANIM_CLIMB - anim_state) * ((CHAR_phys->flags & WALL_SLIDING) > 0);
				
				anim_timer = 0;
			}
			anim_state += (ANIM_DEATH - anim_state) * (deathAnimTimer != 0);
			break;
		break; }
		//
		case ST_DASH: {
			anim_state = ANIM_DASH;
		break; }
	}
}
void CHAR_render(OBJ_ATTR* buffer, int camX, int camY, int *count, int pal){
	
	if (FIXED2INT(CHAR_phys->y) - camY < -0x40 ||
		FIXED2INT(CHAR_phys->y) - camY > 184   ||
		FIXED2INT(CHAR_phys->x) - camX < -0x40 ||
		FIXED2INT(CHAR_phys->x) - camX > 264)
		return;
	
	
	char displayHair = !(levelFlags & LEVELFLAG_TAS);
	
	deathAnimTimer -= deathAnimTimer > 60 && IS_FADING;
	
	// Death Animation
	if (deathAnimTimer && deathAnimTimer <= 60) {
		int currX, currY;
		
		if (deathAnimTimer == 27){
			mmEffectEx(&respawn);
		}
		
		deathAnimTimer -= (deathAnimTimer >= 30 && GAME_fading) ||
						(CHAR_phys->x == saveX && CHAR_phys->y == saveY && deathAnimTimer <= 30);
		
		int offsetX = -camX - 4, offsetY = -camY - 5;
		
		int x = CHAR_phys->x - 0x400;
		int y = CHAR_phys->y - 0x400;
		
		if (FIXED2INT(CHAR_phys->y) - camY > 152)
			y = INT2FIXED(152 + camY);
		
		int offset = (deathAnimTimer > 30 ? (60 - deathAnimTimer) : deathAnimTimer) + 2;
		
		if (offset > 20 && offset & 1)
			return;
		
		offsetX = -camX;
		offsetY = -camY;
		currX = 4;
		currY = 0;
		
		int hairI = 0;
		for (; hairI < 6; ++hairI)
		{
			// Death anim part
			obj_set_attr((buffer + *count),
			ATTR0_SQUARE | ATTR0_Y(FIXED2INT(y + (currY * offset * 0x70)) - camY),	// ATTR0
			ATTR1_SIZE_16 | ATTR1_X(FIXED2INT(x + (currX * offset * 0x70)) - camX),	// ATTR1
			ATTR2_PALBANK(0) | (HAIR_SPRITE_OFFSET) | ATTR2_PRIO(0));		// ATTR2
			
			++*count;
			
			if (!currY)
			{
				currY = currX - INT_SIGN(currX);
				currX = currX / 2;
			}
			else if (INT_SIGN(currY) == INT_SIGN(currX))
			{
				currX = -currX;
			}
			else
			{
				currX = currX * 2;
				currY = 0;
			}
		}
		
		for (hairI = 0; hairI < 6; ++hairI)
		{
			// death anim part
			obj_set_attr((buffer + *count),
			ATTR0_SQUARE | ATTR0_Y(FIXED2INT(y + (currY * offset * 0x70)) - camY),						// ATTR0
			ATTR1_SIZE_16 | ATTR1_X(FIXED2INT(x + (currX * offset * 0x70)) - camX),						// ATTR1
			ATTR2_PALBANK(0) | (((HAIR_PART_COUNT) << 2) + HAIR_SPRITE_OFFSET) | ATTR2_PRIO(0)); // ATTR2
			
			++*count;
			
			if (!currY)
			{
				currY = currX - INT_SIGN(currX);
				currX = currX / 2;
			}
			else if (INT_SIGN(currY) == INT_SIGN(currX))
			{
				currX = -currX;
			}
			else
			{
				currX = currX * 2;
				currY = 0;
			}
		}
		
		return;
	}
	
	int currX = 0, currY = -1;
	
	int offsetX = -camX - 4, offsetY = -camY - 5;
	// Animation
#if 1
	switch (CHAR_state * (deathAnimTimer == 0)) {
		case ST_DREAMDASH: {
			displayHair = 0;
		
		break; }
		//
		case ST_CLIMB: {
			currX = -1 - (anim_timer >> 5);
		break; }
		//
		case ST_ZIPLINE: {
			offsetY += 2;
			
		break; }
		case ST_VIEWER: {
			currY -= 1;
			
		break; }
		//
		case ST_NORMAL: {
			
			if (ON_GROUND) {
				currY += (anim_state == ANIM_DUCK) * 4;
				
				currX -= (anim_state == ANIM_LOOK) * 2;
				
				currY += (anim_state >= ANIM_WALK) * (-1 +(anim_timer >> 4) % 3);
				currX += (anim_state >= ANIM_WALK);
				
				currY += (anim_timer >> 5) * (anim_state < ANIM_WALK);
			} else {
				currY -= (anim_state == ANIM_JUMP);
				currX += (anim_state == ANIM_JUMP);
				currX -= (anim_state == ANIM_CLIMB);
			}
			currX += (-currX - 1) * (anim_state == ANIM_DEATH);
			break;
		break; }
		//
		case ST_DASH: {
			currX = 2;
			currY = 0;
		break; }
	}
#endif
	
	// todo: Only load sprite when frame has changed
	if (anim_state != anim_last) {
		memcpy(tile_mem[4], &madeline[anim_state << 5], SPRITE_16x16);
		anim_last = anim_state;
	}
	
	int prio = CHAR_PRIORITY;
	
	// Character
	obj_set_attr(buffer,
	ATTR0_SQUARE | ATTR0_Y(FIXED2INT(CHAR_phys->y) + offsetY),														// ATTR0
	ATTR1_SIZE_16 | ATTR1_FLIP(CHAR_phys->flags & ACTOR_FLIPX_MASK) | ATTR1_X(FIXED2INT(CHAR_phys->x) + offsetX),	// ATTR1
	ATTR2_PALBANK(0) | ATTR2_PRIO(prio)); 																			// ATTR2
	
	++*count;
	
	if (!displayHair)
		return;
	
	offsetY += currY;
	offsetX += currX * (1 - (((CHAR_phys->flags & ACTOR_FLIPX_MASK) > 0) << 1));
	
	// Umbrella
	if (IS_RAINY && displayHair && !deathAnimTimer) {
		obj_set_attr((buffer + *count),
		ATTR0_SQUARE | ATTR0_Y(FIXED2INT(CHAR_phys->y) + offsetY - 10),											// ATTR0
		ATTR1_SIZE_16 | ATTR1_X(FIXED2INT(CHAR_phys->x) + offsetX - ((CHAR_phys->flags & ACTOR_FLIPX_MASK) ? 2 : -1)),	// ATTR1
		ATTR2_PALBANK(0) | (CHAR_umbrellaOffset) | ATTR2_PRIO(prio)); // ATTR2
		
		if (key_released(KEY_DOWN)){
			int i;
			int velY;
			for (i = 0; i < 4; ++i) {
				int rng = RNG();
				
				int offX = i << 5;
				int offY = (rng) & 0x3F;
				offX -= (CHAR_phys->flags & ACTOR_FLIPX_MASK) ? 0x4F : 0x2F;
				offY -= 0x0F;
				
				offX = ((CHAR_phys->x >> 4) + offX) & 0xFFFF;
				offY = (((CHAR_phys->y - 0xF00) >> 4) + offY) & 0xFFFF;
				
				velY = (rng) & 0x7;
				
				AddParticle(offX | (0x55000000 + (rng & 0x33000000)),
							offY | 0xC0000000 | ((0xE8 + velY) << 16),
							0x100 | 27 | ((rng & 0x3) << 10));
			}
		}
		
		
		memcpy(&tile_mem[4][CHAR_umbrellaOffset], &umbrella[32 * (key_tri_vert() > 0)], umbrellaLen >> 1);
		
		++*count;
	}
	
	obj_set_attr((buffer + *count),
	ATTR0_WIDE | ATTR0_Y(FIXED2INT(CHAR_phys->y) + offsetY),											// ATTR0
	ATTR1_SIZE_16x8 | ATTR1_FLIP(CHAR_phys->flags & ACTOR_FLIPX_MASK) | ATTR1_X(FIXED2INT(CHAR_phys->x) + offsetX),	// ATTR1
	ATTR2_PALBANK(0) | 4 | ATTR2_PRIO(prio));
	
	obj_set_attr((buffer + *count + HAIR_PART_COUNT + 1),
	ATTR0_WIDE | ATTR0_Y(FIXED2INT(CHAR_phys->y) + offsetY),												// ATTR0
	ATTR1_SIZE_16x8 | ATTR1_FLIP(CHAR_phys->flags & ACTOR_FLIPX_MASK) | ATTR1_X(FIXED2INT(CHAR_phys->x) + offsetX),	// ATTR1
	ATTR2_PALBANK(0) | 6 | ATTR2_PRIO(prio)); 																// ATTR2
	
	*count += 2;
	
	int dist;
	
	int prevX = (CHAR_phys->x & ~0xFF) + 0x080, prevY = (CHAR_phys->y & ~0xFF) - 0x400 + ((CURRENT_DASH_STATUS >= 2) << 8);
	
	buffer += *count;
	//*
	int hairI = 0;
	for (; hairI < HAIR_PART_COUNT; ++hairI) {
		//hairPoints[(hairI << 1)] += FACING_LEFT(CHAR_phys->flags) * 0x2 * (hairI + 1) + 0x6;
		hairPoints[(hairI << 1) + (CURRENT_DASH_STATUS < 2)] += (0x120 - 0x0C * (hairI + 1)) *
			(1 - (!((CHAR_phys->flags & ACTOR_FLIPX_MASK) || (CURRENT_DASH_STATUS < 2)) << 1)) * !HEART_FREEZE;
		
		
		currX = hairPoints[(hairI << 1)] - prevX;
		currY = hairPoints[(hairI << 1) + 1] - prevY;
		
		dist = FIXED_MULT(currX, currX) + FIXED_MULT(currY, currY);
		dist = FIXED_DIV(0x5C0 - ((anim_state == ANIM_DUCK) * 0x400), dist);
		
		currX = hairPoints[(hairI << 1)];
		currY = hairPoints[(hairI << 1) + 1];
		
		
		currX += (FIXED_MULT(currX - prevX, dist + 0x50) + prevX - currX) * (dist < 0x100);
		currY += (FIXED_MULT(currY - prevY, dist + 0x50) + prevY - currY) * (dist < 0x100);
		
		prevX = currX;
		prevY = currY;
		hairPoints[(hairI << 1)] = currX;
		hairPoints[(hairI << 1) + 1] = currY;
		
		currY = ATTR0_SQUARE | ATTR0_Y(FIXED2INT(currY) + offsetY);
		currX = ATTR1_SIZE_16 | ATTR1_X(FIXED2INT(currX) + offsetX);
		// render hair part
		obj_set_attr((buffer - 1),
		currY,							// ATTR0
		currX,						// ATTR1
		(((hairI) << 2) + HAIR_SPRITE_OFFSET) | ATTR2_PRIO(prio)); 	// ATTR2
		
		obj_set_attr((buffer + HAIR_PART_COUNT),
		currY,												// ATTR0
		currX,											// ATTR1
		(((hairI + HAIR_PART_COUNT) << 2) + HAIR_SPRITE_OFFSET) | ATTR2_PRIO(prio)); // ATTR2
		
		++*count;
		++buffer;
	}
	*count += HAIR_PART_COUNT;
}
//
void CHAR_LoadLevel(){
	CHAR_phys->flags |= ONGROUND_MASK;
	CHAR_change_state(ST_NORMAL);
	forceMoveXTimer = 0;
	jumpBufferTimer = 0;
	zipJumpBuffer = 0;
	varJumpTimer = 0;
	dashBufferTimer = 0;
	grabbed_entity = 0;
	deathAnimTimer = 0;
	anim_last = anim_timer + 1;
	
	CHAR_flags &= ~(BACKGROUND_FLAG);
	
	int i = 0;
	for (; i < 8; i += 2)
	{
		hairPoints[i] = CHAR_phys->x + 0x400;
		hairPoints[i + 1] = CHAR_phys->y + 0x400;
	}
	
	CHAR_refill_dash(1);
	set_hair_color();
}
void SET_DUCKING(int value){
	if (value == 0 && IS_DUCKING)
	{
		int hit = collide_rect(CHAR_phys->x, CHAR_phys->y, NORMAL_HITBOX_W, NORMAL_HITBOX_H) & COLLISION_WALL;
		if (hit)
			return;
	}
	CHAR_flags = (value ? (CHAR_flags | DUCKING_MASK) : (CHAR_flags & ~DUCKING_MASK));
}
void CHAR_save_loc(){
	saveX = CHAR_phys->x;
	saveY = CHAR_phys->y;
}
void CHAR_on_transition(){
	speedRetentionTimer = 0;
	CHAR_dashTimer = 0;
	varJumpTimer = 0;
	varJumpSpeed = 0;
	forceMoveXTimer = 0;
	speedVert = 0;
	speedHorz = 0;
	coyoteTimer = 0;
}
void set_hair_color(){
	
	// change hair color
	COLOR *palette = pal_obj_mem + 1;
	
	COLOR *lookUp = ((COLOR*)&hairColor);
	
	lookUp += CURRENT_DASH_STATUS << 1;
	
	if (TOTAL_DASH_STATUS == 0)
		lookUp += 2;
	else if (TOTAL_DASH_STATUS == 3)
		lookUp -= 2;
	
	memcpy(palette, lookUp, 2);
	memcpy(palette + 1, lookUp + 1, 2);
}
void set_dash_speed(int *velX, int *velY){
	
	*velY = F_DashSpeed * ((CHAR_flags & 0x8) >> 3) * ((CHAR_flags & 0x10) ? -1 : 1);
	int newVelX = F_DashSpeed * ((CHAR_flags & 0x2) >> 1) * ((CHAR_flags & 0x4) ? -1 : 1);
	
	// slow diagonal speed to minimize 
	if (*velY && newVelX)
	{
		*velY = FIXED_MULT(*velY, 0xB4);
		newVelX = FIXED_MULT(newVelX, 0xB4);
	}
	
	if (INT_ABS(newVelX) > INT_ABS(*velX) || newVelX == 0)
		*velX = newVelX;
	else
		*velX = INT_ABS(*velX) * INT_SIGN(newVelX);
}
void CHAR_stop_zipline(){
	if (!speedRetentionTimer){
		wallSpeedRetained = CHAR_phys->velX;
		speedRetentionTimer = ZIP_RETENTION_TIME;
	}
}
void CHAR_refill_dash(char force){
	if (force || (CAN_REFILL_DASH && !IN_BACKGROUND))
		CHAR_flags = (CHAR_flags & ~DASH_CURR_MASK) | (TOTAL_DASH_STATUS << DASH_CURR_SHIFT);
	set_hair_color();
}
void CHAR_refill_stamina(){
	stamina = F_ClimbMaxStamina;
	staminaBlink = 0;
}
void CHAR_dash_count(int value){
	value &= 0x3;
	CHAR_flags = (CHAR_flags & ~TOTAL_DASH_MASK) | ((value << TOTAL_DASH_SHIFT) & TOTAL_DASH_MASK);
}
unsigned int GetWall(int dir, int size){
	if (dir == 0)
		dir = FACING_LEFT(CHAR_phys->flags) == 1 ? -1 : 1;
	
	unsigned int s 		= collide_rect(CHAR_phys->x + (dir > 0 ? INT2FIXED(NORMAL_HITBOX_W) : -INT2FIXED(size)), CHAR_phys->y, size, WALL_HITBOX_H) & COLLISION_WALL;
	unsigned int danger = collide_rect(CHAR_phys->x + (dir > 0 ? INT2FIXED(NORMAL_HITBOX_W) : -INT2FIXED(size)), CHAR_phys->y, size, WALL_HITBOX_H) & COLLISION_WALLBLOCK;
	
	return s && !danger;
}
void CHAR_Restart() {
	CHAR_phys->x = saveX;
	CHAR_phys->y = saveY;
	
	CHAR_flags &= ~(BACKGROUND_FLAG);
	
	if ((GAME_music_beat & (NOTE_BLOCK_BEAT))) {
		memcpy(&pal_bg_mem[13 << 4], note_a1_pal, 32);
		memcpy(&pal_bg_mem[14 << 4], note_b2_pal, 32);
		GAME_music_beat += (NOTE_BLOCK_BEAT);
	}
	
	int i = 0;
	for (; i < 8; i += 2)
	{
		hairPoints[i] = CHAR_phys->x + 0x400;
		hairPoints[i + 1] = CHAR_phys->y + 0x400;
	}
	
	varJumpTimer = 0;
	varJumpSpeed = 0;
	speedRetentionTimer = 0;
	CHAR_dashTimer = 0;
	forceMoveXTimer = 0;
	speedHorz = 0;
	speedVert = 0;
	jumpBufferTimer = 0;
	
	coyoteTimer = F_JumpGraceTime;
	CHAR_phys->flags &= ~ISDEAD_MASK;
	
	CHAR_refill_dash(1);
	CHAR_refill_stamina();
}
void CHAR_Die(){

	mmEffectEx(&death);
	
	++DEATH_count;
	if (RECORD_DEATHS) {
		++DEATH_tempCount;
	}
	else {
		GAME_IL_timer = 0;
	}
	
	heart_freeze = 0;
	
	CHAR_phys->velX = 0;
	CHAR_phys->velY = 0;
	
	if (CHAR_state == ST_ZIPLINE) {
		PHYS_actors[grabbed_entity].flags |= ACTOR_COLLIDABLE_MASK;
	}
	
	CHAR_change_state(ST_NORMAL);
	
	deathAnimTimer = 63;
	CHAR_phys->flags |= ISDEAD_MASK;
	
	
}
void LargeBounce(int actor) {
				
	int dirX = (CHAR_phys->x + 0x380) - (PHYS_actors[actor].x + (PHYS_actors[actor].width  << 7));
	int dirY = (CHAR_phys->y + 0x550) - (PHYS_actors[actor].y + (PHYS_actors[actor].height << 7));
	int dist = FIXED_sqrt(FIXED_MULT(dirX, dirX) + FIXED_MULT(dirY, dirY));
	
	dirX = FIXED_DIV(dirX, dist);
	dirY = FIXED_DIV(dirY, dist);
	
	//dirY = FIXED_MULT(dirY, 0xD8);
	
	if (dirY < 0xA0 && dirY > -0x80)
	{
		forceMoveX = INT_SIGN(dirX);
		forceMoveXTimer = 3;
		dirY = -0x80;
		dirX = INT_SIGN(dirX) * 0xD0;
	}
	
	CHAR_change_state(ST_NORMAL);
	
	CHAR_phys->velX = dirX * 3;
	CHAR_phys->velY = dirY * 3;
}
void SmallBounce(int actor) {
				
	int dirX = (CHAR_phys->x + 0x380) - (PHYS_actors[actor].x + (PHYS_actors[actor].width  << 7));
	int dirY = (CHAR_phys->y + 0x550) - (PHYS_actors[actor].y + (PHYS_actors[actor].height << 7));
	int dist = FIXED_sqrt(FIXED_MULT(dirX, dirX) + FIXED_MULT(dirY, dirY));
	
	dirX = FIXED_DIV(dirX, dist);
	dirY = FIXED_DIV(dirY, dist);
	
	//CHAR_change_state(ST_NORMAL);
	
	CHAR_phys->velX = dirX * 2;
	CHAR_phys->velY = dirY * 2;
}
void Jump(int *velX, int *velY, int dirX){
	
	jumpBufferTimer = 0;
	CHAR_dashTimer = 0;
	
	mmEffectEx(&jump);
	
	if (ON_GROUND) {
		speedRetentionTimer = 0;
	}
	
	if (dirX == 0 & *velX != 0)
		dirX = INT_SIGN(*velX);
	
	if (dirX != 0){
		*velX += F_JumpHBoost * dirX;
		// Allow double corner boosting?
		//	wallSpeedRetained += F_JumpHBoost * dirX * (speedRetentionTimer > 0);
	}
	*velY = F_JumpSpeed;
	
	varJumpTimer = F_VarJumpTime;
	varJumpSpeed = *velY;
}
void ClimbJump(int *velX, int *velY){
	
	if (!ON_GROUND || (CHAR_flags & GROUND_JUMP_MASK))
	{
		speedRetentionTimer = F_WallSpeedRetentionTime * (speedRetentionTimer > 0);
		stamina -= F_ClimbJumpCost;
		if (!IS_TIRED)
			staminaBlink = 3;
	}
	
	climbJumpSave = FACING_LEFT(CHAR_phys->flags) * -12;
	
	Jump(velX, velY, *velX ? INT_SIGN(*velX) : 0);
	
	int i, rng;
	for (i = 0; i < 4; ++i){
		
		rng = RNG();
		
		int flip = (CHAR_phys->flags & ACTOR_FLIPX_MASK) > 0;
		
		int offX = (rng & 0xF) * (flip ? 1 : -1);
		int offY = -0x60;
		int velX = (0x04 + ((rng >> 5) & 0x7)) * (flip ? 1 : -1);
		int velY =  0xD8 + ((rng >> 8) & 0xF);
		
		offX += (rng >> 12) & 0xF;
		
		offX = ((CHAR_phys->x >> 4) + offX) & 0xFFFF;
		offY = ((CHAR_phys->y >> 4) + offY) & 0xFFFF;
		
		AddParticle(offX | (0x85000000 + (rng & 0x11000000)) | ((velX & 0xFF) << 16),
					offY |  0x80000000						 | ((velY & 0xFF) << 16),
					0x100 | 31 | ((1) << 10));
	}
}
void SuperJump(int *velX, int *velY, int dir){
	if (CHAR_flags & GROUND_JUMP_MASK)
		return;
	
	mmEffectEx(&jumpSuper);
	
	jumpBufferTimer = 0;
	varJumpTimer = F_VarJumpTime;
	speedHorz = 12;
	
	CHAR_dashTimer = 0;
	
	if (!dir)
		dir = (CHAR_flags & 0x4) ? -1 : 1;
	
	int speed = F_SuperJumpH * dir;
	if (INT_ABS(speed) > INT_ABS(*velX))
		*velX = speed;
	
	//*velX += 0x04 * dir;
	*velY = F_JumpSpeed;

	if (IS_DUCKING)
	{
		SET_DUCKING(0);
		*velX = FIXED_MULT(*velX, F_DuckSuperJumpXMult);
		*velY = FIXED_MULT(*velY, F_DuckSuperJumpYMult);
	}

	varJumpSpeed = *velY;
}
void ClimbHop(int *velX, int *velY, int dir){

	*velX = dir * F_ClimbHopX;

	*velY = SIGNED_MIN(*velY, F_ClimbHopY);
	forceMoveX = 0;
	forceMoveXTimer = F_ClimbHopForceTime;
}
void SuperWallJump(int *velX, int *velY, int dir){
	
	mmEffectEx(&jumpWallBounce);
	
	speedVert = 12;
	jumpBufferTimer = 0;
	
	varJumpTimer = F_SuperWallJumpVarTime;
	CHAR_dashTimer = 0;

	*velX = F_SuperWallJumpH * dir;
	*velY = F_SuperWallJumpSpeed;
	varJumpSpeed = *velY;
}
void WallJump(int *velX, int *velY, int dir, int moveX){
	
	climbJumpSave = 0;
	
	varJumpTimer = F_VarJumpTime;
	CHAR_dashTimer = 0;
	jumpBufferTimer = 0;
	
	mmEffectEx(&jump);
	
	if (moveX != 0)
	{
		forceMoveX = INT_SIGN(dir);
		forceMoveXTimer = F_WallJumpForceTime;
		if (IS_RAINY)
			forceMoveXTimer += 3;
	}
	
	*velX = INT_SIGN(dir) * (int)(F_WallJumpHSpeed);
	*velY = F_JumpSpeed;
	varJumpSpeed = *velY;
}
void RemoveDash(){
	if (TOTAL_DASH_STATUS < 3 && CURRENT_DASH_STATUS >= 1)
		CHAR_flags = ((CHAR_flags & DASH_CURR_MASK) - ONE_DASH_MASK) | (CHAR_flags & ~DASH_CURR_MASK);
}

// Update Methods
void CHAR_change_state(int value){
	
	switch (CHAR_state)
	{
	case ST_NORMAL:
		CHAR_end_normal();
		break;
	}
	switch (value)
	{
	case ST_CLIMB:
		updateMethod = CHAR_update_climb;
		CHAR_climb_start();
		break;
	case ST_DASH:
		updateMethod = CHAR_update_dash;
		CHAR_dash_start();
		break;
	case ST_DREAMDASH:
		updateMethod = CHAR_update_dreamdash;
		CHAR_dreamdash_start();
		break;
	case ST_ZIPLINE:
		updateMethod = CHAR_update_zip;
		CHAR_zip_start();
		break;
	case ST_NORMAL:
		updateMethod = CHAR_update_normal;
		break;
	case ST_VIEWER:
		updateMethod = CHAR_update_viewer;
		break;
	}
	CHAR_state = value;
}
void CHAR_end_normal(){
	speedRetentionTimer = 0;
}
void CHAR_dreamdash_start(){
	
	CHAR_dashTimer = 0;
	dreamDashStayTimer = 4;
	speedRetentionTimer = 0;
	
	int velY = F_DashSpeed * ((CHAR_flags & 0x8) >> 3) * ((CHAR_flags & 0x10) ? -1 : 1);
	int velX = F_DashSpeed * ((CHAR_flags & 0x2) >> 1) * ((CHAR_flags & 0x4) ? -1 : 1);
	
	if (velY && velX)
	{
		velX = FIXED_MULT(velX, 0xB4);
		velY = FIXED_MULT(velY, 0xB4);
	}
	
	CHAR_phys->x += velX;
	CHAR_phys->y += velY;
	
	CHAR_phys->velX = velX;
	CHAR_phys->velY = velY;
}
void CHAR_update_dreamdash(){
	
	
	if (dreamDashStayTimer)
	{
		--dreamDashStayTimer;
		return;
	}
	
	int velX = CHAR_phys->velX;
	int velY = CHAR_phys->velY;
	
	int oldX = CHAR_phys->x;
	int oldY = CHAR_phys->y;
	
	if (velX)
		coyoteTimer = F_JumpGraceTime;
	
	int hit = collide_rect(CHAR_phys->x, CHAR_phys->y, NORMAL_HITBOX_W, NORMAL_HITBOX_H);
	
	if (!(hit & COLLISION_DREAM))
	{
		int tempX = INT_SIGN(velX), tempY = INT_SIGN(velY);
		
		hit = collide_rect(CHAR_phys->x + 0x200, CHAR_phys->y + 0x400, NORMAL_HITBOX_W - 4, NORMAL_HITBOX_H - 8);
		if (hit & COLLISION_SOLID)
		{
			if (0)
			{
				velX *= -1;
				velY *= -1;
			}
			else
			{
				CHAR_phys->x = oldX;
				CHAR_phys->y = oldY;
				CHAR_Die();
				return;
			}
		}
		else
		{
			CHAR_refill_stamina();
			CHAR_refill_dash(0);
			
			GAME_freeze = 3;
			
			int i;
			for (; i < 8; i += 2)
			{
				hairPoints[i] = CHAR_phys->x + 0x400;
				hairPoints[i + 1] = CHAR_phys->y + 0x400;
			}
			
			if (jumpBufferTimer && velX && !(CHAR_flags & GROUND_JUMP_MASK))
			{
				Jump(&velX, &velY, horz);
			}
			else
			{
				unsigned int wallL = GetWall(-1, WALL_HITBOX_W);
				unsigned int wallR = GetWall(1, WALL_HITBOX_W);
				
				if (KEY_DOWN_NOW(KEY_SHOULDER) && velX && INT_SIGN(velX) == -horz)
				{
					SET_FACING(CHAR_phys->flags, horz);
					velX = horz * 0x4000;
					velY = 0;
					
					CHAR_change_state(ST_CLIMB);
					goto updateDreamdashReturn;
				}
			}
			
			for (i = 0; i < 8; i += 2)
			{
				hairPoints[i] = CHAR_phys->x + 0x400;
				hairPoints[i + 1] = CHAR_phys->y + 0x400;
			}
			
			CHAR_change_state(ST_NORMAL);
			goto updateDreamdashReturn;
		}
	}
	
	if (dashBufferTimer && CURRENT_DASH_STATUS)
	{
		dashBufferTimer = 0;
		velY = F_DashSpeed * (key_tri_vert());
		velX = F_DashSpeed * (key_tri_horz());
		
		if (velY && velX)
		{
			velX = FIXED_MULT(velX, 0xB4);
			velY = FIXED_MULT(velY, 0xB4);
		}
		if (velX)
			SET_FACING(CHAR_phys->flags, velX);
		
		RemoveDash();
	}
	
	updateDreamdashReturn:
	
	CHAR_phys->velX = velX;
	CHAR_phys->velY = velY;
}
void CHAR_climb_start(){
	CHAR_dashTimer = 0;
	climbNoMoveTimer = F_ClimbNoMoveTime;
	lastClimbMove = 0;
}
void CHAR_update_climb(){
	int velX = CHAR_phys->velX;
	int velY = CHAR_phys->velY;
	int facingLeft = FACING_LEFT(CHAR_phys->flags);
	unsigned int wallL = GetWall(-1, WALL_HITBOX_W);
	unsigned int wallR = GetWall(1, WALL_HITBOX_W);
	unsigned int wall = facingLeft == 1 ? wallL : wallR;
	
	if (climbNoMoveTimer >= F_ClimbNoMoveTime - 1)
	{		
		velY = FIXED_MULT(velY, F_ClimbGrabYMult);
		velX = 0;
	}
	if (climbNoMoveTimer)
		--climbNoMoveTimer;
	
	//Refill stamina on ground
	if (ON_GROUND)
		CHAR_refill_stamina();

	//Wall Jump
	if (jumpBufferTimer)
	{
		if (horz != facingLeft && !(IS_RAINY && velY > 0))
			ClimbJump(&velX, &velY);
		else
			WallJump(&velX, &velY, wallL ? 1 : -1, horz);
		
		CHAR_change_state(ST_NORMAL);
		goto climbUpdateReturn;
	}

	//Let go
	if (!KEY_DOWN_NOW(KEY_SHOULDER))
	{
		CHAR_change_state(ST_NORMAL);
		goto climbUpdateReturn;
	}

	//No wall to hold
	if (!wall)
	{
		//Climbed over ledge?
		if (velY < 0)
		{
			ClimbHop(&velX, &velY, -facingLeft);
		}
		CHAR_change_state(ST_NORMAL);
		goto climbUpdateReturn;
	}
	
		//Climbing
	int target = 0;
	char trySlip = 0;
	if (climbNoMoveTimer <= 0)
	{
		if (vert == -1)
		{
			target = F_ClimbUpSpeed;
			int xPos = CHAR_phys->x + (facingLeft == 1 ? -INT2FIXED(WALL_HITBOX_W) : INT2FIXED(NORMAL_HITBOX_W));
			
			//Up Limit
			if ((collide_rect(xPos, (CHAR_phys->y) + 0x200, WALL_HITBOX_W, 1) & COLLISION_CLIMB_BLOCK) &&
				(collide_rect(xPos, (CHAR_phys->y) + 0xA00, WALL_HITBOX_W, 1) & COLLISION_WALL))
			{
				if (velY < 0)
					velY = 0;
				target = 0;
				trySlip = 1;
			}
		}
		else if (vert == 1)
		{
			target = F_ClimbDownSpeed;
			
			if (ON_GROUND)
			{
				target = 0;
			}
		}
		else
			trySlip = 1;
	}
	else
		trySlip = 1;
	
	if (target == 0)
		lastClimbMove = 0;
	else
		lastClimbMove = INT_SIGN(target);
	
	{
		int xPos = CHAR_phys->x + (facingLeft == 1 ? -INT2FIXED(WALL_HITBOX_W) : INT2FIXED(NORMAL_HITBOX_W));
		
		//Slip down if hands above the ledge and no vertical input
		if (trySlip && !(collide_rect(xPos, CHAR_phys->y, WALL_HITBOX_W, NORMAL_HITBOX_H - 8) & COLLISION_WALL))
			target = F_ClimbSlipSpeed;
	}
	
	//Set Speed
	velY = FIXED_APPROACH(velY, target, F_ClimbAccel);
	
	//Down Limit
	//if (vert != 1 && velY > 0)
		//velY = 0;

	//Stamina
	//*
	if (lastClimbMove == -1)
	{
		stamina -= F_ClimbUpCost;
	}
	else if (lastClimbMove == 0)
		stamina -= F_ClimbStillCost;
	//*/

	//Too tired
	if (stamina <= 0)
	{
		stamina = 0;
		//Speed += LiftBoost;
		CHAR_change_state(ST_NORMAL);
		goto climbUpdateReturn;
	}
	//*/
	
	// [dash]
	if ((!CHALLENGE_MODE || current_chapter != LEVELIDX_PROLOGUE) && dashBufferTimer && CURRENT_DASH_STATUS) // Start dash
	{
		
		RemoveDash();
		CHAR_change_state(ST_DASH);
	}
	
	climbUpdateReturn:
	
	CHAR_phys->velX = velX;
	CHAR_phys->velY = velY;
}
void CHAR_update_normal(){
	CHAR_phys->flags &= ~WALL_SLIDING;
	
	int velX = CHAR_phys->velX;
	int velY = CHAR_phys->velY;
	
	unsigned int wallL = GetWall(-1, WALL_HITBOX_W);
	unsigned int wallR = GetWall(1, WALL_HITBOX_W);
	
	if (KEY_DOWN_NOW(KEY_SHOULDER) && !(IS_TIRED || IS_RAINY))
	{	
		int facingLeft = FACING_LEFT(CHAR_phys->flags);
		//Climbing
		if (velY >= 0 && ((INT_SIGN(velX) == -facingLeft) || velX == 0))
		{
			if (facingLeft == 1 ? wallL : wallR)
			{
				CHAR_phys->x = ((CHAR_phys->x + INT2FIXED(facingLeft == 1 ? 0 : WALL_HITBOX_W + NORMAL_HITBOX_W)) & ~0x7FF) - INT2FIXED((facingLeft == 1) ? 0 : NORMAL_HITBOX_W);
				SET_DUCKING(0);
				CHAR_change_state(ST_CLIMB);
			}
		}
	}
	
	// dash
	if ((!CHALLENGE_MODE || current_chapter != LEVELIDX_PROLOGUE) && dashBufferTimer && CURRENT_DASH_STATUS) // Start dash
	{
		RemoveDash();
		
		CHAR_change_state(ST_DASH);
		goto normalUpdateReturn;
	}
	
	if (CHAR_dashTimer <= 2)
		SET_DUCKING(vert > 0 && ON_GROUND && CHAR_phys->velY <= 0);
	
	horz -= horz * (IS_DUCKING && ON_GROUND);
	
	// force walking
	forceMoveXTimer -= forceMoveXTimer > 0;
	horz += (forceMoveX - horz) * (forceMoveXTimer > 0);
	
	//Walking
	{
		int mult = (ON_GROUND) ? 0x100 : F_AirMult;
		
		int max = F_MaxRun;
		
		if (INT_ABS(velX) > max && INT_SIGN(velX) == horz)
			velX = FIXED_APPROACH(velX, FIXED_MULT(max, horz), FIXED_MULT(F_RunReduce, mult));  //Reduce back from beyond the max speed
		else
			velX = FIXED_APPROACH(velX, max * horz, FIXED_MULT(F_RunAccel, mult));   //Approach the max speed
	}
	
	//Vertical
	{
		int maxFall = 0;
		//Calculate current max fall speed
		maxFall = F_FastMaxFall + (F_MaxFall - F_FastMaxFall) * (vert < 1);

		//Gravity
		if (!ON_GROUND)
		{
			int wallSlideDir = 0;
			
			//Wall Slide
			int facingLeft = FACING_LEFT(CHAR_phys->flags);
			
			if ((horz == -facingLeft || (horz == 0 && KEY_DOWN_NOW(KEY_SHOULDER))) && vert < 1)
			{
				if (velY >= 0 && (facingLeft == 1 ? wallL : wallR))
				{
					SET_DUCKING(0);
					wallSlideDir = (int)facingLeft;
				}
				
				if (wallSlideDir != 0)
				{
					maxFall = FIXED_MULT(maxFall, 0x40);
					CHAR_phys->flags |= WALL_SLIDING;
				}
			}
			if ((IS_RAINY) && !(CHAR_phys->flags & WALL_SLIDING) && vert < 1)
			{
				maxFall = FIXED_MULT(maxFall, 0x70);
			}
		}
		
		int mult = (INT_ABS(velY) < F_HalfGravThreshold && KEY_DOWN_NOW(KEY_A)) ? 0x80 : 0x100;
			
		velY = FIXED_APPROACH(velY, maxFall, FIXED_MULT(F_Gravity, mult));
		
		//Variable Jumping
		if (varJumpTimer > 0)
		{
			--varJumpTimer;
			
			if (KEY_DOWN_NOW(KEY_A))
				velY = varJumpSpeed;
			else
				varJumpTimer = 0;
		}
		
		//Jumping
		if (jumpBufferTimer)
		{
			if (CAN_JUMP && (coyoteTimer || !(wallL | wallR) || (CHAR_dashTimer && velY >= 0 && velX != 0)))
			{
				if (!coyoteTimer){
					RemoveDoubleJump();
				}
				
				if (CHAR_dashTimer > 2 && velY >= 0 && velX != 0)
					SuperJump(&velX, &velY, horz);
				else
					Jump(&velX, &velY, horz);
			}
			else
			{
				if (CHAR_dashTimer > 0 && (IS_DASH_DIR(DASH_UP_FLAGS))) {
					wallL = GetWall(-1, 6);
					wallR = GetWall( 1, 6);
				}
				
				if (wallL || wallR)
				{
					int facingLeft = FACING_LEFT(CHAR_phys->flags);
					int jumpDir = (wallL && wallR) ? -facingLeft : (wallL ? 1 : -1);
					
					if (KEY_DOWN_NOW(KEY_SHOULDER) && stamina > 0 && (facingLeft == 1 ? wallL : wallR) && !(IS_RAINY && INT_ABS(velX) < 0x40)){
						ClimbJump(&velX, &velY);
					}
					else if (CHAR_dashTimer <= 0 || !(IS_DASH_DIR(DASH_UP_FLAGS)))
						WallJump(&velX, &velY, jumpDir, horz);
					else {
						SuperWallJump(&velX, &velY, jumpDir);
					}
						
					CHAR_change_state(ST_NORMAL);
				}
			}
		}
	}
	
	if (ON_GROUND && (width > 32 || height > 22) && key_hit(KEY_SELECT)) {
		CHAR_change_state(ST_VIEWER);
		velX = 0;
		velY = 0;
		forceMoveX = 0;
	}
	
	normalUpdateReturn:
	
	// Flip character based on horizontal input
	if (horz)
		SET_FACING(CHAR_phys->flags, horz);
	
	CHAR_phys->velX = velX;
	CHAR_phys->velY = velY;
}
void CHAR_update_viewer(){
	int ncamX = FIXED2INT(PHYS_actors[0].x) - 120;
		
	if (ncamX < SCREEN_LEFT)
		ncamX = SCREEN_LEFT;
	else if (ncamX > SCREEN_RIGHT)
		ncamX = SCREEN_RIGHT;
	
	SET_FACING(CHAR_phys->flags, camX - ncamX);
	
	if (forceMoveX) {
		int ncamY = FIXED2INT(PHYS_actors[0].y) -  90;
		if (ncamY < SCREEN_TOP)
			ncamY = SCREEN_TOP;
		else if (ncamY > SCREEN_BOTTOM)
			ncamY = SCREEN_BOTTOM;
		
		camX = FIXED_APPROACH(camX, ncamX, 4);
		camY = FIXED_APPROACH(camY, ncamY, 4);
		
		if (camX == ncamX && camY == ncamY)
			CHAR_change_state(ST_NORMAL);
	}
	else {
		camX += (key_tri_horz() * 2) * !forceMoveX;
		camY += (key_tri_vert() * 2) * !forceMoveX;
		
		if (key_hit(KEY_SELECT | KEY_START | KEY_B | KEY_A)) {
			forceMoveX = 1;
			key_mod2(KEY_START | KEY_B | KEY_A);
		}
	}
}
void CHAR_zip_start() {
	CHAR_phys->velY = 0;
	PHYS_actors[grabbed_entity].x = CHAR_phys->x - ZIP_GRAB_OFFSET;
	PHYS_actors[grabbed_entity].velX = 0;
	PHYS_actors[grabbed_entity].flags &= ~ACTOR_COLLIDABLE_MASK;
	
	ZIP_update(&PHYS_actors[grabbed_entity]);
}
void CHAR_update_zip(){
	
	if (speedRetentionTimer)
	{
		CHAR_phys->velX = wallSpeedRetained;
	}
	
	if (CHAR_phys->y != PHYS_actors[grabbed_entity].y + ZIP_GRAB_HEIGHT)
	{
		if (!(collide_rect(CHAR_phys->x, PHYS_actors[grabbed_entity].y + ZIP_GRAB_HEIGHT, NORMAL_HITBOX_W, NORMAL_HITBOX_H) & COLLISION_WALL)) {
			CHAR_phys->y = PHYS_actors[grabbed_entity].y + ZIP_GRAB_HEIGHT;
		}
	}
	
	if (!KEY_DOWN_NOW(KEY_SHOULDER) || !stamina)
	{
		if (!stamina && jumpBufferTimer){
			jumpBufferTimer = 0;
			zipJumpBuffer = 10;
		}
		
		PHYS_actors[grabbed_entity].velX = CHAR_phys->velX;
		CHAR_change_state(ST_NORMAL);
		PHYS_actors[grabbed_entity].velX = FIXED_MULT(PHYS_actors[grabbed_entity].velX, ZIP_SLOWDOWN);
		ZIP_ungrab(grabbed_entity);
		
		return;
	}
	if (dashBufferTimer && CURRENT_DASH_STATUS)
	{
		PHYS_actors[grabbed_entity].velX = CHAR_phys->velX;
		zipJumpBuffer = 10;
		
		dashBufferTimer = 0;
		RemoveDash();
		CHAR_change_state(ST_DASH);
		ZIP_ungrab(grabbed_entity);
		
		return;
	}
	
	if (jumpBufferTimer)
	{
		jumpBufferTimer = 0;
		zipJumpBuffer = 10;
		
		if (INT_ABS(CHAR_phys->velX) > ZIP_SPEED + 48)
			speedHorz = 12;
		
		PHYS_actors[grabbed_entity].velX = CHAR_phys->velX;
		
		PHYS_actors[grabbed_entity].velX = FIXED_MULT(PHYS_actors[grabbed_entity].velX, ZIP_SLOWDOWN);
		
		Jump(&CHAR_phys->velX, &CHAR_phys->velY, horz);
		CHAR_change_state(ST_NORMAL);
		ZIP_ungrab(grabbed_entity);
		
		stamina -= F_ClimbJumpCost >> 1;
		
		if (stamina < 0)
			stamina = 0;
		
		return;
	}
	
	if (horz && !speedRetentionTimer)
	{
		if (INT_SIGN(CHAR_phys->velX) != horz || INT_ABS(CHAR_phys->velX) < ZIP_SPEED)
		{
			CHAR_phys->velX += horz * 0x20;
			stamina -= F_ClimbStillCost;
		}
	}
	
}

void CHAR_update_dash(){
	int velX = CHAR_phys->velX;
	int velY = CHAR_phys->velY;
	
	//initialize dash direction
	if (CHAR_dashTimer == Fr_DashTime - 1)
	{
		CHAR_flags &= ~0x1E;
		
		if (horz) {
			CHAR_flags |= ((horz & 0x2) ? 0x4 : 0) | 0x2;
		}
		if (vert) {
			CHAR_flags |= ((vert & 0x2) ? 0x10 : 0) | 0x8;
		}
		
		// if no dash direction, set to horizontal
		if (!(CHAR_flags & 0x1E)){
			CHAR_flags |= 0x2 | (CHAR_phys->flags & ACTOR_FLIPX_MASK)<<2;
		}
		
		//set speed
		set_dash_speed(&velX, &velY);
		
		if (velX)
			SET_FACING(CHAR_phys->flags, velX);
	}
	
	// End dash
	if (CHAR_dashTimer <= Fr_DashFrameEnd){
		CHAR_change_state(ST_NORMAL);
		
		// Slow down velocity after dash
		if (velY <= 0){
			velY = F_EndDashSpeed * INT_SIGN(velY);
			velX = F_EndDashSpeed * INT_SIGN(velX);
			
			if (velY && velX)
			{
				velY = FIXED_MULT(velY, 0xB4);
				velX = FIXED_MULT(velX, 0xB4);
			}
			if (velY < 0)
			{
				velY = FIXED_MULT(velY, 0xC0);
			}
		}
		CHAR_phys->velX = velX;
		CHAR_phys->velY = velY;
		return;
	}
	else{
		int saveY = velY;
		
		set_dash_speed(&velX, &velY);
		
		if (saveY == 0 && velY > 0)
			velY = 0;
	}
	
	coyoteTimer = coyoteTimer * (velY >= 0);
	
	unsigned int wallL = GetWall(-1, WALL_HITBOX_W);
	unsigned int wallR = GetWall(1, WALL_HITBOX_W);
	
	if (CHAR_flags & 0x2) {
		//Super Jump
		if (jumpBufferTimer && CAN_JUMP)
		{
			if (!coyoteTimer)
				RemoveDoubleJump();
			
			if (velY >= 0)
				SuperJump(&velX, &velY, horz);
			else
				Jump(&velX, &velY, horz);
			CHAR_change_state(ST_NORMAL);
		}
	}
	
	// Special Jumps
	if (IS_DASH_DIR(DASH_UP_FLAGS))
	{
		if (jumpBufferTimer)
		{
			jumpBufferTimer = 0;
			
			if ((wallL | wallR))
			{
				SuperWallJump(&velX, &velY, wallL ? 1 : -1);
				CHAR_change_state(ST_NORMAL);
				
			}
		}
	}
	else
	{
		if (jumpBufferTimer)
		{
			jumpBufferTimer = 0;
			
			if (wallL || wallR)
			{
				if (KEY_DOWN_NOW(KEY_SHOULDER) && !IS_RAINY) {
					ClimbJump(&velX, &velY);
				}
				else {
					WallJump(&velX, &velY, wallL ? 1 : -1, horz);
				}
				CHAR_change_state(ST_NORMAL);
			}
		}
	}
	
	CHAR_phys->velX = velX;
	CHAR_phys->velY = velY;
}
void CHAR_dash_start(){
	
	GAME_freeze = 5;
	speedHorz = 0;
	speedVert = 0;
	CHAR_dashTimer = Fr_DashTime;
	varJumpTimer = 0;
	
	if (CURRENT_DASH_STATUS > 0)
		mmEffectEx(&dash);
	else
		mmEffectEx(&dashDouble);
	
	if (key_tri_vert() > 0)
	{
		SET_DUCKING(1);
	}
	else{
		SET_DUCKING(0);
	}
	int actor_index, actor_id;
	for (actor_index = 0; actor_index < ActorLimit; ++actor_index){
		
		if (!ACTOR_ENABLED(PHYS_actors[actor_index].flags))
			continue;
			
		actor_id = ACTOR_ENTITY_TYPE(PHYS_actors[actor_index].ID);
		
		if (actor_id == 1)
		{
			STRAWB_ondash(&PHYS_actors[actor_index]);
		}
	}
}

// Physics
void check_entities(){
	unsigned int value = collide_entity(0);
	int type = ACTOR_ENTITY_TYPE(value);
	
	if (deathAnimTimer)
		return;
	
	if (value & 0xFFF)
	{
		if (type != ENT_STRAWB && type != ENT_BACKGROUND && IN_BACKGROUND)
			return;
		
		switch (type)
		{
		case ENT_STRAWB:
			STRAWB_playergrab(ACTOR_ENTITY_ID(value));
			break;
		case ENT_DASH:
			if (IS_TIRED || ((TOTAL_DASH_STATUS == 1 || TOTAL_DASH_STATUS == 2) && CURRENT_DASH_STATUS < TOTAL_DASH_STATUS)){
				
				DASHCR_playergrab(ACTOR_ENTITY_ID(value));
				CHAR_refill_dash(1);
				CHAR_refill_stamina();
			}
			break;
		case ENT_SPRING:
			
			CHAR_phys->velY = SPRING_bounce;
			varJumpSpeed = SPRING_bounce;
			
			varJumpTimer = SPRING_hold_time;
			
			CHAR_change_state(ST_NORMAL);
			
			PHYS_actors[ACTOR_ENTITY_ID(value)].flags = PHYS_actors[ACTOR_ENTITY_ID(value)].flags & (~0xF000) | 0x9000;
			
			CHAR_refill_dash(0);
			CHAR_refill_stamina();
			break;
		case ENT_HEART:
			if (CHAR_dashTimer) {
				
				int i;
				for (i = 0; i < 10; ++i) {
					
					int rng = RNG();
					
					int offX = (rng		) & 0x7F;
					int offY = (rng >> 8) & 0x7F;
					offX -= 0x3F;
					offY -= 0x3F;
					
					offX = (((PHYS_actors[ACTOR_ENTITY_ID(value)].x + 0x300) >> 4) + offX) & 0xFFFF;
					offY = (((PHYS_actors[ACTOR_ENTITY_ID(value)].y + 0x300) >> 4) + offY) & 0xFFFF;
					
					int velX = (rng >> 16) & 0x1F;
					offX |= ((velX - 0xF) & 0xFF) << 16;
						velX = (rng >> 22) & 0x1F;
					offY |= ((velX - 0xF) & 0xFF) << 16;
					
					// Lifetime
					offX |= (rng & 0xFF000000) + 0x00000000;
					
					AddParticle(offX,
								offY | 0x42000000,
								Heart_S | ((rng & 0x3) << 10));
				}
				
				heart_freeze = 1;
				
				PHYS_actors[ACTOR_ENTITY_ID(value)].flags &= ~ACTOR_COLLIDABLE_MASK;
				
				if (levelFlags & LEVELFLAG_BSIDE) {
					GB_StopMusic();
				}
				
				break;
			}
			CHAR_refill_dash(1);
			CHAR_refill_stamina();
			
			SmallBounce(ACTOR_ENTITY_ID(value));
			
			break;
		case ENT_BUMPER:
			if (current_chapter == LEVELIDX_CORE && IS_CORE_HOT) {
				CHAR_Die();
			}
			else{
				CHAR_refill_dash(0);
				CHAR_refill_stamina();
				
				BUMPER_playerhit(ACTOR_ENTITY_ID(value));
				
				LargeBounce(ACTOR_ENTITY_ID(value));
				
				break;
			}
			break;
		case ENT_ZIP:
			if ((!zipJumpBuffer || grabbed_entity != ACTOR_ENTITY_ID(value)) && !IS_TIRED && CHAR_state != ST_ZIPLINE && CHAR_state != ST_CLIMB && KEY_DOWN_NOW(KEY_SHOULDER)) {
				grabBufferTimer = 0;
				grabbed_entity = ACTOR_ENTITY_ID(value);
				
				CHAR_change_state(ST_ZIPLINE);
				
				CHAR_phys->y = PHYS_actors[grabbed_entity].y + ZIP_GRAB_HEIGHT;
				ZIP_grab(grabbed_entity);
				speedRetentionTimer = 0;
				forceMoveXTimer = 0;
			}
			break;
		case ENT_KEY:
			KEY_playergrab(ACTOR_ENTITY_ID(value));
			break;
		case ENT_CASSETTE:
			saveFile[CASSETTE_COLL_IDX] |= (1 << current_chapter);
			levelFlags &= ~MUSICAL_LEVEL;
			GAME_music_beat = 0;
			
			STRAWB_SETTEMPCOLL(CASSETTE_TEMP);
			
			
			PHYS_actors[ACTOR_ENTITY_ID(value)].flags &= ~ACTOR_ACTIVE_MASK;
			GB_StopMusic();
			
			memcpy(&pal_bg_mem[13 << 4], note_a1_pal, 32);
			memcpy(&pal_bg_mem[14 << 4], note_b2_pal, 32);
			break;
		case ENT_BACKGROUND:
			DASHCR_playergrab(ACTOR_ENTITY_ID(value));
			
			if ((CHAR_flags & BACKGROUND_FLAG) && (PHYS_actors[ACTOR_ENTITY_ID(value)].flags & 0x100)) {
				CHAR_flags &= ~BACKGROUND_FLAG;
				stamina = 0;
			}
			else if (!(PHYS_actors[ACTOR_ENTITY_ID(value)].flags & 0x100)) {
				CHAR_flags |= BACKGROUND_FLAG;
			}
			
			break;
		}
	}
	else{
	}
}

void CHAR_update(){
	horz = key_tri_horz();
	vert = key_tri_vert();
	
	int nextLevel = 255;
	
	if (deathAnimTimer) {
		deathAnimTimer -= deathAnimTimer > 30;
		
		if (deathAnimTimer == 30 || (key_hit(KEY_A) && deathAnimTimer > 30))
		{
			START_FADE();
			nextGamestate = GS_RESPAWN;
		}
		return;
	}
	else if (!IN_BACKGROUND) {
		// Load next level
		if (CHAR_phys->y <= -0x300){
			nextLevel = level_exits[0];
			enterX = 0;
			enterY = 1;
			CHAR_phys->y =  -0x300;
		}
		else if (FIXED2BLOCK(CHAR_phys->y - 0x800) >= height){
			nextLevel = level_exits[1];
			enterX = 0;
			enterY = -1;
			if (nextLevel == 255) {
				CHAR_Die();
				return;
			}
		}
		else if (CHAR_phys->x <= -0x000){
			nextLevel = level_exits[2];
			enterX = 1;
			enterY = 0;
			CHAR_phys->x = 0;
		}
		else if (FIXED2BLOCK(CHAR_phys->x) >= width){
			nextLevel = level_exits[3];
			enterX = -1;
			enterY = 0;
		}
		
#ifdef	__DEBUG__
//*
	if (KEY_DOWN_NOW(KEY_DOWN) && key_hit(KEY_START) && (lvlIndex + 1) < currentMax){
		nextLevel = lvlIndex + 1;
		if (NOTE_BLOCKS_ACTIVE)
			GB_StopMusic();
	}//*/
#endif
		
		if (nextLevel != 255){
			lvlIndex = nextLevel;
			
			CHAR_change_state(ST_NORMAL);
			PHYS_actors[0].velX = 0;
			PHYS_actors[0].velY = 0;
			CHAR_refill_dash(1);
			CHAR_refill_stamina();
			
			SaveData(saveFileNumber);
			if (NOTE_BLOCKS_ACTIVE)
				GB_StopMusic();
			
			nextGamestate = 1;
			START_FADE();
			return;
		}
	}
	else {
		stamina = F_ClimbMaxStamina;
		// Load next level
		if (CHAR_phys->y <= -0x000){
			CHAR_phys->y = 0;
		}
		else if (FIXED2BLOCK(CHAR_phys->y - 0x800) >= height){
			CHAR_Die();
		}
		else if (CHAR_phys->x <= -0x000){
			CHAR_phys->x = 0;
		}
		else if (FIXED2BLOCK(CHAR_phys->x) >= width){
			CHAR_phys->x = BLOCK2FIXED(width);
		}
	}
	
	
#ifdef __DEBUG__
	if (key_hit(KEY_SELECT) && key_tri_vert() == 1)
		CHAR_dash_count(TOTAL_DASH_STATUS + 1);
#endif
	
	
	
	// If key is down, get buffer
	jumpBufferTimer = SET_VAL_IF(jumpBufferTimer, 5, key_hit(KEY_A));
	jumpBufferTimer &= ~(KEY_DOWN_NOW(KEY_A) > 0);
	jumpBufferTimer -= jumpBufferTimer > 0 && !GAME_freeze;
	
	BUFFER(dashBufferTimer, 5, key_hit(KEY_B), KEY_DOWN_NOW(KEY_B));
	BUFFER(grabBufferTimer, 5, key_hit(KEY_SHOULDER), KEY_DOWN_NOW(KEY_SHOULDER));
	
	// If game is frozen (from dashing or other things), don't do anything
	if (GAME_freeze)
		return;
	
	// Keep speed after running into wall, but only for a short period
	if (speedRetentionTimer) {
		if (CHAR_state != ST_ZIPLINE){
			// Stop speed retention if moving away from wall
			if (CHAR_phys->velX && INT_SIGN(CHAR_phys->velX) == -INT_SIGN(wallSpeedRetained))
				speedRetentionTimer = 0;
			else if (!GetWall(INT_SIGN(wallSpeedRetained), 1))
			{
				CHAR_phys->velX = wallSpeedRetained;
				speedRetentionTimer = 0;
				// Show speed rings if corner boosting
				if (INT_ABS(wallSpeedRetained) >= F_DashSpeed - 0x40) {
					speedHorz = 12;
				}
			}
			else{
				--speedRetentionTimer;
			}
		}
		else{
			speedRetentionTimer -= speedRetentionTimer > 0;
		}
	}
	zipJumpBuffer -= zipJumpBuffer > 0;
	zipJumpBuffer *= KEY_DOWN_NOW(KEY_SHOULDER) > 0;
	
	
	// -- normal game state update --
	updateMethod();
	//return;
	
	check_entities();
	
	CHAR_dashTimer -= CHAR_dashTimer > 0;
	// Ticking timers
	if (CHAR_state == ST_DASH){
		
		int rng = RNG();
		
		int offX = (rng		) & 0x7F;
		int offY = (rng >> 8) & 0x7F;
		offX -= 0x3F;
		offY -= 0x3F;
		
		offX = ((CHAR_phys->x >> 4) + offX) & 0xFFFF;
		offY = (((CHAR_phys->y + 0x200) >> 4) + offY) & 0xFFFF;
		
		// Lifetime
		offX |= (rng & 0x33000000) + 0x22000000;
		
		AddParticle(offX,
					offY | 0xC0000000,
					0x000 | ((rng & 0x3) << 10));
		
	}
	if (climbJumpSave) {
		if (key_tri_horz() == -INT_SIGN(climbJumpSave)){
			stamina += F_ClimbJumpCost;
			CHAR_phys->velX = INT_SIGN(-climbJumpSave) * (int)(F_WallJumpHSpeed);
			CHAR_phys->velY = F_JumpSpeed;
			climbJumpSave = 0;
			varJumpTimer = F_VarJumpTime;
		}
		else
			climbJumpSave -= INT_SIGN(climbJumpSave);
	}
	
	coyoteTimer -= coyoteTimer > 0 && CHAR_state != ST_DASH;
	if (speedHorz){
		--speedHorz;
		
		if (INT_ABS(CHAR_phys->velX) < 0x40)
			speedHorz = 0;
		if (!(speedHorz & 0x3)){
			int offX = ((CHAR_phys->x >> 4)) & 0xFFFF;
			int offY = ((CHAR_phys->y >> 4)) & 0xFFFF;
			
			AddParticle(offX | 0x66000000 | (CHAR_phys->velX > 0 ? 0x080000 : 0xF80000),
						offY | 0xC0000000,
						8);
		}
	}
	else if (speedVert){
		--speedVert;
		if (!(speedVert & 0x3)){
			int offX = ((CHAR_phys->x >> 4)) & 0xFFFF;
			int offY = ((CHAR_phys->y >> 4)) & 0xFFFF;
			
			//offX -= 0x3F;
			//offY -= 0x3F;
			
			AddParticle(offX | 0x66000000,
						offY | 0xC0000000 | 0xF80000,
						4);
		}
	}
	
	if (CHAR_state == ST_DREAMDASH){
		CHAR_phys->x += CHAR_phys->velX;
		CHAR_phys->y += CHAR_phys->velY;
		
		int rng = RNG();
		
		int offX = (rng		) & 0x7F;
		int offY = (rng >> 8) & 0x7F;
		offX -= 0x3F;
		offY -= 0x3F;
		
		offX |= (rng & 0x33000000) + 0x22000000;
		
		offX = ((CHAR_phys->x >> 4) + offX) & 0xFFFF;
		offY = ((CHAR_phys->y >> 4) + offY) & 0xFFFF;
		
		AddParticle(offX,
					offY | 0x80000000,
					20);
	}
	else if (!deathAnimTimer) {
		//Collision.  Make hit box shorter if ducking
		int height = NORMAL_HITBOX_H - (IS_DUCKING ? DUCK_DIFF : 0);
		
		CHAR_phys->y += INT2FIXED(DUCK_DIFF) * (IS_DUCKING > 0);
		
		// velocity before collision
		int prevVelX = CHAR_phys->velX;
		int prevVelY = CHAR_phys->velY;
		
		int hit = collide_char(&CHAR_phys->x, &CHAR_phys->y, &CHAR_phys->velX, &CHAR_phys->velY, NORMAL_HITBOX_W, height);
		int XY = (hit & 0xFFFF) | (hit >> 16);
		
		if ((hit >> X_COLL_SHIFT) && !speedRetentionTimer && prevVelX){
			speedRetentionTimer = F_WallSpeedRetentionTime;
			wallSpeedRetained = prevVelX;
		}
		
		CHAR_phys->y -= INT2FIXED(DUCK_DIFF) * (IS_DUCKING > 0);
		
		// if collided vertically downwards on solid ground, or if ground is beneath player while moving down, on ground
		if (XY & COLLISION_HARM)
		{
			CHAR_Die();
			return;
		}
		else if (prevVelY >= 0 &&
			 (collide_rect(CHAR_phys->x, CHAR_phys->y + INT2FIXED(NORMAL_HITBOX_H), NORMAL_HITBOX_W, 1) 			  & COLLISION_FLOOR) &&
			!(collide_rect(CHAR_phys->x, CHAR_phys->y							  , NORMAL_HITBOX_W, NORMAL_HITBOX_H) & COLLISION_HARM) ){
			CHAR_phys->flags |= ONGROUND_MASK;
			CHAR_refill_stamina();
			coyoteTimer = F_JumpGraceTime;
			
			if (CHAR_dashTimer < Fr_AllowDashBack)
				CHAR_refill_dash(0);
		}
		else // Not on ground
		{
			CHAR_phys->flags &= ~ONGROUND_MASK;
		}
		
		// Enter dream dash state
		if (CHAR_dashTimer && !GAME_freeze && (XY & COLLISION_DREAM) && !(XY & COLLISION_SOLID) && (!prevVelY || (CHAR_flags & 0x8) >> 3))
		{
			CHAR_change_state(ST_DREAMDASH);
		}
	}
	
	if (CHAR_state == ST_ZIPLINE) {
		PHYS_actors[grabbed_entity].x = CHAR_phys->x - ZIP_GRAB_OFFSET;
	}
	
	// change hair color
	
	set_hair_color();
	COLOR *palette = pal_obj_mem + 3;
	COLOR *lookUp = (COLOR*)&skinStaminaColor;
	
	staminaBlink += IS_TIRED;
	lookUp += (IS_TIRED && (staminaBlink & 0x8)) * 8;
	
	if (IN_BACKGROUND){
		memset(palette, 0, 0x10);
	}
	else {
		int i = 0;
		for (;i < 8; ++i)
			memcpy(palette + i, lookUp + i, 2);
	}

}
