#include "game_data.h"
#include "backgrounds.h"
#include "sprites.h"

#include "dialogue.h"

#include "char.h"
#include "text.h"
#include "physics.h"

#define PORTRAIT_SIZE		SPRITE_8x8 << 4
#define PORTRAIT_TILES		SPRITE_8x8 << 2

#define BADELINE			1

short testValue;
short dialogueFlags;
char *dialogue;
char **dialogueList;
short dialogueMax;

short cutsceneIndex;
char isMaddy;

unsigned int *portraitLoc;

short space;
short textOffset, portOffset;
short DLG_wait, pauseType;
short textIndex;

// todo:
// -- Start and end a cutscene
// -- Get the text box to appear/disappear.
// -- Add portraits.
// -- Animate portraits.
// -- Display text
// -- Switch between dialogue lines
// -- 

void DLG_change_hair(){
	// change hair color
	COLOR *palette = &pal_bg_mem[11 << 4] + 1;
	
	COLOR *lookUp = ((COLOR*)&hairColor);
	
	lookUp += CURRENT_DASH_STATUS << 1;
	
	if (TOTAL_DASH_STATUS == 0)
		lookUp += 2;
	else if (TOTAL_DASH_STATUS == 3)
		lookUp -= 2;
	
	memcpy(palette, lookUp, 2);
	memcpy(palette + 1, lookUp + 1, 2);
}
void DLG_update(){
	REG_DISPCNT |= DCNT_BG1;

	if (key_hit(KEY_A) && !DLG_wait)
	{
		pauseType = 0;
		++textIndex;
		
		if (textIndex >= dialogueMax){
			DLG_end();
			
			if (lvlIndex == currentMax - 1){
				START_FADE();
				nextGamestate = GS_END_CUTSCENE;
			}
		}
		else
			DLG_ShowBox(dialogueList[textIndex]);
	}
	
	if (pauseType) {
		--pauseType;
		
		return;
	}
	DLG_change_hair();
	
	if (DLG_wait) {
		++DLG_wait;
		
		if (DLG_wait > 0 && !(DLG_wait & 0x7))
		{
			memcpy(&tile_mem[DIALOGUE_TILESET][portOffset], (char*)(portraitLoc + ((!(DLG_wait & 0x8) || dialogue[space - 1] == 36 || dialogue[space - 1] == 0xF0 || dialogue[space - 1] == 37) ? 0 : PORTRAIT_TILES)), PORTRAIT_SIZE);
			
		}
		
		if (!(DLG_wait & 0x3)) {
			while (dialogue[space] == 0xFF && space < 92){
				++space;
			}
			if (space == 92)
			{
				DLG_wait = 0;
				memcpy(&tile_mem[DIALOGUE_TILESET][portOffset], (char*)portraitLoc, PORTRAIT_SIZE);
			}
			else{
				if (dialogue[space] == 0xF0) {
					pauseType = 15;
				}
				else {
					memcpy(&tile_mem[DIALOGUE_TILESET][textOffset + space], &dialogue_font[(dialogue[space]) << 3], SPRITE_8x8);
				}
				++space;
			}
		}
	}
	
}
void DLG_start(char ** _dialogue, int _max) {
	dialogueFlags = 1;
	textIndex = 0;
	cutsceneSkipped = 0;
	REG_DISPCNT &= ~DCNT_BG1;
	
	if (lvlIndex == currentMax - 1) {
		dialogueFlags |= 0x2;
	}
	
	dialogueList = _dialogue;
	dialogueMax = _max;
	
	DLG_ShowBox(_dialogue[0]);
	
	REG_BG1HOFS = 0;
	REG_BG1VOFS = -25;
	CHAR_PRIORITY += 1;
	
	switch (current_chapter) {
	case LEVELIDX_RAIN:
		
		//Dialogue layer
		REG_BG1CNT = BG_CBB(DIALOGUE_TILESET) | BG_SBB(MIDGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(0);
		
		// Foreground and mid ground
		REG_BG0CNT = BG_CBB(FG_TILESET) | BG_SBB(FOREGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(2);
		
		// Backgrounds
		REG_BG2CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_ONE)	| BG_4BPP | BG_REG_64x32 | BG_PRIO(3);
		REG_BG3CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_EXTRA)	| BG_4BPP | BG_REG_64x32 | BG_PRIO(1);
		break;
	case LEVELIDX_PROLOGUE:
	case LEVELIDX_DREAM:
		
		// Dialogue layer
		REG_BG1CNT = BG_CBB(DIALOGUE_TILESET) | BG_SBB(MIDGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(0);
		
		REG_BG0CNT = BG_CBB(FG_TILESET) | BG_SBB(FOREGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(1);
		
		// Backgrounds
		REG_BG2CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_ONE)	| BG_4BPP | BG_REG_64x64 | BG_PRIO(2);
		REG_BG3CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_EXTRA)	| BG_4BPP | BG_REG_64x64 | BG_PRIO(3);
		
		break;
	}
}
void DLG_end() {
	key_mod(KEY_A);
	key_mod2(KEY_A);
	
	if (lvlIndex < currentMax - 1)
		dialogueFlags = 0;
	CHAR_PRIORITY -= 1;
	pauseType = 0;
	
	
	switch (current_chapter) {
	case LEVELIDX_RAIN:
		// Foreground and mid ground
		REG_BG0CNT = BG_CBB(FG_TILESET) | BG_SBB(FOREGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(1);
		REG_BG1CNT = BG_CBB(FG_TILESET) | BG_SBB(MIDGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(2);
		
		// Backgrounds
		REG_BG2CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_ONE)	| BG_4BPP | BG_REG_64x32 | BG_PRIO(3);
		REG_BG3CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_EXTRA)	| BG_4BPP | BG_REG_64x32 | BG_PRIO(0);
		
		break;
	case LEVELIDX_DREAM:
	case LEVELIDX_PROLOGUE:
	
		REG_BG0CNT = BG_CBB(FG_TILESET) | BG_SBB(FOREGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(0);
		REG_BG1CNT = BG_CBB(FG_TILESET) | BG_SBB(MIDGROUND_LAYER)	| BG_4BPP | BG_REG_32x32 | BG_PRIO(1);
		
		// Backgrounds
		REG_BG2CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_ONE)	| BG_4BPP | BG_REG_64x64 | BG_PRIO(2);
		REG_BG3CNT = BG_CBB(BG_TILESET) | BG_SBB(BACKGROUND_EXTRA)	| BG_4BPP | BG_REG_64x64 | BG_PRIO(3);
		break;
	}
	
	FillCam(camX, camY);
}
void DLG_ShowBox(char* _text) {
	
	char person = *(_text++);
	char port = *(_text++);
	
	isMaddy = 0;
	
	dialogue = _text;
	space = 0;
	DLG_wait = 1;
	
	int offset;
	
	switch (person)
	{
	default:
		isMaddy = 1;
		memcpy(&pal_bg_mem[11 << 4], madeline_port_pal, 64);
		memcpy(&pal_bg_mem[12 << 4], madeline_dialogue_pal, 64);
		
		memcpy(se_mem[MIDGROUND_LAYER], madeline_fg, 0x300);
		memcpy(tile_mem[DIALOGUE_TILESET], madeline_tileset, offset = madeline_tilesetLen);
		
		DLG_change_hair();
		
		portraitLoc = (unsigned int*)maddy_portraits;
		
		break;
	case BADELINE:
		
		memcpy(&pal_bg_mem[11 << 4], badeline_port_pal, 32);
		memcpy(&pal_bg_mem[12 << 4], badeline_dialogue_pal, 32);
		
		memcpy(se_mem[MIDGROUND_LAYER], madeline_fg, pausemenu_fgLen);
		memcpy(&tile_mem[DIALOGUE_TILESET][0], madeline_tileset, offset = madeline_tilesetLen);
		
		portraitLoc = (unsigned int*)baddy_portraits;
		break;
	}
	
	portraitLoc += (PORTRAIT_TILES) * 2 * port;
	
	offset >>= 5;
	textOffset = offset + 16;
	
	int i;
	
	//*
	for (i = 0; i < 4; ++i)
	{
		se_mem[MIDGROUND_LAYER][i +  33] = (offset + i	   ) | 0xB000;
		se_mem[MIDGROUND_LAYER][i +  65] = (offset + i +  4) | 0xB000;
		se_mem[MIDGROUND_LAYER][i +  97] = (offset + i +  8) | 0xB000;
		se_mem[MIDGROUND_LAYER][i + 129] = (offset + i + 12) | 0xB000;
	}
	
	for (i = 0; i < 23; ++i)
	{
		se_mem[MIDGROUND_LAYER][i +  38] = (textOffset + i	   ) | 0xC000;
		se_mem[MIDGROUND_LAYER][i +  70] = (textOffset + i + 23) | 0xC000;
		se_mem[MIDGROUND_LAYER][i + 102] = (textOffset + i + 46) | 0xC000;
		se_mem[MIDGROUND_LAYER][i + 134] = (textOffset + i + 69) | 0xC000;
	}
	//*/
	
	portOffset = offset;
	
	memset(&tile_mem[DIALOGUE_TILESET][textOffset], 0x11, SPRITE_8x8 * 92);
	memcpy(&tile_mem[DIALOGUE_TILESET][portOffset], (char*)portraitLoc, PORTRAIT_SIZE);
	
}

void DLG_prologue() { 
	PHYS_actors[0].velX = FIXED_MULT(PHYS_actors[0].velX, 0xE0);
	
}
void DLG_rainIntroEnd() {
	
	levelFlags |= LEVELFLAG_RAINY;
	if (!(levelFlags & LEVELFLAG_TAS))
		visualFlags |= DCNT_BG3;
}
void DLG_rainIntro() {
	
	if (!DIALOGUE_ENABLED){
		//Start rain
		if (cutsceneIndex == 1)
		{
			cutsceneWait = 120;
			cutsceneIndex = 2;
			
			levelFlags |= LEVELFLAG_RAINY;
			REG_DISPCNT |= DCNT_BG3;
		}
		// Run new dialogue
		else if (cutsceneIndex == 2)
		{
			cutsceneIndex = 3;
			DLG_start((char**)&text_ch2st2, boxCount_text_ch2st2);
		}
		// Run new dialogue
		else if (cutsceneIndex == 3)
		{
			cutsceneCoroutine = 0;
		}
		
		if (!cutsceneIndex) {
			cutsceneWait = 40;
			cutsceneIndex = 1;
		}
	}
	
	
}

