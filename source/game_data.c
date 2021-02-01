#include "game_data.h"

#include "physics.h"
#include "char.h"
#include "level.h"
#include "dashcrystal.h"
#include "bumper.h"
#include "spring.h"
#include "zip.h"
#include "dialogue.h"
#include "transitions.h"
#include "sprites.h"
#include "backgrounds.h"
#include "level.h"
#include "physics.h"

#define UI_BOUNCE_START 	0x240
#define UI_ENTER_SPEED 		0x200
#define UI_GRAV 			0x61

const char** chapter_pointers[LEVELIDX_MAX]= {
	(const char**)(&levels_prologue[0]),
	(const char**)(&levels_dream[0]),
	(const char**)(&levels_forsaken[0]),
	(const char**)(&levels_water[0]),
};
const char** chapter_pointersBside[LEVELIDX_MAX]= {
	(const char**)(&levels_prologue[0]),
	(const char**)(&levels_dreamB[0]),
	(const char**)(&levels_forsakenB[0]),
	(const char**)(&levels_water[0]),
};
const char* chapter_names[LEVELIDX_MAX]= {
	(char*)&prologue_visName,
	(char*)&dream_visName,
	(char*)&forsaken_visName,
	(char*)&water_visName,
};
const int chapter_map_locations[LEVELIDX_MAX << 1]= {
	0x03800, 0x1E000,
	0x08000, 0x1C000,
	0x0E000, 0x1B000,
	0x14000, 0x1B000,
};

const char emptyFileName[FILENAME_LEN]= {
	0x0E,0x16,0x19,0x1D,0x22,0x24,0x24,0x24,0x24,0x24,0x24,0x24,
};

unsigned int wind, rain, strawbUI_Timer;
char *lvlName;
int HOFS, VOFS;


unsigned int GAME_freeze;
unsigned int GAME_music_beat, GAME_life;
unsigned int GAME_speedrun_timer, GAME_IL_timer;
void (*cutsceneCoroutine)(void);
void (*cutsceneSkipped)(void);
unsigned int cutsceneInput, cutsceneWait, heart_freeze;
unsigned int flagIndex;

unsigned short* transition_style;

unsigned int maxEntities;

unsigned char STRAWB_tempColl[15];

int level_exits[4];
int enterX, enterY;
unsigned int visualFlags;

#ifdef	__DEBUG__
int VCountLast, enabled;
#endif

char saveFile[SAVEFILE_LEN], fileName[FILENAME_LEN];
int saveFileNumber;

int current_chapter, lvlIndex, currentMax, levelFlags;
char gamestate, nextGamestate;

int GAME_fadeAmount, GAME_fading, GAME_loadIndex;

int camX, camY, prevCamX, prevCamY;
void (*hBlankInterrupt)(int);

int strawbUI_Pos, strawbUI_Offset1, strawbUI_Offset2, strawbUI_speed1, strawbUI_speed2;

int STRAWB_count, STRAWB_display, STRAWB_levelCount, STRAWB_levelMax;
int STRAWB_tempCount, forceDisplay;
unsigned int DEATH_count, DEATH_tempCount;

void Interrupt(){
	
	int line = REG_VCOUNT;
	
	if (GAME_fadeAmount != TRANSITION_CAP && (line == 228 || line <= 160)) {
		if (IS_FADING) {
			if (line >= 160) {
				REG_WIN0H = transition_style[(GAME_fadeAmount >> 1) * 160];
			}
			else{
				REG_WIN0H = transition_style[line + (GAME_fadeAmount >> 1) * 160];
			}
		}
		
		hBlankInterrupt(line);
	}
#ifdef __DEBUG__
	else
		++VCountLast;
#endif
	
}
void Cloud_Movement(int line){
	
	if (line == 113){
		REG_BG2HOFS = FIXED2INT(FIXED_MULT((INT2FIXED(HOFS) + (GAME_life << 8) + (GAME_life << 7)), 0x40));
	}
	else if (line == 57) {
		REG_BG2HOFS = FIXED2INT(FIXED_MULT((INT2FIXED(HOFS) + (GAME_life << 8) + (GAME_life << 7)), 0x30));
	}
}
void EmptyInterrupt(int line){
	
}
void Chapter1_BG(int line){
	
	int hOffset = INT2FIXED(HOFS);
	
	if (line == 72){
		REG_BG3HOFS = FIXED2INT(FIXED_MULT(hOffset, 0x20));
	}
}

void ChangeHBlankIntr(void (*function)(int)) {
	
	//irq_disable( II_VBLANK | II_HBLANK );
	
	hBlankInterrupt = function;
	
	//irq_enable( II_VBLANK | II_HBLANK );
}
void StrawbUI(int force){

	int count = (STRAWB_count * (gamestate != GS_PLAYING)) +
			(STRAWB_tempCount * (gamestate == GS_PLAYING));
	
	if (strawbUI_speed1 || strawbUI_Offset1)
	{
		strawbUI_Offset1 += strawbUI_speed1;
		strawbUI_speed1 -= UI_GRAV;
		strawbUI_Offset2 += strawbUI_speed2;
		
		if (strawbUI_speed2)
			strawbUI_speed2 -= UI_GRAV;
		
		if (strawbUI_Offset1 < 0)
		{
			strawbUI_Offset1 = 0;
			strawbUI_Offset2 = 0;
			strawbUI_speed1 = 0;
			strawbUI_speed2 = 0;
		}
	}
	else if (count > STRAWB_display || force)
	{
		strawbUI_Pos += UI_ENTER_SPEED;
		if (strawbUI_Pos >= 0x1000)
		{
			if (!force)
			{
				int digitOld1 = STRAWB_display % 10;
				int digitOld2 = STRAWB_display / 10;
				
				++STRAWB_display;
				
				int digitNew1 = STRAWB_display % 10;
				int digitNew2 = STRAWB_display / 10;
				
				if (digitOld1 != digitNew1)
				{
					strawbUI_speed1 = UI_BOUNCE_START;
				}
				if (digitOld2 != digitNew2)
				{
					strawbUI_speed2 = UI_BOUNCE_START;
				}
			}
			strawbUI_Timer = 90;
			strawbUI_Pos = 0x1000;
		}
	}
	else if (strawbUI_Timer)
	{ --strawbUI_Timer; }
	else {
		strawbUI_Pos -= 0x100;
		if (strawbUI_Pos < 0)
		{
			strawbUI_Pos = 0;
		}

	}
	
}

