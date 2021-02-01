#ifndef __DATA__
#define __DATA__

#include <tonc.h>
#include <string.h>
#include "math.h"

typedef struct Actor
{
	int x, y, velX, velY;
	unsigned short width, height;
	unsigned int ID;
	
	unsigned int flags;
	
	
} Actor;

//#define __DEBUG__

#define HEART_TEMP		119
#define CASSETTE_TEMP	118

// sprite offsets
#define MADELINE_SP_OFFSET 	0x00
#define HAIR_SP_OFFSET 		0x04
#define STRAWB_SP_OFFSET	0x28
#define DASHCR_SP_OFFSET	0x30
#define TEXT_SP_OFFSET		0x34
#define CHEST_SP_OFFSET		0x40
#define KEY_SP_OFFSET		0x44
#define SPRING_D_OFFSET		0x48
#define SPRING_U_OFFSET		0x4A
#define PAUSEARROW_OFFSET	0x4C
#define CASSETTE_OFFSET		0x50
#define DASH_OUT_SP_OFFSET	0x54
#define PARTICLE_OFFSET		0x58
#define EXTRA_SP_OFFSET		0xB0

#define MADELINE_PAL		0
#define STRAWB_N_PAL		1
#define STRAWB_O_PAL		2
#define CHEST_PAL			3
#define EXTRA_SP_PAL_OFFSET	4

#define SECRET_PAL_OFFSET	15

#define GS_LEVEL			0
#define GS_PLAYING			1
#define GS_PAUSED			2
#define GS_INTRO			3
#define GS_FILES			4

#define GS_END_LEVEL		0x10
#define GS_UNPAUSE			0x11
#define GS_END_CUTSCENE		0x21
#define GS_RESPAWN			0x31

#define screenWidth 240
#define screenHeight 160

// Level index data
#define LEVELIDX_PROLOGUE	0
#define LEVELIDX_DREAM		1
#define LEVELIDX_RAIN		2
#define LEVELIDX_WATER		3
#define LEVELIDX_CORE		10
#define LEVELIDX_EPI		10

#define LEVELIDX_MAX		3

// Background locations
#define FOREGROUND_LAYER		31
#define MIDGROUND_LAYER			30
#define BACKGROUND_ONE			26
#define BACKGROUND_EXTRA		22

#define BG_TILESET				0
#define FG_TILESET				1
#define DIALOGUE_TILESET		2

#define SPRITE_8x8				32
#define SPRITE_8x16				64
#define SPRITE_16x8				64
#define SPRITE_16x16			128

// save mem offsets
#define STRAWB_SAVE_MASK		0x000F
#define STRAWB_SAVE_ADD				1
#define SAVE_TIMER_ADD				16
#define SAVE_DEATH_OFF				24
#define STRAWB_COUNT_IDX			28
#define FILE_NAME_IDX				36

#define GOLD_COLL_IDX				48
#define WINGED_COLL_IDX				49
#define CASSETTE_COLL_IDX			50
#define HEART_A_COLL_IDX			51
#define HEART_B_COLL_IDX			52

#define LEVEL_UNLOCKED_IDX			53

#define LEVELFLAG_BSIDE 		0x0001
#define LEVELFLAG_COREMODE		0x0002
#define LEVELFLAG_RAINY			0x0004
#define LEVELFLAG_CHALLENGE		0x0008
#define MUSICAL_LEVEL			0x0010
#define LEVELFLAG_TAS			0x0020
#define LEVELFLAG_BEGINNING		0x0040
#define LEVELFLAG_CLEAR			(LEVELFLAG_TAS)

#define IS_CORE_HOT			(levelFlags & LEVELFLAG_COREMODE)
#define IS_RAINY			(levelFlags & LEVELFLAG_RAINY)
#define CHALLENGE_MODE		(levelFlags & LEVELFLAG_CHALLENGE)
#define NOTE_BLOCKS_ACTIVE	(levelFlags & MUSICAL_LEVEL)
#define RECORD_DEATHS		!(levelFlags & LEVELFLAG_BEGINNING)

#define TAS_ACTIVE			(levelFlags & LEVELFLAG_TAS)

#define HEART_FREEZE		((heart_freeze & 0x1) || heart_freeze >= 0x40)
#define HEART_FROZEN		(heart_freeze >= 0x40)

#ifdef __DEBUG__
#define WINGED_UNLOCKED				(1)
#else
#define WINGED_UNLOCKED				(STRAWB_count >= 36)
#endif
#define GOLD_COLL					((saveFile[GOLD_COLL_IDX	] >> current_chapter) & 0x1)
#define GOLD_COLLECT()				 (saveFile[GOLD_COLL_IDX	] |= (1 << current_chapter))
#define WINGED_COLL					((saveFile[WINGED_COLL_IDX	] >> current_chapter) & 0x1)
#define WINGED_COLLECT()			 (saveFile[WINGED_COLL_IDX	] |= (1 << current_chapter))
#define HEART_COLL(b)				((saveFile[HEART_A_COLL_IDX+b]>> current_chapter) & 0x1)
#define HEART_A_COLL				((saveFile[HEART_A_COLL_IDX	] >> current_chapter) & 0x1)
#define HEART_B_COLL				((saveFile[HEART_B_COLL_IDX	] >> current_chapter) & 0x1)

