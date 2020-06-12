#include "game_data.h"
#include "physics.h"
#include "char.h"
#include "toolbox.h"
#include "level.h"
#include "memmap.h"
#include "dashcrystal.h"
#include "bumper.h"
#include "zip.h"
#include "dialogue.h"

#define UI_BOUNCE_START 	0x240
#define UI_ENTER_SPEED 		0x200
#define UI_GRAV 			0x61

unsigned char GAME_freeze = 0;
unsigned int GAME_music_beat = 0;
unsigned long GAME_speedrun_timer = 0, GAME_IL_timer = 0;
unsigned int GAME_life = 0;
unsigned int cutsceneInput, cutsceneWait;

unsigned char STRAWB_tempColl[15];

int screenShakeX, screenShakeY;

void (*cutsceneCoroutine)(void);
void (*cutsceneSkipped)(void);

const char** chapter_pointers[LEVELIDX_MAX]=
{
	(const char**)(&levels_prologue[0]),
	(const char**)(&levels_dream[0]),
	(const char**)(&levels_forsaken[0]),
	(const char**)(&levels_water[0]),
};
const char** chapter_pointersBside[LEVELIDX_MAX]=
{
	(const char**)(&levels_prologue[0]),
	(const char**)(&levels_dreamB[0]),
	(const char**)(&levels_forsaken[0]),
	(const char**)(&levels_water[0]),
};
const int chapter_map_locations[LEVELIDX_MAX << 1]=
{
	0x03800, 0x1E000,
	0x08000, 0x1C000,
	0x0E000, 0x1B000,
	0x14000, 0x1B000,
};
int level_exits[4] = {255,255,255,255};
int enterX = 0, enterY = 1;

const char emptyFileName[FILENAME_LEN]= {
	0x0E,0x16,0x19,0x1D,0x22,0x24,0x24,0x24,0x24,0x24,0x24,0x24,
};

char saveFile[SAVEFILE_LEN], fileName[FILENAME_LEN];
int saveFileNumber;

char *lvlName;
int current_chapter = 0, lvlIndex = 0, currentMax, levelFlags;
char gamestate = 0, nextGamestate;

short directColors[512];
int GAME_fadeAmount;
char GAME_fading;
int GAME_loadIndex;

int strawbUI_Pos, strawbUI_Offset1, strawbUI_Offset2, strawbUI_speed1, strawbUI_speed2, strawbUI_Timer;
int STRAWB_count = 0, STRAWB_levelCount = 0, STRAWB_levelMax = 0, STRAWB_display = 0;
unsigned int DEATH_count = 0, DEATH_tempCount = 0;

int HOFS, VOFS;
int camX, camY, prevCamX, prevCamY;

unsigned int wind;

int wiggleValue, wiggleTick;
int wiggleOffsets[] = {8, 8, 8, 9, 9, 9, 9, 9, 8, 8, 8, 7, 7, 7, 7, 7};

void Interrupt(){
	
	if (REG_VCOUNT < 160){
		++wiggleValue;
		//pal_bg_mem[0] ^= 0x7FFF;
		REG_BG0HOFS = wiggleOffsets[wiggleValue & 0xF];
		REG_BG1HOFS = wiggleOffsets[wiggleValue & 0xF];
	}
	
	REG_IF = IRQ_HBLANK;
}
void Cloud_Movement(){
	
	if (REG_VCOUNT == 60 && gamestate != GS_PAUSED){
		REG_BG2HOFS= FIXED2INT(FIXED_MULT((INT2FIXED(HOFS) + (GAME_life << 8)), 0x30));
	}
	else if (REG_VCOUNT == 110 && gamestate != GS_PAUSED){
		
		REG_BG2HOFS= FIXED2INT(FIXED_MULT((INT2FIXED(HOFS) + (GAME_life << 7)), 0x20));
	}
	
	REG_IF = IRQ_HBLANK;
}
void Chapter1_BG(){
	if (REG_VCOUNT == 72 && gamestate != GS_PAUSED){
		int hOffset = INT2FIXED(HOFS);
		
		REG_BG3HOFS = FIXED2INT(FIXED_MULT(hOffset, 0x20));
	}
	REG_IF = IRQ_HBLANK;

}

