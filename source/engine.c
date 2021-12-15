#include "engine.h"
#include "tonc_vscode.h"

#include "pixtro_basic.h"

#include "entities.h"
#include "game_data.h"
#include "levels.h"
#include "load_data.h"
#include "state_machine.h"

StateMachine gamestate, gamestate_render;

// Cheat codes
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

int levelCode, colorCode, revertCode, tasCode;
int diagonalBuffer, keyIndex;
int cheatCodeBuffer[16];

int heart_freeze;

#define HEART_FREEZE ((heart_freeze & 0x1) || heart_freeze >= 0x40)
#define HEART_FROZEN (heart_freeze >= 0x40)

const unsigned int* levels[3] = {
	&PACK_prologue,
	NULL,
	NULL,
};
const unsigned int* levelsB[3] = {
	&PACK_prologue,
	NULL,
	NULL,
};

void CheatCodes() {
	if (diagonalBuffer) {
		--diagonalBuffer;
	}

	int hit;
	if (hit = key_hit(0xFFF)) {

		if (hit == KEY_UP || hit == KEY_DOWN || hit == KEY_LEFT || hit == KEY_RIGHT) {
			if (diagonalBuffer) {

				if (cheatCodeBuffer[keyIndex] == hit && KEY_DOWN_NOW(0xFFF) == hit) {
					// Clear the buffer to read another potential diagonal input
					diagonalBuffer			  = 0;
					cheatCodeBuffer[keyIndex] = hit;
				} else {
					// Go back an input and add the diagonal to the input
					keyIndex = (keyIndex + 15) & 0xF;
					cheatCodeBuffer[keyIndex] |= hit;
				}

				++keyIndex;
				keyIndex &= 0xF;
			} else {
				cheatCodeBuffer[keyIndex] = hit;
				++keyIndex;
				keyIndex &= 0xF;
			}

			// If buffer is hit (meaning there's still a value in buffer), then clear the buffer.  Otherwise, set the buffer
			diagonalBuffer = 7 * !diagonalBuffer;
		} else {
			cheatCodeBuffer[keyIndex] = hit;
			++keyIndex;
			keyIndex &= 0xF;

			diagonalBuffer = 0;
		}

		int index = keyIndex + 15;

		colorCode  = 0;
		levelCode  = 0;
		revertCode = 0;
		tasCode	   = 0;
		while (index >= keyIndex) {
			colorCode += colorRandomC[index - keyIndex] == cheatCodeBuffer[index & 0xF] || colorRandomC[index - keyIndex] == 0;
			levelCode += levelUnlockC[index - keyIndex] == cheatCodeBuffer[index & 0xF] || levelUnlockC[index - keyIndex] == 0;
			revertCode += colorRevertC[index - keyIndex] == cheatCodeBuffer[index & 0xF] || colorRevertC[index - keyIndex] == 0;
			tasCode += tasHitboxC[index - keyIndex] == cheatCodeBuffer[index & 0xF] || tasHitboxC[index - keyIndex] == 0;

			--index;
		}

		if (colorCode == 16) {

			int rngColor = RNG() & 0x7FFF;
			// hairColor[0] = rngColor;
			// hairColor[1] = COLOR_LERP(rngColor, 0, 0x50);

			// rngColor	 = RNG() & 0x7FFF;
			// hairColor[2] = rngColor;
			// hairColor[3] = COLOR_LERP(rngColor, 0, 0x50);

			// rngColor	 = RNG() & 0x7FFF;
			// hairColor[4] = rngColor;
			// hairColor[5] = COLOR_LERP(rngColor, 0, 0x50);

			key_mod2(KEY_A);
		} else if (tasCode == 16) {
			// levelFlags |= LEVELFLAG_TAS;

			// memcpy(&tile_mem[4][STRAWB_SP_OFFSET], hitbox16, SPRITE_16x16);
			// memcpy(&tile_mem[4][KEY_SP_OFFSET], hitbox16, SPRITE_16x16);
			// memcpy(&tile_mem[4][DASHCR_SP_OFFSET], hitboxDash1, SPRITE_16x16);
			// memcpy(&tile_mem[4][DASH_OUT_SP_OFFSET], hitboxDash2, SPRITE_16x16);
			// memcpy(&tile_mem[4][SPRING_D_OFFSET], springHit, SPRITE_16x16);
		}
		if (levelCode == 16 && gamestate.state == GS_LEVEL) {
			// UNLOCK_LEVEL(LEVELIDX_MAX - 1);
			// SaveData(saveFileNumber);
			key_mod2(KEY_A);
		}
		if (revertCode == 16) {
			revertCode = 0;

			memcpy(&hairColor[0], &PAL_hair[0], 12);
			key_mod2(KEY_A);
		}
	}
}

