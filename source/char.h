#pragma once
#include "entities.h"

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

#define ISDEAD_MASK	  0x00000200
#define WALL_SLIDING  0x00000400
#define ONGROUND_MASK 0x00000800