void VisualizeCamera() {
	
	// set foreground offset
	if (gamestate == GS_PLAYING) {
		REG_BG0HOFS= HOFS;
		REG_BG0VOFS= VOFS;
		
		// set midground offset to the same as foreground
		if (!DIALOGUE_ENABLED){
			REG_BG1HOFS= HOFS;
			REG_BG1VOFS= VOFS;
		}
	}
	
	wind = ((GAME_life) << 8);
	//wind = 0x30000;
	unsigned int hOffset = INT2FIXED(HOFS) + wind;
	unsigned int vOffset = INT2FIXED(VOFS);
	
	if (current_chapter == LEVELIDX_RAIN){
		wind = (GAME_life << 8) + (GAME_life << 7);
		hOffset = INT2FIXED(HOFS) + wind;
		
		++rain;
		REG_BG3HOFS = camX;
		REG_BG3VOFS = -(rain << 3);
		
		
		REG_BG2HOFS = (hOffset * 0x20) >> 16;
		REG_BG2VOFS = 0;
		
	}
	else{
		
		REG_BG2HOFS= (hOffset * 0x40) >> 16;
		REG_BG2VOFS= (vOffset * 0x40) >> 16;
		
		hOffset = INT2FIXED(HOFS) + FIXED_MULT(wind, 0xC0);
		vOffset = INT2FIXED(VOFS);
		
		REG_BG3HOFS = (hOffset * 0x30) >> 16;
		REG_BG3VOFS = 0x18;
		
	}
}
void SetCamPos(){
	
	if (CHAR_state != ST_VIEWER) {
		camX = FIXED2INT(PHYS_actors[0].x) - 120;
		camY = FIXED2INT(PHYS_actors[0].y) -  90;
	}
	
	// keep in bounds of the room
	if (camX < SCREEN_LEFT)
		camX = SCREEN_LEFT;
	else if (camX > SCREEN_RIGHT)
		camX = SCREEN_RIGHT;
	if (camY < SCREEN_TOP)
		camY = SCREEN_TOP;
	else if (camY > SCREEN_BOTTOM)
		camY = SCREEN_BOTTOM;
	
	// Get horizontal offsets using room offset
	HOFS = camX;
	VOFS = camY;
}
void SetMapCamPos(){
	
	camX >>= 8;
	camY >>= 8;
	
	// keep in bounds of the room
	if (camX < 0)
		camX = 0;
	else if (camX > (64 << 3) - 240)
		camX = (64 << 3) - 240;
	if (camY < 0)
		camY = 0;
	else if (camY > (64 << 3) - 160)
		camY = (64 << 3) - 160;
	
	// Get horizontal offsets using room offset
	HOFS = camX;
	VOFS = camY;
	
	// set foreground offset
	REG_BG2HOFS= HOFS;
	REG_BG2VOFS= VOFS;
	// set midground offset to the same as foreground
	REG_BG3HOFS= HOFS;
	REG_BG3VOFS= VOFS;
	
	camX <<= 8;
	camY <<= 8;
}

