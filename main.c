//
// obj_demo.c
// testing various sprite related things
//
// (20031003 - 20060924, Cearn)

// TODO:

#include <string.h>
#include "physics.h"
#include "char.h"
#include "toolbox.h"
#include "dashcrystal.h"
#include "strawb.h"
#include "dialogue.h"
#include "text.h"
#include "particle.h"
#include "char.h"

#define TIMER_OFFSET	96
#define TIMER_DIV		0x3BBF

OBJ_ATTR obj_buffer[128];
OBJ_ATTR *sprite_pointer;
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;

int spriteCount, prevSpriteCount, pauseLoc, pauseArrowLoc;
char showingTime = 0;
char GAME_fading;

#define INTRO_WAIT			150
#define INTRO_COUNT			2
int intro_index, intro_wait = 0;

void video_sync_test(){
	while(REG_VCOUNT > 160);	// wait till VDraw
	while(REG_VCOUNT < 160);
}

char levelCode = 0, colorCode = 0, revertCode = 0;

const short levelUnlockC[10] = {
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
const short colorRandomC[8] = {
	KEY_LEFT,
	KEY_UP,
	KEY_RIGHT,
	KEY_DOWN,
	KEY_UP,
	KEY_UP,
	KEY_UP,
	KEY_A,
};
const short colorRevertC[8] = {
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
	
	if (key_hit(levelUnlockC[levelCode])) {
		++levelCode;
		if (levelCode == 10) {
			levelCode = 0;
			UNLOCK_LEVEL(LEVELIDX_MAX - 1);
			key_mod2(KEY_A);
		}
	}
	else if (key_hit(~levelUnlockC[levelCode])) {
		levelCode = 0;
	}
	
	if (key_hit(colorRandomC[colorCode])) {
		++colorCode;
		if (colorCode == 8) {
			colorCode = 0;
			
			int rngColor = RNG();
			hairColor[0] = rngColor & 0x7FFFFF;
			hairColor[1] = COLOR_LERP(rngColor, 0, 0xD0);
			
			rngColor = RNG();
			hairColor[2] = rngColor & 0x7FFFFF;
			hairColor[3] = COLOR_LERP(rngColor, 0, 0xD0);
			
			rngColor = RNG();
			hairColor[4] = rngColor & 0x7FFFFF;
			hairColor[5] = COLOR_LERP(rngColor, 0, 0xD0);
			
			key_mod2(KEY_A);
		}
	}
	else if (key_hit(~colorRandomC[colorCode])) {
		colorCode = 0;
	}
	
	if (key_hit(colorRevertC[revertCode])) {
		++revertCode;
		if (revertCode == 8) {
			revertCode = 0;
			
			memcpy(&hairColor[0], &hair_pal[0], 12);
			
			key_mod2(KEY_A);
		}
	}
	else if (key_hit(~colorRevertC[revertCode])) {
		revertCode = 0;
	}
}

void LoadFile(int _file, int _display) {
	int i;
	int textOffset = files_tilesetLen >> 5;
	textOffset += _display * (8 + FILENAME_LEN);
	
	memcpy(&tile_mem[1][textOffset], &dialogue_font[(_file + 1) << 3], SPRITE_8x8);
	
	LoadData(_file);
	
	for (i = 0; i < FILENAME_LEN; ++i) {
		memcpy(&tile_mem[1][textOffset + i + 8], &dialogue_font[fileName[i] << 3], SPRITE_8x8);
	}
	
	if (STRAWB_count >= 10) {
		memcpy(&tile_mem[1][textOffset + 1], &dialogue_font[(STRAWB_count / 10) << 3], SPRITE_8x8);
		memcpy(&tile_mem[1][textOffset + 2], &dialogue_font[(STRAWB_count % 10) << 3], SPRITE_8x8);
	}
	else{
		memcpy(&tile_mem[1][textOffset + 1], &dialogue_font[(STRAWB_count) << 3], SPRITE_8x8);
		memcpy(&tile_mem[1][textOffset + 2], &dialogue_font[36 << 3], SPRITE_8x8);
	}
	int timer = GAME_speedrun_timer << ACC;
	timer /= TIMER_DIV;
	
	int hour = timer / 3600;
	int minute10 = (timer / 600) % 6;
	int minute1 = (timer / 60) % 10;
	int second10 = (timer / 10) % 6;
	int second1 = timer % 10;
	
	memcpy(&tile_mem[1][textOffset + 3], &dialogue_font[hour	 << 3], SPRITE_8x8);
	memcpy(&tile_mem[1][textOffset + 4], &dialogue_font[minute10 << 3], SPRITE_8x8);
	memcpy(&tile_mem[1][textOffset + 5], &dialogue_font[minute1	 << 3], SPRITE_8x8);
	memcpy(&tile_mem[1][textOffset + 6], &dialogue_font[second10 << 3], SPRITE_8x8);
	memcpy(&tile_mem[1][textOffset + 7], &dialogue_font[second1	 << 3], SPRITE_8x8);
	
}

void LoadEssentialArt(){
	memcpy(&tile_mem[4][0], madeline, madelineLen);
	
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
	
	memcpy(&tile_mem[4][SPRING_D_OFFSET], spring, springLen);
	memcpy(&tile_mem[4][PAUSEARROW_OFFSET], arrow, arrowLen);
	
	memcpy(directColors + 256, madeline_pal, madeline_palLen);
	memcpy(&hairColor[0], &hair_pal[0], 12);
	memcpy(&directColors[256 + (SPRING_PAL << 4)], bumper_pal, madeline_palLen);
}
void LoadIntro(int _val) {

	REG_BG0CNT = BG_CBB(1) | BG_SBB(31) | BG_4BPP | BG_REG_32x32 | BG_PRIO(3);
	REG_DISPCNT=  DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | DCNT_MODE0;
	
	switch (_val) {
	default:
		memcpy(&directColors[0 << 4], intro_ex_pal, 32);
		memcpy(se_mem[31], intro_ex_fg, intro_ex_fgLen);
		memcpy(&tile_mem[1][0], intro_ex_tileset, intro_ex_tilesetLen);
		break;
	case 1:
		memcpy(&directColors[0 << 4], intro_pix_pal, 32);
		memcpy(se_mem[31], intro_pix_fg, intro_pix_fgLen);
		memcpy(&tile_mem[1][0], intro_pix_tileset, intro_pix_tilesetLen);
		break;
	}
}

int main(){
	//FrasInstall(FRAS_MEDIUM_FRQ);
	//FrasPlayMod(&CountryModInfo);
	GAME_fadeAmount = 0xFF;
	
	GB_init_soundchip();
	LoadEssentialArt();
	
	int actor_index; // index of actor
	int actor_id; // actor's id (type, index, etc)
	
	gamestate = GS_INTRO;
	REG_IME = 0;
	REG_DISPSTAT |= 0x10 | 0x2;
	REG_IE |= IRQ_HBLANK;

	oam_init(obj_buffer, 128);
	
	LoadIntro(0);
	
	
	gameLoop:
	
	++GAME_life;
	video_sync_test(); // v sync
	
	key_poll(); // get key input
	CheatCodes();
	
	if (dialogueEnabled) {
		DLG_update();
	}
	
	if (cutsceneCoroutine && !IS_FADING) {
		if (cutsceneWait) {
			--cutsceneWait;
		}
		else
			(*cutsceneCoroutine)();
			
	}
	if (cutsceneCoroutine || dialogueEnabled) {
		if ((key_hit(KEY_START) && !(current_chapter == 0 && GAME_speedrun_timer < 130)) ||
			(KEY_DOWN_NOW(KEY_START) && current_chapter == 0 && GAME_speedrun_timer == 100)) {
			GAME_fading = 1;
			FinishLevel();
		}
		else
			key_mod(cutsceneInput);
	}
	
	// get sprite array pointer
	sprite_pointer = (OBJ_ATTR*)&obj_buffer;
	spriteCount = 0;
	
	int forceDisplay = 0;
	
	if (gamestate == GS_INTRO) {
		if (!IS_FADING)
		{
			++intro_wait;
			if (intro_wait >= INTRO_WAIT || key_hit(KEY_A))
			{
				intro_wait = 0;
				GAME_fading = 1;
			}
		}
	}
	else if (gamestate == GS_FILES) {
		
		int offset = (GAME_life >> 4) & 0x1;
		
		if (!IS_FADING) {
			if (camX <= 90) {
				if (key_hit(KEY_UP)) {
					
					--saveFileNumber;
					if (saveFileNumber < 0)
						saveFileNumber = 3;
				}
				if (key_hit(KEY_DOWN)) {
					
					++saveFileNumber;
					if (saveFileNumber > 3)
						saveFileNumber = 0;
				}
				if (key_hit(KEY_B)) {
					GAME_fading = 1;
					ResetData(saveFileNumber);
				}
				if (key_hit(KEY_A) || key_hit(KEY_START)) {
					
					LoadData(saveFileNumber);
					
					if (saveFile[0] == 0xFF){
						REG_DISPCNT = DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_MODE0;
						camX = 104;
						REG_BG1HOFS = -48;
						
						enterY = (files_tilesetLen >> 5) + (saveFileNumber * (8 + FILENAME_LEN)) + 8;
						
						for (enterX = 0; enterX < FILENAME_LEN; ++enterX) {
							memcpy(&tile_mem[1][enterY + enterX], &dialogue_font[36 << 3], SPRITE_8x8);
							fileName[enterX] = 36;
						}
						
						enterX = 0;
						enterY = 0;
					}
					else{
						GAME_fading = 1;
						nextGamestate = GS_LEVEL;
						current_chapter = 0;
						while (LEVEL_UNLOCKED(current_chapter + 1) && (current_chapter + 1) < LEVELIDX_MAX)
							++current_chapter;
					}
					
				}
			}
			else {
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
						memcpy(&tile_mem[1][(files_tilesetLen >> 5) + (saveFileNumber * (8 + FILENAME_LEN)) + 8 + enterY], &dialogue_font[enterX << 3], SPRITE_8x8);
						
						++enterY;
					}
					
					if (enterY < FILENAME_LEN) {
						obj_set_attr(sprite_pointer,
						ATTR0_SQUARE | ATTR0_Y(56),
						ATTR1_SIZE_8x8 | ATTR1_X(20 + (enterY << 3) + offset) | ATTR1_FLIP(1),
						ATTR2_PALBANK(1) | (PAUSEARROW_OFFSET));
						
						++sprite_pointer;
						++spriteCount;
					}
				}
				if (key_hit(KEY_B) && enterY) {
					--enterY;
					fileName[enterY] = 36;
					
					// Set Filename Display
					memcpy(&tile_mem[1][(files_tilesetLen >> 5) + (saveFileNumber * (8 + FILENAME_LEN)) + 8 + enterY], &dialogue_font[36 << 3], SPRITE_8x8);	
				}
				else if (key_hit(KEY_B)) {
					for (enterX = 0; enterX < 5; ++enterX)
						memcpy(&tile_mem[1][(files_tilesetLen >> 5) + (saveFileNumber * (8 + FILENAME_LEN)) + 8 + enterX], &dialogue_font[emptyFileName[enterX] << 3], SPRITE_8x8);	
					enterX = -1;
				}
				
				if (key_hit(KEY_START)) {					
					GAME_fading = 1;
					nextGamestate = GS_LEVEL;
					current_chapter = 0;
				}
				
				if (enterX >= 0)
					camX += 5;
				else
					camX -= 8;
				
				if (camX > 164)
					camX = 164;
				
				if (enterX >= 0) {
					int cursorX = ((enterX % 10) * 16) + 50 - offset;
					int cursorY = ((enterX / 10) * 16) - camX + 8;
					
					obj_set_attr(sprite_pointer,
					ATTR0_SQUARE | ATTR0_Y(cursorY),
					ATTR1_SIZE_8x8 | ATTR1_X(cursorX),
					ATTR2_PALBANK(1) | (PAUSEARROW_OFFSET));
					
					++sprite_pointer;
					++spriteCount;
				}
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
		}
		
		if (camX <= 90) {
			obj_set_attr(sprite_pointer,
			ATTR0_SQUARE | ATTR0_Y(76),
			ATTR1_SIZE_8x8 | ATTR1_X(168 - offset) | ATTR1_FLIP(1),
			ATTR2_PALBANK(1) | (PAUSEARROW_OFFSET));
			
			++sprite_pointer;
			++spriteCount;
		}
	}
	else if (gamestate == GS_PAUSED) {
		
		if (key_hit(KEY_START)) {
			nextGamestate = GS_PLAYING;
			GAME_fading = 1;
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
				nextGamestate = GS_PLAYING;
				GAME_fading = 1;
				if (pauseLoc == 1) {
					CHAR_Die();
				}
			}
			else {
				nextGamestate = GS_LEVEL;
				GAME_fading = 1;
			}
		}
		
		int pos = (pauseLoc << 5) + 16;
		
		if (!IS_FADING && pauseArrowLoc != pos) {
			pauseArrowLoc += INT_SIGN(pos - pauseArrowLoc) * 4;
		}
		
		obj_set_attr(sprite_pointer,
		ATTR0_SQUARE | ATTR0_Y(pauseArrowLoc),
		ATTR1_SIZE_8x8 | ATTR1_X(((GAME_life >> 4) & 0x1)),
		ATTR2_PALBANK(2) | (PAUSEARROW_OFFSET));
		
		++sprite_pointer;
		++spriteCount;
	}
	else if (gamestate == GS_PLAYING){
		
		if (!IS_FADING && NOTE_BLOCKS_ACTIVE){
			GB_PlayMusic();
		}
		
		if (!IS_FADING){
			// update each entity
			for (actor_index = 0; actor_index < ActorLimit; ++actor_index){
				if (gamestate == GS_LEVEL)
					break;
					
				if (!ACTOR_ENABLED(PHYS_actors[actor_index].flags))
					continue;
					
				actor_id = PHYS_actors[actor_index].ID;
				
				switch(ACTOR_ENTITY_TYPE(actor_id))
				{
				case 0:
					CHAR_update(&PHYS_actors[0]);
					break;
				case 1:
					STRAWB_update(&PHYS_actors[actor_index]);
					break;
				case 2:
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
					CASS_update(ACTOR_ENTITY_TYPE(actor_id));
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
			if (gamestate == GS_LEVEL)
				goto gameLoop;
		}
		
		SetCamPos();
		
		// load in tilemap around camera (only the ones that need to be updated
		MoveCam(prevCamX, prevCamY, camX, camY);
		
		// set prev cam values for next frame
		prevCamX = camX;
		prevCamY = camY;
		
		int count = 0;
		UpdateParticles(sprite_pointer, camX, camY, &count);
		sprite_pointer += count;
		spriteCount += count;
		
		
		// render each object
		for (actor_index = 0; actor_index < ActorLimit; ++actor_index){
			count = 0;
			
			if (!ACTOR_ENABLED(PHYS_actors[actor_index].flags))
				continue;
				
			actor_id = PHYS_actors[actor_index].ID;
			
			switch(ACTOR_ENTITY_TYPE(actor_id))
			{
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
			}
			sprite_pointer += count;
			spriteCount += count;
		}
		
		if (GAME_freeze)
			--GAME_freeze;
		else if (current_chapter != LEVELIDX_EPI){
			++GAME_speedrun_timer;
			++GAME_IL_timer;
		}
		
		if (current_chapter == LEVELIDX_DREAM) {
			// Copy the outline texture
			unsigned int tilesets[128];
			unsigned int texture[8];
			unsigned int border[128];
			memcpy(tilesets, &dreamblock_tile[((GAME_speedrun_timer >> 3) & 0x3) << 7], 512);
			memcpy(border, &dreamblock_t_tile[((GAME_speedrun_timer >> 3) & 0x3) << 7], 512);
			
			// Get the stary animated texture
			memcpy(texture, &dreamblock_tex[((GAME_speedrun_timer >> 3) & 0x7) << 3], 32);
			
			// X value location
			char xVal = (camX) & 0x3C;
			
			// Add both textures in
			int index = 8;
			while (--index >= 0){
				texture[index] = (texture[index] << xVal) | (texture[index] >> (32 - xVal));
			}
			index = 128;
			while (--index >= 0){
				tilesets[index] |= texture[(index - (camY >> 2)) & 0x7];
			}
			
			//Transparency
			index = 128;
			while (--index >= 0){
				tilesets[index] &= border[index];
			}
			
			memcpy(&tile_mem[FG_TILESET][128], tilesets, 512);
		}
		
		if (key_hit(KEY_START) && !IS_FADING && key_tri_vert() == 0) {
			nextGamestate = GS_PAUSED;
			pauseLoc = 0;
			pauseArrowLoc = 16;
			GAME_fading = 1;
		}
		
		if (CHALLENGE_MODE) {
			obj_set_attr(sprite_pointer,
			ATTR0_SQUARE | ATTR0_Y(0),
			ATTR1_SIZE_16x16 | ATTR1_X(8) | ATTR1_FLIP(1),
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
	}
	else if (gamestate == GS_LEVEL) {
			
			if (!IS_FADING) {
				forceDisplay = 1;
				
				if (key_hit(KEY_RIGHT) && LEVEL_UNLOCKED(current_chapter + 1)) {
					
					++current_chapter;
					if (current_chapter >= LEVELIDX_MAX) {
						current_chapter =  LEVELIDX_MAX - 1;
					}
				}
				if (key_hit(KEY_LEFT)) {
					--current_chapter;
					if (current_chapter < 0) {
						current_chapter = 0;
					}
				}
				if (key_hit(KEY_START)) {
					GAME_fading = 1;
					nextGamestate = GS_FILES;
					forceDisplay = 0;
					
				}
				else if (key_hit(KEY_A)) {
					forceDisplay = 0;
					int hitValue = 0;
					
					if (KEY_DOWN_NOW(KEY_L))
						levelFlags = LEVELFLAG_CHALLENGE;
					else if (KEY_DOWN_NOW(KEY_R) && CASSETTE_COLL)
						levelFlags = LEVELFLAG_BSIDE;
					else
						levelFlags = 0;
					
					GAME_fading = 1;
				}
			}
			
			enterX = FIXED_APPROACH(enterX, chapter_map_locations[(current_chapter << 1) + 0] - (screenWidth  >> 1), 0x300);
			enterY = FIXED_APPROACH(enterY, chapter_map_locations[(current_chapter << 1) + 1] - (screenHeight >> 1), 0x300);
			
			camX = enterX - INT2FIXED(112);
			camY = enterY - INT2FIXED(104);
			
			SetMapCamPos();
			
			obj_set_attr(sprite_pointer,
			ATTR0_SQUARE | ATTR0_Y(FIXED2INT(enterY - camY)),
			ATTR1_SIZE_16x16 | ATTR1_X(FIXED2INT(enterX - camX)),
			ATTR2_PALBANK(1) | STRAWB_SP_OFFSET);
			
			++sprite_pointer;
			++spriteCount;
		}
	
	
	if (STRAWB_count != STRAWB_display || strawbUI_Pos || forceDisplay) {
			StrawbUI(forceDisplay);
			if (forceDisplay)
				showingTime = 1;
			
			if (!CHALLENGE_MODE || gamestate == GS_PLAYING){
			
				int digit1 = STRAWB_display % 10;
				int digit10 = STRAWB_display / 10;
				
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
				unsigned long timer = GAME_speedrun_timer;
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
				
				minute1  =  DEATH_count / 100;
				second10 = (DEATH_count / 10) % 10;
				second1  =  DEATH_count % 10;
				
				obj_set_attr(sprite_pointer + 7,
				ATTR0_SQUARE | ATTR0_Y(FIXED2INT(strawbUI_Pos) - 10),
				ATTR1_SIZE_8x8 | ATTR1_X(TIMER_OFFSET + 104),
				ATTR2_PALBANK(1) | TEXT_SP_OFFSET + 11);
				obj_set_attr(sprite_pointer + 8,
				ATTR0_SQUARE | ATTR0_Y(FIXED2INT(strawbUI_Pos) - 10),
				ATTR1_SIZE_8x8 | ATTR1_X(TIMER_OFFSET + 112),
				ATTR2_PALBANK(1) | TEXT_SP_OFFSET + minute1);
				obj_set_attr(sprite_pointer + 9,
				ATTR0_SQUARE | ATTR0_Y(FIXED2INT(strawbUI_Pos) - 10),
				ATTR1_SIZE_8x8 | ATTR1_X(TIMER_OFFSET + 120),
				ATTR2_PALBANK(1) | TEXT_SP_OFFSET + second10);
				obj_set_attr(sprite_pointer + 10,
				ATTR0_SQUARE | ATTR0_Y(FIXED2INT(strawbUI_Pos) - 10),
				ATTR1_SIZE_8x8 | ATTR1_X(TIMER_OFFSET + 128),
				ATTR2_PALBANK(1) | TEXT_SP_OFFSET + second1);
				
				spriteCount +=		11;
				sprite_pointer +=	11;
		}
	}
	else{
		showingTime = 0;
	}
	
	
	// Hide old sprites
	if (spriteCount < prevSpriteCount){
		int i = spriteCount;
		for (; i < prevSpriteCount; ++i)
		{
			sprite_pointer->attr0 = 0x0200;
			++sprite_pointer;
		}
	}
	
	if (GAME_fading || GAME_fadeAmount) {
		
		if (GAME_fadeAmount <= 0) {
			memcpy(directColors, pal_bg_mem, 512);
			memcpy(&directColors[256], pal_obj_mem, 512);
		}
		
		if (GAME_fading && GAME_fadeAmount < 0xFF)
			GAME_fadeAmount += 24;
		else if (!GAME_fading){
			GAME_fadeAmount -= 24;
			GAME_loadIndex = 0;
		}
		
		if (GAME_fadeAmount >= 0xFF) {
			
			if (GAME_fadeAmount > 0xFF){
				memset(pal_bg_mem,  0, 512);
				memset(pal_obj_mem, 0, 512);
			}
			
			GAME_fadeAmount = 0xFF;
			
			if (gamestate == GS_PAUSED && nextGamestate == GS_PLAYING) {
				LoadLevel();
				gamestate = GS_PLAYING;
				
				GAME_fading = 0;
			}
			else if (gamestate == GS_PLAYING && nextGamestate == GS_PAUSED) {
				OpenPause();
				gamestate = GS_PAUSED;
				GB_PauseMusic();
				GAME_fading = 0;
			}
			else if (gamestate == GS_PLAYING && (cutsceneCoroutine || dialogueEnabled)) {
				
				if (cutsceneCoroutine && cutsceneSkipped) {
					cutsceneSkipped();
				}
				
				cutsceneCoroutine = 0;
				dialogueEnabled = 0;
				
				if (lvlIndex == currentMax - 1) {
				}
				else{
					GAME_fading = 0;
					DLG_end();
				}
			}
			else if ((gamestate == GS_PLAYING || gamestate == GS_PAUSED || gamestate == GS_FILES) && nextGamestate == GS_LEVEL) {
				LeaveToMainMenu();
				GAME_fading = 0;
				SaveData(saveFileNumber);
			}
			else if (nextGamestate == GS_FILES) {
				strawbUI_Pos = 0;
				strawbUI_Offset1 = 0;
				strawbUI_Offset2 = 0;
				strawbUI_speed1 = 0;
				strawbUI_speed2 = 0;
				
				int textOffset = 0;
				
				REG_BG0CNT = BG_CBB(1) | BG_SBB(31) | BG_4BPP | BG_REG_32x32 | BG_PRIO(1);
				REG_BG1CNT = BG_CBB(1) | BG_SBB(30) | BG_4BPP | BG_REG_32x32 | BG_PRIO(0);
				
				REG_DISPCNT=  DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | DCNT_MODE0;
				
				memcpy(&directColors[0 << 4], madeline_dialogue_pal, 32); 
				memcpy(se_mem[FOREGROUND_LAYER], files_bg, files_bgLen);
				memcpy(&tile_mem[1][0], files_tileset, textOffset = files_tilesetLen);
				
				memcpy(se_mem[MIDGROUND_LAYER], files_fg, files_fgLen);
				
				
				textOffset >>= 5;
				
				int i, offset;
				
				for (offset = 0; offset < 1024; offset += 256) {
					se_mem[FOREGROUND_LAYER][offset + 32] = textOffset;
					se_mem[FOREGROUND_LAYER][offset + 173] = textOffset + 3;
					++textOffset;
					
					for (i = 0; i < 2; ++i) {
						se_mem[FOREGROUND_LAYER][i + offset + 50] = textOffset + i;
						se_mem[FOREGROUND_LAYER][i + offset + 175] = textOffset + i + 3;
						se_mem[FOREGROUND_LAYER][i + offset + 178] = textOffset + i + 5;
					}
					textOffset += 7;
					
					for (i = 0; i < FILENAME_LEN; ++i) {
						se_mem[FOREGROUND_LAYER][i + offset + 34] = (textOffset + i);
					}
					textOffset += FILENAME_LEN;
				}
				
				for (offset = 0; offset < 4; ++offset){
					for (i = 0; i < 10; ++i) {
						se_mem[MIDGROUND_LAYER][(i << 1) + 33 + (offset << 6)] = textOffset + i + (offset * 10);
						memcpy(&tile_mem[1][textOffset + i + (offset * 10)], &dialogue_font[(i + (offset * 10)) << 3], SPRITE_8x8);
					}
				}
				
				for (i = 0; i < 4; ++i)
					LoadFile(i, i);
					
				gamestate = GS_FILES;
				GAME_fading = 0;
				
				camY = (saveFileNumber << 6) - 48;
				REG_BG0VOFS = camY;
				REG_BG0HOFS = 0;
			
				camX = 80;
				REG_BG1VOFS = camX;
				memcpy(&directColors[17 << 4], pause1_pal, 32);
			}
			else if (gamestate == GS_INTRO) {
				++intro_index;
				if (intro_index < INTRO_COUNT) {
					LoadIntro(intro_index);
					GAME_fading = 0;
				}
				else {
					nextGamestate = GS_FILES;
				}
			}
			else if (gamestate == GS_LEVEL || gamestate == GS_PLAYING) {
				
				switch (GAME_loadIndex)
				{
				case 0:
					if (gamestate == GS_PLAYING) {
						++GAME_loadIndex;
						break;
					}
					
					
					for (strawbUI_Pos = 0; strawbUI_Pos < 15; ++strawbUI_Pos) {
						STRAWB_tempColl[strawbUI_Pos] = 0;
					}
					
					REG_IME = 0;
					
					GB_init_soundchip(current_chapter);
					
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
					
					GAME_music_beat = 0;
					GB_init_soundchip();
					GAME_IL_timer = 0;
					
					++GAME_loadIndex;
					
					break;
				case 1:
					
					
					if (gamestate != GS_PLAYING) {
						lvlIndex = 0;
						enterX = 0;
						enterY = 1;
						
						if (current_chapter == LEVELIDX_WATER)
							enterY = -1;
					}
					
					if (levelFlags & LEVELFLAG_BSIDE)
						LoadCollisionData(chapter_pointersBside[current_chapter][lvlIndex]);
					else
						LoadCollisionData(chapter_pointers[current_chapter][lvlIndex]);
						
					++GAME_loadIndex;
				case 2:
					++GAME_loadIndex;
					break;
				case 3:
					LoadInVisualDataFG();
					LoadInVisualDataBG();
					LoadInEntityData();
					LoadLevel();
					++GAME_loadIndex;
					break;
				case 4:
	
					//Save player's location
					CHAR_save_loc();
					// Initialize character for start of level
					CHAR_LoadLevel();
					
					GAME_fading = 0;
					
					if (gamestate == GS_PLAYING){
						CHAR_on_transition();
						
						SetCamPos();
						prevCamX = camX;
						prevCamY = camY;
						FillCam(camX, camY);
					}
					else{
						STRAWB_levelCount = STRAWB_COUNT;
						gamestate = GS_PLAYING;
					}
					
					break;
				}
				
			}
		}
		
		if (GAME_fadeAmount <= 0) {
			GAME_fadeAmount = 0;
			memcpy(pal_bg_mem, directColors, 512);
			memcpy(pal_obj_mem, &directColors[256], 512);
		}
		else if (GAME_fadeAmount != 0xFF) {
			
			int index = 0;
			short color;
			if (GAME_fadeAmount & 0x10){
				for (; index < 256; ++index){
					color = directColors[index + 256];
					pal_obj_mem[index] = COLOR_LERP(color, 0, GAME_fadeAmount);
				}
			}
			else{
				for (; index < 256; ++index){
					color = directColors[index];
					pal_bg_mem[index] = COLOR_LERP(color, 0, GAME_fadeAmount);
				}
			}
		}
	}
	
	if (!((GAME_life - 2) & 0x4)){
		memcpy(&tile_mem[4][44], &wing			[(((GAME_life - 2) >> 3) & 0x7)	<< 5],	keyLen >> 3);
		memcpy(&tile_mem[4][48], &dash_crystal	[(((GAME_life - 2) >> 3) % 5)	<< 5],	keyLen >> 3);
	}
	if (!(GAME_life & 0x4)){
		memcpy(&tile_mem[4][40], &strawb		[((GAME_life >> 3) % 7)			<< 5],	keyLen >> 3);
		memcpy(&tile_mem[4][68], &key			[((GAME_life >> 3) & 0x7)		<< 5],	keyLen >> 3);
	}
	
	prevSpriteCount = spriteCount;
	
	
	
	oam_copy(oam_mem, obj_buffer, 64);
	
	goto gameLoop;
	
	
	return 0;
}
