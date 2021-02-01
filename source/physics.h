#ifndef __PHYSICS__
#define __PHYSICS__

#include "game_data.h"

#define ActorLimit 16
extern Actor PHYS_actors[ActorLimit];

extern unsigned char yShift;
extern unsigned short width, height;
extern unsigned char collisionData[4096];

#define ACTOR_ID_BUILD(index, type) (index | (type << ACTORID_TYPE_SHIFT))

#define ACTOR_PERSISTENT(n) 		(n & ACTOR_PERSIST_MASK)
#define ACTOR_ENABLED(n) 			(n & ACTOR_ACTIVE_MASK)
#define ACTOR_CAN_COLLIDE(n) 		(n & ACTOR_COLLIDABLE_MASK)
#define ACTOR_ENTITY_TYPE(n) 		((n & ACTORID_TYPE_MASK) >> ACTORID_TYPE_SHIFT)
#define ACTOR_ENTITY_ID(n) 			(n & ACTORID_INDEX_MASK)

#define ACTORID_INDEX_MASK 		0x0000007F
#define ACTORID_TYPE_MASK 		0x00000F80
#define ACTORID_TYPE_SHIFT 		7

#define ACTOR_FLIPX_MASK 			0x01
#define ACTOR_FLIPY_MASK 			0x02
#define ACTOR_ACTIVE_MASK 			0x04
#define ACTOR_PERSIST_MASK 			0x08
#define ACTOR_COLLIDABLE_MASK		0x10

// 1: solid.
// 2: dreamblock
// 3: platform
// 4-7: spike up, down, left, right
#define COLLISION_SOLID 		0x0001
#define COLLISION_DREAM 		0x0002
#define COLLISION_PLATFORM 		0x0004
#define COLLISION_HARM			0x0008
#define COLLISION_STRAWB		0x0010
#define COLLISION_WATER			0x0020
#define COLLISION_WALLBLOCK		0x0040

#define NOTE_BLOCK_BEAT			0x40
#define NOTE_BLOCK_DIR			0x3F

#define COLLISION_CLIMB_BLOCK	COLLISION_HARM
#define COLLISION_WALL			(COLLISION_SOLID | COLLISION_DREAM)
#define COLLISION_FLOOR			(unsigned short)(COLLISION_SOLID | COLLISION_DREAM | COLLISION_PLATFORM)
#define COLLISION_COLL			(unsigned short)(COLLISION_SOLID | COLLISION_DREAM | COLLISION_PLATFORM)

#define X_COLL_SHIFT 			16

#define ENT_STRAWB			1
#define ENT_DASH			2
#define ENT_SPRING			3
#define ENT_BUMPER			4
#define ENT_ZIP				5
#define ENT_RAIN			6
#define ENT_KEY				7
#define ENT_CASSETTE		8
#define ENT_HEART			9
#define ENT_CUTSCENE		10
#define ENT_BACKGROUND		11
#define ENT_FLAG			12

//Basic actor physics
extern int roomOffsetX, roomOffsetY;
extern unsigned char yShift;
extern unsigned short width, height;

extern unsigned int collide_char(int *x, int *y, int *velX, int *velY, int width, int height);
extern unsigned int collide_rect(int x, int y, int width, int height);
extern unsigned int collide_entity(unsigned int ID);

int GetActorX(int value, int camera);
int GetActorY(int value, int camera);

#define SCREEN_LEFT (8)
#define SCREEN_TOP (8)
#define SCREEN_RIGHT ((BLOCK2INT(width) - 248))
#define SCREEN_BOTTOM ((BLOCK2INT(height) - 168))

#endif
