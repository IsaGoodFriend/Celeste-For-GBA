#include "game_data.h"
#include "levels.h"
#include "physics.h"
#include "sprites.h"

#define UI_BOUNCE_START 0x240
#define UI_ENTER_SPEED	0x200
#define UI_GRAV			0x61

const char emptyFileName[FILENAME_LEN] = {
	0x0E,
	0x16,
	0x19,
	0x1D,
	0x22,
	0x24,
	0x24,
	0x24,
	0x24,
	0x24,
	0x24,
	0x24,
};

unsigned char STRAWB_tempColl[15];

char fileName[FILENAME_LEN];

int levelFlags;

int strawbUI_Pos, strawbUI_Timer, strawbUI_Offset1, strawbUI_Offset2, strawbUI_speed1, strawbUI_speed2;

int STRAWB_count, STRAWB_display, STRAWB_levelCount, STRAWB_levelMax;
int STRAWB_tempCount, forceDisplay;
unsigned int DEATH_count, DEATH_tempCount;

void StrawbUI(int force) {

	int count = STRAWB_count;

	if (strawbUI_speed1 || strawbUI_Offset1) {
		strawbUI_Offset1 += strawbUI_speed1;
		strawbUI_speed1 -= UI_GRAV;
		strawbUI_Offset2 += strawbUI_speed2;

		if (strawbUI_speed2)
			strawbUI_speed2 -= UI_GRAV;

		if (strawbUI_Offset1 < 0) {
			strawbUI_Offset1 = 0;
			strawbUI_Offset2 = 0;
			strawbUI_speed1	 = 0;
			strawbUI_speed2	 = 0;
		}
	} else if (count > STRAWB_display || force) {
		strawbUI_Pos += UI_ENTER_SPEED;
		if (strawbUI_Pos >= 0x1000) {
			if (!force) {
				int digitOld1 = STRAWB_display % 10;
				int digitOld2 = STRAWB_display / 10;

				++STRAWB_display;

				int digitNew1 = STRAWB_display % 10;
				int digitNew2 = STRAWB_display / 10;

				if (digitOld1 != digitNew1) {
					strawbUI_speed1 = UI_BOUNCE_START;
				}
				if (digitOld2 != digitNew2) {
					strawbUI_speed2 = UI_BOUNCE_START;
				}
			}
			strawbUI_Timer = 90;
			strawbUI_Pos   = 0x1000;
		}
	} else if (strawbUI_Timer) {
		--strawbUI_Timer;
	} else {
		strawbUI_Pos -= 0x100;
		if (strawbUI_Pos < 0) {
			strawbUI_Pos = 0;
		}
	}
}
