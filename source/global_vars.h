#pragma once

extern int death_total, death_temp;

extern int strawbs_collected, strawbs_colltemp;

extern int level_flags;

extern Entity* CHAR_ent;

#define IS_CORE_HOT		   (level_flags & LEVELFLAG_COREMODE)
#define IS_RAINY		   (level_flags & LEVELFLAG_RAINY)
#define CHALLENGE_MODE	   (level_flags & LEVELFLAG_CHALLENGE)
#define NOTE_BLOCKS_ACTIVE (level_flags & MUSICAL_LEVEL)
#define START_OF_LEVEL	   !(level_flags & LEVELFLAG_BEGINNING)

#define LEVELFLAG_BSIDE		0x0001
#define LEVELFLAG_COREMODE	0x0002
#define LEVELFLAG_RAINY		0x0004
#define LEVELFLAG_CHALLENGE 0x0008
#define MUSICAL_LEVEL		0x0010
#define LEVELFLAG_BEGINNING 0x0020

extern int file_timer, level_timer;

#define SOLID_COLLISION			 0x0001 // type 1
#define DANGER_GENERIC_COLLISION 0x0002 // type 2
#define SPIKE_LEFT_COLLISION	 0x0004 // type 3
#define SPIKE_RIGHT_COLLISION	 0x0008 // type 4
#define STRAWB_COLLISION		 0x0010 // type 5

#define DANGER_COLLISION (DANGER_GENERIC_COLLISION | SPIKE_LEFT_COLLISION | SPIKE_RIGHT_COLLISION)