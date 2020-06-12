
#include "char.h"
#include "strawb.h"
#include "dashcrystal.h"
#include "particle.h"

#define HAIR_DIST 0x280

#define WALL_HITBOX_W 3
#define WALL_HITBOX_H 10
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
#define DOUBLE_JUMP				0x4000

#define ON_GROUND (CHAR_phys->flags & ONGROUND_MASK)
#define CAN_JUMP ((coyoteTimer || (CHAR_flags & DOUBLE_JUMP)) && (!CHALLENGE_MODE || current_chapter != LEVELIDX_CORE))

#define DASH_UP_FLAGS 0x18
#define DASH_DOWN_FLAGS 0x10
#define IS_DASH_DIR(f) (!((CHAR_flags & DASH_DIR_MASK) ^ f))
#define IS_DUCKING (CHAR_flags & DUCKING_MASK)
#define IS_TIRED (stamina <= F_ClimbTiredThreshold)

#define CAN_REFILL_DASH 			(CHAR_flags & DASH_REFILL_MASK)

#define FACING_LEFT(n) (n & ACTOR_FLIPX_MASK ? 1 : -1)
#define SET_FACING(n, v) (n = (n & ~ACTOR_FLIPX_MASK) | (v < 0 ? 1 : 0))

#define ANIM_IDLE 		0
#define ANIM_WALK 		11
#define ANIM_JUMP 		8
#define ANIM_AIRMID		9
#define ANIM_FALL 		10
#define ANIM_CLIMB 		1
#define ANIM_DASH 		2
#define ANIM_DUCK 		12
#define ANIM_ZIPGRAB	13
#define ANIM_ZIPGRAB_B	14
#define ANIM_ZIPGRAB_F	15
#define ANIM_LOOK		16

#define ANIM_DDASH_U 	3
#define ANIM_DDASH_UL 	4
#define ANIM_DDASH_L 	5
#define ANIM_DDASH_DL 	6
#define ANIM_DDASH_D	7

unsigned short CHAR_flags = TWO_DASH_MASK | TOTAL_DASH2_MASK | DASH_REFILL_MASK;
unsigned char CHAR_state = 0, CHAR_dashTimer = 0, staminaBlink = 0;
unsigned int anim_state = 0, anim_timer = 0;

int saveX, saveY;

int forceMoveX = 0, varJumpSpeed = 0, lastClimbMove = 0, climbNoMoveTimer = 0, stamina = F_ClimbMaxStamina, wallSpeedRetained = 0, climbJumpSave = 0;

Actor *CHAR_phys;

