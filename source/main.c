#include "game_data.h"
#include "sprites.h"

#include <maxmod.h>
#include "soundbank.h"
#include "soundbank_bin.h"

#include "physics.h"
#include "backgrounds.h"
#include "char.h"
#include "dashcrystal.h"
#include "strawb.h"
#include "dialogue.h"
#include "text.h"
#include "particle.h"
#include "char.h"
#include "transitions.h"
#include "level.h"

// Todo:
//		Use THUMB code to load all rom data and change visuals.  Probably a good idea, not sure yet
//		

#define TIMER_OFFSET	96
#define TIMER_DIV		0x3BBF

#define INTRO_WAIT			150
#define INTRO_COUNT			3


				
unsigned int tilesets[128];
unsigned int texture[8];
				
OBJ_ATTR obj_buffer[128];
OBJ_ATTR *sprite_pointer;
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;

int spriteCount, prevSpriteCount, pauseLoc, pauseArrowLoc;
int showingTime;

int intro_index, intro_wait;
	
int actor_index; // index of actor

int levelCode, colorCode, revertCode, tasCode;
int diagonalBuffer, keyIndex;
int cheatCodeBuffer[16];
int times[6];

int dreamBlockAnim;

const int levelUnlockC[16] = {
	0,
	0,
	0,
	0,
	0,
	0,
	KEY_UP,
	KEY_UP,
	KEY_DOWN,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_B,
	KEY_A,
};
const int tasHitboxC[16] = {
#ifndef __DEBUG__
	KEY_START,
	KEY_SELECT,
	KEY_UP | KEY_RIGHT,
	KEY_DOWN | KEY_LEFT,
	KEY_B | KEY_R,
	KEY_UP | KEY_START,
	KEY_B,
	KEY_LEFT,
	KEY_A,
	KEY_UP,
	KEY_LEFT | KEY_R,
	KEY_RIGHT | KEY_L,
	KEY_START,
	KEY_SELECT,
	KEY_START,
	KEY_A | KEY_B | KEY_L | KEY_R | KEY_START | KEY_SELECT | KEY_UP,

#else
	KEY_LEFT,
	KEY_RIGHT,
	KEY_A,
	KEY_A,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_A,
	KEY_A,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_A,
	KEY_A,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_A,
	KEY_A,
#endif

};
const int colorRandomC[16] = {
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	KEY_LEFT,
	KEY_UP,
	KEY_RIGHT,
	KEY_DOWN,
	KEY_UP,
	KEY_UP,
	KEY_UP,
	KEY_A,
};
const int colorRevertC[16] = {
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	KEY_DOWN,
	KEY_DOWN,
	KEY_DOWN,
	KEY_UP,
	KEY_LEFT,
	KEY_DOWN,
	KEY_RIGHT,
	KEY_A,
};

void CheatCodes() {
	
	if (diagonalBuffer){
		--diagonalBuffer;
	}
	
	int hit;
	if (hit = key_hit(0xFFF)){
		
		if (hit == KEY_UP || hit == KEY_DOWN || hit == KEY_LEFT || hit == KEY_RIGHT){
			
			if (diagonalBuffer){
				
				if (cheatCodeBuffer[keyIndex] == hit && KEY_DOWN_NOW(0xFFF) == hit){
					// Clear the buffer to read another potential diagonal input
					diagonalBuffer = 0;
					cheatCodeBuffer[keyIndex] = hit;
				}
				else{
					// Go back an input and add the diagonal to the input
					keyIndex = (keyIndex + 15) & 0xF;
					cheatCodeBuffer[keyIndex] |= hit;
				}
				
				++keyIndex;
				keyIndex &= 0xF;
			}
			else{
				cheatCodeBuffer[keyIndex] = hit;
				++keyIndex;
				keyIndex &= 0xF;
			}
			
			// If buffer is hit (meaning there's still a value in buffer), then clear the buffer.  Otherwise, set the buffer
			diagonalBuffer = 7 * !diagonalBuffer;
		}
		else{
			cheatCodeBuffer[keyIndex] = hit;
			++keyIndex;
			keyIndex &= 0xF;
			
			diagonalBuffer = 0;
		}
		
		int index = keyIndex + 15;
		
		colorCode = 0;
		levelCode = 0;
		revertCode = 0;
		tasCode = 0;
		while (index >= keyIndex) {
			colorCode += colorRandomC[index - keyIndex] == cheatCodeBuffer[index & 0xF] || colorRandomC[index - keyIndex] == 0;
			levelCode += levelUnlockC[index - keyIndex] == cheatCodeBuffer[index & 0xF] || levelUnlockC[index - keyIndex] == 0;
			revertCode += colorRevertC[index - keyIndex] == cheatCodeBuffer[index & 0xF] || colorRevertC[index - keyIndex] == 0;
			tasCode += tasHitboxC[index - keyIndex] == cheatCodeBuffer[index & 0xF] || tasHitboxC[index - keyIndex] == 0;
			
			--index;
		}
		
		if (colorCode == 16) {
			
			int rngColor = RNG() & 0x7FFF;
			hairColor[0] = rngColor;
			hairColor[1] = COLOR_LERP(rngColor, 0, 0x50);
			
			rngColor = RNG() & 0x7FFF;
			hairColor[2] = rngColor;
			hairColor[3] = COLOR_LERP(rngColor, 0, 0x50);
			
			rngColor = RNG() & 0x7FFF;
			hairColor[4] = rngColor;
			hairColor[5] = COLOR_LERP(rngColor, 0, 0x50);
			
			key_mod2(KEY_A);
			
		}
		else if (tasCode == 16) {
			levelFlags |= LEVELFLAG_TAS;
			
			memcpy(&tile_mem[4][STRAWB_SP_OFFSET	], hitbox16,	SPRITE_16x16);
			memcpy(&tile_mem[4][KEY_SP_OFFSET		], hitbox16,	SPRITE_16x16);
			memcpy(&tile_mem[4][DASHCR_SP_OFFSET	], hitboxDash1,	SPRITE_16x16);
			memcpy(&tile_mem[4][DASH_OUT_SP_OFFSET	], hitboxDash2,	SPRITE_16x16);
			memcpy(&tile_mem[4][SPRING_D_OFFSET		], springHit,	SPRITE_16x16);
		}
		if (levelCode == 16 && gamestate == GS_LEVEL) {
			UNLOCK_LEVEL(LEVELIDX_MAX - 1);
			SaveData(saveFileNumber);
			key_mod2(KEY_A);
		}
		if (revertCode == 16) {
			revertCode = 0;
			
			memcpy(&hairColor[0], &hair_pal[0], 12);
			key_mod2(KEY_A);
		}
	}
	return;
}

void LoadFile(int _file) {
	int i;
	int _display = _file & 0x3;
	int textOffset = 10;
	
	i = 32 + (_display << 8);
	se_mem[FOREGROUND_LAYER][i] = _file + 11 | 0x4000;
	
	LoadData(_file);
	
	i += 18;
	if (STRAWB_count >= 10) {
		se_mem[FOREGROUND_LAYER][i] = (STRAWB_count / 10) + 10 | 0x4000;
		se_mem[FOREGROUND_LAYER][i + 1] = (STRAWB_count % 10) + 10 | 0x4000;
	}
	else{
		se_mem[FOREGROUND_LAYER][i] = STRAWB_count + 10 | 0x4000;
	}
	int timer = GAME_speedrun_timer << ACC;
	timer /= TIMER_DIV;
	
	int hour = timer / 3600;
	i = 173 + (_display << 8);
	se_mem[FOREGROUND_LAYER][i] = (10 + hour) | 0x4000;
	
	hour = (timer / 600) % 6;
	i += 2;
	se_mem[FOREGROUND_LAYER][i] = (10 + hour) | 0x4000;
	
	hour = (timer / 60) % 10;
	i += 1;
	se_mem[FOREGROUND_LAYER][i] = (10 + hour) | 0x4000;
	
	hour = (timer / 10) % 6;
	i += 2;
	se_mem[FOREGROUND_LAYER][i] = (10 + hour) | 0x4000;
	
	hour = timer % 10;
	i += 1;
	se_mem[FOREGROUND_LAYER][i] = (10 + hour) | 0x4000;
	
	textOffset = 10;
	
	for (i = 0; i < FILENAME_LEN; ++i) {
		se_mem[FOREGROUND_LAYER][i + 34 + (_display << 8)] = fileName[i] + textOffset | 0x4000;
	}
	
	i = 128 + (_display << 8);
	if (saveFile[0] == 0xFF){
		for (timer = 0; timer < 8; ++timer){
			se_mem[FOREGROUND_LAYER][i + timer] = 4 | 0x4000;
			se_mem[FOREGROUND_LAYER][i + timer + 32] = 4 | 0x4000;
		}
		return;
	}
	
	textOffset = -77;
	char* string = (char*)chapter_names[saveFile[LEVEL_UNLOCKED_IDX]];
	
	for (timer = 0; timer < 8; ++timer){
		if (string[timer] != ' ')
			se_mem[FOREGROUND_LAYER][i + timer] = textOffset + (string[timer]) | 0x4000;
		if (string[timer + 8] != ' ')
			se_mem[FOREGROUND_LAYER][i + timer + 32] = textOffset + (string[timer + 8]) | 0x4000;
	}
	
}

