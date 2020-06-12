#include "physics.h"
#include "toolbox.h"
#include "game_data.h"
#include "zip.h"
#include "strawb.h"
#include "dialogue.h"

//#define BLOCK(x, y) (brinMap[(x & 0x1F) + ((y & 0x1F) << 5) + ((((y & 0x3FF) / 32)*(2) + ((x & 0x3FF) /32))<<10)])

#define RANDOM_FLIP(x, y) (((x) ^ (y >> 2)) & ((x >> 2) ^ (x ^ y)))
#define RANDOM_TILE(x, y) ((((x ^ y) << 3) ^ (y << 2)) | ((x << 2) ^ (y << 3)))
#define VIS_BLOCK_POS(x, y) (((x) & 0x1F) + (((y) & 0x1F) << 5))
#define BLOCK_FIXED_SIZE = 1 << (ACC + BLOCK_SHIFT)

#define BLOCK_SOLID			1
#define BLOCK_DREAM			2
#define BLOCK_PLATFORM		3
#define BLOCK_SPIKEUP		4
#define BLOCK_SPIKEDOWN		5
#define BLOCK_SPIKELEFT		6
#define BLOCK_SPIKERIGHT	7
#define BLOCK_SPINNER		8
#define BLOCK_NOTE1			9
#define BLOCK_NOTE2			10
#define BLOCK_STRAWB		11
#define BLOCK_WATER			12

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
#define BLOCK_X INT2BLOCK(240)
#define BLOCK_Y INT2BLOCK(160)

unsigned char yShift;
unsigned short width, height;
unsigned char collisionData[4096];
unsigned short roomVisuals[4096];
unsigned short midgroundVisuals[4096];

void memset16(unsigned short *p, unsigned short value, unsigned int count)
{
	while (count)
		p[--count] = value;
}
void memset32(unsigned int *p, unsigned int value, unsigned int count)
{
	while (count)
		p[--count] = value;
}

unsigned char *lvlInfo;
void LoadCollisionData(unsigned char *levelInfo){
	
	lvlInfo = levelInfo;
	
	levelFlags &= ~LEVELFLAG_RAINY;
	
	--lvlInfo;
	
	yShift = *(++lvlInfo);
	width = *(++lvlInfo);
	height = *(++lvlInfo);
	
	
	PHYS_actors[0].x = BLOCK2FIXED(*(++lvlInfo));
	PHYS_actors[0].y = BLOCK2FIXED(*(++lvlInfo)) - 0xB00;
	
	int x, y, tempX, tempY;
	
	int index = 0;
	
	for (;index < 3; ++index)
	{
		x = BLOCK2FIXED(*(++lvlInfo));
		y = (BLOCK2FIXED(*(++lvlInfo)) - 0xB00);
		tempX = FIXED_MULT((x - PHYS_actors[0].x), enterX);
		tempY = FIXED_MULT((y - PHYS_actors[0].y), enterY);
		
		if (tempX > 0 || tempY > 0)
		{
			PHYS_actors[0].x = x;
			PHYS_actors[0].y = y;
		}
	}
	
	for (index = 0; index < 4; ++index)
		level_exits[index] = *(++lvlInfo);
	
	unsigned int count = (unsigned char)(*(++lvlInfo));
	unsigned int value = (unsigned char)(*(++lvlInfo));
	
	unsigned short* cpyTo = (unsigned short*)&collisionData;
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
				count = (unsigned char)(*(++lvlInfo));
				value = (unsigned char)(*(++lvlInfo));
				
				countT = (count > (width - indexX)) ? (width - indexX) : count;
			}
		}
		cpyColl += r1;
	}
}void LoadInVisualDataBG() {

	int r1, indexX, indexY;
	unsigned int count = (unsigned char)(*(++lvlInfo));
	unsigned int value = (unsigned short)(*(++lvlInfo) + (*(++lvlInfo)<< 8));
	
	for (indexY = 0; indexY < height; ++indexY){// by row
		r1 = (indexY << yShift);
		for (indexX = 0; indexX < width; ++indexX)
		{
			if (!count)
			{
				count = (unsigned char)(*(++lvlInfo));
				value = (unsigned short)(*(++lvlInfo) + (*(++lvlInfo)<< 8));
				
			}
			
			midgroundVisuals[r1] = value;
			++r1;
			--count;
		}
	}
}