void LoadPack(int pack, int bside) {
	set_foreground_count(2);

	load_level_pack(levels[0], 0);

	move_to_level(0, 0);

	finalize_layers();
	reset_cam();
}
void start_playing() {
	move_to_level(0, 0);
	reset_cam();
}

void on_update() {
	CheatCodes();

	update_statemachine(&gamestate);
}
void on_render() {
	// update_statemachine(&gamestate_render);
}

int playing_update() {

	if (!HEART_FREEZE) {
		// update each entity
		/*
		for (actor_index = 0; actor_index < maxEntities; ++actor_index)
		{

			if (!ACTOR_ENABLED(PHYS_actors[actor_index].flags))
				continue;

			int actor_id = PHYS_actors[actor_index].ID;

			switch (ACTOR_ENTITY_TYPE(actor_id))
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

				switch (current_chapter)
				{
				case LEVELIDX_PROLOGUE:
					if (GAME_IL_timer < 40)
					{
						PHYS_actors[actor_index].flags &= ~ACTOR_ACTIVE_MASK;

						DLG_start(&prologue_text_st, boxCount_prologue_text_st);
					}
					else if (lvlIndex == 0)
					{
						PHYS_actors[actor_index].flags &= ~ACTOR_ACTIVE_MASK;
					}
					else if (lvlIndex != 0 && PHYS_actors[0].x > PHYS_actors[actor_index].x)
					{
						PHYS_actors[actor_index].flags &= ~ACTOR_ACTIVE_MASK;

						DLG_start(&prologue_text, boxCount_prologue_text);
						cutsceneCoroutine = DLG_prologue;
					}
					break;
				case LEVELIDX_DREAM:

					if (PHYS_actors[0].x > PHYS_actors[actor_index].x)
					{
						PHYS_actors[actor_index].flags &= ~ACTOR_ACTIVE_MASK;

						DLG_start(&text_ch1, boxCount_text_ch1);
						cutsceneCoroutine = DLG_prologue;
					}
					break;
				case LEVELIDX_RAIN:

					if (GAME_IL_timer < 40)
					{
						PHYS_actors[actor_index].flags &= ~ACTOR_ACTIVE_MASK;
						cutsceneIndex = 0;

						DLG_start(&text_ch2st1, boxCount_text_ch2st1);
						cutsceneCoroutine = DLG_rainIntro;
						cutsceneSkipped = DLG_rainIntroEnd;
					}
					else if (lvlIndex == 0)
					{
						PHYS_actors[actor_index].flags &= ~ACTOR_ACTIVE_MASK;
					}
					else if (lvlIndex != 0 && PHYS_actors[0].x > PHYS_actors[actor_index].x)
					{
						PHYS_actors[actor_index].flags &= ~ACTOR_ACTIVE_MASK;

						DLG_start(&text_ch2end, boxCount_text_ch2end);

						cutsceneCoroutine = DLG_prologue;
					}
					break;
				}
				break;
			}
		}
		//*/
	}

	// if (current_chapter != LEVELIDX_EPI && !(cutsceneCoroutine || DIALOGUE_ENABLED) && !HEART_FREEZE) {
	// 	++GAME_speedrun_timer;
	// 	++GAME_IL_timer;
	// }

	// 	if (!cutsceneCoroutine && !deathAnimTimer && !heart_freeze && key_hit(KEY_START) && !IS_FADING
	// #ifdef __DEBUG__
	// 		&& key_tri_vert() == 0
	// #endif
	// 	) {
	// 		//nextGamestate = GS_PAUSED;
	// 		pauseLoc	  = 0;
	// 		pauseArrowLoc = 16;
	// 		START_FADE();
	// 	}

	return -1;
}
int intro_update() {

	// if (0) {

	// 	int rng = RNG();

	// 	int offY = (rng % (180 << 4)) - 10;
	// 	offY &= 0xFFFF;

	// 	int velY = 0xF8 + ((rng & 0xF00) >> 8);
	// 	velY &= 0xFF;
	// 	offY |= (velY) << 16;

	// 	velY = (rng << 4) & 0x0F0000;
	// 	velY += 0x140000;

	// 	AddParticle(0xFFC0 | 0xFF000000 | velY,
	// 				offY | MenuSnow_L,
	// 				0x000 | MenuSnow_S);

	// 	if (!(intro_wait & 0x7)) {
	// 		int i;

	// 		for (i = 1; i < 120; i += 3) {
	// 			particle_data[i] |= 0xC0000000;
	// 		}
	// 	}
	// }
	// REG_BG1HOFS = -(intro_wait >> 3);
	// ++intro_wait;

	// if (!IS_FADING) {
	// 	if ((intro_index < INTRO_COUNT - 1 && intro_wait >= INTRO_WAIT) || (key_hit(KEY_A | KEY_B | KEY_START | KEY_SELECT) && (intro_index == INTRO_COUNT - 1 || intro_wait > 55))) {
	// 		START_FADE();

	// 		break;
	// 	}
	// } //*/

	return -1;
}
int level_update() {

	// CheatCodes();

	// if (!IS_FADING) {
	// 	forceDisplay = 1;
	// 	//REG_BG0VOFS = camY;

	// 	if (intro_wait >= 0x74) {

	// 		if (key_hit(KEY_RIGHT) && ((!CASSETTE_COLL && current_chapter != 0) || (intro_index == (current_chapter != 0) && !WINGED_UNLOCKED)))
	// 			;
	// 		else if (key_hit(KEY_RIGHT | KEY_LEFT)) {
	// 			if (key_hit(KEY_RIGHT))
	// 				intro_index += (1 + (current_chapter == 0)) * (intro_index < 2);
	// 			else if (key_hit(KEY_LEFT))
	// 				intro_index -= (1 + (current_chapter == 0)) * (intro_index > 0);
	// 			int i;
	// 			char* name = &"a sideb sidewinged"[intro_index * 6];

	// 			for (i = 3; i <= 8; ++i) {
	// 				if (name[i - 3] != ' ')
	// 					se_mem[FOREGROUND_LAYER][0x100 + i] = (name[i - 3] - 65) | 0x4000;
	// 				else
	// 					se_mem[FOREGROUND_LAYER][0x100 + i] = 3 | 0x4000;
	// 			}

	// 			switch (intro_index) {
	// 				case 0:
	// 					se_mem[FOREGROUND_LAYER][0xA4] = ((STRAWB_COUNT / 10) + 22) | 0x4000;
	// 					se_mem[FOREGROUND_LAYER][0xA5] = ((STRAWB_COUNT % 10) + 22) | 0x4000;
	// 					se_mem[FOREGROUND_LAYER][0xA7] = ((STRAWB_levelMax / 10) + 22) | 0x4000;
	// 					se_mem[FOREGROUND_LAYER][0xA8] = ((STRAWB_levelMax % 10) + 22) | 0x4000;

	// 					se_mem[FOREGROUND_LAYER][0xA6] = chapter_select_bg[0xA6] | 0x4000;
	// 					se_mem[FOREGROUND_LAYER][0xA2] = chapter_select_bg[0xA2] | 0x4000;
	// 					se_mem[FOREGROUND_LAYER][0xA3] = chapter_select_bg[0xA3] | 0x4000;
	// 					se_mem[FOREGROUND_LAYER][0x83] = chapter_select_bg[0x83] | 0x4000;
	// 					se_mem[FOREGROUND_LAYER][0xC3] = chapter_select_bg[0xC3] | 0x4000;

	// 					if (HEART_A_COLL) {
	// 						se_mem[FOREGROUND_LAYER][0x2A] = 14 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x2B] = 15 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x4A] = 16 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x4B] = 17 | 0x4000;
	// 					}
	// 					if (CASSETTE_COLL) {
	// 						se_mem[FOREGROUND_LAYER][0x6A] = 11 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x6B] = 11 | 0x4400;
	// 						se_mem[FOREGROUND_LAYER][0x8A] = 12 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x8B] = 12 | 0x4400;
	// 					} else {
	// 						se_mem[FOREGROUND_LAYER][0x6A] = 3 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x6B] = 3 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x8A] = 3 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x8B] = 3 | 0x4000;
	// 					}
	// 					if (GOLD_COLL) {
	// 						int ex = 0x60 + (0x40 * (current_chapter != 0));

	// 						se_mem[FOREGROUND_LAYER][ex + 0x0A] = 7 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][ex + 0x0B] = 8 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][ex + 0x2A] = 9 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][ex + 0x2B] = 10 | 0x4000;
	// 					}

	// 					break;
	// 				case 1:
	// 					se_mem[FOREGROUND_LAYER][0x82] = 3 | 0x4000;
	// 					se_mem[FOREGROUND_LAYER][0xC2] = 3 | 0x4000;
	// 					for (i = 0xA2; i <= 0xA8; ++i)
	// 						se_mem[FOREGROUND_LAYER][i] = 3 | 0x4000;

	// 					se_mem[FOREGROUND_LAYER][0x2A] = 3 | 0x4000;
	// 					se_mem[FOREGROUND_LAYER][0x2B] = 3 | 0x4000;
	// 					se_mem[FOREGROUND_LAYER][0x4A] = 3 | 0x4000;
	// 					se_mem[FOREGROUND_LAYER][0x4B] = 3 | 0x4000;

	// 					se_mem[FOREGROUND_LAYER][0xAA] = 3 | 0x4000;
	// 					se_mem[FOREGROUND_LAYER][0xAB] = 3 | 0x4000;
	// 					se_mem[FOREGROUND_LAYER][0xCA] = 3 | 0x4000;
	// 					se_mem[FOREGROUND_LAYER][0xCB] = 3 | 0x4000;

	// 					if (HEART_B_COLL) {
	// 						se_mem[FOREGROUND_LAYER][0x6A] = 14 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x6B] = 15 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x8A] = 16 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x8B] = 17 | 0x4000;
	// 					} else {
	// 						se_mem[FOREGROUND_LAYER][0x6A] = 3 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x6B] = 3 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x8A] = 3 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x8B] = 3 | 0x4000;
	// 					}

	// 					break;
	// 				case 2:
	// 					se_mem[FOREGROUND_LAYER][0x82] = 3 | 0x4000;
	// 					se_mem[FOREGROUND_LAYER][0xC2] = 3 | 0x4000;
	// 					for (i = 0xA2; i <= 0xA8; ++i)
	// 						se_mem[FOREGROUND_LAYER][i] = 3 | 0x4000;

	// 					if (WINGED_COLL) {
	// 						se_mem[FOREGROUND_LAYER][0x6A] = 7 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x6B] = 8 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x8A] = 9 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x8B] = 10 | 0x4000;
	// 					} else {
	// 						se_mem[FOREGROUND_LAYER][0x6A] = 3 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x6B] = 3 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x8A] = 3 | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x8B] = 3 | 0x4000;
	// 					}
	// 					break;
	// 			}
	// 		}

	// 		if (key_hit(KEY_B)) {
	// 			intro_wait = -0x70;
	// 		}

	// 		if (key_hit(KEY_A)) {

	// 			forceDisplay = 0;
	// 			levelFlags &= LEVELFLAG_TAS;

	// 			if (intro_index == 1)
	// 				levelFlags |= LEVELFLAG_BSIDE;
	// 			else if (intro_index == 2)
	// 				levelFlags |= LEVELFLAG_CHALLENGE;

	// 			START_FADE();
	// 			switch (current_chapter) {
	// 				default:
	// 					transition_style = curtain_trans0;
	// 			}

	// 			nextGamestate = GS_PLAYING;
	// 		}
	// 	} else if (intro_wait) {

	// 		if ((intro_wait & 0x7)) {

	// 			int i;
	// 			if (intro_wait > 0) {
	// 				int offset = (intro_wait >> 3);

	// 				for (i = 0; i < 10; ++i) {
	// 					se_mem[FOREGROUND_LAYER][(i << 5) + offset] = chapter_select_bg[(i << 5) + offset];
	// 				}
	// 				if (offset >= 1 && offset <= 8) {
	// 					if (chapter_names[current_chapter][offset - 1] != ' ')
	// 						se_mem[FOREGROUND_LAYER][0x20 + offset] = (chapter_names[current_chapter][offset - 1] - 65) | 0x4000;
	// 					if (chapter_names[current_chapter][offset + 7] != ' ')
	// 						se_mem[FOREGROUND_LAYER][0x40 + offset] = (chapter_names[current_chapter][offset + 7] - 65) | 0x4000;
	// 				}
	// 				if (offset == 4) {
	// 					se_mem[FOREGROUND_LAYER][0xA4] = ((STRAWB_COUNT / 10) + 22) | 0x4000;
	// 				}
	// 				if (offset == 5) {
	// 					se_mem[FOREGROUND_LAYER][0xA5] = ((STRAWB_COUNT % 10) + 22) | 0x4000;
	// 				}
	// 				if (offset == 7) {
	// 					se_mem[FOREGROUND_LAYER][0xA7] = ((STRAWB_levelMax / 10) + 22) | 0x4000;
	// 				}
	// 				if (offset == 8) {
	// 					se_mem[FOREGROUND_LAYER][0xA8] = ((STRAWB_levelMax % 10) + 22) | 0x4000;
	// 				}
	// 				if (offset == 3 || (offset >= 5 && offset <= 8)) {
	// 					se_mem[FOREGROUND_LAYER][0x100 + offset] = ("a side"[offset - 3] - 65) | 0x4000;
	// 				}
	// 				if (offset == 10 || offset == 11) {
	// 					if (HEART_A_COLL) {
	// 						se_mem[FOREGROUND_LAYER][0x20 + offset] = 14 + (offset - 10) | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x40 + offset] = 16 + (offset - 10) | 0x4000;
	// 					}
	// 					if (GOLD_COLL) {
	// 						int ex										 = 0x60 + (0x40 * (current_chapter != 0));
	// 						se_mem[FOREGROUND_LAYER][ex + offset]		 = 7 + (offset - 10) | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][ex + 0x20 + offset] = 9 + (offset - 10) | 0x4000;
	// 					}
	// 					if (CASSETTE_COLL) {
	// 						se_mem[FOREGROUND_LAYER][0x60 + offset] = 11 + ((offset - 10) << 10) | 0x4000;
	// 						se_mem[FOREGROUND_LAYER][0x80 + offset] = 12 + ((offset - 10) << 10) | 0x4000;
	// 					}
	// 				}
	// 			} else {
	// 				for (i = 0; i < 10; ++i) {
	// 					se_mem[FOREGROUND_LAYER][(i << 5) + ((8 - intro_wait) >> 3)] = 0;
	// 				}
	// 			}
	// 		}
	// 		REG_BG0HOFS = INT_ABS(intro_wait) + 0x08;
	// 		REG_BG0VOFS = -0x20;
	// 		intro_wait += 4;
	// 	} else {
	// 		if (key_hit(KEY_RIGHT) && LEVEL_UNLOCKED(current_chapter + 1)) {

	// 			++current_chapter;
	// 			if (current_chapter >= LEVELIDX_MAX) {
	// 				current_chapter = LEVELIDX_MAX - 1;
	// 			}
	// 		} else if (key_hit(KEY_LEFT)) {
	// 			--current_chapter;
	// 			if (current_chapter < 0) {
	// 				current_chapter = 0;
	// 			}
	// 		}
	// 		if (key_hit(KEY_B)) {
	// 			START_FADE();
	// 			nextGamestate = GS_FILES;
	// 			forceDisplay  = 0;
	// 		} else if (key_hit(KEY_A)) {
	// 			intro_index = 0;
	// 			intro_wait	= 4;

	// 			switch (current_chapter) {
	// 				case LEVELIDX_RAIN:
	// 					STRAWB_levelMax = forsaken_strawbMax;
	// 					break;
	// 				case LEVELIDX_PROLOGUE:
	// 					STRAWB_levelMax = prologue_strawbMax;
	// 					break;
	// 				case LEVELIDX_DREAM:
	// 					STRAWB_levelMax = dream_strawbMax;
	// 					break;
	// 				case LEVELIDX_WATER:
	// 					STRAWB_levelMax = water_strawbMax;
	// 					break;
	// 			}
	// 		}
	// 	}
	// }

	return -1;
}
int files_update() {

	// CheatCodes();

	// if (!IS_FADING) {
	// 	if (camX <= 90) {
	// 		if (key_hit(KEY_UP)) {

	// 			LoadFile((saveFileNumber + 6) & 0x7);

	// 			if (--saveFileNumber < 0)
	// 				saveFileNumber = 7;
	// 		}
	// 		if (key_hit(KEY_DOWN)) {

	// 			LoadFile((saveFileNumber + 2) & 0x7);

	// 			if (++saveFileNumber > 7)
	// 				saveFileNumber = 0;
	// 		}
	// 		if (key_hit(KEY_B)) {
	// 			LoadData(saveFileNumber);
	// 			if (saveFile[0] != 0xFF) {
	// 				camX		= 104;
	// 				REG_BG1HOFS = -48;
	// 				enterX		= 38;
	// 				//enterY = 0;

	// 				int offset;
	// 				for (offset = 0; offset < 4; ++offset) {
	// 					int i;
	// 					char* string = &"are you sure you   want to delete thisfile?                        yes   no "[offset * 19];
	// 					for (i = 0; i < 19; ++i) {
	// 						if (string[i] == '?')
	// 							se_mem[MIDGROUND_LAYER][i + 33 + (offset << 6)] = (49) | 0x4000;
	// 						else if (string[i] != ' ')
	// 							se_mem[MIDGROUND_LAYER][i + 33 + (offset << 6)] = (string[i] - 77) | 0x4000;
	// 						else
	// 							se_mem[MIDGROUND_LAYER][i + 33 + (offset << 6)] = 4 | 0x4000;
	// 					}
	// 				}
	// 			}
	// 		}

	// 		if (key_hit(KEY_A)) {

	// 			LoadData(saveFileNumber);

	// 			if (saveFile[0] == 0xFF) {

	// 				int offset;
	// 				for (offset = 0; offset < 4; ++offset) {
	// 					int i;
	// 					for (i = 0; i < 19; ++i) {
	// 						se_mem[MIDGROUND_LAYER][i + 33 + (offset << 6)] = 4 | 0x4000;
	// 					}
	// 					for (i = 0; i < 10; ++i) {
	// 						se_mem[MIDGROUND_LAYER][(i << 1) + 33 + (offset << 6)] = 10 + i + (offset * 10) | 0x4000;
	// 					}
	// 				}

	// 				visualFlags |= DCNT_BG1;
	// 				camX		= 104;
	// 				REG_BG1HOFS = -48;

	// 				enterY = +34 + ((saveFileNumber & 0x3) << 8);

	// 				for (enterX = 0; enterX < FILENAME_LEN; ++enterX) {
	// 					se_mem[FOREGROUND_LAYER][enterY + enterX] = 46 | 0x4000;
	// 					fileName[enterX]						  = 36;
	// 				}

	// 				enterX = 0;
	// 				enterY = 0;
	// 			} else {
	// 				START_FADE();

	// 				nextGamestate	= GS_LEVEL;
	// 				current_chapter = 0;
	// 				while (LEVEL_UNLOCKED(current_chapter + 1) && (current_chapter + 1) < LEVELIDX_MAX)
	// 					++current_chapter;
	// 			}
	// 		}
	// 	} else {
	// 		if (saveFile[0] == 0xFF) {
	// 			if (enterX >= 0) {
	// 				if (key_hit(KEY_UP)) {
	// 					enterX -= 10;
	// 					if (enterX < 0)
	// 						enterX += 40;
	// 				}
	// 				if (key_hit(KEY_DOWN)) {
	// 					enterX += 10;
	// 					if (enterX >= 40)
	// 						enterX -= 40;
	// 				}
	// 				if (key_hit(KEY_LEFT)) {
	// 					--enterX;
	// 					if (enterX < 0 || (enterX % 10) == 9)
	// 						enterX += 10;
	// 				}
	// 				if (key_hit(KEY_RIGHT)) {
	// 					++enterX;
	// 					if (!(enterX % 10))
	// 						enterX -= 10;
	// 				}

	// 				if (key_hit(KEY_A) && enterY < FILENAME_LEN) {
	// 					fileName[enterY] = enterX;

	// 					// Set Filename Display
	// 					se_mem[FOREGROUND_LAYER][enterY + 34 + ((saveFileNumber & 0x3) << 8)] = enterX + 10 | 0x4000;

	// 					++enterY;
	// 				}
	// 			}
	// 			if (key_hit(KEY_B) && enterY) {
	// 				--enterY;
	// 				fileName[enterY] = 36;

	// 				se_mem[FOREGROUND_LAYER][enterY + 34 + ((saveFileNumber & 0x3) << 8)] = 46 | 0x4000;
	// 			} else if (key_hit(KEY_B)) {
	// 				for (enterX = 0; enterX < 5; ++enterX)
	// 					se_mem[FOREGROUND_LAYER][enterX + 34 + ((saveFileNumber & 0x3) << 8)] = emptyFileName[enterX] + 10 | 0x4000;
	// 				enterX = -1;
	// 			}

	// 			if (key_hit(KEY_START) && enterY && fileName[0] != 36) {
	// 				START_FADE();

	// 				nextGamestate	= GS_LEVEL;
	// 				current_chapter = 0;
	// 			}
	// 		} else {

	// 			if (key_hit(KEY_LEFT | KEY_RIGHT)) {
	// 				enterX += 3;
	// 				enterX -= 6 * (enterX == 41);
	// 			}

	// 			if (key_hit(KEY_B) || (key_hit(KEY_A) && enterX == 38)) {
	// 				enterX = -1;
	// 			} else if (key_hit(KEY_A)) {
	// 				enterX = -1;

	// 				ResetData(saveFileNumber);
	// 				LoadFile(saveFileNumber);
	// 			}
	// 		}

	// 		if (enterX >= 0)
	// 			camX += 5;
	// 		else
	// 			camX -= 8;

	// 		if (camX > 164)
	// 			camX = 164;
	// 	}
	// }

	return -1;
}
int paused_update() {

	// CheatCodes();

	// if (key_hit(KEY_START)) {
	// 	nextGamestate = GS_UNPAUSE;
	// 	START_FADE();

	// 	pauseLoc = 0;
	// }
	// if (key_hit(KEY_UP)) {
	// 	--pauseLoc;
	// 	if (pauseLoc < 0)
	// 		pauseLoc = 2;
	// }
	// if (key_hit(KEY_DOWN)) {
	// 	++pauseLoc;
	// 	if (pauseLoc > 2)
	// 		pauseLoc = 0;
	// }
	// if (key_hit(KEY_A)) {
	// 	if (pauseLoc < 2) {
	// 		nextGamestate = GS_UNPAUSE;
	// 		START_FADE();

	// 		if (pauseLoc == 1) {
	// 			CHAR_Die();
	// 		}
	// 	} else {
	// 		nextGamestate = GS_LEVEL;
	// 		START_FADE();
	// 		transition_style = sidebar_trans0;
	// 	}
	// }

	return -1;
}