void LoadEssentialArt(){
	memcpy(&tile_mem[4][4], madeline_hair, SPRITE_16x16);
	
#ifdef	__DEBUG__
	memcpy(pal_obj_mem + 16, pickup_gold_pal, pickup_palLen);	
#endif
	int tileOffset = 8;
	
	// create hair sprites procedurally
	{
		int hairSp = 0;
		int hairX, hairY, i;
		unsigned int value, darkValue;
		unsigned int *hairP = (unsigned int*)&tile_mem[4][tileOffset];
		
		int tempX = INT2FIXED(hairX);
		int tempY = INT2FIXED(hairY);
		
		for (; hairSp < HAIR_PART_COUNT; ++hairSp){
			for (hairY = 0; hairY < 16; ++hairY){
				for (hairX = 0; hairX < 16;){
					value = 0;
					darkValue = 0;
					for (i = 0; i < 8; ++i)
					{
						value >>= 4;
						darkValue >>= 4;
						
						tempX = INT2FIXED(hairX - 8) + 0x80;
						tempY = INT2FIXED(hairY - 8) + 0x80;
						
						if (FIXED_sqrt(FIXED_MULT(tempX, tempX) + FIXED_MULT(tempY, tempY)) < (0x360 - (hairSp * 148)))
							value = (value | 0x10000000);
						if (FIXED_sqrt(FIXED_MULT(tempX, tempX) + FIXED_MULT(tempY, tempY)) < (0x440 - (hairSp * 140)))
							darkValue = (darkValue | 0xB0000000);
						
						++hairX;
					}
					
					hairP[(hairY & 0x7) + ((~hairX & 0x8)) + ((hairY & 0x8) << 1) + (hairSp << 5)] = value;
					hairP[(hairY & 0x7) + ((~hairX & 0x8)) + ((hairY & 0x8) << 1) + ((hairSp + HAIR_PART_COUNT) << 5)] = darkValue;
				}
			}
			tileOffset += 8;
		}
	}
	
	
	tileOffset += 4;
	
	memcpy(&tile_mem[4][tileOffset], wing, springLen);
	tileOffset += 4;
	
	tileOffset += 4;
	
	memcpy(&tile_mem[4][tileOffset], number8x8, number8x8Len);
	
	memcpy(&tile_mem[4][SPRING_D_OFFSET], spring, SPRITE_16x16);
	memcpy(&tile_mem[4][PAUSEARROW_OFFSET], arrow, SPRITE_8x8);
	memcpy(&tile_mem[4][DASH_OUT_SP_OFFSET], dash_outline, SPRITE_16x16);
	
	memcpy(pal_obj_mem, madeline_pal, madeline_palLen);
	memcpy(&hairColor[0], &hair_pal[0], 12);
	memcpy(&skinStaminaColor[0], &stamina_pal[0], 32);
	
}
void LoadIntro(int _val) {

	REG_BG0CNT = BG_CBB(1) | BG_SBB(31) | BG_4BPP | BG_REG_32x32 | BG_PRIO(0);
	REG_BG1CNT = BG_CBB(1) | BG_SBB(30) | BG_4BPP | BG_REG_32x32 | BG_PRIO(1);
	REG_DISPCNT = DCNT_OBJ_1D | DCNT_BG0 | DCNT_MODE0 | DCNT_WIN0;
	
	switch (_val) {
	default:
		memcpy(&pal_bg_mem[0 << 4], intro_ex_pal, 32);
		memcpy(se_mem[31], intro_ex_fg, intro_ex_fgLen);
		memcpy(&tile_mem[1][0], intro_ex_tileset, intro_ex_tilesetLen);
		break;
	case 1:
		memcpy(&pal_bg_mem[0 << 4], intro_pix_pal, 32);
		memcpy(se_mem[31], intro_pix_fg, intro_pix_fgLen);
		memcpy(&tile_mem[1][0], intro_pix_tileset, intro_pix_tilesetLen);
		break;
	case 2:
		memcpy(&pal_bg_mem[0 << 4], intro_logo_pal, 32);
		memcpy(&pal_bg_mem[1 << 4], intro_logo2_pal, 32);
		memcpy(se_mem[31], intro_logo_bg, intro_logo_bgLen);
		memcpy(se_mem[30], intro_logo_fg, intro_logo_fgLen);
		memcpy(&tile_mem[1][0], intro_logo_tileset, intro_logo_tilesetLen);
		
		
		visualFlags |= DCNT_OBJ | DCNT_BG1;
		REG_BG1HOFS = -(intro_wait >> 3);
		
		REG_BG0HOFS = -0x1;
		
		break;
	}
}

