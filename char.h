//*
#include "physics.h"
#include "game_data.h"
#include "toolbox.h"

#define HAIR_PART_COUNT 4
#define HAIR_SPRITE_OFFSET 8

#define NORMAL_HITBOX_W 8
#define NORMAL_HITBOX_H 11
#define ZIP_GRAB_OFFSET 	0x400
#define ZIP_GRAB_HEIGHT		0x000
#define ZIP_GRAB_DISP		0x2
#define ZIP_RETENTION_TIME 	7
#define ZIP_SPEED			0x240
#define ZIP_SLOWDOWN		0xD0

#define F_MaxFall 680
#define F_Gravity 64
#define F_HalfGravThreshold 170

#define F_FastMaxFall 1020
#define F_FastMaxAccel 0x18

#define F_MaxRun 380
#define F_RunAccel 65
#define F_RunReduce 25
#define F_AirMult 0xA0

#define F_HoldingMaxRun 70 << ACC
#define F_HoldMinTime .35f

#define F_BounceAutoJumpTime .1f

#define F_DuckFriction 500 << ACC
#define DuckCorrectCheck 4
#define F_DuckCorrectSlide 50 << ACC

#define F_DodgeSlideSpeedMult 1.2f
#define F_DuckSuperJumpXMult 0x140
#define F_DuckSuperJumpYMult 0x88

#define F_JumpGraceTime 6
#define F_JumpSpeed -446
#define F_JumpHBoost 180
#define F_VarJumpTime 12
#define F_CeilingVarJumpGrace .05f
#define UpwardCornerCorrection 4
#define F_WallSpeedRetentionTime 6

#define WallJumpCheckDist 3
#define F_WallJumpForceTime 12
#define F_WallJumpHSpeed 560

#define F_WallSlideStartMax 20 << ACC
#define F_WallSlideTime 1.2f

#define F_BounceVarJumpTime .2f
#define F_BounceSpeed -140 << ACC
#define F_SuperBounceVarJumpTime .2f
#define F_SuperBounceSpeed -185 << ACC

#define F_SuperJumpSpeed JumpSpeed
#define F_SuperJumpH 1105
#define F_SuperWallJumpSpeed -700
#define F_SuperWallJumpVarTime 16
#define F_SuperWallJumpForceTime .2f
#define F_SuperWallJumpH (F_MaxRun + F_JumpHBoost * 2)

#define F_DashSpeed 1020
#define F_EndDashSpeed 680
#define F_EndDashUpMult .75f
#define Fr_DashTime 15
#define Fr_DashFrameEnd 5
#define Fr_AllowDashBack 10
#define F_DashCooldown .2f
#define F_DashRefillCooldown .1f
#define DashHJumpThruNudge 6
#define DashCornerCorrection 4
#define DashVFloorSnapDist 3
#define F_DashAttackTime .3f

#define F_BoostMoveSpeed 80 << ACC
#define F_BoostTime .25f

#define F_DuckWindMult 0
#define WindWallDistance 3

#define F_ReboundSpeedX 120 << ACC
#define F_ReboundSpeedY -120 << ACC
#define F_ReboundVarJumpTime .15f

#define F_ReflectBoundSpeed 220 << ACC

#define F_DreamDashSpeed DashSpeed
#define DreamDashEndWiggle 5
#define F_DreamDashMinTime .1f

#define F_ClimbMaxStamina 660
#define F_ClimbUpCost 5
#define F_ClimbUpCostSwim 3
#define F_ClimbStillCost 1
#define F_ClimbJumpCost (F_ClimbMaxStamina >> 2)

#define ClimbCheckDist 2
#define ClimbUpCheckDist 2
#define F_ClimbNoMoveTime 6
#define F_ClimbTiredThreshold 132
#define F_ClimbUpSpeed -191
#define F_ClimbDownSpeed 340
#define F_ClimbSlipSpeed 128
#define F_ClimbAccel 100
#define F_ClimbGrabYMult 0x80
#define F_ClimbHopY -350
#define F_ClimbHopX 300
#define F_ClimbHopForceTime 7
#define F_ClimbJumpBoostTime .2f
#define F_ClimbHopNoWindTime .3f

#define F_LaunchSpeed 280 << ACC
#define F_LaunchCancelThreshold 220 << ACC

#define F_LiftYCap -130 << ACC
#define F_LiftXCap 250 << ACC

#define F_JumpThruAssistSpeed -40 << ACC

#define F_InfiniteDashesTime 2 << ACC
#define F_InfiniteDashesFirstTime .5f
#define F_FlyPowerFlashTime .5f

#define F_ThrowRecoil 80 << ACC
#define F_CARRYOFF_X 0
#define F_CARRYOFF_Y -12 << ACC

#define F_ChaserStateMaxTime 4 << ACC

#define F_WalkSpeed 64 << ACC

#define ST_NORMAL 0
#define ST_CLIMB 1
#define ST_DASH 2
#define ST_SWIM 3
#define StBoost 4
#define StRedDash 5
#define StHitSquash 6
#define StLaunch 7
#define StPickup 8
#define ST_DREAMDASH 9
#define StSummitLaunch 10
#define StDummy 11
#define ST_ZIPLINE 12
#define StIntroJump 13
#define StIntroRespawn 14
#define StIntroWakeUp 15
#define StBirdDashTutorial 16
#define StFrozen 17
#define StReflectionFall 18
#define StStarFly 19
#define StTempleFall 20
#define StCassetteFly 21
#define StAttract 22

// Public flags
#define ONGROUND_MASK 			0x0010
#define ISDEAD_MASK 			0x0020
#define WALL_SLIDING 			0x0040

#define DASH_CURR_MASK 			0x0060
#define ONE_DASH_MASK 			0x0020
#define TWO_DASH_MASK 			0x0040
#define INFINI_DASH_MASK 		0x0060
#define DASH_CURR_SHIFT			5

#define TOTAL_DASH_MASK			0x0600
#define TOTAL_DASH0_MASK		0x0000
#define TOTAL_DASH1_MASK		0x0200
#define TOTAL_DASH2_MASK		0x0400
#define TOTAL_DASHINF_MASK		0x0600
#define TOTAL_DASH_SHIFT		9

#define CURRENT_DASH_STATUS 		((CHAR_flags & DASH_CURR_MASK) >> DASH_CURR_SHIFT)
#define TOTAL_DASH_STATUS 			((CHAR_flags & TOTAL_DASH_MASK) >> TOTAL_DASH_SHIFT)

COLOR hairColor[6];

Actor *CHAR_phys;
unsigned short CHAR_flags;
int CHAR_PRIORITY;
int CHAR_umbrellaOffset;
unsigned char CHAR_state;

void CHAR_update();
void CHAR_save_loc();
void CHAR_end_normal();
void CHAR_zip_start();
void CHAR_update_normal();
void CHAR_dash_update();
void CHAR_change_state(int value);
void CHAR_dash_start();
void CHAR_climb_start();
void CHAR_dreamdash_start();
void CHAR_dash_count(int value);
void CHAR_stop_zipline();
void CHAR_Die();
void CHAR_Restart();
void CHAR_refill_dash(char v);

//*/