void LoadInVisualDataFG() {

	int r1, indexX, indexY;
	unsigned int count = (unsigned char)(*(++lvlInfo));
	unsigned int value = (unsigned short)(*(++lvlInfo) + (*(++lvlInfo)<< 8));
	
	
	// --- FOREGROUND VISUALS ---
	for (indexY = 0; indexY < height; ++indexY){// by row
		r1 = (indexY << yShift);
		for (indexX = 0; indexX < width; ++indexX)
		{
			if (!count)
			{
				count = (unsigned char)(*(++lvlInfo));
				value = (unsigned short)(*(++lvlInfo) + (*(++lvlInfo)<< 8));
			}
			
			roomVisuals[r1] = value;
			++r1;
			--count;
		}
	}
}
void LoadInEntityData() {
	
	if (!CHALLENGE_MODE)
		memcpy(directColors + 256 + 16, pickup_pal, pickup_palLen);
	
	int entity_count = 1;
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
				
				PHYS_actors[entity_count] = PHYS_actors[index];
				
				++entity_count;
			}
			else{
				PHYS_actors[index].flags = 0;
			}
			
		}
	}
	if (CHALLENGE_MODE)
		memcpy(directColors + 256 + 16, pickup_gold_pal, pickup_palLen);
	
	levelFlags &= ~MUSICAL_LEVEL;
	
	// load entities
	unsigned char type;
	do
	{
		type = (unsigned char)(*(++lvlInfo));
		unsigned char
			x = (unsigned char)(*(++lvlInfo)),
			y = (unsigned char)(*(++lvlInfo));
		PHYS_actors[entity_count].velX = 0;
		PHYS_actors[entity_count].velY = 0;
		
		if (type)
		{
			PHYS_actors[entity_count].x = BLOCK2FIXED(x);
			PHYS_actors[entity_count].y = BLOCK2FIXED(y);
			PHYS_actors[entity_count].ID = ACTOR_ID_BUILD(entity_count, type);
			PHYS_actors[entity_count].flags = ACTOR_ACTIVE_MASK | ACTOR_COLLIDABLE_MASK;
			
			// Null next entity
			PHYS_actors[entity_count + 1].ID = 0;
			
			switch (type)
			{
			case ENT_STRAWB:
				STRAWB_waitTime = -1;
				PHYS_actors[entity_count].width = 16;
				PHYS_actors[entity_count].height = 16;
				
				// Save initial position
				PHYS_actors[entity_count].flags |= (x << 16) | (y << 24);
				
				// Getting new data
				x = (char)*(++lvlInfo);
				y = (char)*(++lvlInfo);
				
				PHYS_actors[entity_count].ID |= x << 24;
				PHYS_actors[entity_count].ID |= y << 16;
				
				if ((x == 2 && DEATH_tempCount) || (x == 3 && !CHALLENGE_MODE) || STRAWB_TEMPCOLL(y) || (x < 2 && CHALLENGE_MODE))
				{
					PHYS_actors[entity_count].flags &= ~ACTOR_ACTIVE_MASK;
					--entity_count;
					break;
				}
				
				// If strawberry already collected, don't load
				if (x >= 2 || CHALLENGE_MODE){
					memcpy(directColors + 256 + 16, pickup_gold_pal, pickup_palLen);
				}
				else
				{
					if (STRAWB_SAVEISCOLL(y))
					{
						PHYS_actors[entity_count].ID |= 0x10000000;
						memcpy(directColors + 256 + 16, pickupOld_pal, pickup_palLen);
						memcpy(directColors + 256 + 32, chest_wing_pal, pickup_palLen);
					}
				}
				break;
			case ENT_DASH:
			
				if (CHALLENGE_MODE && current_chapter == LEVELIDX_DREAM) {
					PHYS_actors[entity_count].flags &= ~ACTOR_ACTIVE_MASK;
					--entity_count;
					break;
				}
				
				PHYS_actors[entity_count].width = 16;
				PHYS_actors[entity_count].height = 16;
				break;
			case ENT_SPRING:
				PHYS_actors[entity_count].width = 16;
				PHYS_actors[entity_count].height = 8;
				break;
			case ENT_BUMPER:
				PHYS_actors[entity_count].x -= 0x200;
				PHYS_actors[entity_count].y -= 0x200;
				PHYS_actors[entity_count].width = 20;
				PHYS_actors[entity_count].height = 20;
				break;
			case ENT_ZIP:
				
				if (CHALLENGE_MODE) {
					lvlInfo += 2;
					
					PHYS_actors[entity_count].flags &= ~ACTOR_ACTIVE_MASK;
					--entity_count;
					break;
				}
				
				PHYS_actors[entity_count].y += 0x400;
				
				PHYS_actors[entity_count].width = 16;
				PHYS_actors[entity_count].height = 10;
				PHYS_actors[entity_count].flags |= x << ZIP_OG_XPOS_SHIFT;
				
				x = (char)*(++lvlInfo);
				y = (char)*(++lvlInfo);
				
				PHYS_actors[entity_count].x += BLOCK2FIXED(y);
				
				PHYS_actors[entity_count].flags |= (x << ZIP_LENGTH_SHIFT) | (y << ZIP_OFFSET_SHIFT);
				break;
			case ENT_RAIN:
				PHYS_actors[entity_count].flags &= ~ACTOR_ACTIVE_MASK;
				--entity_count;
				if (GAME_IL_timer >= 40 || CHALLENGE_MODE){
					levelFlags |= LEVELFLAG_RAINY;
				}
				break;
			case ENT_KEY:
				PHYS_actors[entity_count].width = 16;
				PHYS_actors[entity_count].height = 16;
				chestAnim = 0;
				
				PHYS_actors[entity_count].flags &= ~ACTOR_ACTIVE_MASK;
				--entity_count;
				
				x = 1;
				while (PHYS_actors[x].ID != 0)
				{
					if (ACTOR_ENTITY_TYPE(PHYS_actors[x].ID) == ENT_STRAWB && !(PHYS_actors[x].flags & 0x3000))
					{
						if (PHYS_actors[x].flags & ACTOR_ACTIVE_MASK){
							PHYS_actors[x].ID |= 0x0C000000;
							PHYS_actors[x].flags &= ~ACTOR_COLLIDABLE_MASK;
							
							
							++entity_count;
							PHYS_actors[entity_count].flags |= ACTOR_ACTIVE_MASK | (x << 16);
						}
						break;
					}
					++x;
				}
				break;
			case ENT_CASSETTE:
			case ENT_HEART:
				if ((CASSETTE_COLL && type == ENT_CASSETTE) || (HEART_COLL && type == ENT_HEART)) {
					memcpy(directColors + 256 + 32, cassette_old_pal, cassette_palLen);
				}
				else
					memcpy(directColors + 256 + 32, cassette_pal, cassette_palLen);
				
				
				PHYS_actors[entity_count].x -= 0x200;
				PHYS_actors[entity_count].y -= 0x200;
				PHYS_actors[entity_count].width = 20;
				PHYS_actors[entity_count].height = 20;
				
				levelFlags |= MUSICAL_LEVEL;
				break;
			case ENT_CUTSCENE:
				if (CHALLENGE_MODE){					
					PHYS_actors[entity_count].flags &= ~ACTOR_ACTIVE_MASK;
					--entity_count;
				}
				
				break;
			}
			++entity_count;
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
	
	do {
		
		if (startX != endX) {
			
			
			min = startY - (BLOCK_Y + 2) * dirY;
			max = startY;
			
			for (; min != max; min += dirY){
				position = VIS_BLOCK_POS(startX, min);
				foreground[position] = roomVisuals		[startX + (min << yShift)];
				if (!dialogueEnabled)
					background[position] = midgroundVisuals	[startX + (min << yShift)];
			}
			startX += dirX;
		}
		
		if (startY != endY) {
			
			min = startX - (BLOCK_X + 2) * dirX;
			max = startX;
			
			for (; min != max; min += dirX){
				position = VIS_BLOCK_POS(min, startY);
				foreground[position] = roomVisuals		[min + (startY << yShift)];
				if (!dialogueEnabled)
					background[position] = midgroundVisuals	[min + (startY << yShift)];
			}
			startY += dirY;
		}
	}
	while (startX != endX || startY != endY);
}
void FillCam(int x, int y){
	x = INT2BLOCK(x);
	y = INT2BLOCK(y);
	
	int yMax = y + BLOCK_Y + Y_TILE_BUFFER;
	y -= Y_TILE_BUFFER;
	int xMin = x - X_TILE_BUFFER;
	int xMax = x + BLOCK_X + X_TILE_BUFFER;
	
	int position;
	unsigned short *foreground = &se_mem[FOREGROUND_LAYER][0];
	unsigned short *background = &se_mem[MIDGROUND_LAYER][0];
	int pos = 0;
	
	for (; y <= yMax; ++y)
	{
		pos = (y << yShift) + xMin;
		for (x = xMin; x <= xMax; ++x){
		
			position = VIS_BLOCK_POS(x + roomOffsetX, y + roomOffsetY);
			
			foreground[position] = roomVisuals[pos];
			if (!dialogueEnabled)
				background[position] = midgroundVisuals[pos];
			++pos;
		}
	}
}	



unsigned short GetBlock(int x, int y){
	if (x < 0) x = 0;
	if (x >= width) x = width - 1;
	if (y < 0) y = 0;
	if (y >= height) y = height - 1;
	return collisionData[x + (y << yShift)];
}
//Physics Collision for any entity
// Returns y collision data on 0x00007FFF, and x collision data on 0x7FFF0000.
// 0x80000000 is set if x velocity was positive, and 0x00008000 is set if y velocity was positive
unsigned int collide_char(int *x, int *y, int *velX, int *velY, int width, int height){
	// Get the sign (-/+) of the velocity components
	int signX = (*velX >> 31) | 1, signY = (*velY >> 31) | 1;
	int yIsPos= -(~(*velY) >> 31); // If y is positive, equals 1, else 0;
	int yIsNeg = *velY >> 31; // If y is negative, equals -1, else 0;
	int xIsPos= -(~(*velX) >> 31); // If x is positive, equals 1, else 0;
	int xIsNeg = *velX >> 31; // If x is negative, equals -1, else 0;
	
	int borderX = signX * 0x200, borderY = signY * 0x200;
	if (signY > 0)
	{
		borderX = signX;
	}
	
	// Box collision indexes - Tile values;
	int index = 0, index2 = 0;
	
	int top = *y					 - yIsNeg * (*velY),
		bot = *y + INT2FIXED(height) + yIsPos * (*velY),
		lef = *x					 - xIsNeg * (*velX),
		rgt = *x + INT2FIXED(width)  + xIsPos * (*velX);
	
	//Get the start and end of the base collisionbox
	int yMin = *y - yIsNeg * (INT2FIXED(height) - 1),
		yMax = *y + yIsPos * (INT2FIXED(height) - 1),
		xMin = *x - xIsNeg * (INT2FIXED(width)  - 1),
		xMax = *x + xIsPos * (INT2FIXED(width)  - 1);
	
	// Block values that were hit - flag
	short hitValueX = 0, hitValueY = 0, hitValue = 0;
	
	int offsetX = 0xFFFFFF, offsetY = 0xFFFFFF, tempOffset, tempOffset2;
	
	for (index = FIXED2BLOCK(xMin); index != FIXED2BLOCK(xMax + *velX) + signX; index += signX){
		for (index2 = FIXED2BLOCK(yMin + borderY); index2 != FIXED2BLOCK(yMax - borderY) + signY; index2 += signY){
			
			hitValue = 0;
			
			switch (GetBlock(index, index2)) {
			case BLOCK_SOLID:
				hitValue = COLLISION_SOLID;
				break;
			case BLOCK_NOTE1:
				if (~GAME_music_beat & NOTE_BLOCK_BEAT)
					hitValue = COLLISION_SOLID;
				break;
			case BLOCK_NOTE2:
				if (GAME_music_beat & NOTE_BLOCK_BEAT)
					hitValue = COLLISION_SOLID;
				break;
			case BLOCK_DREAM:
				hitValue = COLLISION_DREAM;
				break;
			case BLOCK_WATER:
				hitValue = COLLISION_WATER;
				break;
			case BLOCK_SPIKELEFT:
				if ((*velX <= 0) && (FIXED2BLOCK(SIGNED_MIN(xMin, xMax) + *velX + 0x500) <= index)) {
						if (GetBlock(index - 1, index2) == BLOCK_NOTE1 && ( GAME_music_beat & NOTE_BLOCK_BEAT))
							break;
						if (GetBlock(index - 1, index2) == BLOCK_NOTE2 && (~GAME_music_beat & NOTE_BLOCK_BEAT))
							break;
						hitValue = COLLISION_HARM;
					}
				break;
			case BLOCK_SPIKERIGHT:
				if (!xIsNeg && (FIXED2BLOCK((xMax + *velX) - 0x500) >= index)) {
						if (GetBlock(index + 1, index2) == BLOCK_NOTE1 && ( GAME_music_beat & NOTE_BLOCK_BEAT))
							break;
						if (GetBlock(index + 1, index2) == BLOCK_NOTE2 && (~GAME_music_beat & NOTE_BLOCK_BEAT))
							break;
						hitValue = COLLISION_HARM;
					}
				break;
			case BLOCK_SPINNER:
				if (bot >= BLOCK2FIXED(index2) + 0x500 && top <= BLOCK2FIXED(index2) + 0x600 &&
					rgt >= BLOCK2FIXED(index)  + 0x380 && lef <= BLOCK2FIXED(index)  + 0x480)
					hitValue = COLLISION_HARM;
				break;
			}
			
			if (hitValue) {
				tempOffset = (BLOCK2FIXED(index - xIsNeg) - INT2FIXED(width * xIsPos)) - *x; // Get offsets to align to grid
				tempOffset2 = (BLOCK2FIXED(index + xIsPos) - INT2FIXED(width * -xIsNeg)) - *x; 
				
				if (INT_ABS(tempOffset) < INT_ABS(offsetX)) { // If new movement is smaller, set collision data.
					offsetX = tempOffset; // Set offset
					hitValueX &= COLLISION_HARM;
					hitValueX |= hitValue;
				}
				else if (tempOffset == offsetX)
				{
					hitValueX |= hitValue;
					if ((hitValueX & COLLISION_HARM) && (hitValueX & COLLISION_SOLID))
						hitValueX &= ~COLLISION_HARM;
				}
				if (INT_ABS(tempOffset2) < INT_ABS(offsetX)) { // If new movement is smaller, set collision data.
					offsetX = tempOffset2; // Set offset
					hitValueX = hitValue;
				}
				else if (tempOffset2 == offsetX)
				{
					hitValueX |= hitValue;
					if ((hitValueX & COLLISION_HARM) && (hitValueX & COLLISION_SOLID))
						hitValueX &= ~COLLISION_HARM;
				}
			}
			if (hitValueX & COLLISION_SOLID)
				break;
		}
		if (hitValueX & COLLISION_COLL){
			*x += offsetX;
			offsetX += *velX;
			
			if (*velX != 0 && signX == INT_SIGN((BLOCK2FIXED(index) + 0x400) - (*x + (width >> 1))))
				*velX = 0;
			else
				hitValueX = 0;
			break;
		}
	}
	if (!(hitValueX & COLLISION_COLL))
		*x += *velX;
	
	xMin = *x - xIsNeg * (INT2FIXED(width) - 1);
	xMax = *x + xIsPos * (INT2FIXED(width) - 1);
	
	// Have two pixels of leniency where it will still move the player properly.  Velocity is added in the for loops and not in 
	for (index = FIXED2BLOCK(yMin); index != FIXED2BLOCK(yMax + *velY) + signY; index += signY){
		for (index2 = FIXED2BLOCK(xMin + borderX); index2 != FIXED2BLOCK(xMax - borderX) + signX; index2 += signX){
			
			hitValue = 0;
			switch (GetBlock(index2, index)) {
			case BLOCK_SOLID:
				hitValue = COLLISION_SOLID;
				break;
			case BLOCK_NOTE1:
				if (~GAME_music_beat & NOTE_BLOCK_BEAT)
					hitValue = COLLISION_SOLID;
				break;
			case BLOCK_NOTE2:
				if (GAME_music_beat & NOTE_BLOCK_BEAT)
					hitValue = COLLISION_SOLID;
				break;
			case BLOCK_DREAM:
				hitValue = COLLISION_DREAM;
				break;
			case BLOCK_WATER:
				hitValue = COLLISION_WATER;
				break;
			case BLOCK_PLATFORM:
				if ((*velY >= 0) && yMax <= BLOCK2FIXED(index) + 0x300)
					hitValue = COLLISION_PLATFORM;
				break;
			coll_spikeup:
			case BLOCK_SPIKEUP:
				if ((*velY >= 0) && FIXED2BLOCK((yMax + *velY) - 0x700) >= index){
					if (GetBlock(index2, index + 1) == BLOCK_NOTE1 && ( GAME_music_beat & NOTE_BLOCK_BEAT))
						break;
					if (GetBlock(index2, index + 1) == BLOCK_NOTE2 && (~GAME_music_beat & NOTE_BLOCK_BEAT))
						break;
					hitValue = COLLISION_HARM;
				}
				break;
			case BLOCK_SPIKEDOWN:
				if ((*velY <= 0) && FIXED2BLOCK(SIGNED_MIN(yMin, yMax) + *velY + 0x500) <= index){
					hitValue = COLLISION_HARM;
				}
				break;
			case BLOCK_SPINNER:
				if (GetBlock(index2, index + 1) == BLOCK_SOLID) {
					goto coll_spikeup;
				}
				else {
					if (bot >= BLOCK2FIXED(index)  + 0x500 && top <= BLOCK2FIXED(index)  + 0x600 &&
						rgt >= BLOCK2FIXED(index2) + 0x380 && lef <= BLOCK2FIXED(index2) + 0x480)
						hitValue = COLLISION_HARM;
				}
				break;
			}
			
			if (hitValue) {
				tempOffset = (BLOCK2FIXED(index - yIsNeg) - INT2FIXED(height * yIsPos)) - *y;
				tempOffset2 = (BLOCK2FIXED(index + yIsPos) - INT2FIXED(height * -yIsNeg)) - *y;
				
				if (INT_ABS(tempOffset) < INT_ABS(offsetY)) { // If new movement is smaller, set collision data.
					offsetY = tempOffset; // Set offset
					hitValueY &= COLLISION_HARM;
					hitValueY |= hitValue;
				}
				else if (tempOffset == offsetY)
				{
					hitValueY |= hitValue;
					if ((hitValueY & COLLISION_HARM) && (hitValueY & COLLISION_SOLID))
						hitValueY &= ~COLLISION_HARM;
				}
				if (INT_ABS(tempOffset2) < INT_ABS(offsetY)) { // If new movement is smaller, set collision data.
					offsetY = tempOffset2; // Set offset
					hitValueY = hitValue;
				}
				else if (tempOffset2 == offsetY)
				{
					hitValueY |= hitValue;
					if ((hitValueY & COLLISION_HARM) && (hitValueY & COLLISION_SOLID))
						hitValueY &= ~COLLISION_HARM;
				}
			}
			if (hitValueY & COLLISION_SOLID)
				break;
		}
		if (hitValueY & COLLISION_COLL){
			*y += offsetY;
			offsetY += *velY;
			
			if (*velY != 0 && signY == INT_SIGN((BLOCK2FIXED(index) + 0x400) - (*y + (height >> 1))))
				*velY = 0;
			else
				hitValueY = 0;
			break;
		}
	}
	if (!(hitValueY & COLLISION_COLL))
		*y += *velY;
	
	return (((hitValueX << 16) | hitValueY) & ~0x80008000) | ((hitValueX && (offsetX <= 0) ? 1 : 0)<<31) | ((hitValueY && (offsetY <= 0) ? 1 : 0)<<15);
}

unsigned short collide_rect(int x, int y, int width, int height){
	int yMin = y,
	yMax = y + INT2FIXED(height) - 1;
	int xMin = x,
	xMax = FIXED2BLOCK(x + INT2FIXED(width)- 1) + 1;	
	
	// Block values that were hit - flag
	int blockValue;
	short hitValue = 0;
	int xCoor, yCoor;
	
	for (xCoor = FIXED2BLOCK(xMin); xCoor != xMax; ++xCoor){
		for (yCoor = FIXED2BLOCK(yMin); yCoor != FIXED2BLOCK(yMax) + 1; ++yCoor){
			switch (GetBlock(xCoor, yCoor)) {
			case BLOCK_SOLID:
				hitValue |= COLLISION_SOLID;
				break;
			case BLOCK_NOTE1:
				if (~GAME_music_beat & NOTE_BLOCK_BEAT)
					hitValue |= COLLISION_SOLID;
				break;
			case BLOCK_NOTE2:
				if (GAME_music_beat & NOTE_BLOCK_BEAT)
					hitValue |= COLLISION_SOLID;
				break;
			case BLOCK_DREAM:
				hitValue |= COLLISION_DREAM;
				break;
			case BLOCK_WATER:
				hitValue |= COLLISION_WATER;
				break;
			case BLOCK_STRAWB:
				hitValue |= COLLISION_STRAWB;
				break;
			case BLOCK_PLATFORM:
				if (FIXED2BLOCK(yMax + 0x700) <= yCoor){
					hitValue |= COLLISION_PLATFORM;
				}
			case BLOCK_SPINNER:
			case BLOCK_SPIKEUP:
			case BLOCK_SPIKEDOWN:
			case BLOCK_SPIKELEFT:
			case BLOCK_SPIKERIGHT:
				hitValue |= COLLISION_HARM;
			}
		}
	}
	
	return hitValue;
}
unsigned int collide_entity(unsigned int ID){
	int i = 0;
	
	Actor *id = &PHYS_actors[ID], *iterP;
	
	int id_LX = FIXED2INT(id->x);
	int id_LY = FIXED2INT(id->y);
	int id_RX = id_LX + id->width - 1;
	int id_RY = id_LY + id->height - 1;
	int iter_LX, iter_LY, iter_RX, iter_RY;
	
	for (; i < ActorLimit; ++i)
	{
		if (i == ID)
			continue;
		
		iterP = &PHYS_actors[i];
		
		if (!ACTOR_ENABLED(iterP->flags) || !ACTOR_CAN_COLLIDE(iterP->flags))
			continue;
		
		iter_LX = FIXED2INT(iterP->x);
		iter_LY = FIXED2INT(iterP->y);
		iter_RX = iter_LX + iterP->width - 1;
		iter_RY = iter_LY + iterP->height - 1;
		
		if (id_RX < iter_LX || iter_RX < id_LX)
			continue;
		if (id_RY < iter_LY || iter_RY < id_LY)
			continue;
		
		return iterP->ID;
	}
	return 0xFFFFFFFF;
}
int GetActorX(int value, int camera){
	value = FIXED2INT(value) - camera;
	
	if (value < -64)
		return -64;
	
	if (value > 0x19F)
		return 0x19F;
	
	
	return value;
}
int GetActorY(int value, int camera){
	value = FIXED2INT(value) - camera;
	
	if (value < -64)
		return -64;
	
	if (value > 0x9F)
		return 0x9F;
	
	
	return value;
}