int playing_render() {

	// Show gold berry for challenge mode
	if (CHALLENGE_MODE) {
		// obj_set_attr(sprite_pointer,
		// 			 ATTR0_SQUARE | ATTR0_Y(0),
		// 			 ATTR1_SIZE_16x16 | ATTR1_X(8),
		// 			 ATTR2_PALBANK(1) | (STRAWB_SP_OFFSET));

		// obj_set_attr(sprite_pointer + 1,
		// 			 ATTR0_SQUARE | ATTR0_Y(0),
		// 			 ATTR1_SIZE_16x16 | ATTR1_X(-4) | ATTR1_FLIP(1),
		// 			 ATTR2_PALBANK(1) | (STRAWB_SP_OFFSET + 4));

		// obj_set_attr(sprite_pointer + 2,
		// 			 ATTR0_SQUARE | ATTR0_Y(0),
		// 			 ATTR1_SIZE_16x16 | ATTR1_X(20) | ATTR1_FLIP(0),
		// 			 ATTR2_PALBANK(1) | (STRAWB_SP_OFFSET + 4));

		// spriteCount += 3;
		// sprite_pointer += 3;
	}
	// Render actors

	// int actor_id = PHYS_actors[actor_index].ID;

	// switch (ACTOR_ENTITY_TYPE(actor_id)) {
	// 	case 0:
	// 		CHAR_render(sprite_pointer, camX, camY, &count);
	// 		break;
	// 	case ENT_STRAWB:
	// 		STRAWB_render(sprite_pointer, camX, camY, &count, &PHYS_actors[actor_index]);
	// 		break;
	// 	case ENT_SPRING:
	// 		SPRING_render(sprite_pointer, camX, camY, &count, &PHYS_actors[actor_index]);
	// 		break;
	// 	case ENT_DASH:
	// 		DASHCR_render(sprite_pointer, camX, camY, &count, &PHYS_actors[actor_index]);
	// 		break;
	// 	case ENT_BUMPER:
	// 		BUMPER_render(sprite_pointer, camX, camY, &count, &PHYS_actors[actor_index]);
	// 		break;
	// 	case ENT_ZIP:
	// 		ZIP_render(sprite_pointer, camX, camY, &count, &PHYS_actors[actor_index]);
	// 		break;
	// 	case ENT_KEY:
	// 		KEY_render(sprite_pointer, camX, camY, &count, &PHYS_actors[actor_index]);
	// 		break;
	// 	case ENT_CASSETTE:
	// 	case ENT_HEART:
	// 		CASS_render(sprite_pointer, camX, camY, &count, &PHYS_actors[actor_index]);
	// 		break;
	// 	case ENT_BACKGROUND:
	// 		BG_TOGG_render(sprite_pointer, camX, camY, &count, &PHYS_actors[actor_index]);
	// 		break;
	// 	case ENT_FLAG:
	// 		count = 1;

	// 		obj_set_attr(sprite_pointer,
	// 					 ATTR0_SQUARE | ATTR0_Y(GetActorY(PHYS_actors[actor_index].y, camY)),
	// 					 ATTR1_SIZE_16x16 | ATTR1_X(GetActorX(PHYS_actors[actor_index].x, camX)),
	// 					 ATTR2_PALBANK(15) | flagIndex | ATTR2_PRIO(CHAR_PRIORITY + 1));

	// 		break;
	// }

	return -1;
}
int level_render() {

	// enterX = FIXED_APPROACH(enterX, (chapter_map_locations[(current_chapter << 1) + 0] - (screenWidth >> 1)), 0x300);
	// enterY = FIXED_APPROACH(enterY, (chapter_map_locations[(current_chapter << 1) + 1] - (screenHeight >> 1)), 0x300);

	// camX = enterX - INT2FIXED(112);
	// camY = enterY - INT2FIXED(104);

	// SetMapCamPos();

	// obj_set_attr(sprite_pointer,
	// 			 ATTR0_SQUARE | ATTR0_Y(FIXED2INT(enterY - camY)),
	// 			 ATTR1_SIZE_16x16 | ATTR1_X(FIXED2INT(enterX - camX)),
	// 			 ATTR2_PALBANK(1) | STRAWB_SP_OFFSET);

	// ++sprite_pointer;
	// ++spriteCount;

	// if (intro_wait) {

	// 	int offset = INT_ABS(intro_wait);

	// 	if (intro_index > 0) {
	// 		obj_set_attr(sprite_pointer,
	// 					 ATTR0_SQUARE | ATTR0_Y(96),
	// 					 ATTR1_SIZE_8x8 | ATTR1_X(260 - offset + ((GAME_life >> 4) & 0x1)) | ATTR1_FLIP(1),
	// 					 ATTR2_PALBANK(2) | (PAUSEARROW_OFFSET));

	// 		++sprite_pointer;
	// 		++spriteCount;
	// 	}

	// 	if (intro_index < 2 && (CASSETTE_COLL || current_chapter == 0) && (intro_index != (current_chapter != 0) || WINGED_UNLOCKED)) {
	// 		obj_set_attr(sprite_pointer,
	// 					 ATTR0_SQUARE | ATTR0_Y(96),
	// 					 ATTR1_SIZE_8x8 | ATTR1_X(332 - offset - ((GAME_life >> 4) & 0x1)),
	// 					 ATTR2_PALBANK(2) | (PAUSEARROW_OFFSET));
	// 	}

	// 	++sprite_pointer;
	// 	++spriteCount;
	// }

	return -1;
}
int files_render() {
	// VisualizeCamera();

	// int offset = (GAME_life >> 4) & 0x1;
	// if (camX > 90 && enterX >= 0) {

	// 	int cursorX = ((enterX % 10) * 16) + 50 - offset;
	// 	int cursorY = ((enterX / 10) * 16) - camX + 8;

	// 	obj_set_attr(sprite_pointer,
	// 				 ATTR0_SQUARE | ATTR0_Y(cursorY),
	// 				 ATTR1_SIZE_8x8 | ATTR1_X(cursorX),
	// 				 ATTR2_PALBANK(1) | (PAUSEARROW_OFFSET));

	// 	++sprite_pointer;
	// 	++spriteCount;
	// }

	// int needed = (saveFileNumber << 6) - 48;
	// int diff   = needed - camY;

	// while (diff > 128)
	// 	diff -= 256;
	// while (diff < -128)
	// 	diff += 256;

	// camY += INT_SIGN(diff) * 8;

	// REG_BG0VOFS = camY;
	// REG_BG1VOFS = camX;

	// if (camX <= 90) {
	// 	obj_set_attr(sprite_pointer,
	// 				 ATTR0_SQUARE | ATTR0_Y(76),
	// 				 ATTR1_SIZE_8x8 | ATTR1_X(168 - offset) | ATTR1_FLIP(1),
	// 				 ATTR2_PALBANK(1) | (PAUSEARROW_OFFSET));

	// 	++sprite_pointer;
	// 	++spriteCount;
	// }
	return -1;
}
int paused_render() {

	// int pos = (pauseLoc << 5) + 16;

	// pauseArrowLoc += (INT_SIGN(pos - pauseArrowLoc) * 8) * (pauseArrowLoc != pos);

	// obj_set_attr(sprite_pointer,
	// 			 ATTR0_SQUARE | ATTR0_Y(pauseArrowLoc),
	// 			 ATTR1_SIZE_8x8 | ATTR1_X(((GAME_life >> 4) & 0x1)),
	// 			 ATTR2_PALBANK(0) | (PAUSEARROW_OFFSET));

	// ++sprite_pointer;
	// ++spriteCount;

	return -1;
}