void InitializeChapter(){
	// Load in game data.
	
	memcpy(&tile_mem[FG_TILESET][16], note_block, note_blockLen);
	
	memcpy(&pal_bg_mem[13 << 4], note_a1_pal, 32);
	memcpy(&pal_bg_mem[14 << 4], note_b2_pal, 32);
	
	int index;
	
	flagIndex = 0;
	
	lvlName = (char*)chapter_names[current_chapter];
	
	switch (current_chapter){
	case LEVELIDX_RAIN: {
		
		STRAWB_levelMax = forsaken_strawbMax;
		
		ChangeHBlankIntr(Cloud_Movement);
		
		CHAR_PRIORITY = 1;
		
		// Set dash count to one and refills on ground
		CHAR_dash_count(1);
		CHAR_flags = CHAR_flags | (0x0800);
		
		if (levelFlags & LEVELFLAG_BSIDE)
			currentMax = MAX_levels_forsakenB;
		else
			currentMax = MAX_levels_forsaken;
		
		
		// Foreground and mid ground
		REG_BG0CNT = BG_CBB(FG_TILESET) | BG_SBB(FOREGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(1);
		REG_BG1CNT = BG_CBB(FG_TILESET) | BG_SBB(MIDGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(2);
		
		// Display everything
		visualFlags = DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_MODE0 | DCNT_WIN0;
		
		if (levelFlags & LEVELFLAG_TAS){
			memcpy(&tile_mem[4][ZIP_index = EXTRA_SP_OFFSET], hitbox1610, zipLen);
			
			break;
		}
		
		if (IS_RAINY)
			visualFlags |= DCNT_BG3;
		
		//Load in fore/midground tilesets
		memcpy(&tile_mem[FG_TILESET][0], lvl2_base, lvl2_baseLen);
		memcpy(&tile_mem[FG_TILESET][32], stone_tile, snow_tileLen);
		memcpy(&tile_mem[FG_TILESET][64], girder_tile, girder_tileLen);
		memcpy(&tile_mem[FG_TILESET][96], stone2_tile, stone2_tileLen);
		memcpy(&tile_mem[FG_TILESET][128], spinner_tile, spinner_tileLen);
		
		// foreground palettes (4 - 15)
		memcpy(&pal_bg_mem[4 << 4], stone_pal, 32);
		memcpy(&pal_bg_mem[5 << 4], girder_pal, 32);
		memcpy(&pal_bg_mem[6 << 4], stone_pal, 32);
		memcpy(&pal_bg_mem[7 << 4], stoneBG_pal, 32);
		memcpy(&pal_bg_mem[8 << 4], girderBG_pal, 32);
		memcpy(&pal_bg_mem[9 << 4], stoneBG_pal, 32);
		memcpy(&pal_bg_mem[10 << 4], lvl2_base_pal, 32);
		memcpy(&pal_bg_mem[15 << 4], spinner_pal, 32);
		
		
		memcpy(&pal_obj_mem[((ZIP_pal = EXTRA_SP_PAL_OFFSET) << 4)], zip_pal, zip_palLen);
		memcpy(&tile_mem[4][ZIP_index = EXTRA_SP_OFFSET],		   zip, zipLen);
		memcpy(&tile_mem[4][CHAR_umbrellaOffset = (EXTRA_SP_OFFSET + 4)], umbrella, umbrellaLen >> 1);
		
		break; }
	case LEVELIDX_CORE:
		CHAR_dash_count(2);
		CHAR_flags = CHAR_flags & (~0x0800);
	case LEVELIDX_PROLOGUE: {
		
		STRAWB_levelMax = prologue_strawbMax;
		
		ChangeHBlankIntr(Chapter1_BG);
		
		CHAR_dash_count(1);
		CHAR_flags = CHAR_flags | (0x0800);
		
		CHAR_PRIORITY = 0;
		
		currentMax = MAX_levels_prologue;
		
		// Foreground and mid ground
		REG_BG0CNT = BG_CBB(FG_TILESET) | BG_SBB(FOREGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(0);
		REG_BG1CNT = BG_CBB(FG_TILESET) | BG_SBB(MIDGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(1);
		
		// Backgrounds
		REG_BG2CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_ONE)	| BG_4BPP | BG_REG_64x64 | BG_PRIO(2);
		REG_BG3CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_EXTRA)	| BG_4BPP | BG_REG_64x64 | BG_PRIO(3);
		
		// Display everything
		visualFlags = DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3 | DCNT_MODE0 | DCNT_WIN0;
		
		
		if (levelFlags & LEVELFLAG_TAS)
			break;
		
		//Load in fore/midground tilesets
		memcpy(&tile_mem[FG_TILESET][0], lvl0_base, lvl0_baseLen);
		memcpy(&tile_mem[FG_TILESET][32], snow_tile, snow_tileLen);
		memcpy(&tile_mem[FG_TILESET][64], girder_tile, girder_tileLen);
		memcpy(&tile_mem[FG_TILESET][96], dirt_tile, snow_tileLen);
		
		// background palettes (0 - 3)
		memcpy(&pal_bg_mem[0 << 4], background1_pal, 32);
		// foreground palettes (4 - 15)
		memcpy(&pal_bg_mem[4 << 4], snow0_pal, 32);
		memcpy(&pal_bg_mem[5 << 4], girder0_pal, 32);
		memcpy(&pal_bg_mem[6 << 4], dirt0_pal, 32);
		memcpy(&pal_bg_mem[7 << 4], snow0bg_pal, 32);
		memcpy(&pal_bg_mem[8 << 4], girder0BG_pal, 32);
		memcpy(&pal_bg_mem[9 << 4], dirtBG_pal, 32);
		memcpy(&pal_bg_mem[10 << 4], lvl0_base_pal, 32);
		//memcpy(&pal_bg_mem[15 << 4], spinner_pal, 32);
		
		memcpy(&pal_obj_mem[(SPRING_PAL = EXTRA_SP_PAL_OFFSET) << 4], bumper_pal, 32);
		
		break; }
	case LEVELIDX_WATER: {
		memcpy(&tile_mem[FG_TILESET][128], spinner_tile, spinner_tileLen);
		memcpy(&pal_bg_mem[15 << 4], spinner_pal, 32);
		memcpy(&tile_mem[4][bubble_sp_index = EXTRA_SP_OFFSET], bubble, zipLen);
		
		STRAWB_levelMax = water_strawbMax;
		
		ChangeHBlankIntr(Chapter1_BG);
		
		CHAR_dash_count(1);
		CHAR_flags = CHAR_flags | (0x0800);
		
		CHAR_PRIORITY = 0;
		
		currentMax = MAX_levels_water;
		
		// Foreground and mid ground
		REG_BG0CNT = BG_CBB(FG_TILESET) | BG_SBB(FOREGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(0);
		REG_BG1CNT = BG_CBB(FG_TILESET) | BG_SBB(MIDGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(1);
		
		// Backgrounds
		REG_BG2CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_ONE)	| BG_4BPP | BG_REG_64x64 | BG_PRIO(2);
		REG_BG3CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_EXTRA)	| BG_4BPP | BG_REG_64x64 | BG_PRIO(3);
		
		// Display everything
		visualFlags = DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3 | DCNT_MODE0 | DCNT_WIN0;
		
		//Load in fore/midground tilesets
		memcpy(&tile_mem[FG_TILESET][0], lvl3_base, lvl3_baseLen);
		memcpy(&tile_mem[FG_TILESET][32], small_brick_tile, snow_tileLen);
		memcpy(&tile_mem[FG_TILESET][64], girder_tile, girder_tileLen);
		memcpy(&tile_mem[FG_TILESET][96], snow_tile, snow_tileLen);
		
		// background palettes (0 - 3)
		memcpy(&pal_bg_mem[0 << 4], background3_pal, 32);
		// foreground palettes (4 - 15)
		memcpy(&pal_bg_mem[4 << 4], small_brick_pal, 32);
		memcpy(&pal_bg_mem[5 << 4], girder_pal, 32);
		memcpy(&pal_bg_mem[6 << 4], snow_pal, 32);
		memcpy(&pal_bg_mem[7 << 4], small_brickbg_pal, 32);
		memcpy(&pal_bg_mem[8 << 4], girderBG_pal, 32);
		memcpy(&pal_bg_mem[9 << 4], snowbg_pal, 32);
		memcpy(&pal_bg_mem[10 << 4], lvl3_base_pal, 32);
		memcpy(&pal_bg_mem[11 << 4], dreamblock_pal, 32);
		
		break; }
	case LEVELIDX_DREAM: {
		
		STRAWB_levelMax = dream_strawbMax;
		
		ChangeHBlankIntr(Chapter1_BG);
		
		CHAR_dash_count(1);
		CHAR_flags = CHAR_flags | (0x0800);
		
		CHAR_PRIORITY = 0;
		
		if (levelFlags & LEVELFLAG_BSIDE)
			currentMax = MAX_levels_dreamB;
		else
			currentMax = MAX_levels_dream;
		
		
		// Foreground and mid ground
		REG_BG0CNT = BG_CBB(FG_TILESET) | BG_SBB(FOREGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(0);
		REG_BG1CNT = BG_CBB(FG_TILESET) | BG_SBB(MIDGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(1);
		
		// Display everything
		visualFlags = DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3 | DCNT_MODE0 | DCNT_WIN0;
		
		memcpy(&pal_bg_mem[11 << 4], dreamblock_pal, 32);
		
		if (TAS_ACTIVE)
			break;
		
		//Load in fore/midground tilesets
		memcpy(&tile_mem[FG_TILESET][0], lvl1_base, lvl1_baseLen);
		memcpy(&tile_mem[FG_TILESET][32], small_brick_tile, snow_tileLen);
		memcpy(&tile_mem[FG_TILESET][64], girder_tile, girder_tileLen);
		memcpy(&tile_mem[FG_TILESET][96], snow_tile, snow_tileLen);
		memcpy(&tile_mem[FG_TILESET][144], lvl_dream_deco, lvl_dream_decoLen);
		
		// foreground palettes (4 - 15)
		memcpy(&pal_bg_mem[4 << 4], small_brick_pal, 32);
		memcpy(&pal_bg_mem[5 << 4], girder_pal, 32);
		memcpy(&pal_bg_mem[6 << 4], snow_pal, 32);
		memcpy(&pal_bg_mem[7 << 4], small_brickbg_pal, 32);
		memcpy(&pal_bg_mem[8 << 4], girderBG_pal, 32);
		memcpy(&pal_bg_mem[9 << 4], snowbg_pal, 32);
		memcpy(&pal_bg_mem[10 << 4], lvl1_base_pal, 32);
		
		memcpy(&pal_obj_mem[(SPRING_PAL = EXTRA_SP_PAL_OFFSET) << 4], bumper_pal, 32);
		
		
		flagIndex = EXTRA_SP_OFFSET;
		memcpy(&pal_obj_mem[15 << 4], secret_flag_pal, 32);
		
		break; }
	}
	
	if (levelFlags & LEVELFLAG_TAS) {
		
		memset(&pal_bg_mem[0 << 4], 0, 128);
		memset(&pal_bg_mem[7 << 4], 0, 96);
		
		memcpy(&pal_bg_mem[4 << 4], tas_pal, 32);
		memcpy(&pal_bg_mem[5 << 4], tas_pal, 32);
		memcpy(&pal_bg_mem[6 << 4], tas_pal, 32);
		memcpy(&pal_bg_mem[10 << 4], tas_pal, 32);
		memcpy(&pal_bg_mem[15 << 4], tas_pal, 32);
		
		memcpy(&tile_mem[FG_TILESET][0], lvlTas_base, lvlTas_baseLen);
		memcpy(&tile_mem[FG_TILESET][32], tas_tile, snow_tileLen);
		memcpy(&tile_mem[FG_TILESET][64], tas_tile, girder_tileLen);
		memcpy(&tile_mem[FG_TILESET][96], tas_tile, snow_tileLen);
		if (current_chapter != LEVELIDX_DREAM)
			memcpy(&tile_mem[FG_TILESET][128], tasSpin_tile, spinner_tileLen);
	}
}
void InitializeRoom(){
	
	switch (current_chapter){
	case LEVELIDX_RAIN: {
		
		CHAR_PRIORITY = 1;
		
		// Foreground and mid ground
		REG_BG0CNT = BG_CBB(FG_TILESET) | BG_SBB(FOREGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(1);
		REG_BG1CNT = BG_CBB(FG_TILESET) | BG_SBB(MIDGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(2);
		
		// Display everything
		visualFlags = DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_MODE0 | DCNT_WIN0;
		
		if (levelFlags & LEVELFLAG_TAS){
			
			break;
		}
		
		if (IS_RAINY)
			visualFlags |= DCNT_BG3;
		
		break; }
	case LEVELIDX_PROLOGUE: {
		
		CHAR_PRIORITY = 0;
		
		// Foreground and mid ground
		REG_BG0CNT = BG_CBB(FG_TILESET) | BG_SBB(FOREGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(0);
		REG_BG1CNT = BG_CBB(FG_TILESET) | BG_SBB(MIDGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(1);
		
		// Display everything
		visualFlags = DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3 | DCNT_MODE0 | DCNT_WIN0;
		
		break; }
	case LEVELIDX_WATER: {
		
		ChangeHBlankIntr(Chapter1_BG);
		
		CHAR_PRIORITY = 0;
		
		// Foreground and mid ground
		REG_BG0CNT = BG_CBB(FG_TILESET) | BG_SBB(FOREGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(0);
		REG_BG1CNT = BG_CBB(FG_TILESET) | BG_SBB(MIDGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(1);
		
		// Display everything
		visualFlags = DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3 | DCNT_MODE0 | DCNT_WIN0;
		
		break; }
	case LEVELIDX_DREAM: {
		
		CHAR_PRIORITY = 0;
		
		// Foreground and mid ground
		REG_BG0CNT = BG_CBB(FG_TILESET) | BG_SBB(FOREGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(0);
		REG_BG1CNT = BG_CBB(FG_TILESET) | BG_SBB(MIDGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(1);
		
		// Display everything
		visualFlags = DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3 | DCNT_MODE0 | DCNT_WIN0;
		
		break; }
	}
	
	set_hair_color();
}
void End() {
	if (levelFlags & LEVELFLAG_BSIDE)
		LoadCollisionData(chapter_pointersBside[current_chapter][lvlIndex]);
	else
		LoadCollisionData(chapter_pointers[current_chapter][lvlIndex]);
	
	SetCamPos();
	prevCamX = camX;
	prevCamY = camY;
	
	CHAR_LoadLevel();
	FillCam(camX, camY);
	CHAR_save_loc();
	CHAR_on_transition();
	
	
	switch (current_chapter)
	{
	case LEVELIDX_RAIN:
		if (IS_RAINY)
			visualFlags |= DCNT_BG3;
		else
			visualFlags &= ~DCNT_BG3;
		break;
	}
}
void ResetLevel(){
	int actor_index, actor_id;
	
	for (actor_index = 0; actor_index < maxEntities; ++actor_index){
		
		if (!(PHYS_actors[actor_index].flags & ACTOR_ACTIVE_MASK))
			continue;
		
		actor_id = ACTOR_ENTITY_TYPE(PHYS_actors[actor_index].ID);
		
		switch(actor_id)
		{
		case 1:
			STRAWB_reset(&PHYS_actors[actor_index]);
			break;
		case 2:
			DASHCR_reset(&PHYS_actors[actor_index]);
			break;
		case 5:
			ZIP_reset(&PHYS_actors[actor_index]);
			break;
		case ENT_KEY:
			KEY_reset(&PHYS_actors[actor_index]);
			break;
		}
	}
}
void OpenPause() {
	int i, offset;
	int letterOffset;
	
	if (!(levelFlags & LEVELFLAG_BSIDE)) {
		memcpy(&pal_bg_mem[7  << 4], heart_pal, 32);
		
		offset = 0x140 << 5;
		
		memcpy(se_mem[FOREGROUND_LAYER], pausemenu_fg, pausemenu_fgLen);
		memcpy(&tile_mem[1][offset >> 5], pausemenu_tileset, pausemenu_tilesetLen);
		
		offset += pausemenu_tilesetLen;
		letterOffset = offset >> 5;
	}
	else {
		memcpy(&pal_bg_mem[7  << 4], heartb_pal, 32);
		letterOffset = pausemenu_b_tilesetLen >> 5;
		memcpy(se_mem[FOREGROUND_LAYER], pausemenu_b_fg, pausemenu_b_fgLen);
		memcpy(&tile_mem[1][0], pausemenu_b_tileset, offset = pausemenu_b_tilesetLen);
	}
	memset(se_mem[MIDGROUND_LAYER], 0, pausemenu_fgLen);
	offset >>= 5;
	
	memcpy(&tile_mem[1][offset], pause_font, pause_fontLen);
	offset += pause_fontLen >> 5;
	
	memcpy(&tile_mem[1][offset], gold_wing, gold_wingLen);
	offset += gold_wingLen >> 5;
	
	memcpy(&tile_mem[1][offset], gold_berry, gold_berryLen);
	offset += gold_berryLen >> 5;
	
	memcpy(&tile_mem[1][offset], heart_pause, heart_pauseLen);
	
	memcpy(&pal_bg_mem[4  << 4], pause1_pal, 32);
	memcpy(&pal_bg_mem[5  << 4], pause2_pal, 32);
	memcpy(&pal_bg_mem[6  << 4], pause_gold_pal, 32);
	memcpy(&pal_obj_mem[0 << 4], pause1_pal, 32);
	
	REG_BG0CNT = BG_CBB(FG_TILESET) | BG_SBB(FOREGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(1);
	REG_BG1CNT = BG_CBB(FG_TILESET) | BG_SBB(MIDGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(0);
	
	
	if (current_chapter == LEVELIDX_RAIN) {
		REG_BG2CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_ONE)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(3);
		REG_BG3CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_EXTRA)	| BG_4BPP | BG_REG_64x32 | BG_PRIO(2);
	}
	
	
	visualFlags = DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3 | DCNT_MODE0 | DCNT_WIN0;
	
	REG_BG0HOFS= 0;
	REG_BG0VOFS= 0;
	REG_BG1HOFS= 0;
	REG_BG1VOFS= 0;
	
	
	int wingOffset = letterOffset + 36;
	int goldOffset = wingOffset + 40;
	int cassetteOffset = goldOffset + 16;
	
	if (WINGED_COLL && !(levelFlags & LEVELFLAG_BSIDE)) {
		i = 275; // Tile offset
		offset = 0;
		
		for (offset = 0; offset < 10; ++offset) {
			se_mem[MIDGROUND_LAYER][i + offset	  ] = (wingOffset + offset	   ) | 0x6000;
			se_mem[MIDGROUND_LAYER][i + offset + 32] = (wingOffset + offset + 10) | 0x6000;
			se_mem[MIDGROUND_LAYER][i + offset + 64] = (wingOffset + offset + 20) | 0x6000;
			se_mem[MIDGROUND_LAYER][i + offset + 96] = (wingOffset + offset + 30) | 0x6000;
		}
	}
	if (GOLD_COLL && !(levelFlags & LEVELFLAG_BSIDE)) {
		i = 502;
		offset = 0;
		
		for (offset = 0; offset < 4; ++offset) {
			se_mem[MIDGROUND_LAYER][i + offset	  ] = (goldOffset + offset	   ) | 0x6000;
			se_mem[MIDGROUND_LAYER][i + offset + 32] = (goldOffset + offset +  4) | 0x6000;
			se_mem[MIDGROUND_LAYER][i + offset + 64] = (goldOffset + offset +  8) | 0x6000;
			se_mem[MIDGROUND_LAYER][i + offset + 96] = (goldOffset + offset + 12) | 0x6000;
		}
	}
	if (HEART_A_COLL && !(levelFlags & LEVELFLAG_BSIDE)) {
		i = 53;
		offset = 0;
		
		for (offset = 0; offset < 6; ++offset) {
			se_mem[MIDGROUND_LAYER][i + offset	  ] = (cassetteOffset + offset	   ) | 0x7000;
			se_mem[MIDGROUND_LAYER][i + offset + 32] = (cassetteOffset + offset +  6) | 0x7000;
			se_mem[MIDGROUND_LAYER][i + offset + 64] = (cassetteOffset + offset + 12) | 0x7000;
		}
	}
	
	if (HEART_B_COLL && (levelFlags & LEVELFLAG_BSIDE)) {
		i = 176;
		offset = 0;
		
		for (offset = 0; offset < 6; ++offset) {
			se_mem[MIDGROUND_LAYER][i + offset	  ] = (cassetteOffset + offset	   ) | 0x7000;
			se_mem[MIDGROUND_LAYER][i + offset + 32] = (cassetteOffset + offset +  6) | 0x7000;
			se_mem[MIDGROUND_LAYER][i + offset + 64] = (cassetteOffset + offset + 12) | 0x7000;
		}
	}
	
	
	i = 512 + 15;
	i -= (levelFlags & LEVELFLAG_BSIDE) * 127;
	if (DEATH_tempCount >= 100) {
		se_mem[MIDGROUND_LAYER][i	] = letterOffset + (DEATH_tempCount / 100) | 0x4000;
		se_mem[MIDGROUND_LAYER][i + 1] = letterOffset + ((DEATH_tempCount / 10) % 10) | 0x4000;
		se_mem[MIDGROUND_LAYER][i + 2] = letterOffset + (DEATH_tempCount % 10) | 0x4000;
	}
	else if (DEATH_tempCount >= 10) {
		se_mem[MIDGROUND_LAYER][i	] = letterOffset + (DEATH_tempCount / 10) | 0x4000;
		se_mem[MIDGROUND_LAYER][i + 1] = letterOffset + (DEATH_tempCount % 10) | 0x4000;
	}
	else {
		se_mem[MIDGROUND_LAYER][i	] = letterOffset + (DEATH_tempCount) | 0x4000;
	}
	
	i = 65;
	int index = 0;
	char *string = (char*)&"continue";
	
	for (index = 0; index < 8; ++index)
		se_mem[MIDGROUND_LAYER][i + index] = letterOffset + (string[index] - 87) | 0x4000;
	
	if (DIALOGUE_ENABLED)
		string = (char*)&"  skip";
	else
		string = (char*)&" retry";
	i += 128;
	for (index = 0; index < 6; ++index){
		if (string[index] != ' ')
			se_mem[MIDGROUND_LAYER][i + index] = letterOffset + (string[index] - 87) | 0x4000;
	}
	
	string = (char*)&"  quit";
	i += 128;
	for (index = 0; index < 6; ++index){
		if (string[index] != ' ')
			se_mem[MIDGROUND_LAYER][i + index] = letterOffset + (string[index] - 87) | 0x4000;
	}
	
	string = lvlName;
	i += 192;
	for (index = 0; index < 8; ++index){
		if (string[index] != ' ')
			se_mem[MIDGROUND_LAYER][i + index] = letterOffset + (string[index] - 87) | 0x4000;
		if (string[index + 8] != ' ')
			se_mem[MIDGROUND_LAYER][i + index + 32] = letterOffset + (string[index + 8] - 87) | 0x4000;
	}
	
	// Don't display strawberry info on B sides
	if (levelFlags & LEVELFLAG_BSIDE)
		return;
	
	i = 288 + 15;
	
	if (STRAWB_count >= 100) {
		se_mem[MIDGROUND_LAYER][i	] = letterOffset + (STRAWB_count / 100) | 0x4000;
		se_mem[MIDGROUND_LAYER][i + 1] = letterOffset + ((STRAWB_count / 10) % 10) | 0x4000;
		se_mem[MIDGROUND_LAYER][i + 2] = letterOffset + (STRAWB_count % 10) | 0x4000;
	}
	else if (STRAWB_count >= 10) {
		se_mem[MIDGROUND_LAYER][i	] = letterOffset + (STRAWB_count / 10) | 0x4000;
		se_mem[MIDGROUND_LAYER][i + 1] = letterOffset + (STRAWB_count % 10) | 0x4000;
	}
	else {
		se_mem[MIDGROUND_LAYER][i	] = letterOffset + (STRAWB_count) | 0x4000;
	}
	
	i = 64 + 14;
	if (STRAWB_levelCount >= 10)
		se_mem	[MIDGROUND_LAYER][i	] = letterOffset + (STRAWB_levelCount / 10) | 0x4000;
	se_mem		[MIDGROUND_LAYER][i + 1] = letterOffset +  STRAWB_levelCount % 10 | 0x4000;
	
	if (STRAWB_levelMax >= 10) {
		se_mem[MIDGROUND_LAYER][i + 3] = letterOffset + (STRAWB_levelMax / 10) | 0x4000;
		se_mem[MIDGROUND_LAYER][i + 4] = letterOffset +  STRAWB_levelMax % 10 | 0x4000;
	}
	else {
		se_mem[MIDGROUND_LAYER][i + 3] = letterOffset + (STRAWB_levelMax) | 0x4000;
	}
}
void ClosePause() {
	memcpy(pal_obj_mem, madeline_pal, madeline_palLen);
	
	if (current_chapter == LEVELIDX_RAIN)
	{
		REG_BLDCNT = 0x8 | 0x3700 | 0x40;
		REG_BLDALPHA = 0x8 | 0x600;
	}
	
	switch (current_chapter) {
		case LEVELIDX_PROLOGUE:
			memcpy(&pal_bg_mem[4 << 4], snow0_pal, 32);
			memcpy(&pal_bg_mem[5 << 4], girder0_pal, 32);
			memcpy(&pal_bg_mem[6 << 4], dirt0_pal, 32);
			memcpy(&pal_bg_mem[7 << 4], snow0bg_pal, 32);
			break;
		case LEVELIDX_RAIN:
			memcpy(&pal_bg_mem[4 << 4], stone_pal, 32);
			memcpy(&pal_bg_mem[5 << 4], girder_pal, 32);
			memcpy(&pal_bg_mem[6 << 4], stone_pal, 32);
			memcpy(&pal_bg_mem[7 << 4], stoneBG_pal, 32);
			break;
		case LEVELIDX_DREAM:
			memcpy(&pal_bg_mem[4 << 4], small_brick_pal, 32);
			memcpy(&pal_bg_mem[5 << 4], girder_pal, 32);
			memcpy(&pal_bg_mem[6 << 4], snow_pal, 32);
			memcpy(&pal_bg_mem[7 << 4], small_brickbg_pal, 32);
			break;
	}
}
void DisplayFileBG(){
	
	REG_BLDCNT = 0;
	REG_BLDALPHA = 0;
	
	switch (current_chapter) {
	default:
		
		ChangeHBlankIntr(Chapter1_BG);
		
		memcpy(se_mem[BACKGROUND_EXTRA], city_clouds_fg, city_clouds_fgLen);
		memcpy(se_mem[BACKGROUND_ONE]  , city_clouds_bg, city_clouds_bgLen);
		memcpy(&tile_mem[BG_TILESET][0], city_clouds_tileset, city_clouds_tilesetLen);
		
		REG_BG2CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_ONE)	| BG_4BPP | BG_REG_64x64 | BG_PRIO(2);
		REG_BG3CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_EXTRA)	| BG_4BPP | BG_REG_64x64 | BG_PRIO(3);
		
		memcpy(&pal_bg_mem[0 << 4], background1_pal, 32);
		break;
		
	case LEVELIDX_DREAM:
		
		ChangeHBlankIntr(Chapter1_BG);
		
		memcpy(se_mem[BACKGROUND_EXTRA], city_clouds_fg, city_clouds_fgLen);
		memcpy(se_mem[BACKGROUND_ONE]  , city_clouds_bg, city_clouds_bgLen);
		memcpy(&tile_mem[BG_TILESET][0], city_clouds_tileset, city_clouds_tilesetLen);
		
		REG_BG2CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_ONE)	| BG_4BPP | BG_REG_64x32 | BG_PRIO(2);
		REG_BG3CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_EXTRA)	| BG_4BPP | BG_REG_64x32 | BG_PRIO(3);
		
		memcpy(&pal_bg_mem[0 << 4], background2_pal, 32);
		break;
	case LEVELIDX_RAIN:
		ChangeHBlankIntr(Cloud_Movement);
		
		REG_BLDCNT = 0x8 | 0x3700 | 0x40;
		REG_BLDALPHA = 0x8 | 0x600;
		
		memcpy(&pal_bg_mem[0 << 4], rainBG1_pal, 32);
		memcpy(&pal_bg_mem[1 << 4], rainBG2_pal, 32);
		memcpy(&pal_bg_mem[2 << 4], rainBG3_pal, 32);
		
		REG_BG2CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_ONE)	| BG_4BPP | BG_REG_64x32 | BG_PRIO(3);
		REG_BG3CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_EXTRA)	| BG_4BPP | BG_REG_64x32 | BG_PRIO((gamestate != GS_PLAYING) << 1);
		
		memcpy(se_mem[BACKGROUND_ONE]  , rain_fg, rain_fgLen);
		memcpy(se_mem[BACKGROUND_EXTRA], rain_bg, rain_bgLen);
		memcpy(&tile_mem[BG_TILESET][0], rain_tileset, rain_tilesetLen);
		break;
	}
}
void LeaveToMainMenu() {
	DEATH_tempCount = 0;
	heart_freeze = 0;
	
	STRAWB_display = STRAWB_count;
	strawbUI_Pos = 0x1000;
	forceDisplay = 1;
	
	lvlIndex = current_chapter;
	
	ChangeHBlankIntr(EmptyInterrupt);
	
	memcpy(pal_obj_mem + 16, pickup_pal, pickup_palLen);
	memcpy(&pal_bg_mem[4 << 4], menu_pal, 32);
	memcpy(&pal_obj_mem[2 << 4], pause1_pal, 32);
	
	REG_BLDCNT = 0;
	REG_BLDALPHA = 0;
	
	REG_BG0CNT = BG_CBB(BG_TILESET) | BG_SBB(FOREGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(1);
	REG_BG3CNT = BG_CBB(FG_TILESET) | BG_SBB(BACKGROUND_ONE) 	| BG_4BPP | BG_REG_64x64 | BG_PRIO(3);
	visualFlags = DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG3 | DCNT_MODE0 | DCNT_WIN0;
	
	memcpy(&pal_bg_mem[0 << 4], map_pal, 32);
	
	memset(se_mem[FOREGROUND_LAYER], 0, 2048);
	memcpy(tile_mem[BG_TILESET], chapter_select_tileset, chapter_select_tilesetLen);
	memcpy(&tile_mem[BG_TILESET][chapter_select_tilesetLen >> 5], dialogue_font, dialogue_fontLen);
	
	memcpy(se_mem[BACKGROUND_ONE], map_fg, map_fgLen);
	memcpy(tile_mem[FG_TILESET], map_tileset, map_tilesetLen);
	
	enterX = chapter_map_locations[(current_chapter << 1) + 0] - (screenWidth  >> 1);
	enterY = chapter_map_locations[(current_chapter << 1) + 1] - (screenHeight >> 1);
	
	int index = 1;
	for (; index < maxEntities; ++index){
		PHYS_actors[index].flags = 0;
		PHYS_actors[index].ID = 0;
	}
}
void ResetData(int _file){
	int index;
	int index2 = _file << SAVEFILE_SHIFT;
	
	for (index = 0; index < SAVEFILE_LEN; ++index)
	{
		sram_mem[index + index2] = 0xFF;
	}
	saveFile[0] = 0xFF;
}
void SaveData(int _file){
	
	unsigned long v = GAME_speedrun_timer;
	STRAWB_COUNT_SET(STRAWB_levelCount);
	saveFile[0] = 0;
	
	int index = 4;
	while (--index >= 0){
		saveFile[SAVE_TIMER_ADD + index] = v & 0xFF;
		v >>= 8;
	}
	
	v = DEATH_count;
	index = 4;
	while (--index >= 0){
		saveFile[SAVE_DEATH_OFF + index] = v & 0xFF;
		v >>= 8;
	}
	
	index = -1;
	while (++index < FILENAME_LEN){
		saveFile[FILE_NAME_IDX + index] = fileName[index];
	}
	
	int index2 = _file << SAVEFILE_SHIFT;
	
	for (index = 0; index < SAVEFILE_LEN; ++index)
	{
		sram_mem[index + index2] = saveFile[index];
	}
}
void LoadData(int _save){
	
	int index = 0;
	int index2 = _save << SAVEFILE_SHIFT;

	for (index = 0; index < SAVEFILE_LEN; ++index)
	{
		saveFile[index] = sram_mem[index + index2];
	}
	
	if (saveFile[0] == 0xFF) {
		STRAWB_count = 0;
		GAME_speedrun_timer = 0;
		DEATH_count = 0;
		STRAWB_levelCount = 0;
		
		for (index = 0; index < FILENAME_LEN; ++index) {
			fileName[index] = emptyFileName[index];
		}
		for (index = 1; index < SAVEFILE_LEN; ++index) {
			saveFile[index] = 0;
		}
		saveFile[index] = 0xFF;
	}
	else {
		STRAWB_count = 0;
		for (index = 0; index < 15; ++index){
			char value = saveFile[STRAWB_SAVE_ADD + index];
			for (index2 = 0; index2 < 8; ++index2){
				if (value & 1) ++STRAWB_count;
				value >>= 1;
			}
		}

		GAME_speedrun_timer = 0;
		DEATH_count = 0;
		
		index = -1;
		while (++index < 4){
			GAME_speedrun_timer <<= 8;
			GAME_speedrun_timer |= saveFile[SAVE_TIMER_ADD + index];
		}
		
		index = -1;
		while (++index < 4){
			DEATH_count <<= 8;
			DEATH_count |= saveFile[SAVE_DEATH_OFF + index];
		}
		
		index = -1;
		while (++index < FILENAME_LEN){
			fileName[index] = saveFile[FILE_NAME_IDX + index];
		}
	}
	STRAWB_display = STRAWB_count;
}
void SetCore(int value){
	if (value && !IS_CORE_HOT)
	{
		levelFlags |=  LEVELFLAG_COREMODE;
		memcpy(&pal_bg_mem[4 << 4], corehot_pal,    32);
		memcpy(&pal_bg_mem[5 << 4], corehot_bg_pal, 32);
	}
	else if (!value && IS_CORE_HOT)
	{
		levelFlags &= ~LEVELFLAG_COREMODE;
		memcpy(&pal_bg_mem[4 << 4], corecold_pal,    32);
		memcpy(&pal_bg_mem[5 << 4], corecold_bg_pal, 32);

	}
}

