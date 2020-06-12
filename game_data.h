#include "sprites.h"
#include "backgrounds.h"
#include "level.h"

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
#define PARTICLE_OFFSET		0x54
#define EXTRA_SP_OFFSET		0x80

#define MADELINE_PAL		0
#define STRAWB_PAL			1
#define CHEST_PAL			2
#define SPRING_PAL			3
#define EXTRA_SP_PAL_OFFSET	4

#define GS_LEVEL		0
#define GS_PLAYING		1
#define GS_PAUSED		2
#define GS_INTRO		3
#define GS_FILES		4

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
#define HEART_COLL_IDX				51

#define LEVEL_UNLOCKED_IDX			52

#define LEVELFLAG_BSIDE 		0x0001
#define LEVELFLAG_COREMODE		0x0002
#define LEVELFLAG_RAINY			0x0004
#define LEVELFLAG_CHALLENGE		0x0008
#define MUSICAL_LEVEL			0x0010

#define IS_CORE_HOT			(levelFlags & LEVELFLAG_COREMODE)
#define IS_RAINY			(levelFlags & LEVELFLAG_RAINY)
#define CHALLENGE_MODE		(levelFlags & LEVELFLAG_CHALLENGE)
#define NOTE_BLOCKS_ACTIVE	(levelFlags & MUSICAL_LEVEL)

#define GOLD_COLL					((saveFile[GOLD_COLL_IDX	] >> current_chapter) & 0x1)
#define GOLD_COLLECT()				 (saveFile[GOLD_COLL_IDX	] |= (1 << current_chapter))
#define WINGED_COLL					((saveFile[WINGED_COLL_IDX	] >> current_chapter) & 0x1)
#define WINGED_COLLECT()			 (saveFile[WINGED_COLL_IDX	] |= (1 << current_chapter))
#define HEART_COLL					((saveFile[HEART_COLL_IDX	] >> current_chapter) & 0x1)
#define HEART_COLLECT()				 (saveFile[HEART_COLL_IDX	] |= (1 << current_chapter))
#define CASSETTE_COLL				((saveFile[CASSETTE_COLL_IDX] >> current_chapter) & 0x1)
#define UNLOCK_LEVEL(n)				 (saveFile[LEVEL_UNLOCKED_IDX] = n)
#define LEVEL_UNLOCKED(n)			 (saveFile[LEVEL_UNLOCKED_IDX] >= n)

#define STRAWB_COUNT				(saveFile[STRAWB_COUNT_IDX + current_chapter])
#define STRAWB_COUNT_SET(n)			(saveFile[STRAWB_COUNT_IDX + current_chapter] = n)
#define STRAWB_COUNT_RESET(n)		(saveFile[STRAWB_COUNT_IDX + n]			   = 0)

#define STRAWB_SAVEISCOLL(n)		(		saveFile[(n>>3) + STRAWB_SAVE_ADD] & (1 << (n & 0x7)))
#define STRAWB_TEMPCOLL(n)			(STRAWB_tempColl[(n>>3)					 ] & (1 << (n & 0x7)))
#define STRAWB_SAVESETCOLL(n, v)			saveFile[(n>>3) + STRAWB_SAVE_ADD] = (		  saveFile[(n>>3) + STRAWB_SAVE_ADD] & ~(1 << (n & 0x7))) | ((v & 0x1) << (n & 0x7));\
									 STRAWB_tempColl[(n>>3)					 ] = (STRAWB_tempColl[(n>>3)]					 & ~(1 << (n & 0x7))) | ((v & 0x1) << (n & 0x7))

unsigned char GAME_freeze;
unsigned int GAME_music_beat, GAME_life;
unsigned long GAME_speedrun_timer, GAME_IL_timer;
void (*cutsceneCoroutine)(void);
void (*cutsceneSkipped)(void);
unsigned int cutsceneInput, cutsceneWait;

unsigned char STRAWB_tempColl[15];

const char** chapter_pointers[LEVELIDX_MAX];
const char** chapter_pointersBside[LEVELIDX_MAX];
const int chapter_map_locations[LEVELIDX_MAX << 1];

int level_exits[4];
int enterX, enterY;
int screenShakeX, screenShakeY;

#define SAVEFILE_LEN		256
#define SAVEFILE_SHIFT		8
#define FILENAME_LEN		12
char saveFile[SAVEFILE_LEN], fileName[FILENAME_LEN];
int saveFileNumber;
const char emptyFileName[FILENAME_LEN];

int current_chapter, lvlIndex, currentMax, levelFlags;
char gamestate, nextGamestate;

#define IS_FADING (GAME_fadeAmount || GAME_fading)
int GAME_fadeAmount;
char GAME_fading;
short directColors[512];
int GAME_loadIndex;

int camX, camY, prevCamX, prevCamY;

int strawbUI_Pos, strawbUI_Offset1, strawbUI_Offset2, strawbUI_speed1, strawbUI_speed2;

int STRAWB_count, STRAWB_display, STRAWB_levelCount, STRAWB_levelMax;
unsigned int DEATH_count, DEATH_tempCount;

void SetCamPos();
void LeaveToMainMenu();
void Interrupt();
void StrawbUI(int force);
void LoadLevel();
void SaveTimer();
void LoadTimer();
void SetCore(int value);