// Run before anything else happens in the game
void init() {

	init_statemachine(&gamestate, GAMESTATE_COUNT);
	init_statemachine(&gamestate_render, GAMESTATE_COUNT);

	set_update_state(&gamestate, playing_update, GS_PLAYING);
	set_update_state(&gamestate, intro_update, GS_INTRO);
	set_update_state(&gamestate, level_update, GS_LEVEL);
	set_update_state(&gamestate, files_update, GS_FILES);
	set_update_state(&gamestate, paused_update, GS_PAUSED);

	set_update_state(&gamestate_render, playing_render, GS_PLAYING);
	set_update_state(&gamestate_render, level_render, GS_LEVEL);
	set_update_state(&gamestate_render, files_render, GS_FILES);
	set_update_state(&gamestate_render, paused_render, GS_PAUSED);

	entity_inits[0]	 = CHAR_init;
	entity_update[0] = CHAR_update;
	entity_render[0] = CHAR_render;

	load_bg_pal(&PAL_dirt, 0);
	load_bg_pal(&PAL_snow, 1);
	load_bg_pal(&PAL_dirt0_pal, 2);
	load_bg_pal(&PAL_dirt0_pal, 3);

	set_foreground_count(2);
	finalize_layers();

	load_level_pack_async(&PACK_prologue, 0);
	onfinish_async_loading = start_playing;

	LOAD_TILESET(Prologue);

	// move_to_level(0, 0);

	// reset_cam();

	// add_entity(0, 0, ENTITY_CHARACTER);

	gamestate.state = GS_PLAYING;

	custom_update = &on_update;
	custom_render = &on_render;
}

// Run the first time the game is initialized.  Mainly used for setting default settings
void init_settings() {
}