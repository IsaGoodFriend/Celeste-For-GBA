#include "game_data.h"
#include "sprites.h"

#include "load_data.h"

#include "physics.h"
#include "zip.h"
#include "strawb.h"
#include "dialogue.h"

#define RANDOM_FLIP(x, y) (((x) ^ (y >> 2)) & ((x >> 2) ^ (x ^ y)))
#define RANDOM_TILE(x, y) ((((x ^ y) << 3) ^ (y << 2)) | ((x << 2) ^ (y << 3)))
#define VIS_BLOCK_POS(x, y) (((x) & 0x1F) + (((y) & 0x1F) << 5))
#define BLOCK_FIXED_SIZE = 1 << (ACC + BLOCK_SHIFT)

// Pseudo code.

// Have an array of chars for collision information, probably best to have a static length.
// -- This is pure collision data.  Not visual data.
// Have values for roomOffset(X,Y), room width/height,
// Have an array of unsigned shorts

#define YSHIFT_1W 5
#define YSHIFT_2W 6
#define YSHIFT_4W 7

#define X_TILE_BUFFER 1
#define Y_TILE_BUFFER 1
#define BLOCK_X 30
#define BLOCK_Y 20

const unsigned short *fgVisuals = (unsigned short*)0x2001000, *bgVisuals = (unsigned short*)0x2003000;
unsigned char *lvlInfo;