int hairPoints[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	
COLOR hairColor[6] = {0x7A8D, 0x54E4, 0x08B3, 0x084B, 0x621E, 0x30D8};
const COLOR skinStaminaColor[16] = 
{	0x4F1D, 0x329B, 0x71AB, 0x38E7, 0x10D0, 0x1CA8, 0x2D48, 0x45CC,
	0x001D, 0x001B, 0x000B, 0x0007, 0x0010, 0x0008, 0x0008, 0x001C};

// timers
unsigned char varJumpTimer, zipJumpBuffer, jumpBufferTimer, dashBufferTimer, grabBufferTimer, forceMoveXTimer, coyoteTimer, speedRetentionTimer, deathAnimTimer, dreamDashStayTimer;
int grabbed_entity;

char speedVert, speedHorz;

int hairTimer;

void CHAR_LoadLevel(){
	CHAR_phys->flags |= ONGROUND_MASK;
	CHAR_state = 0;
	
	
	int i = 0;
	for (; i < 8; i += 2)
	{
		hairPoints[i] = CHAR_phys->x + 0x400;
		hairPoints[i + 1] = CHAR_phys->y + 0x400;
	}
	varJumpTimer = 0;
	varJumpSpeed = 0;
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
void CHAR_render(OBJ_ATTR* buffer, int camX, int camY, int *count){
	
	unsigned int anim = 0;
	char displayHair = 1;
	int velX = CHAR_phys->velX;
	int velY = CHAR_phys->velY;
	int* sprite;
	int offsetX = -camX - 4, offsetY = -camY - 5;
	int hairI = 0;
	int currX, currY;
	
	if (FIXED2INT(CHAR_phys->y) - camY < -0x40)
		return;
	if (FIXED2INT(CHAR_phys->x) - camX < -0x40)
		return;
	if (FIXED2INT(CHAR_phys->x) - camX > 264)
		return;
	
	// Death Animation
	if (deathAnimTimer)
	{
		if (IS_FADING)
			--deathAnimTimer;
		
		int x = CHAR_phys->x - 0x400;
		int y = CHAR_phys->y - 0x400;
		
		if (FIXED2INT(CHAR_phys->y) - camY > 152)
			y = INT2FIXED(152 + camY);
		
		int offset = 60-deathAnimTimer;
		if (offset > 30)
			offset = deathAnimTimer;
		
		if (offset > 20 && offset & 1)
			return;
		
		offsetX = -camX;
		offsetY = -camY;
		currX = 4;
		currY = 0;
		
		for (; hairI < 6; ++hairI)
		{
			// render hair part
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
			
			// render hair part
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
	
	int prio = CHAR_PRIORITY;
	
	if (CHAR_state == ST_DREAMDASH) {
		prio = 0;
		
		if (CHAR_phys->velX){
			if (CHAR_phys->velY > 0)
				anim = ANIM_DDASH_DL;
			else if (CHAR_phys->velY < 0)
				anim = ANIM_DDASH_UL;
			else
				anim = ANIM_DDASH_L;
		}
		else if (CHAR_phys->velY > 0)
			anim = ANIM_DDASH_D;
		else
			anim = ANIM_DDASH_U;
		
		displayHair = 0;
	}
	else if (CHAR_state == ST_CLIMB || (CHAR_phys->flags & WALL_SLIDING)) {
		anim = ANIM_CLIMB;
	}
	else if (CHAR_state == ST_ZIPLINE){
		anim = ANIM_ZIPGRAB;
		if (key_tri_horz() && !IS_TIRED)
		{
			if (FACING_LEFT(CHAR_phys->flags) == key_tri_horz())
				anim = ANIM_ZIPGRAB_B;
			else
				anim = ANIM_ZIPGRAB_F;
		}
		offsetY += ZIP_GRAB_DISP;
	}
	else if (IS_DUCKING && ((CHAR_flags & DASH_VERT_MASK) != 0x08 || CHAR_state != ST_DASH || coyoteTimer)){
		anim = ANIM_DUCK;
	}
	else if (CHAR_state == ST_NORMAL || CHAR_state == ST_SWIM) {
		
		if (IS_RAINY && anim_state == ANIM_DUCK && ON_GROUND){
			int i;
			int velY;
			for (i = 0; i < 4; ++i) {
				int rng = RNG();
				
				int offX = i << 5;
				int offY = (rng) & 0x3F;
				offX -= (CHAR_phys->flags & ACTOR_FLIPX_MASK) ? 0x4F : 0x2F;
				offY -= 0x0F;
				
				offX = ((CHAR_phys->x >> 4) + offX) & 0xFFFF;
				offY = (((CHAR_phys->y - 0xE00) >> 4) + offY) & 0xFFFF;
				
				velY = (rng) & 0x7;
				
				AddParticle(offX | (0x55000000 + (rng & 0x33000000)),
							offY | 0xC0000000 | ((0xE8 + velY) << 16),
							0x100 | 27 | ((rng & 0x3) << 10));
			}
		}
		if (ON_GROUND){
			
			anim = ANIM_IDLE;
			if (key_tri_horz())
			{
				if (key_tri_horz() == INT_SIGN(velX))
					anim = ANIM_WALK;
			}
			else{
				if (key_tri_vert() != 0)
					anim = ANIM_LOOK;
			}

		}
		else {
			if (velY > 0x80)
				anim = ANIM_FALL;
			else if (velY < -0x180)
				anim = ANIM_JUMP;
			else
				anim = ANIM_AIRMID;
		}
	}
	else if (CHAR_state == ST_DASH) {
		anim = ANIM_DASH;
	}
	
	
	if (anim_state != anim) {
		switch (anim)
		{
		default:
			memcpy(&tile_mem[4][0], madeline, madelineLen >> 1);
			anim_timer = 0;
			break;
		case ANIM_CLIMB:
			memcpy(&tile_mem[4][0], climb, madelineLen >> 1);
			anim_timer = 0;
			break;
		case ANIM_WALK:
			memcpy(&tile_mem[4][0], walk, madelineLen >> 1);
			anim_timer = 0;
			break;
		case ANIM_DASH:
			memcpy(&tile_mem[4][0], dash, madelineLen >> 1);
			break;
		case ANIM_DUCK:
			memcpy(&tile_mem[4][0], duck, madelineLen >> 1);
			break;
		case ANIM_ZIPGRAB:
			memcpy(&tile_mem[4][0], grabzip, madelineLen >> 1);
			break;
		case ANIM_ZIPGRAB_F:
			memcpy(&tile_mem[4][0], grabzip_f, madelineLen >> 1);
			break;
		case ANIM_ZIPGRAB_B:
			memcpy(&tile_mem[4][0], grabzip_b, madelineLen >> 1);
			break;
		case ANIM_JUMP:
			memcpy(&tile_mem[4][0], jump, madelineLen >> 1);
			break;
		case ANIM_AIRMID:
			memcpy(&tile_mem[4][0], airfloat, madelineLen >> 1);
			break;
		case ANIM_FALL:
			memcpy(&tile_mem[4][0], fall, madelineLen >> 1);
			break;
		case ANIM_LOOK:
			memcpy(&tile_mem[4][0], lookUp, madelineLen >> 1);
			break;
		case ANIM_DDASH_U:
			memcpy(&tile_mem[4][0], ddash_u, madelineLen >> 1);
			break;
		case ANIM_DDASH_D:
			memcpy(&tile_mem[4][0], ddash_d, madelineLen >> 1);
			break;
		case ANIM_DDASH_UL:
			memcpy(&tile_mem[4][0], ddash_lu, madelineLen >> 1);
			break;
		case ANIM_DDASH_DL:
			memcpy(&tile_mem[4][0], ddash_ld, madelineLen >> 1);
			break;
		case ANIM_DDASH_L:
			memcpy(&tile_mem[4][0], ddash_l, madelineLen >> 1);
			break;
		}
	}
	anim_state = anim;
	
	if (anim_state == ANIM_CLIMB)
	{
		if (velY < 0) {
			++anim_timer;
			if (anim_timer > 0x1F)
			{
				anim_timer = 0;
			}
			if (!(anim_timer & 0x7))
			{
				memcpy(&tile_mem[4][0], climb + ((anim_timer & 0x18) << 2), madelineLen >> 1);
			}
		}
		else{
			if (anim_timer)
			{
				anim_timer = 0;
				memcpy(&tile_mem[4][0], climb, madelineLen >> 1);
			}
		}
	}
	else if (anim_state == ANIM_WALK)
	{
		++anim_timer;
		if (anim_timer >= 0x30)
		{
			anim_timer = 0;
		}
		if (!(anim_timer & 0x7))
		{
			memcpy(&tile_mem[4][0], walk + ((anim_timer & 0x38) << 2), madelineLen >> 1);
		}
	}
	else if (anim_state == ANIM_IDLE)
	{
		++anim_timer;
		if (anim_timer >= 0x40)
		{
			anim_timer = 0;
		}
		if (!(anim_timer & 0xF))
		{
			memcpy(&tile_mem[4][0], idle + ((anim_timer & 0x38) << 1), madelineLen >> 1);
		}
	}
	
	int prevX = (CHAR_phys->x & ~0xFF) + 0x400, prevY = (CHAR_phys->y & ~0xFF) + 0x400;
	
	if (1){
		obj_set_attr(buffer,
		ATTR0_SQUARE | ATTR0_Y(FIXED2INT(CHAR_phys->y) + offsetY ),														// ATTR0
		ATTR1_SIZE_16 | ATTR1_FLIP(CHAR_phys->flags & ACTOR_FLIPX_MASK) | ATTR1_X(FIXED2INT(CHAR_phys->x) + offsetX),	// ATTR1
		ATTR2_PALBANK(0) | ATTR2_PRIO(prio)); 																		// ATTR2
		
		++*count;
	}
	
	int hairY = 0;
	switch (anim_state)
	{
	case ANIM_CLIMB:
		offsetX += (CHAR_phys->flags & ACTOR_FLIPX_MASK ? 1 : -1) * (anim_timer > 0xF ? 2 : 1);
		break;
	case ANIM_WALK:
		offsetX += CHAR_phys->flags & ACTOR_FLIPX_MASK ? -1 : 1;
		
		hairY = ((anim_timer >> 3) % 3) - 1;
		break;
	case ANIM_IDLE:
		offsetY += (anim_timer >> 5);
		break;
	case ANIM_DUCK:
		offsetY += 0;
		hairY += 4;
		break;
	case ANIM_DASH:
		offsetX += CHAR_phys->flags & ACTOR_FLIPX_MASK ? -2 : 2;
		offsetY += 1;
		break;
	case ANIM_JUMP:
		offsetX += CHAR_phys->flags & ACTOR_FLIPX_MASK ? -1 : 1;
		offsetY -= 1;
		break;
	case ANIM_LOOK:
		offsetX += CHAR_phys->flags & ACTOR_FLIPX_MASK ? 2 : -2;
		offsetY += 1;
		break;
	}
	
	if (IS_RAINY)
	{
		obj_set_attr((buffer + *count),
		ATTR0_SQUARE | ATTR0_Y(FIXED2INT(CHAR_phys->y) + offsetY - 11 + hairY),											// ATTR0
		ATTR1_SIZE_16 | ATTR1_X(FIXED2INT(CHAR_phys->x) + offsetX - ((CHAR_phys->flags & ACTOR_FLIPX_MASK) ? 2 : -1)),	// ATTR1
		ATTR2_PALBANK(0) | (CHAR_umbrellaOffset) | ATTR2_PRIO(prio)); // ATTR2
		
		if (key_tri_vert() > 0)
			memcpy(&tile_mem[4][CHAR_umbrellaOffset], &umbrella[32], umbrellaLen >> 1);
		else
			memcpy(&tile_mem[4][CHAR_umbrellaOffset], &umbrella[0], umbrellaLen >> 1);
		
		
		++*count;
	}
	
	if (displayHair) {
		obj_set_attr((buffer + *count),
		ATTR0_WIDE | ATTR0_Y(FIXED2INT(CHAR_phys->y) + offsetY - 2 + hairY),											// ATTR0
		ATTR1_SIZE_16x8 | ATTR1_FLIP(CHAR_phys->flags & ACTOR_FLIPX_MASK) | ATTR1_X(FIXED2INT(CHAR_phys->x) + offsetX),	// ATTR1
		ATTR2_PALBANK(0) | 4 | ATTR2_PRIO(prio));																	// ATTR2
		++*count;
	
		offsetX += 4;
		hairTimer += 1;
		prevY += hairY * 0x100;
		
		for (; hairI < HAIR_PART_COUNT; ++hairI)
		{
			currX = hairPoints[(hairI << 1)];
			currY = hairPoints[(hairI << 1) + 1];
			
			if (!IS_FADING){
				if (CURRENT_DASH_STATUS > 1)
				{
					int add = hairTimer - hairI * 0x28;
					add = (add & 0xFF) - 0x7F;
					currY += FIXED_MULT(INT_ABS(add), 0x280) - 0x80;
					currX += FACING_LEFT(CHAR_phys->flags) * 0xC0 * (hairI + 1);
				}
				else{
					// gravity
					currY += 0xD0 - 0x0C * (hairI + 1) - INT_ABS(FIXED_MULT(CHAR_phys->velX, 0x38));
					// keep hair slightly behind player (prevents hair from sticking while falling
					currX += FACING_LEFT(CHAR_phys->flags) * 0x2 * (hairI + 1) + 0x6;
				}
				
				int diffX = currX - prevX, diffY = currY - prevY;
				int dist = FIXED_sqrt(FIXED_MULT(diffX, diffX) + FIXED_MULT(diffY, diffY));
				
				dist = FIXED_DIV(anim_state == ANIM_DUCK ? 0x80 : HAIR_DIST, dist);
				
				if (dist < 0x100){
					diffX = currX - prevX;
					diffY = currY - prevY;
					
					currX = FIXED_MULT(diffX, dist) + prevX;
					currY = FIXED_MULT(diffY, dist) + prevY;
				}
			}
			
			// render hair part
			obj_set_attr((buffer + *count),
			ATTR0_SQUARE | ATTR0_Y(FIXED2INT(currY) + offsetY - 8),								// ATTR0
			ATTR1_SIZE_16 | ATTR1_X(FIXED2INT(currX) + offsetX - 8),							// ATTR1
			ATTR2_PALBANK(0) | (((hairI) << 2) + HAIR_SPRITE_OFFSET) | ATTR2_PRIO(prio)); 	// ATTR2
			
			++*count;
			
			prevX = currX;
			prevY = currY;
			hairPoints[(hairI << 1)] = currX;
			hairPoints[(hairI << 1) + 1] = currY;
		}
		
		obj_set_attr((buffer + *count),
		ATTR0_WIDE | ATTR0_Y(FIXED2INT(CHAR_phys->y) + offsetY - 2 + hairY),												// ATTR0
		ATTR1_SIZE_16x8 | ATTR1_FLIP(CHAR_phys->flags & ACTOR_FLIPX_MASK) | ATTR1_X(FIXED2INT(CHAR_phys->x) + offsetX - 4),	// ATTR1
		ATTR2_PALBANK(0) | 6 | ATTR2_PRIO(prio)); 																		// ATTR2
		
		++*count;
		
		for (hairI = 0; hairI < HAIR_PART_COUNT; ++hairI)
		{
			currX = hairPoints[(hairI << 1)];
			currY = hairPoints[(hairI << 1) + 1];
			
			// render hair part
			obj_set_attr((buffer + *count),
			ATTR0_SQUARE | ATTR0_Y(FIXED2INT(currY) + offsetY - 8),												// ATTR0
			ATTR1_SIZE_16 | ATTR1_X(FIXED2INT(currX) + offsetX - 8),											// ATTR1
			ATTR2_PALBANK(0) | (((hairI + HAIR_PART_COUNT) << 2) + HAIR_SPRITE_OFFSET) | ATTR2_PRIO(prio)); // ATTR2
			
			++*count;
		}
	}
}
void CHAR_change_state(int value){
	if (value == ST_NORMAL && IsInWater())
		value = ST_SWIM;
	
	switch (CHAR_state)
	{
	case ST_NORMAL:
		CHAR_end_normal();
		break;
	}
	switch (value)
	{
	case ST_CLIMB:
		CHAR_climb_start();
		break;
	case ST_DASH:
		CHAR_dash_start();
		break;
	case ST_DREAMDASH:
		CHAR_dreamdash_start();
		break;
	case ST_ZIPLINE:
		CHAR_zip_start();
		break;
	}
	CHAR_state = value;
}
void CHAR_on_transition(){
	speedRetentionTimer = 0;
	CHAR_dashTimer = 0;
	varJumpTimer = 0;
	varJumpSpeed = 0;
	forceMoveXTimer = 0;
	speedVert = 0;
	speedHorz = 0;
	coyoteTimer = F_JumpGraceTime;
}
void set_hair_color(){
	
	// change hair color
	COLOR *palette = pal_obj_mem + 1;
	if (IS_FADING)
		palette = directColors + 17;
	
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
	if (force || CAN_REFILL_DASH)
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
unsigned short GetWall(int dir, int size){
	if (dir == 0)
		dir = FACING_LEFT(CHAR_phys->flags) == 1 ? -1 : 1;
		
	unsigned short mask = COLLISION_WALL;
	
	return collide_rect(CHAR_phys->x + (dir > 0 ? INT2FIXED(NORMAL_HITBOX_W) : -INT2FIXED(size)), CHAR_phys->y, size, WALL_HITBOX_H) & mask;
}
char IsInWater(){
	return current_chapter == LEVELIDX_WATER;//collide_rect(CHAR_phys->x, CHAR_phys->y, NORMAL_HITBOX_W, NORMAL_HITBOX_H) & COLLISION_WATER;
}
void CHAR_Restart() {
	CHAR_phys->x = saveX;
	CHAR_phys->y = saveY;
	
	if ((GAME_music_beat & (NOTE_BLOCK_BEAT))) {
		memcpy(&pal_bg_mem[13 << 4], note_a1_pal, 32);
		memcpy(&pal_bg_mem[14 << 4], note_b2_pal, 32);
		GAME_music_beat += (NOTE_BLOCK_BEAT);
	}
	
	
	CHAR_LoadLevel();
	ResetLevel();
	
	SetCamPos();
	
	FillCam(camX, camY);
	prevCamX = camX;
	prevCamY = camY;
	
	coyoteTimer = F_JumpGraceTime;
	
	CHAR_refill_dash(1);
	CHAR_refill_stamina();
}
void CHAR_Die(){
	
	++DEATH_count;
	++DEATH_tempCount;
	
	CHAR_phys->velX = 0;
	CHAR_phys->velY = 0;
	
	if (CHAR_state == ST_ZIPLINE) {
		PHYS_actors[grabbed_entity].flags |= ACTOR_COLLIDABLE_MASK;
	}
	
	CHAR_state = ST_NORMAL;
	
	varJumpTimer = 0;
	varJumpSpeed = 0;
	speedRetentionTimer = 0;
	CHAR_dashTimer = 0;
	forceMoveXTimer = 0;
	speedHorz = 0;
	speedVert = 0;
	
	deathAnimTimer = 60;
	CHAR_phys->flags |= ISDEAD_MASK;
	
	
}
void Bounce(int actor) {
				
	int dirX = CHAR_phys->x - PHYS_actors[actor].x - INT2FIXED(6);
	int dirY = CHAR_phys->y - PHYS_actors[actor].y - 0x200;
	int dist = FIXED_sqrt(FIXED_MULT(dirX, dirX) + FIXED_MULT(dirY, dirY));
	
	dirX = FIXED_DIV(dirX, dist);
	dirY = FIXED_DIV(dirY, dist);
	
	dirY = FIXED_MULT(dirY, 0xD8);
	
	if (dirY < 0xA0 && dirY > -0x80)
	{
		forceMoveX = INT_SIGN(dirX);
		forceMoveXTimer = 3;
		dirY = -0x80;
		dirX = INT_SIGN(dirX) * 0xD0;
	}
	
	CHAR_state = ST_NORMAL;
	
	CHAR_phys->velX = dirX * (IsInWater() ? 3 : 6);
	CHAR_phys->velY = dirY * (IsInWater() ? 4 : 6);
}
void Jump(int *velX, int *velY, int dirX){
	
	jumpBufferTimer = 0;
	CHAR_dashTimer = 0;
	
	if (ON_GROUND)
		speedRetentionTimer = 0;
	
	if (dirX == 0 & *velX != 0)
		dirX = INT_SIGN(*velX);
	
	if (dirX != 0)
		*velX += F_JumpHBoost * dirX;
	*velY = F_JumpSpeed;
	
	varJumpTimer = F_VarJumpTime;
	varJumpSpeed = *velY;
}
void ClimbJump(int *velX, int *velY){
	if (!ON_GROUND || (CHAR_flags & GROUND_JUMP_MASK))
	{
		stamina -= F_ClimbJumpCost;
		if (!IS_TIRED)
			staminaBlink = 3;
	}
	if (speedRetentionTimer){
		speedRetentionTimer = F_WallSpeedRetentionTime;
	}
	climbJumpSave =  FACING_LEFT(CHAR_phys->flags) ? -4 : 4;
	
	Jump(velX, velY, *velX ? INT_SIGN(*velX) : 0);
}
void SuperJump(int *velX, int *velY, int dir){
	if (CHAR_flags & GROUND_JUMP_MASK)
		return;
	
	
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
	
	speedVert = 12;
	jumpBufferTimer = 0;
	
	varJumpTimer = F_SuperWallJumpVarTime;
	CHAR_dashTimer = 0;

	*velX = F_SuperWallJumpH * dir;
	*velY = F_SuperWallJumpSpeed;
	varJumpSpeed = *velY;
}
void WallJump(int *velX, int *velY, int dir, int moveX){
	
	
	varJumpTimer = F_VarJumpTime;
	CHAR_dashTimer = 0;
	jumpBufferTimer = 0;
	
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
void RemoveDoubleJump() {

	CHAR_flags &= ~DOUBLE_JUMP;
	int i;
	for (i = 0; i < 8; ++i) {
		int rng = RNG();
		
		int offX = i << 5;
		int offY = (rng) & 0x3F;
		offX -= 0x5F;
		offY -= 0x0F;
		
		offX = ((CHAR_phys->x >> 4) + offX) & 0xFFFF;
		offY = (((CHAR_phys->y + 0x400) >> 4) + offY) & 0xFFFF;
		
		offX |= (rng & 0x77000000) + 0x11000000;
		
		char shorter = rng & 0x1;
		
		AddParticle(offX,
					offY | (shorter ? 0xC0000000 : 0x80000000),
					0x300 | (24 - shorter) | ((rng & 0x3) << 10));
	}
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
		collide_char(&CHAR_phys->x, &CHAR_phys->y, &tempX, &tempY, NORMAL_HITBOX_W, NORMAL_HITBOX_H);
		
		hit = collide_rect(CHAR_phys->x, CHAR_phys->y, NORMAL_HITBOX_W, NORMAL_HITBOX_H);
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
		else// if (dreamDashCanEndTimer <= 0)
		{
			CHAR_refill_stamina();
			CHAR_refill_dash(0);
			
			GAME_freeze = 3;
			
			int horz = key_tri_horz();
			
			if (jumpBufferTimer && velX && !(CHAR_flags & GROUND_JUMP_MASK))
			{
				//dreamJump = true;
				Jump(&velX, &velY, horz);
			}
			else
			{
				unsigned short wallL = GetWall(-1, WALL_HITBOX_W);
				unsigned short wallR = GetWall(1, WALL_HITBOX_W);
				
				if (KEY_DOWN_NOW(KEY_SHOULDER) && velX && INT_SIGN(velX) == -horz)
				{
					SET_FACING(CHAR_phys->flags, horz);
					velX = horz * 0x4000;
					velY = 0;
					
					CHAR_change_state(ST_CLIMB);
					goto updateDreamdashReturn;
				}
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
	//wallSlideTimer = WallSlideTime;
}
void CHAR_update_climb(){
	int velX = CHAR_phys->velX;
	int velY = CHAR_phys->velY;
	int facingLeft = FACING_LEFT(CHAR_phys->flags);
	int horz = key_tri_horz(), vert = key_tri_vert();
	unsigned short wallL = GetWall(-1, WALL_HITBOX_W);
	unsigned short wallR = GetWall(1, WALL_HITBOX_W);
	unsigned short wall = facingLeft == 1 ? wallL : wallR;
	
	unsigned short water = IsInWater();
	
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
		stamina -= water ? F_ClimbUpCostSwim : F_ClimbUpCost;
	}
	else if (lastClimbMove == 0 && !water)
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
	
	// dash
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
	
	unsigned short wallL = GetWall(-1, WALL_HITBOX_W);
	unsigned short wallR = GetWall(1, WALL_HITBOX_W);
	
	unsigned short water = IsInWater();
	
	if (water && CHAR_state == ST_NORMAL){
		CHAR_flags |= DOUBLE_JUMP;
	}
	else if (!water){
		CHAR_flags &= ~DOUBLE_JUMP;
	}
	CHAR_state = water ? ST_SWIM : ST_NORMAL;
	
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
	
	// Get D pad input
	int horz = key_tri_horz();
	int vert = key_tri_vert();
	
	// Duck if on ground
	if (ON_GROUND){
		SET_DUCKING(vert > 0);
		if (CHAR_state == ST_SWIM)
			CHAR_flags |= DOUBLE_JUMP;
	}
	else if (CHAR_phys->velY > 0)
		SET_DUCKING(0);
	
	// if ducking, don't walk
	if (IS_DUCKING && ON_GROUND)
		horz = 0;
	
	// force walking
	if (forceMoveXTimer)
	{
		--forceMoveXTimer;
		horz = forceMoveX;
	}
	
	//Walking
	{
		int mult = (ON_GROUND) ? 0x100 : F_AirMult;
		if (water)
			mult = FIXED_MULT(mult, ((INT_SIGN(horz) == -INT_SIGN(velX)) ? 0x60 : 0xE0));
		
		int max = water ? FIXED_MULT(F_MaxRun, 0xE0) : F_MaxRun;
		
		if (INT_ABS(velX) > max && INT_SIGN(velX) == horz)
			velX = FIXED_APPROACH(velX, FIXED_MULT(max, horz), FIXED_MULT(F_RunReduce, mult));  //Reduce back from beyond the max speed
		else
			velX = FIXED_APPROACH(velX, max * horz, FIXED_MULT(F_RunAccel, mult));   //Approach the max speed
	}
	
	//Vertical
	{
		int maxFall = 0;
		//Calculate current max fall speed
		{
			int mf = F_MaxFall;
			int fmf = F_FastMaxFall;
			
			//Fast Fall
			if (vert == 1 && velY >= mf)
			{
				maxFall = fmf;
			}
			else
				maxFall = mf;
		}

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
			else if (water && !(CHAR_phys->flags & WALL_SLIDING) && vert < 1)
			{
				maxFall = FIXED_MULT(maxFall, 0xA0);
			}
		}
		int mult = (INT_ABS(velY) < F_HalfGravThreshold && KEY_DOWN_NOW(KEY_A)) ? 0x80 : 0x100;
		if (CHAR_state == ST_SWIM)
			mult = FIXED_MULT(mult, 0xA0);
			
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
				
				if (CHAR_dashTimer && velY >= 0 && velX != 0)
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
					
					if (KEY_DOWN_NOW(KEY_SHOULDER) && stamina > 0 && (facingLeft == 1 ? wallL : wallR) && !(IS_RAINY && INT_ABS(velX) < 0x80)){
						ClimbJump(&velX, &velY);
					}
					else if (CHAR_dashTimer <= 0 || !(IS_DASH_DIR(DASH_UP_FLAGS)))
						WallJump(&velX, &velY, jumpDir, horz);
					else{
						SuperWallJump(&velX, &velY, jumpDir);
					}
						
					CHAR_change_state(ST_NORMAL);
				}
			}
		}
	}
	
	normalUpdateReturn:
	
	CHAR_phys->velX = velX;
	CHAR_phys->velY = velY;
}
void CHAR_zip_start() {
	CHAR_phys->velY = 0;
	PHYS_actors[grabbed_entity].x = CHAR_phys->x - ZIP_GRAB_OFFSET;
	PHYS_actors[grabbed_entity].velX = 0;
	PHYS_actors[grabbed_entity].flags &= ~ACTOR_COLLIDABLE_MASK;
	
	ZIP_update(&PHYS_actors[grabbed_entity]);
}
void CHAR_zip_update(){
	
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
	int horz = key_tri_horz();
	
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

void CHAR_dash_update(){
	int velX = CHAR_phys->velX;
	int velY = CHAR_phys->velY;
	
	int horz = key_tri_horz();
	int vert = key_tri_vert();
	
	//initialize dash direction
	if (CHAR_dashTimer == Fr_DashTime - 1)
	{
		CHAR_flags &= ~0x1E;
		
		if (horz) {
			CHAR_flags |= ((horz & 0x2) ? 0x4 : 0) | 0x2;
			screenShakeX = 0x10;
		}
		if (vert) {
			CHAR_flags |= ((vert & 0x2) ? 0x10 : 0) | 0x8;
			screenShakeY = 0x10;
		}
		if (horz && vert) {
			screenShakeY = 0x0C;
			screenShakeY = 0x0C;
		}
		
		// if no dash direction, set to horizontal
		if (!(CHAR_flags & 0x1E)){
			CHAR_flags |= 0x2 | (CHAR_phys->flags & ACTOR_FLIPX_MASK)<<2;
		}
		
		//set speed
		set_dash_speed(&velX, &velY);
	}
	
	// End dash
	if (CHAR_dashTimer <= Fr_DashFrameEnd){
		CHAR_change_state(ST_NORMAL);
		
		// Slow down velocity after dash
		if (velY <= 0){
			velY = F_EndDashSpeed * ((CHAR_flags & 0x8) >> 3) * ((CHAR_flags & 0x10) ? -1 : 1);
			velX = F_EndDashSpeed * ((CHAR_flags & 0x2) >> 1) * ((CHAR_flags & 0x4) ? -1 : 1);
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
	
	if (!velY && (CHAR_flags & 0x8))
	{
		//velY = ((CHAR_flags & 0x8) >> 3) * ((CHAR_flags & 0x10) ? -1 : 1);
	}
	
	unsigned short wallL = GetWall(-1, WALL_HITBOX_W);
	unsigned short wallR = GetWall(1, WALL_HITBOX_W);
	
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
			
			if (wallL || wallR)
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

void check_entities(){
	unsigned int value = collide_entity(0);
	int type = ACTOR_ENTITY_TYPE(value);
	
	if (deathAnimTimer)
		return;
	
	
	if (value & 0xFFF)
	{
		
		switch (type)
		{
		case ENT_STRAWB:
			STRAWB_playergrab(ACTOR_ENTITY_ID(value));
			break;
		case ENT_DASH:
			if (IsInWater() && (!CHAR_dashTimer || CHALLENGE_MODE)) {
				Bounce(ACTOR_ENTITY_ID(value));
				CHAR_refill_stamina();
				if (CHAR_state == ST_DASH)
					CHAR_change_state(ST_NORMAL);
				break;
			}
			if (IS_TIRED || (IsInWater() && !(CHAR_flags & DOUBLE_JUMP)) || ((TOTAL_DASH_STATUS == 1 || TOTAL_DASH_STATUS == 2) && CURRENT_DASH_STATUS < TOTAL_DASH_STATUS)){
				
				DASHCR_playergrab(ACTOR_ENTITY_ID(value));
				CHAR_refill_dash(1);
				CHAR_refill_stamina();
				if (IsInWater())
					CHAR_flags |= DOUBLE_JUMP;
			}
			break;
		case ENT_SPRING:
			
			CHAR_phys->velY = SPRING_bounce;
			varJumpSpeed = SPRING_bounce;
			
			varJumpTimer = SPRING_hold_time;
			
			CHAR_state = ST_NORMAL;
			
			PHYS_actors[ACTOR_ENTITY_ID(value)].flags = PHYS_actors[ACTOR_ENTITY_ID(value)].flags & (~0xF000) | 0x9000;
			
			CHAR_refill_dash(0);
			CHAR_refill_stamina();
			break;
		case ENT_HEART:
			if (CHAR_state == ST_DASH) {
				nextGamestate = GS_LEVEL;
				GAME_fading = 1;
				
				GB_PauseMusic();
				HEART_COLLECT();
				
				if (!LEVEL_UNLOCKED(current_chapter + 1))
				{
					UNLOCK_LEVEL(current_chapter + 1);
				}
				
				break;
			}
		case ENT_BUMPER:
			if (current_chapter == LEVELIDX_CORE && IS_CORE_HOT && type == ENT_BUMPER) {
				CHAR_Die();
			}
			else{
				CHAR_refill_dash(0);
				CHAR_refill_stamina();
				
				BUMPER_playerhit(ACTOR_ENTITY_ID(value));
				
				Bounce(ACTOR_ENTITY_ID(value));
				
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
			}
			break;
		case ENT_KEY:
			KEY_playergrab(ACTOR_ENTITY_ID(value));
			break;
		case ENT_CASSETTE:
			saveFile[CASSETTE_COLL_IDX] |= (1 << current_chapter);
			levelFlags &= ~MUSICAL_LEVEL;
			GAME_music_beat = 0;
			
			PHYS_actors[ACTOR_ENTITY_ID(value)].flags &= ~ACTOR_ACTIVE_MASK;
			GB_PauseMusic();
			
			memcpy(&pal_bg_mem[13 << 4], note_a1_pal, 32);
			memcpy(&pal_bg_mem[14 << 4], note_b2_pal, 32);
			break;
		}
	}
}
void CHAR_update(){
	
	//Debug
	int nextLevel = 255;
	
	if (!deathAnimTimer){
		// Load next level
		if (CHAR_phys->y <= -0x000){
			nextLevel = level_exits[0];
			enterX = 0;
			enterY = 1;
		}
		if (FIXED2BLOCK(CHAR_phys->y - 0x800) >= height){
			nextLevel = level_exits[1];
			enterX = 0;
			enterY = -1;
			if (nextLevel == 255)
				CHAR_Die();
		}
		if (CHAR_phys->x <= -0x000){
			nextLevel = level_exits[2];
			enterX = 1;
			enterY = 0;
		}
		if (FIXED2BLOCK(CHAR_phys->x) >= width){
			nextLevel = level_exits[3];
			enterX = -1;
			enterY = 0;
		}
	}
	
	if (KEY_DOWN_NOW(KEY_DOWN) && key_hit(KEY_START) && (lvlIndex + 1) < currentMax){
		nextLevel = lvlIndex + 1;
		GB_PauseMusic();
	}
	
	if (nextLevel == 254)
	{
		nextGamestate = GS_LEVEL;
		GAME_fading = 1;
		if (!LEVEL_UNLOCKED(current_chapter + 1))
		{
			UNLOCK_LEVEL(current_chapter + 1);
		}
		
		return;
	}
	else if (nextLevel != 255)
	{
		lvlIndex = nextLevel;
		
		CHAR_change_state(ST_NORMAL);
		PHYS_actors[0].velX = 0;
		PHYS_actors[0].velY = 0;
		CHAR_refill_dash(1);
		CHAR_refill_stamina();
		
		SaveData();
		
		nextGamestate = 1;
		GAME_fading = 1;
		return;
	}
	
	if (key_hit(KEY_SELECT) && key_tri_vert() == 1)
		CHAR_dash_count(TOTAL_DASH_STATUS + 1);
	
	if (key_hit(KEY_A)) // If key is down, get buffer
		jumpBufferTimer = 5;
	else if (!KEY_DOWN_NOW(KEY_A))
		jumpBufferTimer = 0;
	else if (jumpBufferTimer > 0)
		--jumpBufferTimer;
		
	if (key_hit(KEY_B))
		dashBufferTimer = 5;
	else if (!KEY_DOWN_NOW(KEY_B))
		dashBufferTimer = 0;
	else if (dashBufferTimer > 0)
		--dashBufferTimer;
	
	if (key_hit(KEY_SHOULDER))
		grabBufferTimer = 5;
	else if (!KEY_DOWN_NOW(KEY_SHOULDER))
		grabBufferTimer = 0;
	else if (grabBufferTimer > 0)
		--grabBufferTimer;
	
	if (deathAnimTimer)
	{
		--deathAnimTimer;
		
		if (!deathAnimTimer)
		{
			CHAR_phys->flags &= ~ISDEAD_MASK;
		}
		if (deathAnimTimer == 30)
		{
			CHAR_Restart();
		}
		return;
	}
	
	// If game is frozen (from dashing or other things), don't do anything
	if (GAME_freeze)
		return;
	
	// Keep speed after running into wall, but only for a short period
	if (speedRetentionTimer > 0)
	{
		if (CHAR_state != ST_ZIPLINE){
			if (CHAR_phys->velX != 0 && INT_SIGN(CHAR_phys->velX) == -INT_SIGN(wallSpeedRetained))
				speedRetentionTimer = 0;
			else if (!GetWall(INT_SIGN(wallSpeedRetained), WALL_HITBOX_W))
			{
				CHAR_phys->velX = wallSpeedRetained;
				speedRetentionTimer = 0;
				if (INT_ABS(wallSpeedRetained) >= F_DashSpeed - 0x40) {
					speedHorz = 12;
				}
			}
			else
				--speedRetentionTimer;
		}
		else{
			if (speedRetentionTimer)
				--speedRetentionTimer;
		}
	}
	
	if (zipJumpBuffer)
		--zipJumpBuffer;
	if (!KEY_DOWN_NOW(KEY_SHOULDER))
		zipJumpBuffer = 0;
	
	// -- normal game state update --
	switch (CHAR_state)
	{
	case ST_SWIM:
	case ST_NORMAL:
		CHAR_update_normal();
		break;
	case ST_CLIMB:
		CHAR_update_climb();
		break;
	case ST_DASH:
		CHAR_dash_update();
		break;
	case ST_DREAMDASH:
		CHAR_update_dreamdash();
		break;
	case ST_ZIPLINE:
		CHAR_zip_update();
		break;
	}
	
	check_entities();
	
	// D pad input
	int horz = key_tri_horz(), vert = key_tri_vert();
	
	// Flip character based on horizontal input
	if (horz && CHAR_state != ST_CLIMB && CHAR_state != ST_DREAMDASH && CHAR_state != ST_ZIPLINE)
		SET_FACING(CHAR_phys->flags, horz);
	
	// Ticking timers
	if (CHAR_dashTimer){
		--CHAR_dashTimer;
		
		if (1){
			
			int rng = RNG();
		
			int offX = (rng		) & 0x7F;
			int offY = (rng >> 8) & 0x7F;
			offX -= 0x3F;
			offY -= 0x3F;
			
			offX = ((CHAR_phys->x >> 4) + offX) & 0xFFFF;
			offY = (((CHAR_phys->y + 0x200) >> 4) + offY) & 0xFFFF;
			
			offX |= (rng & 0x33000000) + 0x55000000;
			
			AddParticle(offX,
						offY | 0xC0000000,
						0x000 | ((rng & 0x3) << 10));
		}
	}
	if (climbJumpSave) {
		if (key_tri_horz() == -INT_SIGN(climbJumpSave)){
			stamina += F_ClimbJumpCost;
			climbJumpSave = 0;
		}
		else
			climbJumpSave -= INT_SIGN(climbJumpSave);
	}
	if (coyoteTimer > 0)
		--coyoteTimer;
	if (speedHorz){
		--speedHorz;
		if (!(speedHorz & 0x3)){
			int offX = ((CHAR_phys->x >> 4)) & 0xFFFF;
			int offY = ((CHAR_phys->y >> 4)) & 0xFFFF;
			
			AddParticle(offX | 0x88000000 | (CHAR_phys->velX > 0 ? 0x080000 : 0xF80000),
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
			
			AddParticle(offX | 0x88000000,
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
		
		if (IS_DUCKING)
			CHAR_phys->y += INT2FIXED(DUCK_DIFF);
		
		// velocity before collision
		int prevVelX = CHAR_phys->velX;
		int prevVelY = CHAR_phys->velY;
		
		int hit = collide_char(&CHAR_phys->x, &CHAR_phys->y, &CHAR_phys->velX, &CHAR_phys->velY, NORMAL_HITBOX_W, height);
		int XY = (hit & 0xFFFF) | (hit >> 16);
		
		if (((hit >> X_COLL_SHIFT) & 0xFFFF)){
			if (!speedRetentionTimer && prevVelX)
			{
				speedRetentionTimer = F_WallSpeedRetentionTime;
				wallSpeedRetained = prevVelX;
			}
		}
		
		if (IS_DUCKING)
			CHAR_phys->y -= INT2FIXED(DUCK_DIFF);
		
		// if collided vertically downwards on solid ground, or if ground is beneath player while moving downm, on ground
		if (XY & COLLISION_HARM)
		{
			CHAR_Die();
			return;
		}
		else if (prevVelY >= 0 && (collide_rect(CHAR_phys->x, CHAR_phys->y + INT2FIXED(NORMAL_HITBOX_H), NORMAL_HITBOX_W, 1) & COLLISION_FLOOR)){
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
	
	if (CHAR_state == ST_ZIPLINE)
	{
		PHYS_actors[grabbed_entity].x = CHAR_phys->x - ZIP_GRAB_OFFSET;
	}
	
	// change hair color
	
	set_hair_color();
	COLOR *palette = pal_obj_mem + 3;
	COLOR *lookUp = (COLOR*)&skinStaminaColor;
	
	if (IS_TIRED){
		++staminaBlink;
		lookUp += (IS_TIRED ? (staminaBlink & 0x8 ? 8 : 0) : 0);
	}
	
	int i = 0;
	for (;i < 8; ++i)
		memcpy(palette + i, lookUp + i, 2);
}