#define HEART_COLLECT(b)			 (saveFile[HEART_A_COLL_IDX+b]|= (1 << current_chapter))

#define CASSETTE_COLL				((saveFile[CASSETTE_COLL_IDX] >> current_chapter) & 0x1)
#define UNLOCK_LEVEL(n)				 (saveFile[LEVEL_UNLOCKED_IDX] = n)
#define LEVEL_UNLOCKED(n)			 (saveFile[LEVEL_UNLOCKED_IDX] >= n)

#define STRAWB_COUNT				(saveFile[STRAWB_COUNT_IDX + current_chapter])
#define STRAWB_COUNT_SET(n)			(saveFile[STRAWB_COUNT_IDX + current_chapter] = n)
#define STRAWB_COUNT_RESET(n)		(saveFile[STRAWB_COUNT_IDX + n]			   = 0)

#define STRAWB_SAVEISCOLL(n)		(		saveFile[(n>>3) + STRAWB_SAVE_ADD] & (1 << (n & 0x7)))
#define STRAWB_TEMPCOLL(n)			(STRAWB_tempColl[(n>>3)					 ] & (1 << (n & 0x7)))
#define STRAWB_SETTEMPCOLL(n)		STRAWB_tempColl[(n>>3)					 ] = (STRAWB_tempColl[(n>>3)]			 & ~(1 << (n & 0x7))) | ((1 & 0x1) << (n & 0x7))
#define STRAWB_SAVESETCOLL(n, v)			saveFile[(n>>3) + STRAWB_SAVE_ADD] = (saveFile[(n>>3) + STRAWB_SAVE_ADD] & ~(1 << (n & 0x7))) | ((v & 0x1) << (n & 0x7));\
									 STRAWB_tempColl[(n>>3)					 ] = (STRAWB_tempColl[(n>>3)]			 & ~(1 << (n & 0x7))) | ((v & 0x1) << (n & 0x7))
									 
#define START_FADE()				GAME_fading = 1; GAME_fadeAmount = 0; GAME_loadIndex = 0
#define TRANSITION_CAP		16

extern unsigned int GAME_freeze;
extern unsigned int GAME_music_beat, GAME_life;
extern unsigned int GAME_speedrun_timer, GAME_IL_timer;
extern void (*cutsceneCoroutine)(void);
extern void (*cutsceneSkipped)(void);
extern unsigned int cutsceneInput, cutsceneWait, heart_freeze;
extern unsigned int flagIndex;

extern unsigned short* transition_style;

extern unsigned int maxEntities;

extern unsigned char STRAWB_tempColl[15];

extern const char** chapter_pointers[LEVELIDX_MAX];
extern const char** chapter_pointersBside[LEVELIDX_MAX];
extern const int chapter_map_locations[LEVELIDX_MAX << 1];
extern const char* chapter_names[LEVELIDX_MAX];

extern int level_exits[4];
extern int enterX, enterY;
extern unsigned int visualFlags;

#ifdef	__DEBUG__
extern int VCountLast, enabled;
#endif

#define SAVEFILE_LEN		256
#define SAVEFILE_SHIFT		8
#define FILENAME_LEN		12
extern char saveFile[SAVEFILE_LEN], fileName[FILENAME_LEN];
extern int saveFileNumber;
extern const char emptyFileName[FILENAME_LEN];

extern int current_chapter, lvlIndex, currentMax, levelFlags;
extern char gamestate, nextGamestate;

#define IS_FADING (GAME_fading || GAME_fadeAmount)
extern int GAME_fadeAmount, GAME_fading, GAME_loadIndex;

extern int camX, camY, prevCamX, prevCamY;

extern int strawbUI_Pos, strawbUI_Offset1, strawbUI_Offset2, strawbUI_speed1, strawbUI_speed2;

extern int STRAWB_count, STRAWB_display, STRAWB_levelCount, STRAWB_levelMax;
extern int STRAWB_tempCount, forceDisplay;
extern unsigned int DEATH_count, DEATH_tempCount;

void SetCamPos();
void VisualizeCamera();
void LeaveToMainMenu();
void EmptyInterrupt(int _line);
void Interrupt();
void StrawbUI(int force);
void InitializeChapter();
void SaveTimer();
void LoadTimer();
void DisplayFileBG();
void SetCore(int value);
void ChangeHBlankIntr(void (*function)(int));

INLINE void key_mod(u32 key);
INLINE void key_mod2(u32 key);

INLINE void key_mod(u32 key)
{	__key_curr= key & KEY_MASK;	}

INLINE void key_mod2(u32 key)
{	__key_prev= key & KEY_MASK;	}

#endif