void LoadCollisionData(unsigned char *levelInfo){
	
	lvlInfo = levelInfo;
	
	levelFlags &= ~LEVELFLAG_RAINY;
	
	
	yShift = lvlInfo[0];
	width =  lvlInfo[1];
	height = lvlInfo[2];
	
	
	PHYS_actors[0].x = BLOCK2FIXED(lvlInfo[3]);
	PHYS_actors[0].y = BLOCK2FIXED(lvlInfo[4]) - 0xB00;
	
	int x, y, tempX, tempY;
	
	int index = 0;
	
	lvlInfo += 5;
	
	for (;index < 3; ++index)
	{
		x = BLOCK2FIXED(lvlInfo[0]);
		y = (BLOCK2FIXED(lvlInfo[1]) - 0xB00);
		tempX = FIXED_MULT((x - PHYS_actors[0].x), enterX);
		tempY = FIXED_MULT((y - PHYS_actors[0].y), enterY);
		
		if (tempX > 0 || tempY > 0)
		{
			PHYS_actors[0].x = x;
			PHYS_actors[0].y = y;
		}
		lvlInfo += 2;
	}
	
	for (index = 0; index < 4; ++index)
		level_exits[index] = lvlInfo[index];
	
	unsigned int count = (lvlInfo[4]);
	unsigned int value = (lvlInfo[5]);
	
	lvlInfo += 6;
	
	unsigned char* cpyColl = (unsigned char*)&collisionData;
	
	int indexX, indexY;
	
	// get the memset count value, and the block index value.
	// set a temporary count variable that prevents count from going off screen.
	// per row:
	// 		use memset to place blocks down based on value given, and the temp count variable
	//		subtract countT from count and add countT to indexX.
	//		if index hasn't reached the end, set the new count and countTemp values, the block id value,
	
	// decompress file data
	// --- COLLISION ---
	int r1 = (1 << yShift) - width;
	unsigned int countT;
	
	for (indexY = 0; indexY < height; ++indexY){// by row
		countT = (count > width) ? width : count; // get the mem set count value.  if larger than the width
		
		for (indexX = 0; indexX < width;)
		{
			memset(cpyColl, value, countT);
			count -= countT;
			indexX += countT;
			cpyColl += countT;
			
			if (indexX < width)
			{
				count = lvlInfo[0];
				
				countT = (count > (width - indexX)) ? (width - indexX) : count;	
				
				value = lvlInfo[1];
				lvlInfo += 2;
			}
		}
		cpyColl += r1;
	}
}
void LoadInVisualDataFG() {

	int r1 = (1 << yShift) - width, indexX, indexY;
	unsigned int count = lvlInfo[0], countT;
	unsigned int value = (lvlInfo[2] << 8) | lvlInfo[1];
	lvlInfo += 3;
	unsigned short* cpyColl = fgVisuals;
	
	for (indexY = 0; indexY < height; ++indexY){// by row
		countT = (count > width) ? width : count; // get the mem set count value.  if larger than the width
		
		for (indexX = 0; indexX < width;)
		{
			int a = countT;
			while (--a >= 0)
				cpyColl[a] = value;
			
			//memset(cpyColl, value, countT);
			count -= countT;
			indexX += countT;
			cpyColl += countT;
			
			if (indexX < width)
			{
				count = lvlInfo[0];
				
				countT = (count > (width - indexX)) ? (width - indexX) : count;	
				
				value = lvlInfo[2] << 8;
				value |= lvlInfo[1];
				
				lvlInfo += 3;
			}
		}
		cpyColl += r1;
	}
}
void LoadInVisualDataBG() {

	int r1 = (1 << yShift) - width, indexX, indexY;
	unsigned int count = lvlInfo[0], countT;
	unsigned int value = (lvlInfo[2] << 8) | lvlInfo[1];
	lvlInfo += 3;
	unsigned short* cpyColl = bgVisuals;
	
	for (indexY = 0; indexY < height; ++indexY){// by row
		countT = (count > width) ? width : count; // get the mem set count value.  if larger than the width
		
		for (indexX = 0; indexX < width;)
		{
			int a = countT;
			while (--a >= 0)
				cpyColl[a] = value;
			
			//memset(cpyColl, value, countT);
			count -= countT;
			indexX += countT;
			cpyColl += countT;
			
			if (indexX < width)
			{
				count = lvlInfo[0];
				
				countT = (count > (width - indexX)) ? (width - indexX) : count;	
				
				value = lvlInfo[2] << 8;
				value |= lvlInfo[1];
				
				lvlInfo += 3;
			}
		}
		cpyColl += r1;
	}
}
void LoadInEntityData() {
	
	--lvlInfo;
	if (!CHALLENGE_MODE){
		memcpy(pal_obj_mem + (STRAWB_N_PAL << 4), pickup_pal, pickup_palLen);
		memcpy(pal_obj_mem + (STRAWB_O_PAL << 4), pickupOld_pal, pickup_palLen);
		memcpy(pal_obj_mem + (CHEST_PAL << 4), chest_pal, pickup_palLen);
	}
	
	maxEntities = 1;
	// unload entities
	{
		// foreach non player entity
		//		if non persistent, set flag to empty
		//		run type specific code for unloading if needs be.
		int index = 1;
		for (; index < ActorLimit; ++index){
			
			if (!ACTOR_ENABLED(PHYS_actors[index].flags))
				continue;
			
			if (ACTOR_PERSISTENT(PHYS_actors[index].flags))
			{
				PHYS_actors[index].x = PHYS_actors[0].x;
				PHYS_actors[index].y = PHYS_actors[0].y;
				
				PHYS_actors[maxEntities] = PHYS_actors[index];
				
				++maxEntities;
			}
			else{
				PHYS_actors[index].flags = 0;
			}
			
		}
	}
	if (CHALLENGE_MODE)
		memcpy(pal_obj_mem + 16, pickup_gold_pal, pickup_palLen);
	
	levelFlags &= ~MUSICAL_LEVEL;
	
	// load entities
	unsigned char type;
	do {
		type = (unsigned char)(*(++lvlInfo));
		unsigned char
			x = (unsigned char)(*(++lvlInfo)),
			y = (unsigned char)(*(++lvlInfo));
		PHYS_actors[maxEntities].velX = 0;
		PHYS_actors[maxEntities].velY = 0;
		
		if (type)
		{
			PHYS_actors[maxEntities].x = BLOCK2FIXED(x);
			PHYS_actors[maxEntities].y = BLOCK2FIXED(y);
			PHYS_actors[maxEntities].ID = ACTOR_ID_BUILD(maxEntities, type);
			PHYS_actors[maxEntities].flags = ACTOR_ACTIVE_MASK | ACTOR_COLLIDABLE_MASK;
			
			
			switch (type)
			{
			case ENT_STRAWB:
				STRAWB_waitTime = -1;
				PHYS_actors[maxEntities].width = 16;
				PHYS_actors[maxEntities].height = 16;
				
				// Save initial position
				PHYS_actors[maxEntities].flags |= (x << 16) | (y << 24);
				
				// Getting new data
				x = (char)*(++lvlInfo);
				y = (char)*(++lvlInfo);
				
				PHYS_actors[maxEntities].ID |= x << 24;
				PHYS_actors[maxEntities].ID |= y << 16;
				
				if ((x == 2 && (DEATH_tempCount > 3)) || (x == 3 && !CHALLENGE_MODE) || STRAWB_TEMPCOLL(y) || (x < 2 && CHALLENGE_MODE))
				{
					PHYS_actors[maxEntities].flags &= ~ACTOR_ACTIVE_MASK;
					--maxEntities;
					break;
				}
				
				// If strawberry already collected, don't load
				if (x >= 2 || CHALLENGE_MODE){
					PHYS_actors[maxEntities].ID |= 0x10000000;
					memcpy(pal_obj_mem + 32, pickup_gold_pal, pickup_palLen);
				}
				else
				{
					if (STRAWB_SAVEISCOLL(y))
					{
						PHYS_actors[maxEntities].ID |= 0x10000000;
						memcpy(pal_obj_mem + (CHEST_PAL << 4), chest_wing_pal, pickup_palLen);
					}
				}
				break;
			case ENT_DASH:
			
				if (CHALLENGE_MODE && current_chapter == LEVELIDX_DREAM) {
					PHYS_actors[maxEntities].flags &= ~ACTOR_ACTIVE_MASK;
					--maxEntities;
					break;
				}
				
				PHYS_actors[maxEntities].width = 16;
				PHYS_actors[maxEntities].height = 16;
				break;
			case ENT_SPRING:
				PHYS_actors[maxEntities].width = 16;
				PHYS_actors[maxEntities].height = 8;
				break;
			case ENT_BUMPER:
				PHYS_actors[maxEntities].x -= 0x200;
				PHYS_actors[maxEntities].y -= 0x200;
				PHYS_actors[maxEntities].width = 20;
				PHYS_actors[maxEntities].height = 20;
				break;
			case ENT_ZIP:
				
				if (CHALLENGE_MODE) {
					lvlInfo += 2;
					
					PHYS_actors[maxEntities].flags &= ~ACTOR_ACTIVE_MASK;
					--maxEntities;
					break;
				}
				
				PHYS_actors[maxEntities].y += 0x400;
				
				PHYS_actors[maxEntities].width = 16;
				PHYS_actors[maxEntities].height = 10;
				PHYS_actors[maxEntities].flags |= x << ZIP_OG_XPOS_SHIFT;
				
				x = (char)*(++lvlInfo);
				y = (char)*(++lvlInfo);
				
				PHYS_actors[maxEntities].x += BLOCK2FIXED(y);
				
				PHYS_actors[maxEntities].flags |= (x << ZIP_LENGTH_SHIFT) | (y << ZIP_OFFSET_SHIFT);
				break;
			case ENT_RAIN:
				PHYS_actors[maxEntities].flags &= ~ACTOR_ACTIVE_MASK;
				--maxEntities;
				if (GAME_IL_timer || CHALLENGE_MODE || (levelFlags & LEVELFLAG_BSIDE)){
					levelFlags |= LEVELFLAG_RAINY;
				}
				break;
			case ENT_KEY:
				PHYS_actors[maxEntities].width = 16;
				PHYS_actors[maxEntities].height = 16;
				chestAnim = 0;
				
				PHYS_actors[maxEntities].flags &= ~ACTOR_ACTIVE_MASK;
				--maxEntities;
				
				x = 1;
				while (PHYS_actors[x].ID != 0)
				{
					if (ACTOR_ENTITY_TYPE(PHYS_actors[x].ID) == ENT_STRAWB && !(PHYS_actors[x].flags & 0x3000))
					{
						if (PHYS_actors[x].flags & ACTOR_ACTIVE_MASK){
							PHYS_actors[x].ID |= 0x0C000000;
							PHYS_actors[x].flags &= ~ACTOR_COLLIDABLE_MASK;
							
							
							++maxEntities;
							PHYS_actors[maxEntities].flags |= ACTOR_ACTIVE_MASK | (x << 16);
						}
						break;
					}
					++x;
				}
				break;
			case ENT_CASSETTE:
			case ENT_HEART:
			
				if (CHALLENGE_MODE)
				{
					PHYS_actors[maxEntities].flags &= ~ACTOR_ACTIVE_MASK;
					--maxEntities;
					break;
				}
				
				if (type == ENT_HEART) {
					if (STRAWB_TEMPCOLL(HEART_TEMP)){
						PHYS_actors[maxEntities].flags &= ~ACTOR_ACTIVE_MASK;
						--maxEntities;
						break;
					}
					
					if (HEART_COLL(((levelFlags & LEVELFLAG_BSIDE) > 0)))
						memcpy(pal_obj_mem + 32, heart_old_pal, cassette_palLen);
					else{
						if (levelFlags & LEVELFLAG_BSIDE)
							memcpy(pal_obj_mem + 32, heartb_pal, cassette_palLen);
						else
							memcpy(pal_obj_mem + 32, heart_pal, cassette_palLen);
					}
				}
				else {
					if (STRAWB_TEMPCOLL(CASSETTE_TEMP)){
						PHYS_actors[maxEntities].flags &= ~ACTOR_ACTIVE_MASK;
						--maxEntities;
						break;
					}
					if (CASSETTE_COLL)
						memcpy(pal_obj_mem + 32, cassette_old_pal, cassette_palLen);
					else
						memcpy(pal_obj_mem + 32, cassette_pal, cassette_palLen);
				}
				
				PHYS_actors[maxEntities].width = 16;
				PHYS_actors[maxEntities].height = 16;
				
				if ((levelFlags & LEVELFLAG_BSIDE) > 0 || type == ENT_CASSETTE) {
					GB_init_soundchip(current_chapter);
					levelFlags |= MUSICAL_LEVEL;
				}
				
				break;
			case ENT_CUTSCENE:
				if (CHALLENGE_MODE && lvlIndex < currentMax - 1){
					PHYS_actors[maxEntities].flags &= ~ACTOR_ACTIVE_MASK;
					--maxEntities;
				}
				
				break;
			case ENT_BACKGROUND:
				PHYS_actors[maxEntities].width = 20;
				PHYS_actors[maxEntities].height = 20;
				
				PHYS_actors[maxEntities].x -= 2;
				PHYS_actors[maxEntities].y -= 2;
				PHYS_actors[maxEntities].flags |= ((char)*(++lvlInfo) != 0) * 0x100;
				
				break;
			}
			++maxEntities;
		}
	} while (type);
}

