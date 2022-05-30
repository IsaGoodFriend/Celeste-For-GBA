#pragma once

#include "math.h"
#include <string.h>
#include <tonc.h>

#define GS_LEVEL_SELECT 0
#define GS_PLAYING		1
#define GS_PAUSED		2
#define GS_INTRO		3
#define GS_FILES		4
#define GAMESTATE_COUNT 5

#define screenWidth	 240
#define screenHeight 160

// Level index data
#define LEVELIDX_PROLOGUE 0
#define LEVELIDX_DREAM	  1
#define LEVELIDX_RAIN	  2
#define LEVELIDX_WATER	  3
#define LEVELIDX_CORE	  10
#define LEVELIDX_EPI	  10

#define LEVELIDX_MAX 3

// save mem offsets
#define STRAWB_SAVE_MASK 0x000F
#define STRAWB_SAVE_ADD	 1
#define SAVE_TIMER_ADD	 16
#define SAVE_DEATH_OFF	 24
#define STRAWB_COUNT_IDX 28
#define FILE_NAME_IDX	 36

#define GOLD_COLL_IDX	  48
#define WINGED_COLL_IDX	  49
#define CASSETTE_COLL_IDX 50
#define HEART_A_COLL_IDX  51
#define HEART_B_COLL_IDX  52

#define LEVEL_UNLOCKED_IDX 53

extern unsigned char STRAWB_tempColl[15];

#define FILENAME_LEN 12
extern char fileName[FILENAME_LEN];
extern const char emptyFileName[FILENAME_LEN];