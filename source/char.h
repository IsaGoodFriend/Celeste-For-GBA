#pragma once

#include "core.h"

#define CHARST_NORMAL	 0
#define CHARST_CLIMB	 1
#define CHARST_DASH		 2
#define CHARST_DREAMDASH 3
#define CHARST_ZIPLINE	 4
#define CHARST_COUNT	 5

#define ST_VIEWER		   5
#define StHitSquash		   6
#define StLaunch		   7
#define StPickup		   8
#define StSummitLaunch	   10
#define StDummy			   11
#define StIntroJump		   13
#define StIntroRespawn	   14
#define StIntroWakeUp	   15
#define StBirdDashTutorial 16
#define StFrozen		   17
#define StReflectionFall   18
#define StStarFly		   19
#define StTempleFall	   20
#define StCassetteFly	   21
#define StAttract		   22

extern COLOR hairColor[6];
extern COLOR skinStaminaColor[16];

extern Entity* CHAR_ent;
extern unsigned int deathAnimTimer;
extern int CHAR_PRIORITY;
extern int CHAR_umbrellaOffset;

extern int saveX, saveY;

int CHAR_init(unsigned int index, unsigned char* data, unsigned char* is_loading);
void CHAR_update(unsigned int);
void CHAR_render(unsigned int);

// void CHAR_update();
// void CHAR_save_loc();

// void CHAR_change_state(int value);

// void CHAR_dash_start();
// void CHAR_climb_start();
// void CHAR_zip_start();
// void CHAR_dreamdash_start();

// void CHAR_update_normal();
// void CHAR_update_dreamdash();
// void CHAR_update_climb();
// void CHAR_update_dash();
// void CHAR_update_zip();
// void CHAR_update_viewer();

// void CHAR_end_normal();

// void CHAR_dash_count(int value);
// void CHAR_stop_zipline();
// void CHAR_Die();
// void CHAR_Restart();
// void CHAR_refill_dash(char v);
// void set_hair_color();
// int IsInWater();
// void BubbleParticle(int offX, int rng);
// void JumpParticle(int offX, int rng);