void MoveCam(int xFrom, int yFrom, int xTo, int yTo){
	int moveX = INT2BLOCK(xTo) - INT2BLOCK(xFrom),
		moveY = INT2BLOCK(yTo) - INT2BLOCK(yFrom);
	
	if (!moveX && !moveY)
		return;
	
	int xMin = INT2BLOCK(SIGNED_MIN(xFrom, xTo)) - 1;
	int xMax = INT2BLOCK(SIGNED_MAX(xFrom, xTo)) + BLOCK_X + 1;
	int yMin = INT2BLOCK(SIGNED_MIN(yFrom, yTo)) - 1;
	int yMax = INT2BLOCK(SIGNED_MAX(yFrom, yTo)) + BLOCK_Y + 1;
	
	unsigned short *foreground = &se_mem[FOREGROUND_LAYER][0];
	unsigned short *background = &se_mem[MIDGROUND_LAYER][0];
	int position;
	
	// Get the start X and Y rows needed to edit, and the direction each is needed to move.
	// Get the end destinations for x and y.
	
	int dirX = INT_SIGN(moveX), dirY = INT_SIGN(moveY);
	int startX = (dirX < 0) ? xMin : xMax, startY = (dirY < 0) ? yMin : yMax;
	int endX = startX + moveX, endY = startY + moveY;
	int min, max;
		
	if (startX != endX)
		startX -= dirX;
	if (startY != endY)
		startY -= dirY;
	
	if (dirX == 0)
		dirX = 1;
	if (dirY == 0)
		dirY = 1;
	
	if (ACTOR_ENTITY_TYPE(PHYS_actors[1].ID) == 10) {
		do {
			
			if (startX != endX) {
				
				
				min = startY - (BLOCK_Y + 2) * dirY;
				max = startY;
				
				for (; min != max; min += dirY){
					position = VIS_BLOCK_POS(startX, min);
					
					foreground[position] = fgVisuals[startX + (min << yShift)];
				}
				startX += dirX;
			}
			
			if (startY != endY) {
				
				min = startX - (BLOCK_X + 2) * dirX;
				max = startX;
				
				for (; min != max; min += dirX){
					position = VIS_BLOCK_POS(min, startY);
					
					foreground[position] = fgVisuals[min + (startY << yShift)];
				}
				startY += dirY;
			}
		}
		while (startX != endX || startY != endY);
	}
	else {
	
		do {
			
			if (startX != endX) {
				min = startY - (BLOCK_Y + 2) * dirY;
				max = startY;
				
				for (; min != max; min += dirY){
					position = VIS_BLOCK_POS(startX, min);
					
					foreground[position] = fgVisuals[startX + (min << yShift)];
					background[position] = bgVisuals[startX + (min << yShift)];
				}
				startX += dirX;
			}
			
			if (startY != endY) {
				
				min = startX - (BLOCK_X + 2) * dirX;
				max = startX;
				
				for (; min != max; min += dirX){
					position = VIS_BLOCK_POS(min, startY);
					
					foreground[position] = fgVisuals[min + (startY << yShift)];
					background[position] = bgVisuals[min + (startY << yShift)];
				}
				startY += dirY;
			}
		}
		while (startX != endX || startY != endY);
	}
}
void FillCam(int x, int y){
	x = INT2BLOCK(x);
	y = INT2BLOCK(y);
	
	y -= Y_TILE_BUFFER;
	x -= X_TILE_BUFFER;
	x &= ~0x1;
	//int yMin = y;
	//int xMin = x - X_TILE_BUFFER;
	
	unsigned int position;
	unsigned int pos;
	
	unsigned short *foreground = se_mem[FOREGROUND_LAYER];
	unsigned short *background = se_mem[MIDGROUND_LAYER];
	
	if (ACTOR_ENTITY_TYPE(PHYS_actors[1].ID) == 10){
		memset(background, 0, 0x800);
		
		int val = 32;
		while (val > 0){
			--val;
			
			int p1 = VIS_BLOCK_POS(x, y);
			int p2 = (y << yShift) + x;
			
			memcpy(&foreground[p1], &fgVisuals[p2], 64 - (x << 1));
			
			p1 &= 0xFE0;
			p2 += 32 - x;
			
			memcpy(&foreground[p1], &fgVisuals[p2], (x << 1));
			++y;
			
		}
	}
	else {
		
		int val = 32;
		while (val > 0){
			--val;
			
			int p1 = VIS_BLOCK_POS(x, y);
			int p2 = (y << yShift) + x;
			++y;
			
			memcpy(&foreground[p1], &fgVisuals[p2], 64 - (x << 1));
			memcpy(&background[p1], &bgVisuals[p2], 64 - (x << 1));
			
			p1 &= 0xFE0;
			p2 += 32 - x;
			
			memcpy(&foreground[p1], &fgVisuals[p2], (x << 1));
			memcpy(&background[p1], &bgVisuals[p2], (x << 1));
			
		}
	}
}	



