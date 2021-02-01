
#include "physics.h"
#include <tonc.h>
#include "game_data.h"
#include "zip.h"
#include "strawb.h"
#include "dialogue.h"
#include "load_data.h"
#include "char.h"

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
#define BLOCK_X 30
#define BLOCK_Y 20

Actor PHYS_actors[ActorLimit];

unsigned char yShift;
unsigned short width, height;
unsigned char collisionData[4096];

//
unsigned int GetBlock(int x, int y){
	x -= x * (x < 0);
	y -= y * (y < 0);
	x += ((width  - 1) - x) * (x >= width);
	y += ((height - 1) - y) * (y >= height);
	
	return collisionData[x + (y << yShift)];
}
//Physics Collision for any entity
// Returns y collision data on 0x00007FFF, and x collision data on 0x7FFF0000.
// 0x80000000 is set if x velocity was positive, and 0x00008000 is set if y velocity was positive
unsigned int collide_char(int *x, int *y, int *velX, int *velY, int width, int height) {
	// Get the sign (-/+) of the velocity components
	int signX = (*velX >> 31) | 1, signY = (*velY >> 31) | 1;
	int yIsPos= -(~(*velY) >> 31); // If y is positive, equals 1, else 0;
	int yIsNeg = *velY >> 31; // If y is negative, equals -1, else 0;
	int xIsPos= -(~(*velX) >> 31); // If x is positive, equals 1, else 0;
	int xIsNeg = *velX >> 31; // If x is negative, equals -1, else 0;
	
	int borderX = signX * 0x300, borderY = signY * 0x200;
	if (signY > 0)
	{
		borderX = signX;
	}
	
	// Box collision indexes - Tile values;
	int index = 0, index2 = 0;
	
	int top = *y,
		bot = *y + INT2FIXED(height),
		lef = *x					 - xIsNeg * (*velX),
		rgt = *x + INT2FIXED(width)  + xIsPos * (*velX);
	
	//Get the start and end of the base collisionbox
	int yMin = *y - yIsNeg * (INT2FIXED(height) - 1),
		yMax = *y + yIsPos * (INT2FIXED(height) - 1),
		xMin = *x - xIsNeg * (INT2FIXED(width)  - 1),
		xMax = *x + xIsPos * (INT2FIXED(width)  - 1);
	
	// Block values that were hit - flag
	int hitValueX = 0, hitValueY = 0;
	
	int offsetX = 0xFFFFFF, offsetY = 0xFFFFFF;
	
	for (index = FIXED2BLOCK(xMin); index != FIXED2BLOCK(xMax + *velX) + signX; index += signX){
		for (index2 = FIXED2BLOCK(yMin + borderY); index2 != FIXED2BLOCK(yMax - borderY) + signY; index2 += signY){
			
			int hitValue = 0;
			
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
				if (!xIsNeg && (FIXED2BLOCK((xMax + *velX) - 0x5FF) >= index)) {
						if (GetBlock(index + 1, index2) == BLOCK_NOTE1 && ( GAME_music_beat & NOTE_BLOCK_BEAT))
							break;
						if (GetBlock(index + 1, index2) == BLOCK_NOTE2 && (~GAME_music_beat & NOTE_BLOCK_BEAT))
							break;
						hitValue = COLLISION_HARM;
					}
				break;
			case BLOCK_SPINNER:
				if (GetBlock(index, index2 + 1) == BLOCK_SOLID) {
					break;
				}
				else {
					// --SPINNN
					if (bot >= BLOCK2FIXED(index2) + 0x400 && top <= BLOCK2FIXED(index2) + 0x4FF &&
						rgt >= BLOCK2FIXED(index)  + 0x300 && lef <= BLOCK2FIXED(index)  + 0x5FF)
						hitValue = COLLISION_HARM;
				}
				break;
			}
			
			if (hitValue) {
				int tempOffset = (BLOCK2FIXED(index - xIsNeg) - INT2FIXED(width * xIsPos)) - *x; // Get offsets to align to grid
				
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
				
				tempOffset = (BLOCK2FIXED(index + xIsPos) - INT2FIXED(width * -xIsNeg)) - *x; 
				if (INT_ABS(tempOffset) < INT_ABS(offsetX)) { // If new movement is smaller, set collision data.
					offsetX = tempOffset; // Set offset
					hitValueX = hitValue;
				}
				else if (tempOffset == offsetX)
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
	*x += *velX * !(hitValueX & COLLISION_COLL);
	
	xMin = *x - xIsNeg * (INT2FIXED(width) - 1);
	xMax = *x + xIsPos * (INT2FIXED(width) - 1);
	
	// Have two pixels of leniency where it will still move the player properly.  Velocity is added in the for loops and not in 
	for (index = FIXED2BLOCK(yMin); index != FIXED2BLOCK(yMax + *velY) + signY; index += signY){
		for (index2 = FIXED2BLOCK(xMin + borderX); index2 != FIXED2BLOCK(xMax - borderX) + signX; index2 += signX){
			
			int hitValue = 0;
			
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
				if (!IN_BACKGROUND && (*velY >= 0) && yMax <= BLOCK2FIXED(index) + 0x300)
					hitValue = COLLISION_PLATFORM;
				break;
			coll_spikeup:
			case BLOCK_SPIKEUP:
				if ((*velY >= 0) && FIXED2BLOCK((yMax + *velY) - 0x7FF) >= index){
					if (GetBlock(index2, index + 1) == BLOCK_NOTE1 && ( GAME_music_beat & NOTE_BLOCK_BEAT))
						break;
					if (GetBlock(index2, index + 1) == BLOCK_NOTE2 && (~GAME_music_beat & NOTE_BLOCK_BEAT))
						break;
					hitValue = COLLISION_HARM;
				}
				break;
			case BLOCK_SPIKEDOWN:
				if ((*velY <= 0) && FIXED2BLOCK(SIGNED_MIN(yMin, yMax) + *velY + 0x4FF) <= index){
					hitValue = COLLISION_HARM;
				}
				break;
			case BLOCK_SPINNER:
				if (GetBlock(index2, index + 1) == BLOCK_SOLID) {
					goto coll_spikeup;
				}
				else {
					// --SPINNN
					if (bot >= BLOCK2FIXED(index)  + 0x400 && top <= BLOCK2FIXED(index)  + 0x4FF &&
						rgt >= BLOCK2FIXED(index2) + 0x300 && lef <= BLOCK2FIXED(index2) + 0x5FF)
						hitValue = COLLISION_HARM;
				}
				break;
			}
			
			if (hitValue) {
				int tempOffset = (BLOCK2FIXED(index - yIsNeg) - INT2FIXED(height * yIsPos)) - *y;
				
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
				
				tempOffset = (BLOCK2FIXED(index + yIsPos) - INT2FIXED(height * -yIsNeg)) - *y;
				if (INT_ABS(tempOffset) < INT_ABS(offsetY)) { // If new movement is smaller, set collision data.
					offsetY = tempOffset; // Set offset
					hitValueY = hitValue;
				}
				else if (tempOffset == offsetY)
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
	*y += *velY * !(hitValueY & COLLISION_COLL);
	
	return (hitValueX << 16) | hitValueY;
}
unsigned int collide_rect(int x, int y, int width, int height){
	int yMin = y,
	yMax = y + INT2FIXED(height) - 1;
	int xMin = x,
	xMax = x + INT2FIXED(width)  - 1;	
	
	// Block values that were hit - flag
	int blockValue;
	int hitValue = 0;
	int xCoor, yCoor;
	
	for (xCoor = FIXED2BLOCK(xMin); xCoor != FIXED2BLOCK(xMax) + 1; ++xCoor){
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
				if (IN_BACKGROUND)
					break;
				if (FIXED2BLOCK(yMax + 0x700) <= yCoor){
					hitValue |= COLLISION_PLATFORM;
				}
				break;
			case BLOCK_SPIKELEFT:
				if (PHYS_actors[0].velX > 0 || FIXED2BLOCK(xMin + 0x500) > xCoor)
					break;
				hitValue |= COLLISION_HARM | COLLISION_WALLBLOCK;
				break;
			case BLOCK_SPIKERIGHT:
				if (PHYS_actors[0].velX < 0 || FIXED2BLOCK(xMax - 0x500) < xCoor)
					break;
				hitValue |= COLLISION_HARM | COLLISION_WALLBLOCK;
				break;
			case BLOCK_SPINNER:
			case BLOCK_SPIKEUP:
			case BLOCK_SPIKEDOWN:
				hitValue |= COLLISION_HARM;
				break;
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
		
		if (id_RX < iter_LX || iter_RX < id_LX || id_RY < iter_LY || iter_RY < id_LY)
			continue;
		
		return iterP->ID;
	}
	return 0xFFFFFFFF;
}
int GetActorX(int value, int camera){
	value = FIXED2INT(value) - camera;
	
	value -= (value + 64) * (value < -64);
	value += (0x19F - value) * (value > 0x19F);
	
	
	return value;
}
int GetActorY(int value, int camera){
	value = FIXED2INT(value) - camera;
	
	value -= (value + 64) * (value < -64);
	value += (0x19F - value) * (value > 0x9F);
	
	return value;
}