void ReadyWiggle(){
	wiggleValue = GAME_life >> 3;
	int i, j = 0;
	
	for (i = camY; i < camY + 16; ++i)
	{
		wiggleOffsets[j] = camX;
		if (IS_CORE_HOT && (i & 7) >= 2)
			wiggleOffsets[j] += ((i & 8) ? 1 : -1);
		
		++j;
	}
}

void StrawbUI(int force){
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
	else if (STRAWB_count > STRAWB_display || force)
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

void SetCamPos(){
	// get camera position offset from player
	camX = FIXED2INT(PHYS_actors[0].x) - 120;
	camY = FIXED2INT(PHYS_actors[0].y) -  90;
	
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
	HOFS = camX + BLOCK2INT(roomOffsetX);
	VOFS = camY + BLOCK2INT(roomOffsetY);
	
	HOFS += screenShakeX / 16;
	VOFS += screenShakeY / 16;
	
	screenShakeX = -(screenShakeX - INT_SIGN(screenShakeX));
	screenShakeY = -(screenShakeY - INT_SIGN(screenShakeY));
	
	// set foreground offset
	REG_BG0HOFS= HOFS;
	REG_BG0VOFS= VOFS;
	
	if (!dialogueEnabled){
		// set midground offset to the same as foreground
		REG_BG1HOFS= HOFS;
		REG_BG1VOFS= VOFS;
	}
	
	wind = (GAME_life << 8);
	int hOffset = INT2FIXED(HOFS) + wind;
	int vOffset = INT2FIXED(VOFS);
	
	if (gamestate != GS_PAUSED) {
		if (current_chapter == LEVELIDX_RAIN){
			wind = (GAME_life << 8) + (GAME_life << 7);
			hOffset = INT2FIXED(HOFS) + wind;
			
			if (IS_RAINY)
			{
				++wiggleTick;
				REG_BG3HOFS = camX;
				REG_BG3VOFS = -(wiggleTick << 3);
			}
			
			REG_BG2HOFS= FIXED2INT(FIXED_MULT(hOffset, 0x40));
			REG_BG2VOFS= 0;
			
		}
		else{
			
			REG_BG2HOFS= FIXED2INT(FIXED_MULT(hOffset, 0x40));
			REG_BG2VOFS= FIXED2INT(FIXED_MULT(vOffset, 0x40));
			
			int hOffset = INT2FIXED(HOFS) + FIXED_MULT(wind, 0xC0);
			int vOffset = INT2FIXED(VOFS);
			
			REG_BG3HOFS = FIXED2INT(FIXED_MULT(hOffset, 0x30));
			REG_BG3VOFS = 0x18;
			
		}
	}
	
	ReadyWiggle();

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
	HOFS = camX + BLOCK2INT(roomOffsetX);
	VOFS = camY + BLOCK2INT(roomOffsetY);
	
	// set foreground offset
	REG_BG2HOFS= HOFS;
	REG_BG2VOFS= VOFS;
	// set midground offset to the same as foreground
	REG_BG3HOFS= HOFS;
	REG_BG3VOFS= VOFS;
	
	camX <<= 8;
	camY <<= 8;
}

void LoadLevel(){
	// Load in game data.
	
	SetCamPos();
	prevCamX = camX;
	prevCamY = camY;
	FillCam(camX, camY);
	
	memcpy(&tile_mem[FG_TILESET][16], note_block, note_blockLen);
	
	memcpy(&directColors[13 << 4], note_a1_pal, 32);
	memcpy(&directColors[14 << 4], note_b2_pal, 32);
	
	int index;
	
	for (index = 1; index < ActorLimit; ++index) {
		if (ACTOR_ENTITY_TYPE(PHYS_actors[index].ID) == ENT_STRAWB) {
			if (!STRAWB_SAVEISCOLL(((PHYS_actors[index].ID >> 16) & 0xFF)))
				memcpy(directColors + 288, chest_pal, pickup_palLen);
			else
				memcpy(directColors + 288, chest_wing_pal, pickup_palLen);
		}
	}
	
	switch (current_chapter){
	case LEVELIDX_RAIN:
	
		lvlName = (char*)&forsaken_visName;
		
		STRAWB_levelMax = forsaken_strawbMax;
		
		REG_ISR_MAIN = Cloud_Movement;
		REG_IME = 1;
		CHAR_PRIORITY = 1;
		
		CHAR_dash_count(1);
		CHAR_flags = CHAR_flags | (0x0800);
		
		if (levelFlags & LEVELFLAG_BSIDE)
			currentMax = MAX_levels_forsaken;
		else
			currentMax = MAX_levels_forsaken;
		
		// Set dash count to one.  Refills on ground
		
		// Foreground and mid ground
		REG_BG0CNT = BG_CBB(FG_TILESET) | BG_SBB(FOREGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(1);
		REG_BG1CNT = BG_CBB(FG_TILESET) | BG_SBB(MIDGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(2);
		
		// Backgrounds
		REG_BG2CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_ONE)	| BG_4BPP | BG_REG_64x32 | BG_PRIO(3);
		REG_BG3CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_EXTRA)	| BG_4BPP | BG_REG_64x32 | BG_PRIO(0);
		
		REG_BLDCNT = 0x8 | 0x3700 | 0x40;
		REG_BLDALPHA = 0x8 | 0x600;
		
		// Display everything
		REG_DISPCNT = DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_MODE0;
		
		if (IS_RAINY)
			REG_DISPCNT |= DCNT_BG3;
		
		//Load in fore/midground tilesets
		memcpy(&tile_mem[FG_TILESET][0], lvl2_base, lvl2_baseLen);
		memcpy(&tile_mem[FG_TILESET][32], stone_tile, snow_tileLen);
		memcpy(&tile_mem[FG_TILESET][64], girder_tile, girder_tileLen);
		memcpy(&tile_mem[FG_TILESET][96], dirt_tile, snow_tileLen);
		
		// background palettes (0 - 3)
		memcpy(&directColors[0 << 4], rainBG_pal, 32);
		memcpy(&directColors[1 << 4], background1_pal, 32);
		// foreground palettes (4 - 15)
		memcpy(&directColors[4 << 4], stone_pal, 32);
		memcpy(&directColors[5 << 4], girder_pal, 32);
		memcpy(&directColors[6 << 4], dirt_pal, 32);
		memcpy(&directColors[7 << 4], stoneBG_pal, 32);
		memcpy(&directColors[8 << 4], girderBG_pal, 32);
		memcpy(&directColors[9 << 4], dirtBG_pal, 32);
		memcpy(&directColors[10 << 4], lvl2_base_pal, 32);
		memcpy(&directColors[15 << 4], spinner_pal, 32);
		
		// Load in background tilesets
		memcpy(se_mem[BACKGROUND_ONE]  , rain_fg, rain_fgLen);
		memcpy(se_mem[BACKGROUND_EXTRA], rain_bg, rain_bgLen);
		memcpy(&tile_mem[BG_TILESET][0], rain_tileset, rain_tilesetLen);
		memcpy(&tile_mem[FG_TILESET][128], spinner_tile, spinner_tileLen);
		
		memcpy(&directColors[((ZIP_pal = EXTRA_SP_PAL_OFFSET) << 4) + 256], zip_pal, zip_palLen);
		memcpy(&tile_mem[4][ZIP_index = EXTRA_SP_OFFSET],		   zip, zipLen);
		memcpy(&tile_mem[4][CHAR_umbrellaOffset = (EXTRA_SP_OFFSET + 4)], umbrella, umbrellaLen >> 1);
		
		break;
	case LEVELIDX_CORE:
		CHAR_dash_count(2);
		CHAR_flags = CHAR_flags & (~0x0800);
	case LEVELIDX_PROLOGUE:
		
		lvlName = (char*)&prologue_visName;
		
		STRAWB_levelMax = prologue_strawbMax;
		
		REG_ISR_MAIN = Chapter1_BG;
		REG_IME = 1;
		
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
		REG_DISPCNT= DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3 | DCNT_MODE0;
		
		//Load in fore/midground tilesets
		memcpy(&tile_mem[FG_TILESET][0], lvl0_base, base_tileLen);
		memcpy(&tile_mem[FG_TILESET][32], snow_tile, snow_tileLen);
		memcpy(&tile_mem[FG_TILESET][64], girder_tile, girder_tileLen);
		memcpy(&tile_mem[FG_TILESET][96], dirt_tile, snow_tileLen);
		
		// background palettes (0 - 3)
		memcpy(&directColors[0 << 4], background1_pal, 32);
		// foreground palettes (4 - 15)
		memcpy(&directColors[4 << 4], snow_pal, 32);
		memcpy(&directColors[5 << 4], girder_pal, 32);
		memcpy(&directColors[6 << 4], dirt_pal, 32);
		memcpy(&directColors[7 << 4], snowbg_pal, 32);
		memcpy(&directColors[8 << 4], girderBG_pal, 32);
		memcpy(&directColors[9 << 4], dirtBG_pal, 32);
		memcpy(&directColors[10 << 4], lvl0_base_pal, 32);
		memcpy(&directColors[15 << 4], spinner_pal, 32);
		
		// Load in background tilesets
		memcpy(se_mem[BACKGROUND_EXTRA], city_clouds_bg, city_clouds_bgLen);
		memcpy(se_mem[BACKGROUND_ONE]  , city_clouds_fg, city_clouds_fgLen);
		memcpy(&tile_mem[BG_TILESET][0], city_clouds_tileset, city_clouds_tilesetLen);
		
		break;
	case LEVELIDX_WATER:
		memcpy(&tile_mem[FG_TILESET][128], spinner_tile, spinner_tileLen);
		memcpy(&directColors[15 << 4], spinner_pal, 32);
		memcpy(&tile_mem[4][bubble_sp_index = EXTRA_SP_OFFSET], bubble, zipLen);
		
		lvlName = (char*)&water_visName;
		STRAWB_levelMax = water_strawbMax;
		
		REG_ISR_MAIN = Chapter1_BG;
		REG_IME = 1;
		
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
		REG_DISPCNT= DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3 | DCNT_MODE0;
		
		//Load in fore/midground tilesets
		memcpy(&tile_mem[FG_TILESET][0], base_tile, base_tileLen);
		memcpy(&tile_mem[FG_TILESET][32], small_brick_tile, snow_tileLen);
		memcpy(&tile_mem[FG_TILESET][64], girder_tile, girder_tileLen);
		memcpy(&tile_mem[FG_TILESET][96], snow_tile, snow_tileLen);
		memcpy(&tile_mem[FG_TILESET][144], small_brick_spiked, small_brick_spikedLen);
		
		// background palettes (0 - 3)
		memcpy(&directColors[0 << 4], background1_pal, 32);
		// foreground palettes (4 - 15)
		memcpy(&directColors[4 << 4], small_brick_pal, 32);
		memcpy(&directColors[5 << 4], girder_pal, 32);
		memcpy(&directColors[6 << 4], snow_pal, 32);
		memcpy(&directColors[7 << 4], small_brickbg_pal, 32);
		memcpy(&directColors[8 << 4], girderBG_pal, 32);
		memcpy(&directColors[9 << 4], snowbg_pal, 32);
		memcpy(&directColors[10 << 4], lvl1_base_pal, 32);
		memcpy(&directColors[11 << 4], dreamblock_pal, 32);
		
		// Load in background tilesets
		memcpy(se_mem[BACKGROUND_EXTRA], city_clouds_bg, city_clouds_fgLen);
		memcpy(se_mem[BACKGROUND_ONE]  , city_clouds_fg, city_clouds_bgLen);
		memcpy(&tile_mem[BG_TILESET][0], city_clouds_tileset, city_clouds_tilesetLen);
		
		break;
	case LEVELIDX_DREAM:
		
		lvlName = (char*)&dream_visName;
		STRAWB_levelMax = dream_strawbMax;
		
		REG_ISR_MAIN = Chapter1_BG;
		REG_IME = 1;
		
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
		
		// Backgrounds
		REG_BG2CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_ONE)	| BG_4BPP | BG_REG_64x64 | BG_PRIO(2);
		REG_BG3CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_EXTRA)	| BG_4BPP | BG_REG_64x64 | BG_PRIO(3);
		
		// Display everything
		REG_DISPCNT= DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3 | DCNT_MODE0;
		
		//Load in fore/midground tilesets
		memcpy(&tile_mem[FG_TILESET][0], base_tile, base_tileLen);
		memcpy(&tile_mem[FG_TILESET][32], small_brick_tile, snow_tileLen);
		memcpy(&tile_mem[FG_TILESET][64], girder_tile, girder_tileLen);
		memcpy(&tile_mem[FG_TILESET][96], snow_tile, snow_tileLen);
		memcpy(&tile_mem[FG_TILESET][144], small_brick_spiked, small_brick_spikedLen);
		
		// background palettes (0 - 3)
		memcpy(&directColors[0 << 4], background1_pal, 32);
		// foreground palettes (4 - 15)
		memcpy(&directColors[4 << 4], small_brick_pal, 32);
		memcpy(&directColors[5 << 4], girder_pal, 32);
		memcpy(&directColors[6 << 4], snow_pal, 32);
		memcpy(&directColors[7 << 4], small_brickbg_pal, 32);
		memcpy(&directColors[8 << 4], girderBG_pal, 32);
		memcpy(&directColors[9 << 4], snowbg_pal, 32);
		memcpy(&directColors[10 << 4], lvl1_base_pal, 32);
		memcpy(&directColors[11 << 4], dreamblock_pal, 32);
		
		// Load in background tilesets
		memcpy(se_mem[BACKGROUND_EXTRA], city_clouds_bg, city_clouds_fgLen);
		memcpy(se_mem[BACKGROUND_ONE]  , city_clouds_fg, city_clouds_bgLen);
		memcpy(&tile_mem[BG_TILESET][0], city_clouds_tileset, city_clouds_tilesetLen);
		
		break;
	}
	
	set_hair_color();
	
	if (dialogueEnabled)
		DLG_start();
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
			REG_DISPCNT |= DCNT_BG3;
		else
			REG_DISPCNT &= ~DCNT_BG3;
		break;
	}
}
void FinishLevel() {
	GAME_fading = 1;
	nextGamestate = GS_LEVEL;
	
	if (!LEVEL_UNLOCKED(current_chapter + 1))
	{
		UNLOCK_LEVEL(current_chapter + 1);
	}
}
void ResetLevel(){
	int actor_index, actor_id;
	
	for (actor_index = 0; actor_index < ActorLimit; ++actor_index){
		
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
	memcpy(se_mem[BACKGROUND_ONE], pausemenu_fg, pausemenu_fgLen);
	memset(se_mem[BACKGROUND_EXTRA], 0, pausemenu_fgLen);
	memcpy(&tile_mem[1][0], pausemenu_tileset, offset = pausemenu_tilesetLen);
	offset >>= 5;
	
	memcpy(&tile_mem[1][offset], pause_font, pause_fontLen);
	offset += pause_fontLen >> 5;
	
	memcpy(&tile_mem[1][offset], gold_wing, gold_wingLen);
	offset += gold_wingLen >> 5;
	
	memcpy(&tile_mem[1][offset], gold_berry, gold_berryLen);
	offset += gold_berryLen >> 5;
	
	memcpy(&tile_mem[1][offset], cassette_pause, cassette_pauseLen);
	
	memcpy(&directColors[0  << 4], pause1_pal, 32);
	memcpy(&directColors[1  << 4], pause2_pal, 32);
	memcpy(&directColors[2  << 4], pause_gold_pal, 32);
	memcpy(&directColors[3  << 4], cassette_pal, 32);
	memcpy(&directColors[18 << 4], pause1_pal, 32);
	
	REG_BG2CNT = BG_CBB(FG_TILESET) | BG_SBB(BACKGROUND_ONE)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(1);
	REG_BG3CNT = BG_CBB(FG_TILESET) | BG_SBB(BACKGROUND_EXTRA)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(0);
	
	REG_BLDCNT = 0;
	REG_BLDALPHA = 0;
	
	REG_DISPCNT= DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG2 | DCNT_BG3 | DCNT_MODE0;
	
	REG_BG2HOFS= 0;
	REG_BG2VOFS= 0;
	REG_BG3HOFS= 0;
	REG_BG3VOFS= 0;
	
	
	int letterOffset = pausemenu_tilesetLen >> 5;
	int wingOffset = letterOffset + 36;
	int goldOffset = wingOffset + 40;
	int cassetteOffset = goldOffset + 16;
	
	if (WINGED_COLL) {
		i = 224 + 19;
		offset = 0;
		
		for (offset = 0; offset < 10; ++offset) {
			se_mem[BACKGROUND_EXTRA][i + offset	  ] = (wingOffset + offset	   ) | 0x2000;
			se_mem[BACKGROUND_EXTRA][i + offset + 32] = (wingOffset + offset + 10) | 0x2000;
			se_mem[BACKGROUND_EXTRA][i + offset + 64] = (wingOffset + offset + 20) | 0x2000;
			se_mem[BACKGROUND_EXTRA][i + offset + 96] = (wingOffset + offset + 30) | 0x2000;
		}
	}
	if (GOLD_COLL) {
		i = 448 + 22;
		offset = 0;
		
		for (offset = 0; offset < 4; ++offset) {
			se_mem[BACKGROUND_EXTRA][i + offset	  ] = (goldOffset + offset	   ) | 0x2000;
			se_mem[BACKGROUND_EXTRA][i + offset + 32] = (goldOffset + offset +  4) | 0x2000;
			se_mem[BACKGROUND_EXTRA][i + offset + 64] = (goldOffset + offset +  8) | 0x2000;
			se_mem[BACKGROUND_EXTRA][i + offset + 96] = (goldOffset + offset + 12) | 0x2000;
		}
	}
	if (CASSETTE_COLL) {
		i = 32 + 21;
		offset = 0;
		
		for (offset = 0; offset < 6; ++offset) {
			se_mem[BACKGROUND_EXTRA][i + offset	  ] = (cassetteOffset + offset	   ) | 0x3000;
			se_mem[BACKGROUND_EXTRA][i + offset + 32] = (cassetteOffset + offset +  6) | 0x3000;
			se_mem[BACKGROUND_EXTRA][i + offset + 64] = (cassetteOffset + offset + 12) | 0x3000;
		}
	}
	
	
	i = 288 + 15;
	
	if (STRAWB_count >= 100) {
		se_mem[BACKGROUND_EXTRA][i	] = letterOffset + (STRAWB_count / 100);
		se_mem[BACKGROUND_EXTRA][i + 1] = letterOffset + (STRAWB_count / 10) % 10;
		se_mem[BACKGROUND_EXTRA][i + 2] = letterOffset + (STRAWB_count % 10);
	}
	else if (STRAWB_count >= 10) {
		se_mem[BACKGROUND_EXTRA][i	] = letterOffset + (STRAWB_count / 10);
		se_mem[BACKGROUND_EXTRA][i + 1] = letterOffset + (STRAWB_count % 10);
	}
	else {
		se_mem[BACKGROUND_EXTRA][i	] = letterOffset + (STRAWB_count);
	}
	
	i = 64 + 14;
	if (STRAWB_levelCount >= 10)
		se_mem	[BACKGROUND_EXTRA][i	] = letterOffset + (STRAWB_levelCount / 10);
	se_mem		[BACKGROUND_EXTRA][i + 1] = letterOffset +  STRAWB_levelCount % 10;
	
	if (STRAWB_levelMax >= 10) {
		se_mem[BACKGROUND_EXTRA][i + 3] = letterOffset + (STRAWB_levelMax / 10);
		se_mem[BACKGROUND_EXTRA][i + 4] = letterOffset +  STRAWB_levelMax % 10;
	}
	else {
		se_mem[BACKGROUND_EXTRA][i + 3] = letterOffset + (STRAWB_levelMax);
	}
	
	i = 512 + 15;
	if (DEATH_count >= 100) {
		se_mem[BACKGROUND_EXTRA][i	] = letterOffset + (DEATH_count / 100);
		se_mem[BACKGROUND_EXTRA][i + 1] = letterOffset + (DEATH_count / 10) % 10;
		se_mem[BACKGROUND_EXTRA][i + 2] = letterOffset + (DEATH_count % 10);
	}
	else if (DEATH_count >= 10) {
		se_mem[BACKGROUND_EXTRA][i	] = letterOffset + (DEATH_count / 10);
		se_mem[BACKGROUND_EXTRA][i + 1] = letterOffset + (DEATH_count % 10);
	}
	else {
		se_mem[BACKGROUND_EXTRA][i	] = letterOffset + (DEATH_count);
	}
	
	i = 65;
	int index = 0;
	char *string = (char*)&"continue";
	
	for (index = 0; index < 8; ++index)
		se_mem[BACKGROUND_EXTRA][i + index] = letterOffset + (string[index] - 87);
	
	if (dialogueEnabled)
		string = (char*)&"  skip";
	else
		string = (char*)&" retry";
	i += 128;
	for (index = 0; index < 6; ++index){
		if (string[index] != ' ')
			se_mem[BACKGROUND_EXTRA][i + index] = letterOffset + (string[index] - 87);
	}
	
	string = (char*)&"  quit";
	i += 128;
	for (index = 0; index < 6; ++index){
		if (string[index] != ' ')
			se_mem[BACKGROUND_EXTRA][i + index] = letterOffset + (string[index] - 87);
	}
	
	string = lvlName;
	i += 192;
	for (index = 0; index < 8; ++index){
		if (string[index] != ' ')
			se_mem[BACKGROUND_EXTRA][i + index] = letterOffset + (string[index] - 87);
		if (string[index + 8] != ' ')
			se_mem[BACKGROUND_EXTRA][i + index + 32] = letterOffset + (string[index + 8] - 87);
	}
}
void ClosePause() {
	if (current_chapter == LEVELIDX_RAIN)
	{
		REG_BLDCNT = 0x8 | 0x3700 | 0x40;
		REG_BLDALPHA = 0x8 | 0x600;
	}
}
void LeaveToMainMenu() {
	gamestate = GS_LEVEL;
	DEATH_tempCount = 0;
	
	REG_IME = 0;
	
	memcpy(directColors + 256 + 16, pickup_pal, pickup_palLen);
	
	REG_BLDCNT = 0;
	REG_BLDALPHA = 0;
	
	REG_BG3CNT = BG_CBB(1) | BG_SBB(20) | BG_4BPP | BG_REG_64x64 | BG_PRIO(3);
	REG_DISPCNT=  DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG3 | DCNT_MODE0;
	
	memcpy(&directColors[0 << 4], map_pal, 32);
	
	memcpy(se_mem[20], map_fg, map_fgLen);
	memcpy(&tile_mem[1][0], map_tileset, map_tilesetLen);
	
	enterX = chapter_map_locations[(current_chapter << 1) + 0] - (screenWidth  >> 1);
	enterY = chapter_map_locations[(current_chapter << 1) + 1] - (screenHeight >> 1);
	
	int index = 1;
	for (; index < ActorLimit; ++index){
		
		PHYS_actors[index].flags = 0;
		PHYS_actors[index].ID = 0;
		
	}
}
void ResetData(int _file){
	int index;
	int index2 = _file << SAVEFILE_SHIFT;
	
	for (index = 0; index < SAVEFILE_LEN; ++index)
	{
		save_data[index + index2] = 0xFF;
	}
	
}
void SaveData(int _file){
	unsigned long v = GAME_speedrun_timer;
	STRAWB_COUNT_SET(STRAWB_levelCount);
	saveFile[0] = 0;
	
	int index = 8;
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
		save_data[index + index2] = saveFile[index];
	}
}
void LoadData(int _save){
	
	int index = 0;
	int index2 = _save << SAVEFILE_SHIFT;

	for (index = 0; index < SAVEFILE_LEN; ++index)
	{
		saveFile[index] = save_data[index + index2];
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
		while (++index < 8){
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