int main(){
	
    mmInitDefault( (mm_addr)soundbank_bin, 8 );
	
    REG_SNDDSCNT |= SDS_AR | SDS_AL | SDS_BR | SDS_BL;// | SDS_BR | SDS_BL;
	
	ChangeHBlankIntr(EmptyInterrupt);
	
	irq_init(NULL);
	
	irq_add( II_HBLANK, Interrupt );
	irq_add( II_VBLANK, mmVBlank );
	
	GAME_fadeAmount = TRANSITION_CAP - 1;
	transition_style = (unsigned short*)sidebar_trans0;
	
	GB_PauseMusic();
	LoadEssentialArt();
	
	gamestate = nextGamestate = GS_INTRO;
	
	REG_WININ = 0xFFFF;
#ifdef __DEBUG__
	REG_WINOUT = 0xF0;
#else
	REG_WINOUT = 0x00;
#endif
	
	
	REG_WIN0H = 0x0000;
	REG_WIN0V = 0x00A0;
	

	oam_init(obj_buffer, 128);
	
	
	mmSetModuleVolume(0x140);
	
	LoadIntro(0);
	mmStart( MOD_FLATOUTLIES, MM_PLAY_LOOP );
	
	while (1){
		
		// Key input and cheat codes (to prevent anything interrupting cheat codes)
		key_poll();
		
#ifdef __DEBUG__
		forceDisplay = 1;
#endif
		
		prevSpriteCount = spriteCount;
		spriteCount = 0;
		sprite_pointer = (OBJ_ATTR*)&obj_buffer;

#if 1 // Update
		if (DIALOGUE_ENABLED) {
			DLG_update();
		}
		
		if (cutsceneCoroutine && !IS_FADING) {
			if (cutsceneWait) {
				--cutsceneWait;
			}
			else
				(*cutsceneCoroutine)();
				
		}
		if (cutsceneCoroutine || DIALOGUE_ENABLED) {
			int starting = current_chapter == 0 && lvlIndex == 0;
			
			if (!GAME_fading && ((key_hit(KEY_START) && !starting) ||
				(KEY_DOWN_NOW(KEY_START) && starting && !DLG_wait))) {
				
				START_FADE();
				nextGamestate = GS_END_CUTSCENE;
			}
			else
				key_mod(cutsceneInput);
		}
		{
			int count = 0;
			UpdateParticles(sprite_pointer, camX, camY, &count);
			sprite_pointer += count;
			spriteCount += count;
		}
		
		if (!heart_freeze){
			GB_PlayMusic();
		}
		
		// [Update]
		if (!GAME_fading || GAME_fadeAmount != TRANSITION_CAP) {
			// Update game state
			switch (gamestate) {
				// Intro logos
				case GS_INTRO: {
				
				CheatCodes();
				
				// 2000 iterations
				// Dry - 78
				
				/*
				actor_index = 0;
				for (; actor_index <= 2000; actor_index++){
					actor_id += (0x324 / 0x162) << 8;
				}
				/*/
				
				if (!(intro_wait & 0x3)){
					
					int rng = RNG();
					
					int offY = (rng % (180 << 4)) - 10;
					offY &= 0xFFFF;
					
					int velY = 0xF8 + ((rng & 0xF00) >> 8);
					velY &= 0xFF;
					offY |= (velY) << 16;
					
					velY = (rng << 4) & 0x0F0000;
					velY += 0x140000;
					
					AddParticle(0xFFC0 | 0xFF000000 | velY,
								offY | MenuSnow_L,
								0x000 | MenuSnow_S);
					
					if (!(intro_wait & 0x7)) {
						int i;
						
						for (i = 1; i < 120; i += 3) {
							particle_data[i] |= 0xC0000000;
						}
					}
					
				}
				REG_BG1HOFS = -(intro_wait >> 3);
				++intro_wait;
				
				if (!IS_FADING)
				{
					if ((intro_index < INTRO_COUNT - 1 && intro_wait >= INTRO_WAIT) || (key_hit(KEY_A | KEY_B | KEY_START | KEY_SELECT) && (intro_index == INTRO_COUNT - 1 || intro_wait > 55)))
					{
						START_FADE();
						
						break;
					}
				}//*/
				break; }
				// Save files
				case GS_FILES: {
				
				CheatCodes();
				
				if (!IS_FADING) {
					if (camX <= 90) {
						if (key_hit(KEY_UP)) {
							
							LoadFile((saveFileNumber + 6) & 0x7);
							
							if (--saveFileNumber < 0)
								saveFileNumber = 7;
						}
						if (key_hit(KEY_DOWN)) {
							
							LoadFile((saveFileNumber + 2) & 0x7);
							
							if (++saveFileNumber > 7)
								saveFileNumber = 0;
						}
						if (key_hit(KEY_B)) {
							LoadData(saveFileNumber);
							if (saveFile[0] != 0xFF) {
								camX = 104;
								REG_BG1HOFS = -48;
								enterX = 38;
								//enterY = 0;
								
								int offset;
								for (offset = 0; offset < 4; ++offset) {
									int i;
									char* string = &"are you sure you   want to delete thisfile?                        yes   no "[offset * 19];
									for (i = 0; i < 19; ++i) {
										if (string[i] == '?')
											se_mem[MIDGROUND_LAYER][i + 33 + (offset << 6)] = (49) | 0x4000;
										else if (string[i] != ' ')
											se_mem[MIDGROUND_LAYER][i + 33 + (offset << 6)] = (string[i] - 77) | 0x4000;
										else
											se_mem[MIDGROUND_LAYER][i + 33 + (offset << 6)] = 4 | 0x4000;
									}
								}
							}
						}
						
						if (key_hit(KEY_A)) {
							
							LoadData(saveFileNumber);
							
							if (saveFile[0] == 0xFF){
							
								int offset;
								for (offset = 0; offset < 4; ++offset){
									int i;
									for (i = 0; i < 19; ++i) {
										se_mem[MIDGROUND_LAYER][i + 33 + (offset << 6)] = 4 | 0x4000;
									}
									for (i = 0; i < 10; ++i) {
										se_mem[MIDGROUND_LAYER][(i << 1) + 33 + (offset << 6)] = 10 + i + (offset * 10) | 0x4000;
									}
								}
								
								visualFlags |= DCNT_BG1;
								camX = 104;
								REG_BG1HOFS = -48;
								
								enterY =  + 34 + ((saveFileNumber & 0x3) << 8);
								
								for (enterX = 0; enterX < FILENAME_LEN; ++enterX) {
									se_mem[FOREGROUND_LAYER][enterY + enterX] = 46 | 0x4000;
									fileName[enterX] = 36;
								}
								
								enterX = 0;
								enterY = 0;
							}
							else{
								START_FADE();
								
								nextGamestate = GS_LEVEL;
								current_chapter = 0;
								while (LEVEL_UNLOCKED(current_chapter + 1) && (current_chapter + 1) < LEVELIDX_MAX)
									++current_chapter;
							}
							
						}
					}
					else {
						if (saveFile[0] == 0xFF) {
							if (enterX >= 0) {
								if (key_hit(KEY_UP)) {
									enterX -= 10;
									if (enterX < 0)
										enterX += 40;
								}
								if (key_hit(KEY_DOWN)) {
									enterX += 10;
									if (enterX >= 40)
										enterX -= 40;
								}
								if (key_hit(KEY_LEFT)) {
									--enterX;
									if (enterX < 0 || (enterX % 10) == 9)
										enterX += 10;
								}
								if (key_hit(KEY_RIGHT)) {
									++enterX;
									if (!(enterX % 10))
										enterX -= 10;
								}
								
								if (key_hit(KEY_A) && enterY < FILENAME_LEN) {
									fileName[enterY] = enterX;
									
									// Set Filename Display
									se_mem[FOREGROUND_LAYER][enterY + 34 + ((saveFileNumber & 0x3) << 8)] = enterX + 10 | 0x4000;
									
									++enterY;
								}
							}
							if (key_hit(KEY_B) && enterY) {
								--enterY;
								fileName[enterY] = 36;
								
								se_mem[FOREGROUND_LAYER][enterY + 34 + ((saveFileNumber & 0x3) << 8)] = 46 | 0x4000;
							}
							else if (key_hit(KEY_B)) {
								for (enterX = 0; enterX < 5; ++enterX)
									se_mem[FOREGROUND_LAYER][enterX + 34 + ((saveFileNumber & 0x3) << 8)] = emptyFileName[enterX] + 10 | 0x4000;
								enterX = -1;
							}
							
							if (key_hit(KEY_START) && enterY && fileName[0] != 36) {
								START_FADE();
								
								nextGamestate = GS_LEVEL;
								current_chapter = 0;
							}
						}
						else {
							
							if (key_hit(KEY_LEFT | KEY_RIGHT)) {
								enterX += 3;
								enterX -= 6 * (enterX == 41);
							}
							
							if (key_hit(KEY_B) || (key_hit(KEY_A) && enterX == 38)) {
								enterX = -1;
							}
							else if (key_hit(KEY_A)) {
								enterX = -1;
								
								ResetData(saveFileNumber);
								LoadFile(saveFileNumber);
							}
						}
						
						if (enterX >= 0)
							camX += 5;
						else
							camX -= 8;
						
						if (camX > 164)
							camX = 164;
					}
					
					
				}
				
				break; }
				
				// Game paused
				case GS_PAUSED: {
				
				CheatCodes();
				
				if (key_hit(KEY_START)) {
					nextGamestate = GS_UNPAUSE;
					START_FADE();
					
					pauseLoc = 0;
				}
				if (key_hit(KEY_UP)) {
					--pauseLoc;
					if (pauseLoc < 0)
						pauseLoc = 2;
				}
				if (key_hit(KEY_DOWN)) {
					++pauseLoc;
					if (pauseLoc > 2)
						pauseLoc = 0;
				}
				if (key_hit(KEY_A)){
					if (pauseLoc < 2){
						nextGamestate = GS_UNPAUSE;
						START_FADE();
						
						if (pauseLoc == 1) {
							CHAR_Die();
						}
					}
					else {
						nextGamestate = GS_LEVEL;
						START_FADE();
						transition_style = sidebar_trans0;
					}
				}
				
				break;
				}
				// Game playing
				case GS_PLAYING: {
				//break;
				
				heart_freeze += heart_freeze && heart_freeze < 0x40;
				if (HEART_FROZEN){
					heart_freeze -= heart_freeze * (key_hit(KEY_A) && HEART_FROZEN);
					key_mod2(KEY_A);
				}
				
				if (!IS_FADING && !HEART_FREEZE){
					// update each entity
					//*
					for (actor_index = 0; actor_index < maxEntities; ++actor_index){
						
						if (!ACTOR_ENABLED(PHYS_actors[actor_index].flags))
							continue;
						
						int actor_id = PHYS_actors[actor_index].ID;
						
						switch(ACTOR_ENTITY_TYPE(actor_id))
						{
						case 0:
							CHAR_update();
							break;
						case 1:
							STRAWB_update(&PHYS_actors[actor_index]);
							break;
						case 2:
						case ENT_BACKGROUND:
							DASHCR_update(&PHYS_actors[actor_index]);
							break;
						case 4:
							BUMPER_update(&PHYS_actors[actor_index]);
							break;
						case 5:
							ZIP_update(&PHYS_actors[actor_index]);
							break;
						case ENT_CASSETTE:
						case ENT_HEART:
							CASS_update(actor_index);
							break;
						case ENT_CUTSCENE:
							
							switch (current_chapter){
							case LEVELIDX_PROLOGUE:
								if (GAME_IL_timer < 40) {
									PHYS_actors[actor_index].flags &= ~ACTOR_ACTIVE_MASK;
									
									DLG_start(&prologue_text_st, boxCount_prologue_text_st);
								}
								else if (lvlIndex == 0) {
									PHYS_actors[actor_index].flags &= ~ACTOR_ACTIVE_MASK;
								}
								else if (lvlIndex != 0 && PHYS_actors[0].x > PHYS_actors[actor_index].x){
									PHYS_actors[actor_index].flags &= ~ACTOR_ACTIVE_MASK;
									
									DLG_start(&prologue_text, boxCount_prologue_text);
									cutsceneCoroutine = DLG_prologue;
								}
								break;
							case LEVELIDX_DREAM:
								
								if (PHYS_actors[0].x > PHYS_actors[actor_index].x){
									PHYS_actors[actor_index].flags &= ~ACTOR_ACTIVE_MASK;
									
									DLG_start(&text_ch1, boxCount_text_ch1);
									cutsceneCoroutine = DLG_prologue;
								}
								break;
							case LEVELIDX_RAIN:
								
								
								if (GAME_IL_timer < 40) {
									PHYS_actors[actor_index].flags &= ~ACTOR_ACTIVE_MASK;
									cutsceneIndex = 0;
									
									DLG_start(&text_ch2st1, boxCount_text_ch2st1);
									cutsceneCoroutine = DLG_rainIntro;
									cutsceneSkipped = DLG_rainIntroEnd;
								}
								else if (lvlIndex == 0) {
									PHYS_actors[actor_index].flags &= ~ACTOR_ACTIVE_MASK;
								}
								else if (lvlIndex != 0 && PHYS_actors[0].x > PHYS_actors[actor_index].x){
									PHYS_actors[actor_index].flags &= ~ACTOR_ACTIVE_MASK;
									
									DLG_start(&text_ch2end, boxCount_text_ch2end);
									
									cutsceneCoroutine = DLG_prologue;
								}
								break;
							}
							break;
						}
					}
				}
				
				if (GAME_freeze)
					--GAME_freeze;
				else if (current_chapter != LEVELIDX_EPI && !(cutsceneCoroutine || DIALOGUE_ENABLED) && !HEART_FREEZE){
					++GAME_speedrun_timer;
					++GAME_IL_timer;
				}
				
				if (!cutsceneCoroutine && !deathAnimTimer && !heart_freeze && key_hit(KEY_START) && !IS_FADING
	#ifdef	__DEBUG__
	&& key_tri_vert() == 0
	#endif
	) {
					nextGamestate = GS_PAUSED;
					pauseLoc = 0;
					pauseArrowLoc = 16;
					START_FADE();
					
				}
				
				break; }
				// Level select
				case GS_LEVEL: {
				
				CheatCodes();
				
				if (!IS_FADING) {
					forceDisplay = 1;
					//REG_BG0VOFS = camY;
					
					if (intro_wait >= 0x74) {
						
						if (key_hit(KEY_RIGHT) && ((!CASSETTE_COLL && current_chapter != 0) || (intro_index == (current_chapter != 0) && !WINGED_UNLOCKED)));
						else if (key_hit(KEY_RIGHT | KEY_LEFT)) {
							if (key_hit(KEY_RIGHT))
								intro_index += (1 + (current_chapter == 0)) * (intro_index < 2);
							else if (key_hit(KEY_LEFT))
								intro_index -= (1 + (current_chapter == 0)) * (intro_index > 0);
							int i;
							char* name = &"a sideb sidewinged"[intro_index * 6];
							
							for (i = 3; i <= 8; ++i){
								if (name[i - 3] != ' ')
									se_mem[FOREGROUND_LAYER][0x100 + i] = (name[i - 3] - 65) | 0x4000;
								else
									se_mem[FOREGROUND_LAYER][0x100 + i] = 3 | 0x4000;
							}
							
							switch (intro_index) {
							case 0:
								se_mem[FOREGROUND_LAYER][0xA4] = ((STRAWB_COUNT / 10) + 22) | 0x4000;
								se_mem[FOREGROUND_LAYER][0xA5] = ((STRAWB_COUNT % 10) + 22) | 0x4000;
								se_mem[FOREGROUND_LAYER][0xA7] = ((STRAWB_levelMax / 10) + 22) | 0x4000;
								se_mem[FOREGROUND_LAYER][0xA8] = ((STRAWB_levelMax % 10) + 22) | 0x4000;
								
								se_mem[FOREGROUND_LAYER][0xA6] = chapter_select_bg[0xA6] | 0x4000;
								se_mem[FOREGROUND_LAYER][0xA2] = chapter_select_bg[0xA2] | 0x4000;
								se_mem[FOREGROUND_LAYER][0xA3] = chapter_select_bg[0xA3] | 0x4000;
								se_mem[FOREGROUND_LAYER][0x83] = chapter_select_bg[0x83] | 0x4000;
								se_mem[FOREGROUND_LAYER][0xC3] = chapter_select_bg[0xC3] | 0x4000;
								
								if (HEART_A_COLL) {
									se_mem[FOREGROUND_LAYER][0x2A] = 14 | 0x4000;
									se_mem[FOREGROUND_LAYER][0x2B] = 15 | 0x4000;
									se_mem[FOREGROUND_LAYER][0x4A] = 16 | 0x4000;
									se_mem[FOREGROUND_LAYER][0x4B] = 17 | 0x4000;
								}
								if (CASSETTE_COLL){
									se_mem[FOREGROUND_LAYER][0x6A] = 11 | 0x4000;
									se_mem[FOREGROUND_LAYER][0x6B] = 11 | 0x4400;
									se_mem[FOREGROUND_LAYER][0x8A] = 12 | 0x4000;
									se_mem[FOREGROUND_LAYER][0x8B] = 12 | 0x4400;
								}
								else {
									se_mem[FOREGROUND_LAYER][0x6A] = 3 | 0x4000;
									se_mem[FOREGROUND_LAYER][0x6B] = 3 | 0x4000;
									se_mem[FOREGROUND_LAYER][0x8A] = 3 | 0x4000;
									se_mem[FOREGROUND_LAYER][0x8B] = 3 | 0x4000;
								}
								if (GOLD_COLL) {
									int ex = 0x60 + (0x40 * (current_chapter != 0));
									
									se_mem[FOREGROUND_LAYER][ex + 0x0A] = 7 | 0x4000;
									se_mem[FOREGROUND_LAYER][ex + 0x0B] = 8 | 0x4000;
									se_mem[FOREGROUND_LAYER][ex + 0x2A] = 9 | 0x4000;
									se_mem[FOREGROUND_LAYER][ex + 0x2B] = 10| 0x4000;
								}
								
								break;
							case 1:
								se_mem[FOREGROUND_LAYER][0x82] = 3 | 0x4000;
								se_mem[FOREGROUND_LAYER][0xC2] = 3 | 0x4000;
								for (i = 0xA2; i <= 0xA8; ++i)
									se_mem[FOREGROUND_LAYER][i] = 3 | 0x4000;
								
								se_mem[FOREGROUND_LAYER][0x2A] = 3 | 0x4000;
								se_mem[FOREGROUND_LAYER][0x2B] = 3 | 0x4000;
								se_mem[FOREGROUND_LAYER][0x4A] = 3 | 0x4000;
								se_mem[FOREGROUND_LAYER][0x4B] = 3 | 0x4000;
								
								se_mem[FOREGROUND_LAYER][0xAA] = 3 | 0x4000;
								se_mem[FOREGROUND_LAYER][0xAB] = 3 | 0x4000;
								se_mem[FOREGROUND_LAYER][0xCA] = 3 | 0x4000;
								se_mem[FOREGROUND_LAYER][0xCB] = 3 | 0x4000;
								
								if (HEART_B_COLL){
									se_mem[FOREGROUND_LAYER][0x6A] = 14 | 0x4000;
									se_mem[FOREGROUND_LAYER][0x6B] = 15 | 0x4000;
									se_mem[FOREGROUND_LAYER][0x8A] = 16 | 0x4000;
									se_mem[FOREGROUND_LAYER][0x8B] = 17 | 0x4000;
								}
								else {
									se_mem[FOREGROUND_LAYER][0x6A] = 3 | 0x4000;
									se_mem[FOREGROUND_LAYER][0x6B] = 3 | 0x4000;
									se_mem[FOREGROUND_LAYER][0x8A] = 3 | 0x4000;
									se_mem[FOREGROUND_LAYER][0x8B] = 3 | 0x4000;
								}
								
								break;
							case 2:
								se_mem[FOREGROUND_LAYER][0x82] = 3 | 0x4000;
								se_mem[FOREGROUND_LAYER][0xC2] = 3 | 0x4000;
								for (i = 0xA2; i <= 0xA8; ++i)
									se_mem[FOREGROUND_LAYER][i] = 3 | 0x4000;
								
								if (WINGED_COLL) {
									se_mem[FOREGROUND_LAYER][0x6A] = 7 | 0x4000;
									se_mem[FOREGROUND_LAYER][0x6B] = 8 | 0x4000;
									se_mem[FOREGROUND_LAYER][0x8A] = 9 | 0x4000;
									se_mem[FOREGROUND_LAYER][0x8B] = 10| 0x4000;
								}
								else {
									se_mem[FOREGROUND_LAYER][0x6A] = 3 | 0x4000;
									se_mem[FOREGROUND_LAYER][0x6B] = 3 | 0x4000;
									se_mem[FOREGROUND_LAYER][0x8A] = 3 | 0x4000;
									se_mem[FOREGROUND_LAYER][0x8B] = 3 | 0x4000;
								}
								break;
							}
						}
						
						if (key_hit(KEY_B)) {
							intro_wait = -0x70;
						}
						
						if (key_hit(KEY_A)) {
						
							forceDisplay = 0;
							levelFlags &= LEVELFLAG_TAS;
							
							if (intro_index == 1)
								levelFlags |= LEVELFLAG_BSIDE;
							else if (intro_index == 2)
								levelFlags |= LEVELFLAG_CHALLENGE;
							
							START_FADE();
							switch (current_chapter) {
							default:
								transition_style = curtain_trans0;
							}
							
							nextGamestate = GS_PLAYING;
						}
					}
					else if (intro_wait) {
						
						if ((intro_wait & 0x7)) {
							
							int i;
							if (intro_wait > 0) {
								int offset = (intro_wait >> 3);
								
								for (i = 0; i < 10; ++i) {
									se_mem[FOREGROUND_LAYER][(i << 5) + offset] = chapter_select_bg[(i << 5) + offset];
								}
								if (offset >= 1 && offset <= 8) {
									if (chapter_names[current_chapter][offset - 1] != ' ')
										se_mem[FOREGROUND_LAYER][0x20 + offset] = (chapter_names[current_chapter][offset - 1] - 65) | 0x4000;
									if (chapter_names[current_chapter][offset + 7] != ' ')
										se_mem[FOREGROUND_LAYER][0x40 + offset] = (chapter_names[current_chapter][offset + 7] - 65) | 0x4000;
								}
								if (offset == 4) {
									se_mem[FOREGROUND_LAYER][0xA4] = ((STRAWB_COUNT / 10) + 22) | 0x4000;
								}
								if (offset == 5) {
									se_mem[FOREGROUND_LAYER][0xA5] = ((STRAWB_COUNT % 10) + 22) | 0x4000;
								}
								if (offset == 7) {
									se_mem[FOREGROUND_LAYER][0xA7] = ((STRAWB_levelMax / 10) + 22) | 0x4000;
								}
								if (offset == 8) {
									se_mem[FOREGROUND_LAYER][0xA8] = ((STRAWB_levelMax % 10) + 22) | 0x4000;
								}
								if (offset == 3 || (offset >= 5 && offset <= 8)) {
									se_mem[FOREGROUND_LAYER][0x100 + offset] = ("a side"[offset - 3] - 65) | 0x4000;
								}
								if (offset == 10 || offset == 11) {
									if (HEART_A_COLL) {
										se_mem[FOREGROUND_LAYER][0x20 + offset] = 14 + (offset - 10) | 0x4000;
										se_mem[FOREGROUND_LAYER][0x40 + offset] = 16 + (offset - 10) | 0x4000;
									}
									if (GOLD_COLL){
										int ex = 0x60 + (0x40 * (current_chapter != 0));
										se_mem[FOREGROUND_LAYER][ex + offset] = 7 + (offset - 10) | 0x4000;
										se_mem[FOREGROUND_LAYER][ex + 0x20 + offset] = 9 + (offset - 10) | 0x4000;
									}
									if (CASSETTE_COLL) {
										se_mem[FOREGROUND_LAYER][0x60 + offset] = 11 + ((offset - 10) << 10) | 0x4000;
										se_mem[FOREGROUND_LAYER][0x80 + offset] = 12 + ((offset - 10) << 10) | 0x4000;
										
									}
								}
							}
							else {
								for (i = 0; i < 10; ++i) {
									se_mem[FOREGROUND_LAYER][(i << 5) + ((8 - intro_wait) >> 3)] = 0;
								}
							}
							
						}
						REG_BG0HOFS = INT_ABS(intro_wait) + 0x08;
						REG_BG0VOFS = -0x20;
						intro_wait += 4;
					}
					else {
						if (key_hit(KEY_RIGHT) && LEVEL_UNLOCKED(current_chapter + 1)) {
							
							++current_chapter;
							if (current_chapter >= LEVELIDX_MAX) {
								current_chapter =  LEVELIDX_MAX - 1;
							}
						}
						else if (key_hit(KEY_LEFT)) {
							--current_chapter;
							if (current_chapter < 0) {
								current_chapter = 0;
							}
						}
						if (key_hit(KEY_B)) {
							START_FADE();
							nextGamestate = GS_FILES;
							forceDisplay = 0;
						}
						else if (key_hit(KEY_A)) {
							intro_index = 0;
							intro_wait = 4;
							
							switch (current_chapter) {
								case LEVELIDX_RAIN:
									STRAWB_levelMax = forsaken_strawbMax;
									break;
								case LEVELIDX_PROLOGUE:
									STRAWB_levelMax = prologue_strawbMax;
									break;
								case LEVELIDX_DREAM:
									STRAWB_levelMax = dream_strawbMax;
									break;
								case LEVELIDX_WATER:
									STRAWB_levelMax = water_strawbMax;
									break;
							}
						}
					}
				}
				
				
				break; }
				
				// End
			}
		}
		else {
		}
		
		

#ifdef __DEBUG__
		VCountLast = (REG_VCOUNT + 68) % 228;
#endif
		mmFrame();
		VBlankIntrWait();
		
		if (KEY_DOWN_NOW(KEY_RESET) == KEY_RESET){
			//HardReset();
			//continue;
		}
		//}
		++GAME_life;
		
		// [Fading] [Fade]
		if (GAME_fading || GAME_fadeAmount) {
			
			if (GAME_fading && GAME_fadeAmount < TRANSITION_CAP) {
				++GAME_fadeAmount;
				
				if (GAME_fadeAmount == TRANSITION_CAP) {
					visualFlags = REG_DISPCNT;
#ifdef __DEBUG__
					REG_DISPCNT = DCNT_OBJ;
#else
					REG_DISPCNT = 0;
#endif

				}
			}
			else if (!GAME_fading) {
				++GAME_fadeAmount;
				if (GAME_fadeAmount == (TRANSITION_CAP << 1)) {
					REG_WIN0H = 0x00F0;
					GAME_fadeAmount = 0;
				}
			}
			
			if (GAME_fadeAmount >= TRANSITION_CAP && GAME_fading) {
				
				switch (nextGamestate){
					// Level select
				case GS_LEVEL: {
					
					STRAWB_tempCount = 0;
					SaveData(saveFileNumber);
					
					LeaveToMainMenu();
					
					GAME_fading = 0;
					
					
					intro_index = 0;
					intro_wait = 0;
					
					break; }
					// File select
				case GS_FILES: {
					
					strawbUI_Pos = 0;
					strawbUI_Offset1 = 0;
					strawbUI_Offset2 = 0;
					strawbUI_speed1 = 0;
					strawbUI_speed2 = 0;
					
					int textOffset = 0;
					
					REG_BG0CNT = BG_CBB(1) | BG_SBB(31) | BG_4BPP | BG_REG_32x32 | BG_PRIO(1);
					REG_BG1CNT = BG_CBB(1) | BG_SBB(30) | BG_4BPP | BG_REG_32x32 | BG_PRIO(0);
					
					visualFlags = DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3 | DCNT_MODE0 | DCNT_WIN0;
					
					memcpy(&pal_bg_mem[4 << 4], menu_pal, 32);
					memcpy(se_mem[FOREGROUND_LAYER], files_bg, files_bgLen);
					memcpy(&tile_mem[1][0], files_tileset, textOffset = files_tilesetLen);
					
					memcpy(se_mem[MIDGROUND_LAYER], files_fg, files_fgLen);
					
					textOffset >>= 5;
					
					memcpy(&tile_mem[1][textOffset], dialogue_font, dialogue_fontLen);
					
					int i;
					for (i = 0; i < 3; ++i)
						LoadFile(i);
					LoadFile(7);
					
					camY = (saveFileNumber << 6) - 48;
					REG_BG0VOFS = camY;
					REG_BG0HOFS = 0;
				
					camX = 80;
					REG_BG1VOFS = camX;
					memcpy(&pal_obj_mem[1 << 4], pause1_pal, 32);
					
					GAME_fading = 0;
					
					current_chapter = lvlIndex;
					DisplayFileBG();
					
					break; }
					// Loading levels [load] [loading]
				case GS_PLAYING: {
				
				#ifdef __DEBUG__
				VCountLast = 0;
				enabled = 1;
				#endif
				switch (GAME_loadIndex) {
				case 0:
					levelFlags &= ~LEVELFLAG_BEGINNING;
					
					if (gamestate != GS_PLAYING) {
						for (strawbUI_Pos = 0; strawbUI_Pos < 15; ++strawbUI_Pos) {
							STRAWB_tempColl[strawbUI_Pos] = 0;
						}
						
						// Set the character's actor to the first.  This *should* never change
						CHAR_phys = &PHYS_actors[0];
						// Set character active, and set width and height
						PHYS_actors[0].flags = ACTOR_ACTIVE_MASK;
						PHYS_actors[0].velX = 0;
						PHYS_actors[0].velY = 0;
						PHYS_actors[0].width = NORMAL_HITBOX_W;
						PHYS_actors[0].height = NORMAL_HITBOX_H;
					
						strawbUI_Pos = 0;
						strawbUI_Offset1 = 0;
						strawbUI_Offset2 = 0;
						strawbUI_speed1 = 0;
						strawbUI_speed2 = 0;
						StrawbUI(0);
						STRAWB_display = 0;
						
						GAME_IL_timer = 0;
						GAME_music_beat = 0;
						levelFlags |= LEVELFLAG_BEGINNING;
						
						DisplayFileBG();
						
						lvlIndex = 0;
						enterX = 0;
						enterY = 1;
						
						if (current_chapter == LEVELIDX_WATER)
							enterY = -1;
						break;
					}
					
					GAME_loadIndex = 1;
					
				case 1:
					
					if (levelFlags & LEVELFLAG_BSIDE)
						LoadCollisionData(chapter_pointersBside[current_chapter][lvlIndex]);
					else
						LoadCollisionData(chapter_pointers[current_chapter][lvlIndex]);
					break;
					
				case 2:
					LoadInVisualDataFG();
					
					break;
				case 3:
					LoadInVisualDataBG();
					
					LoadInEntityData();
					
					break;
				case 4:
					if (gamestate == GS_PLAYING){
						CHAR_on_transition();
					}
					else {
						STRAWB_levelCount = STRAWB_COUNT;
						InitializeChapter();
					}
					InitializeRoom();
					
					break;
				case 5:
					
					//Save player's location
					CHAR_save_loc();
					// Initialize character for start of level
					memcpy(pal_obj_mem, madeline_pal, madeline_palLen);
					CHAR_LoadLevel();
					
					SetCamPos();
					prevCamX = camX;
					prevCamY = camY;
					FillCam(camX, camY);
					
					GAME_fading = 0;
					
					break;
				}
				
				#ifdef __DEBUG__
				times[GAME_loadIndex] = VCountLast;
				#endif
				
				++GAME_loadIndex;
				break; }
					// Unpause
				case GS_UNPAUSE: {
					
					InitializeRoom();
					ClosePause();
					set_hair_color();
					
					SetCamPos();
					prevCamX = camX;
					prevCamY = camY;
					
					FillCam(camX, camY);
					
					GAME_fading = 0;
					
					break; }
					// Pause Game
				case GS_PAUSED: {
					
					OpenPause();
					GB_PauseMusic();
					
					GAME_fading = 0;
					
					
					break; }
					// Reset Cam after respawn
				case GS_RESPAWN: {
				
					CHAR_LoadLevel();
					CHAR_Restart();
					ResetLevel();
					deathAnimTimer = 30;
					
					SetCamPos();
					prevCamX = camX;
					prevCamY = camY;
					
					FillCam(camX, camY);
					
					GAME_fading = 0;
					
					break;}
					// End level from playing
				case GS_END_LEVEL: {
						
					GAME_fading = 0;
					
					intro_wait = 0;
					
					if (!LEVEL_UNLOCKED(current_chapter + 1)){
						UNLOCK_LEVEL(current_chapter + 1);
					}
					
					STRAWB_tempCount = 0;
					SaveData(saveFileNumber);
					
					LeaveToMainMenu();
						
					break; }
					// End cutscene
				case GS_END_CUTSCENE: {
					
					if (cutsceneSkipped) {
						cutsceneSkipped();
					}
					cutsceneCoroutine = 0;
					
					if (END_CUTSCENE) {
						nextGamestate = GS_END_LEVEL;
					}
					else {
						GAME_fading = 0;
						
					}
					
					DLG_end();
					dialogueFlags = 0;
					
					break; }
					// Intro logos
				case GS_INTRO: {
					
					++intro_index;
					intro_wait = 0;
					
					if (intro_index < INTRO_COUNT) {
						
						LoadIntro(intro_index);
						GAME_fading = 0;
					}
					else {
						nextGamestate = GS_FILES;
					}
					
					break; }
				}
				
				if (!GAME_fading){
					gamestate = nextGamestate & 0xF;
					
					REG_DISPCNT = (visualFlags | DCNT_WIN0);
					if (nextGamestate != GS_INTRO){
						ClearParticles();
					}
				}
			}
		}
		
		// [Render]
		if (!GAME_fading || GAME_fadeAmount != TRANSITION_CAP) {
			switch (gamestate) {
				// Save files
				case GS_FILES: {
				
				VisualizeCamera();
				
				int offset = (GAME_life >> 4) & 0x1;
				if (camX > 90 && enterX >= 0) {
					
					int cursorX = ((enterX % 10) * 16) + 50 - offset;
					int cursorY = ((enterX / 10) * 16) - camX + 8;
					
					obj_set_attr(sprite_pointer,
					ATTR0_SQUARE | ATTR0_Y(cursorY),
					ATTR1_SIZE_8x8 | ATTR1_X(cursorX),
					ATTR2_PALBANK(1) | (PAUSEARROW_OFFSET));
					
					++sprite_pointer;
					++spriteCount;
				}
				
				int needed = (saveFileNumber << 6) - 48;
				int diff = needed - camY;
				
				while (diff > 128)
					diff -= 256;
				while (diff < -128)
					diff += 256;
				
				camY += INT_SIGN(diff) * 8;
				
				REG_BG0VOFS = camY;
				REG_BG1VOFS = camX;
				
				if (camX <= 90) {
					obj_set_attr(sprite_pointer,
					ATTR0_SQUARE | ATTR0_Y(76),
					ATTR1_SIZE_8x8 | ATTR1_X(168 - offset) | ATTR1_FLIP(1),
					ATTR2_PALBANK(1) | (PAUSEARROW_OFFSET));
					
					++sprite_pointer;
					++spriteCount;
				}
				break; }
				
				// Game paused
				case GS_PAUSED: {
				
				int pos = (pauseLoc << 5) + 16;
				
				pauseArrowLoc += (INT_SIGN(pos - pauseArrowLoc) * 8) * (pauseArrowLoc != pos);
				
				obj_set_attr(sprite_pointer,
				ATTR0_SQUARE | ATTR0_Y(pauseArrowLoc),
				ATTR1_SIZE_8x8 | ATTR1_X(((GAME_life >> 4) & 0x1)),
				ATTR2_PALBANK(0) | (PAUSEARROW_OFFSET));
				
				++sprite_pointer;
				++spriteCount;
				
				break; }
				// Game playing
				case GS_PLAYING: {
				
				SetCamPos();
				
				// load in tilemap around camera (only the ones that need to be updated
				MoveCam(prevCamX, prevCamY, camX, camY);
				
				if (current_chapter == LEVELIDX_DREAM && !HEART_FREEZE)
				{
					++dreamBlockAnim;
					dreamBlockAnim &= 0x7F;
					
					int val = (camX & 0x1C) << 4;
					
					// Get the stary animated texture
					memcpy(texture, &dreamblock_tex[(dreamBlockAnim & 0x38) + val], 32);
					
					// X value location
					int xVal = ((camY) >> 2) + (dreamBlockAnim >> 4);
					int index = 128;
					
					while (--index >= 0){
						++xVal;
						tilesets[index] = dreamblock_tile[index] | texture[(xVal) & 0x7];
					}
					
					memcpy(&tile_mem[FG_TILESET][128], tilesets, 512);
					
				}
				
				prevCamX = camX;
				prevCamY = camY;
				
				CHAR_updateAnim();
				
				VisualizeCamera();
				
				forceDisplay |= (lvlIndex == currentMax - 1);
				
				// Show gold berry for challenge mode
				if (CHALLENGE_MODE) {
					obj_set_attr(sprite_pointer,
					ATTR0_SQUARE | ATTR0_Y(0),
					ATTR1_SIZE_16x16 | ATTR1_X(8),
					ATTR2_PALBANK(1) | (STRAWB_SP_OFFSET));
					
					obj_set_attr(sprite_pointer + 1,
					ATTR0_SQUARE | ATTR0_Y(0),
					ATTR1_SIZE_16x16 | ATTR1_X(-4) | ATTR1_FLIP(1),
					ATTR2_PALBANK(1) | (STRAWB_SP_OFFSET + 4));
					
					obj_set_attr(sprite_pointer + 2,
					ATTR0_SQUARE | ATTR0_Y(0),
					ATTR1_SIZE_16x16 | ATTR1_X(20) | ATTR1_FLIP(0),
					ATTR2_PALBANK(1) | (STRAWB_SP_OFFSET + 4));
					
					spriteCount += 3;
					sprite_pointer += 3;
				}
				// Render actors
				for (actor_index = 0; actor_index < maxEntities && gamestate != GS_PAUSED; ++actor_index){
					int count = 0;
				
					if (!ACTOR_ENABLED(PHYS_actors[actor_index].flags))
						continue;
					
					int actor_id = PHYS_actors[actor_index].ID;
					
					switch(ACTOR_ENTITY_TYPE(actor_id)) {
						case 0:
							CHAR_render(sprite_pointer, camX, camY, &count);
							break;
						case ENT_STRAWB:
							STRAWB_render(sprite_pointer, camX, camY, &count, &PHYS_actors[actor_index]);
							break;
						case ENT_SPRING:
							SPRING_render(sprite_pointer, camX, camY, &count, &PHYS_actors[actor_index]);
							break;
						case ENT_DASH:
							DASHCR_render(sprite_pointer, camX, camY, &count, &PHYS_actors[actor_index]);
							break;
						case ENT_BUMPER:
							BUMPER_render(sprite_pointer, camX, camY, &count, &PHYS_actors[actor_index]);
							break;
						case ENT_ZIP:
							ZIP_render(sprite_pointer, camX, camY, &count, &PHYS_actors[actor_index]);
							break;
						case ENT_KEY:
							KEY_render(sprite_pointer, camX, camY, &count, &PHYS_actors[actor_index]);
							break;
						case ENT_CASSETTE:
						case ENT_HEART:
							CASS_render(sprite_pointer, camX, camY, &count, &PHYS_actors[actor_index]);
							break;
						case ENT_BACKGROUND:
							BG_TOGG_render(sprite_pointer, camX, camY, &count, &PHYS_actors[actor_index]);
							break;
						case ENT_FLAG:
							count = 1;
							
							obj_set_attr(sprite_pointer,
							ATTR0_SQUARE | ATTR0_Y(GetActorY(PHYS_actors[actor_index].y, camY)),
							ATTR1_SIZE_16x16 | ATTR1_X(GetActorX(PHYS_actors[actor_index].x, camX)),
							ATTR2_PALBANK(15) | flagIndex | ATTR2_PRIO(CHAR_PRIORITY + 1));
							
							
							break;
					}
					sprite_pointer += count;
					spriteCount += count;
				}
				
				break; }
				// Level select
				case GS_LEVEL: {
				
				enterX = FIXED_APPROACH(enterX, (chapter_map_locations[(current_chapter << 1) + 0] - (screenWidth  >> 1)), 0x300);
				enterY = FIXED_APPROACH(enterY, (chapter_map_locations[(current_chapter << 1) + 1] - (screenHeight >> 1)), 0x300);
				
				camX = enterX - INT2FIXED(112);
				camY = enterY - INT2FIXED(104);
				
				SetMapCamPos();
				
				obj_set_attr(sprite_pointer,
				ATTR0_SQUARE | ATTR0_Y(FIXED2INT(enterY - camY)),
				ATTR1_SIZE_16x16 | ATTR1_X(FIXED2INT(enterX - camX)),
				ATTR2_PALBANK(1) | STRAWB_SP_OFFSET);
				
				++sprite_pointer;
				++spriteCount;
				
				if (intro_wait) {
					
					int offset = INT_ABS(intro_wait);
					
					if (intro_index > 0) {
						obj_set_attr(sprite_pointer,
						ATTR0_SQUARE | ATTR0_Y(96),
						ATTR1_SIZE_8x8 | ATTR1_X(260 - offset + ((GAME_life >> 4) & 0x1)) | ATTR1_FLIP(1),
						ATTR2_PALBANK(2) | (PAUSEARROW_OFFSET));
						
						++sprite_pointer;
						++spriteCount;
					}
					
					if (intro_index < 2 && (CASSETTE_COLL || current_chapter == 0) && (intro_index != (current_chapter != 0) || WINGED_UNLOCKED)) {
						obj_set_attr(sprite_pointer,
						ATTR0_SQUARE | ATTR0_Y(96),
						ATTR1_SIZE_8x8 | ATTR1_X(332 - offset - ((GAME_life >> 4) & 0x1)),
						ATTR2_PALBANK(2) | (PAUSEARROW_OFFSET));
					}
					
					++sprite_pointer;
					++spriteCount;
				}
				
				break; }
				
				// End
			}
		}
		
		
		//GAME_fading = 1;
		
		
		
		int count = (STRAWB_count * (gamestate != GS_PLAYING)) +
				(STRAWB_tempCount * (gamestate == GS_PLAYING));
		
		// [HUD]
		if (gamestate != GS_PAUSED && (count != STRAWB_display || strawbUI_Pos || forceDisplay)) {
			StrawbUI(forceDisplay);
#ifdef __DEBUG__
			strawbUI_Pos = 0x1000;
#endif
			
			showingTime = forceDisplay;
			
			if (!CHALLENGE_MODE || gamestate != GS_PLAYING){
				
				
				int digit1 = count % 10;
				int digit10 = count / 10;
				
				
				//strawberry icon
				obj_set_attr(sprite_pointer,
				ATTR0_SQUARE | ATTR0_Y(FIXED2INT(strawbUI_Pos) - 16),
				ATTR1_SIZE_16x16 | ATTR1_X(8),
				ATTR2_PALBANK(1) | STRAWB_SP_OFFSET);
				
				// digit 10
				obj_set_attr(sprite_pointer + 1,
				ATTR0_SQUARE | ATTR0_Y(FIXED2INT(strawbUI_Pos + strawbUI_Offset2) - 10),
				ATTR1_SIZE_8x8 | ATTR1_X(22),
				ATTR2_PALBANK(1) | TEXT_SP_OFFSET + digit10);
				
				// digit 1
				obj_set_attr(sprite_pointer + 2,
				ATTR0_SQUARE | ATTR0_Y(FIXED2INT(strawbUI_Pos + strawbUI_Offset1) - 10),
				ATTR1_SIZE_8x8 | ATTR1_X(30),
				ATTR2_PALBANK(1) | TEXT_SP_OFFSET + digit1);
				
				spriteCount += 3;
				sprite_pointer += 3;
			}
			
			// if at the summit, display speedrun timer
			if (showingTime) {
				
				unsigned long timer = (GAME_speedrun_timer * (gamestate != GS_PLAYING)) + 
									  (GAME_IL_timer	   * (gamestate == GS_PLAYING));
				timer <<= ACC;
				//timer = timer / 0x3BBF;
				timer = timer / TIMER_DIV;
				
				int hour = timer / 3600;
				int minute10 = (timer / 600) % 6;
				int minute1 = (timer / 60) % 10;
				int second10 = (timer / 10) % 6;
				int second1 = timer % 10;
				
				// x:00:00
				obj_set_attr(sprite_pointer,
				ATTR0_SQUARE | ATTR0_Y(FIXED2INT(strawbUI_Pos) - 10),
				ATTR1_SIZE_8x8 | ATTR1_X(TIMER_OFFSET),
				ATTR2_PALBANK(1) | TEXT_SP_OFFSET + hour);
				
				
				obj_set_attr(sprite_pointer + 1,
				ATTR0_SQUARE | ATTR0_Y(FIXED2INT(strawbUI_Pos) - 10),
				ATTR1_SIZE_8x8 | ATTR1_X(TIMER_OFFSET + 8),
				ATTR2_PALBANK(1) | TEXT_SP_OFFSET + 10);
				
				
				// 0:x0:00
				obj_set_attr(sprite_pointer + 2,
				ATTR0_SQUARE | ATTR0_Y(FIXED2INT(strawbUI_Pos) - 10),
				ATTR1_SIZE_8x8 | ATTR1_X(TIMER_OFFSET + 16),
				ATTR2_PALBANK(1) | TEXT_SP_OFFSET + minute10);
				// 0:0x:00
				obj_set_attr(sprite_pointer + 3,
				ATTR0_SQUARE | ATTR0_Y(FIXED2INT(strawbUI_Pos) - 10),
				ATTR1_SIZE_8x8 | ATTR1_X(TIMER_OFFSET + 24),
				ATTR2_PALBANK(1) | TEXT_SP_OFFSET + minute1);
				
				
				obj_set_attr(sprite_pointer + 4,
				ATTR0_SQUARE | ATTR0_Y(FIXED2INT(strawbUI_Pos) - 10),
				ATTR1_SIZE_8x8 | ATTR1_X(TIMER_OFFSET + 32),
				ATTR2_PALBANK(1) | TEXT_SP_OFFSET + 10);
				
				// 0:00:x0
				obj_set_attr(sprite_pointer + 5,
				ATTR0_SQUARE | ATTR0_Y(FIXED2INT(strawbUI_Pos) - 10),
				ATTR1_SIZE_8x8 | ATTR1_X(TIMER_OFFSET + 40),
				ATTR2_PALBANK(1) | TEXT_SP_OFFSET + second10);
				//0:00:0x
				obj_set_attr(sprite_pointer + 6,
				ATTR0_SQUARE | ATTR0_Y(FIXED2INT(strawbUI_Pos) - 10),
				ATTR1_SIZE_8x8 | ATTR1_X(TIMER_OFFSET + 48),
				ATTR2_PALBANK(1) | TEXT_SP_OFFSET + second1);
				
				int death = (DEATH_count	 * (gamestate != GS_PLAYING)) + 
							(DEATH_tempCount * (gamestate == GS_PLAYING));
				
#ifdef	__DEBUG__
				death = VCountLast;
#endif
				
				minute1  =  death / 100;
				second10 = (death / 10) % 10;
				second1  =  death % 10;
				
#ifdef	__DEBUG__
				if (GAME_fadeAmount >= TRANSITION_CAP - 1 && GAME_fading) {
					obj_set_attr(sprite_pointer + 7,
					ATTR0_SQUARE | ATTR0_Y(FIXED2INT(strawbUI_Pos) - 10),
					ATTR1_SIZE_8x8 | ATTR1_X(TIMER_OFFSET + 104),
					ATTR2_PALBANK(1) | TEXT_SP_OFFSET + GAME_loadIndex);
					spriteCount++;
					sprite_pointer++;
				}
#else
				obj_set_attr(sprite_pointer + 7,
				ATTR0_SQUARE | ATTR0_Y(FIXED2INT(strawbUI_Pos) - 10),
				ATTR1_SIZE_8x8 | ATTR1_X(TIMER_OFFSET + 104),
				ATTR2_PALBANK(1) | TEXT_SP_OFFSET + 11);
				spriteCount++;
				sprite_pointer++;
#endif
				
				
				obj_set_attr(sprite_pointer + 7,
				ATTR0_SQUARE | ATTR0_Y(FIXED2INT(strawbUI_Pos) - 10),
				ATTR1_SIZE_8x8 | ATTR1_X(TIMER_OFFSET + 112),
				ATTR2_PALBANK(1) | TEXT_SP_OFFSET + minute1);
				
				obj_set_attr(sprite_pointer + 8,
				ATTR0_SQUARE | ATTR0_Y(FIXED2INT(strawbUI_Pos) - 10),
				ATTR1_SIZE_8x8 | ATTR1_X(TIMER_OFFSET + 120),
				ATTR2_PALBANK(1) | TEXT_SP_OFFSET + second10);
				
				obj_set_attr(sprite_pointer + 9,
				ATTR0_SQUARE | ATTR0_Y(FIXED2INT(strawbUI_Pos) - 10),
				ATTR1_SIZE_8x8 | ATTR1_X(TIMER_OFFSET + 128),
				ATTR2_PALBANK(1) | TEXT_SP_OFFSET + second1);
				
				spriteCount +=		10;
				sprite_pointer +=	10;
			}
		}
		else {
			showingTime = 0;
		}
		
		if (!(levelFlags & LEVELFLAG_TAS)){
			switch ((GAME_life) & 0x7) {
			case 0:
				memcpy(&tile_mem[4][44],				&wing			[((GAME_life >> 3) & 0x7)	<< 5],	SPRITE_16x16);
				
				if (flagIndex != 0)
					memcpy(&tile_mem[4][flagIndex], 		&secret_flag_a	[((GAME_life >> 3) & 0x3)	<< 5],	SPRITE_16x16);
				break;
			case 2:
				memcpy(&tile_mem[4][48], 				&dash_crystal	[((GAME_life >> 3) % 5)	<< 5],	SPRITE_16x16);
				break;
			case 4:
				memcpy(&tile_mem[4][STRAWB_SP_OFFSET],	&strawb			[((GAME_life >> 3) % 7)		<< 5],	SPRITE_16x16);
				break;
			case 6:
				memcpy(&tile_mem[4][68],				&key			[((GAME_life >> 3) & 0x7)	<< 5],	SPRITE_16x16);
			}
		}
#endif
		
#if 1 // Rendering
		if (gamestate == GS_PAUSED) {
		
			prevCamX = camX;
			prevCamY = camY;
			VisualizeCamera();
		}
		
		if (spriteCount < prevSpriteCount){
			int i = spriteCount;
			for (; i < prevSpriteCount; ++i)
			{
				sprite_pointer->attr0 = 0x0200;
				++sprite_pointer;
			}
		}
		
		oam_copy(oam_mem, obj_buffer, 128);
		
		//REG_DISPCNT = 0;
#endif
	}
}
