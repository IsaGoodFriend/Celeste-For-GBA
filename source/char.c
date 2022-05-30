#include "char.h"
//#include "core.h"
#include "global_vars.h"
#include "graphics.h"
#include "input.h"
#include "level_handler.h"
#include "physics.h"
#include "sprites.h"
#include "state_machine.h"

#define IS_RAINY 0

Entity* CHAR_ent;
StateMachine CHAR_state;

int charsp_indices[5];

unsigned int CHAR_dashTimer, staminaBlink;

int horz;
int vert;

int saveX, saveY;

int forceMoveX, varJumpSpeed, lastClimbMove, climbNoMoveTimer, stamina, wallSpeedRetained, climbJumpSave;

int hairPoints[8] = {0, 0, 0, 0, 0, 0, 0, 0};

COLOR hairColor[6];
COLOR skinStaminaColor[16];

// // timers
int speedRetentionTimer;
unsigned int varJumpTimer, zipJumpBuffer, grabBufferTimer, forceMoveXTimer, coyoteTimer, deathAnimTimer, dreamDashStayTimer;
int grabbed_entity;

int squish_x, squish_y, anim_state, hairPos;

int speedVert, speedHorz;
int CHAR_PRIORITY;
int CHAR_umbrellaOffset;

#pragma region

#define BLOCKCLIMB_COLLISION (DANGER_GENERIC_COLLISION)

#define JUMP_BUFFER 5
#define DASH_BUFFER 5

#define HAIR_DIST 0x280

#define WALL_HITBOX			   (IsDashingBuffered && IS_DASH_DIR(DASHING_UP) ? WALL_HITBOX_WallBounce : WALL_HITBOX_Normal)
#define WALL_HITBOX_Normal	   3
#define WALL_HITBOX_WallBounce 5
#define WALL_HITBOX_H		   11
#define DUCK_DIFF			   5

#define SPRING_hold_time 4
#define SPRING_bounce	 -0x4C0

#define HAIR_PART_COUNT	   4
#define HAIR_SPRITE_OFFSET 8

#define NORMAL_HITBOX_W 8
#define NORMAL_HITBOX_H 11

#define ZIP_GRAB_OFFSET	   0x400
#define ZIP_GRAB_HEIGHT	   0x000
#define ZIP_GRAB_DISP	   0x2
#define ZIP_RETENTION_TIME 7
#define ZIP_SPEED		   0x240
#define ZIP_SLOWDOWN	   0xD0

#define F_MaxRun	0x180
#define F_RunAccel	65
#define F_RunReduce 25
#define F_AirMult	0xA8
#define COYOTE_MAX	6 // CoyoteTimer

#define F_JumpSpeed			  -0x1C0
#define F_JumpHBoost		  0xA8
#define F_VarJumpTime		  12
#define F_CeilingVarJumpGrace 3

#define F_SuperJumpSpeed		 JumpSpeed
#define F_SuperJumpH			 F_DashSpeed
#define F_SuperWallJumpSpeed	 -700
#define F_SuperWallJumpVarTime	 14
#define F_SuperWallJumpForceTime .2f
#define F_SuperWallJumpH		 0x2D4

#define F_DashSpeed			 0x400
#define F_EndDashSpeed		 0x200
#define F_EndDashUpMult		 .75f
#define IsDashing			 (CHAR_dashTimer > Fr_DashBuffer)
#define IsDashingBuffered	 (CHAR_dashTimer > 0)
#define Fr_DashInit			 (Fr_DashTime + Fr_DashBuffer)
#define Fr_DashTime			 10
#define Fr_DashBuffer		 5
#define Fr_AllowDashBack	 (Fr_DashInit - 6)
#define F_DashCooldown		 .2f
#define F_DashRefillCooldown .1f
#define DashHJumpThruNudge	 6
#define DashCornerCorrection 5
#define DashVFloorSnapDist	 3
#define F_DashAttackTime	 .3f

#define UpwardCornerCorrection	 4
#define F_WallSpeedRetentionTime 6

#define F_Gravity	  0x40
#define F_MaxFall	  0x2A8
#define F_FastMaxFall 0x400

#define F_HalfGravThreshold 170
#define F_FastMaxAccel		0x18

#define F_HoldingMaxRun 70 << ACC
#define F_HoldMinTime	.35f

#define F_BounceAutoJumpTime .1f

#define F_DuckFriction	   500 << ACC
#define DuckCorrectCheck   4
#define F_DuckCorrectSlide 50 << ACC

#define F_DodgeSlideSpeedMult 1.2f
#define F_DuckSuperJumpXMult  0x140
#define F_DuckSuperJumpYMult  0x88

#define WallJumpCheckDist	3
#define F_WallJumpForceTime 12
#define F_WallJumpHSpeed	560

#define F_WallSlideStartMax 20 << ACC
#define F_WallSlideTime		1.2f

#define F_BoostMoveSpeed 80 << ACC
#define F_BoostTime		 .25f

#define F_DuckWindMult	 0
#define WindWallDistance 3

#define F_ReboundSpeedX		 120 << ACC
#define F_ReboundSpeedY		 -120 << ACC
#define F_ReboundVarJumpTime .15f

#define F_ReflectBoundSpeed 220 << ACC

#define F_DreamDashSpeed   DashSpeed
#define DreamDashEndWiggle 5
#define F_DreamDashMinTime .1f

#define F_ClimbMaxStamina 660
#define F_ClimbUpCost	  5
#define F_ClimbUpCostSwim 3
#define F_ClimbStillCost  1
#define F_ClimbJumpCost	  (F_ClimbMaxStamina >> 2)

#define ClimbCheckDist		  2
#define ClimbUpCheckDist	  2
#define F_ClimbNoMoveTime	  6
#define F_ClimbTiredThreshold 132
#define F_ClimbUpSpeed		  -191
#define F_ClimbDownSpeed	  340
#define F_ClimbSlipSpeed	  128
#define F_ClimbAccel		  100
#define F_ClimbGrabYMult	  0x80
#define F_ClimbHopY			  -350
#define F_ClimbHopX			  300
#define F_ClimbHopForceTime	  7
#define F_ClimbJumpBoostTime  .2f
#define F_ClimbHopNoWindTime  .3f

#define F_LaunchSpeed			280 << ACC
#define F_LaunchCancelThreshold 220 << ACC

#define F_LiftYCap -130 << ACC
#define F_LiftXCap 250 << ACC

#define F_JumpThruAssistSpeed -40 << ACC

#define F_InfiniteDashesTime	  2 << ACC
#define F_InfiniteDashesFirstTime .5f
#define F_FlyPowerFlashTime		  .5f

#define F_ThrowRecoil 80 << ACC
#define F_CARRYOFF_X  0
#define F_CARRYOFF_Y  -12 << ACC

#define F_ChaserStateMaxTime 4 << ACC

#define F_WalkSpeed 64 << ACC

#pragma endregion

// Flag zero (Dashing)
#define DASH_HORZ_MASK	0x00000003
#define DASH_VERT_MASK	0x0000000C
#define DASH_DIR_MASK	0x0000000F
#define DASHING_UP		0x0000000C
#define DASHING_DOWN	0x00000004
#define IS_DASH_DIR(f)	(!((CHAR_ent->flags[0] & DASH_DIR_MASK) ^ f))
#define HAS_DASH_DIR(f) ((CHAR_ent->flags[0] & DASH_DIR_MASK) & f)

#define DASH_AMOUNT_MASK	0x00000030
#define SINGLE_DASH_MASK	0x00000010
#define DASH_AMOUNT_SHIFT	4
#define CURRENT_DASH_STATUS ((CHAR_ent->flags[0] & DASH_AMOUNT_MASK) >> DASH_AMOUNT_SHIFT)

#define TOTAL_DASH_MASK		0x000000C0
#define TOTAL_DASH_SHIFT	6
#define DASH_COUNT0_MASK	0x00000000
#define DASH_COUNT1_MASK	0x00000040
#define DASH_COUNT2_MASK	0x00000080
#define DASH_COUNT_INF_MASK 0x000000C0
#define TOTAL_DASH_STATUS	((CHAR_ent->flags[0] & TOTAL_DASH_MASK) >> TOTAL_DASH_SHIFT)

#define DASH_REFILL_MASK 0x00000100
#define CAN_REFILL_DASH	 (~CHAR_ent->flags[0] & DASH_REFILL_MASK)

#define ISDEAD_MASK	  0x00000200
#define WALL_SLIDING  0x00000400
#define ONGROUND_MASK 0x00000800

#define FACE_LEFT		 0x00001000
#define FACING_DIRECTION ((CHAR_ent->flags[0] & FACE_LEFT) ? -1 : 1)
#define SET_FACING(v)	 CHAR_ent->flags[0] = (CHAR_ent->flags[0] & ~FACE_LEFT) | (v < 0 ? FACE_LEFT : 0)

// Non flag code
#define IS_TIRED   (stamina <= F_ClimbTiredThreshold)
#define CAN_JUMP   (coyoteTimer)
#define IS_DUCKING (CHAR_ent->height < NORMAL_HITBOX_H)

#define ON_GROUND (coyoteTimer == COYOTE_MAX)

#if 1 // Animations

typedef enum {
	ANIM_IDLE,
	ANIM_WALK,
	ANIM_SKID,
	ANIM_CROUCH,
	ANIM_JUMP,
	ANIM_JUMP_MID,
	ANIM_DASH,
	ANIM_DEAD,
	ANIM_LOOKUP,
	ANIM_FALL,
} AnimationState;

#endif

int NORMAL_update();
int DASH_update();
int CLIMB_update();
void DASH_begin();
unsigned int GetWall(int, int);

void Jump();

void CHAR_dash_count(int);

void CHAR_render(unsigned int index) {
	CHAR_update_anim();

	set_hair_color();
	COLOR* palette = &pal_obj_mem[3];
	COLOR* lookUp  = (COLOR*)&skinStaminaColor;

	staminaBlink += IS_TIRED;
	lookUp += (IS_TIRED && (staminaBlink & 0x8)) * 8;

	AffineMatrix squish = matrix_multiply(matrix_multiply(matrix_identity(), matrix_trans(0, -0x800)), matrix_scale(squish_x * ((CHAR_ent->flags[0] & FACE_LEFT) ? -1 : 1), squish_y));

	AffineMatrix globalTransform = matrix_multiply(matrix_identity(), matrix_trans(CHAR_ent->x + 0x400, CHAR_ent->y + (IS_DUCKING ? 0x600 : 0xB00)));

	AffineMatrix matrix = matrix_multiply(squish, globalTransform);

	draw_affine_big(matrix_multiply(squish, globalTransform), charsp_indices[0], 0, 0);
	// matrix = matrix_multiply(matrix, matrix_trans(0x1000, 0));

	matrix = matrix_multiply(matrix_multiply(matrix_trans(INT2FIXED((hairPos & 0xF) - 8), -INT2FIXED(hairPos >> 4)), squish), globalTransform);
	draw_affine_big(matrix, charsp_indices[1], 0, 0);
	draw_affine_big(matrix, charsp_indices[2], 0, 0);
}
int CHAR_init(unsigned int index, unsigned char* data, unsigned char* is_loading) {

	if (CHAR_ent) {
		*is_loading = 0;
		return 0;
	}

	memcpy16(&hairColor[0], &PAL_hair[1], 6);
	// hairColor[0] = &PAL_hair[1];

	anim_state = 1;

	charsp_indices[0] = load_sprite(SPR_madeline_idle, SPRITE16x16);
	charsp_indices[1] = load_sprite(SPR_bangs_color, SPRITE16x8);
	charsp_indices[2] = load_sprite(SPR_bangs_outline, SPRITE16x8);
	charsp_indices[3] = load_sprite(SPR_hair_color, SPRITE16x16);
	charsp_indices[4] = load_sprite(SPR_hair_outline, SPRITE16x16);

	CHAR_ent = &entities[index];

	squish_x = 0x100;
	squish_y = 0x100;

	CHAR_ent->y -= 0x300;
	CHAR_ent->width	 = NORMAL_HITBOX_W;
	CHAR_ent->height = NORMAL_HITBOX_H;

	CHAR_dash_count(1);

	CHAR_refill_dash(1);
	CHAR_refill_stamina();

	init_statemachine(&CHAR_state, CHARST_COUNT);
	set_update_state(&CHAR_state, NORMAL_update, CHARST_NORMAL);
	set_update_state(&CHAR_state, DASH_update, CHARST_DASH);
	set_update_state(&CHAR_state, CLIMB_update, CHARST_CLIMB);
	set_begin_state(&CHAR_state, DASH_begin, CHARST_DASH);

	CHAR_state.state = CHARST_NORMAL;

	load_obj_pal(PAL_madeline, 0);

	ENABLE_ENT_FLAG(PERSISTENT, index);

	set_cam();

	return 0;
}
void CHAR_update_anim() {

	int new_state = ANIM_IDLE;

	if (deathAnimTimer != 0) {
		new_state = ANIM_DEAD;
	} else {
		switch (CHAR_state.state) {
			case CHARST_DREAMDASH:
				break;
			//
			case CHARST_CLIMB:
				break;
			//
			case ST_VIEWER:
				break;
			//
			case CHARST_ZIPLINE:
				break;
			case CHARST_NORMAL:
				{
					if (IS_DUCKING) {
						new_state = ANIM_CROUCH;
					} else if (ON_GROUND) {
						if (CHAR_ent->vel_x == 0) {
							new_state = vert < 0 ? ANIM_LOOKUP : ANIM_IDLE;
						} else if (INT_SIGN(CHAR_ent->vel_x) == horz) {
							new_state = ANIM_WALK;
						} else {
							new_state = ANIM_WALK;
						}
					} else {
						if (CHAR_ent->vel_y > 0) {
							new_state = ANIM_FALL;
						} else {
							if (varJumpTimer)
								new_state = ANIM_JUMP;
							else
								new_state = ANIM_JUMP_MID;
						}
					}

					break;
				}
			//
			case CHARST_DASH:
				{
					if (IS_DUCKING)
						new_state = ANIM_CROUCH;
					else
						new_state = ANIM_DASH;
					break;
				}
		}
	}

	if (new_state != anim_state) {
		anim_state = new_state;

		switch (anim_state) {
			default:
			case ANIM_IDLE:
				load_anim_sprite_at(&SPR_madeline_idle, charsp_indices[0], SPRITE16x16, 4, 16);
				break;
			case ANIM_CROUCH:
				load_sprite_at(&SPR_madeline_duck, charsp_indices[0], SPRITE16x16);
				hairPos = 0x18;
				break;
			case ANIM_WALK:
				load_anim_sprite_at(&SPR_madeline_walk, charsp_indices[0], SPRITE16x16, 6, 8);
				break;
			case ANIM_DEAD:
				load_sprite_at(&SPR_madeline_dead, charsp_indices[0], SPRITE16x16);
				hairPos = 0x57;
				break;
			case ANIM_DASH:
				load_sprite_at(&SPR_madeline_dash, charsp_indices[0], SPRITE16x16);
				hairPos = 0x4A;
				break;
			case ANIM_LOOKUP:
				load_sprite_at(&SPR_madeline_look_up, charsp_indices[0], SPRITE16x16);
				hairPos = 0x46;
				break;
			case ANIM_FALL:
				load_sprite_at(&SPR_madeline_fall, charsp_indices[0], SPRITE16x16);
				hairPos = 0x58;
				break;
			case ANIM_JUMP:
				load_sprite_at(&SPR_madeline_jump, charsp_indices[0], SPRITE16x16);
				hairPos = 0x69;
				break;
			case ANIM_JUMP_MID:
				load_sprite_at(&SPR_madeline_air_mid, charsp_indices[0], SPRITE16x16);
				hairPos = 0x58;
				break;
		}
	}
	switch (anim_state) {
		default:
			break;
		case ANIM_IDLE:
			hairPos = 0x58 - ((get_anim_frame(charsp_indices[0]) & 0x2) << 3);
			break;
		case ANIM_WALK:
			hairPos = 0x60 - ((get_anim_frame(charsp_indices[0]) % 3) << 4);
			hairPos |= 0x9;
			break;
	}
}

// Actions
#pragma region
void Jump(int dirX) {

	// if (CAN_JUMP) {
	// 	speedRetentionTimer = 0;
	// }

	coyoteTimer	   = 0;
	CHAR_dashTimer = 0;

	squish_x = 0xA0;
	squish_y = 0x160;

	if (dirX != 0) {
		// Allow double corner boosting?
		if (speedRetentionTimer)
			wallSpeedRetained += F_JumpHBoost * dirX;
		else
			CHAR_ent->vel_x += F_JumpHBoost * dirX;
	}
	CHAR_ent->vel_y = F_JumpSpeed;

	varJumpTimer = F_VarJumpTime;
	varJumpSpeed = CHAR_ent->vel_y;
}
void ClimbJump() {

	if (!ON_GROUND) { // || (CHAR_ent->flags[0] & GROUND_JUMP_MASK)
		stamina -= F_ClimbJumpCost;
		// if (!IS_TIRED)
		// 	staminaBlink = 3;
	}

	climbJumpSave = FACING_DIRECTION * -12;

	Jump(horz);
}
void SuperJump(int dir) {
	// if (CHAR_ent->flags[0] & GROUND_JUMP_MASK)
	// 	return;

	varJumpTimer   = F_VarJumpTime;
	speedHorz	   = 12;
	coyoteTimer	   = 0;
	CHAR_dashTimer = 0;

	if (!dir)
		dir = FACING_DIRECTION;

	// CHAR_ent->vel_x += 0x04 * dir;
	CHAR_ent->vel_x = F_SuperJumpH * dir;
	CHAR_ent->vel_y = F_JumpSpeed;

	if (IS_DUCKING) {
		SET_DUCKING(0);
		CHAR_ent->vel_x = FIXED_MULT(CHAR_ent->vel_x, F_DuckSuperJumpXMult);
		CHAR_ent->vel_y = FIXED_MULT(CHAR_ent->vel_y, F_DuckSuperJumpYMult);
	}

	squish_x = 0xA0;
	squish_y = 0x160;

	varJumpSpeed = CHAR_ent->vel_y;
}
void ClimbHop(int dir) {

	CHAR_ent->vel_x = dir * F_ClimbHopX;

	CHAR_ent->vel_y = SIGNED_MIN(CHAR_ent->vel_y, F_ClimbHopY);
	forceMoveX		= 0;
	forceMoveXTimer = F_ClimbHopForceTime;
}
void SuperWallJump(int dir) {

	speedVert = 12;

	varJumpTimer	= F_SuperWallJumpVarTime;
	CHAR_dashTimer	= 0;
	forceMoveXTimer = 0;

	CHAR_ent->vel_x = F_SuperWallJumpH * dir;
	CHAR_ent->vel_y = F_SuperWallJumpSpeed;
	varJumpSpeed	= CHAR_ent->vel_y;

	squish_x = 0xA0;
	squish_y = 0x160;
}
void WallJump(int dir, int moveX) {

	climbJumpSave = 0;

	varJumpTimer   = F_VarJumpTime;
	CHAR_dashTimer = 0;

	if (moveX != 0) {
		forceMoveX		= INT_SIGN(dir);
		forceMoveXTimer = F_WallJumpForceTime;
		if (IS_RAINY)
			forceMoveXTimer += 3;
	}

	squish_x = 0xB0;
	squish_y = 0x148;

	CHAR_ent->vel_x = INT_SIGN(dir) * (int)(F_WallJumpHSpeed);
	CHAR_ent->vel_y = F_JumpSpeed;
	varJumpSpeed	= CHAR_ent->vel_y;
}

#pragma endregion

// State machine updates
#pragma region
int NORMAL_update() {

	CHAR_ent->flags[0] &= ~WALL_SLIDING;

	if (forceMoveXTimer) {
		if (forceMoveX)
			SET_FACING(forceMoveX);
	} else if (horz)
		SET_FACING(horz);
	// Walking
	if (IS_DUCKING && ON_GROUND)
		horz = 0;

	// force walking
	forceMoveXTimer -= forceMoveXTimer > 0;
	if (forceMoveXTimer > 0)
		horz = forceMoveX;

	unsigned int wallL, wallR;

	wallL = GetWall(-1, WALL_HITBOX);
	wallR = GetWall(1, WALL_HITBOX);

	// Start climbing state
	if (pixtro_key_check(KEY_SHOULDER) && !(IS_TIRED || IS_RAINY)) {
		int facingLeft = FACING_DIRECTION;

		if (CHAR_ent->vel_y >= 0 && ((INT_SIGN(CHAR_ent->vel_x) == -facingLeft) || CHAR_ent->vel_x == 0)) {
			if (facingLeft == -1 ? wallL : wallR) {
				CHAR_ent->x = ((CHAR_ent->x + INT2FIXED(facingLeft == 1 ? 0 : WALL_HITBOX + NORMAL_HITBOX_W)) & ~0x7FF) - INT2FIXED((facingLeft == 1) ? 0 : NORMAL_HITBOX_W);
				SET_DUCKING(0);

				return CHARST_CLIMB;
			}
		}
	}

	// Dash
	if (/*(!CHALLENGE_MODE || current_chapter != LEVELIDX_PROLOGUE) &&*/ key_pressed(KEY_B, 4) && CURRENT_DASH_STATUS) // Start dash
	{
		clear_press(KEY_B);
		RemoveDash();

		return CHARST_DASH;
	}

	SET_DUCKING(vert > 0 && ON_GROUND && CHAR_ent->vel_y <= 0);

	// Walking
	int mult = (ON_GROUND) ? 0x100 : F_AirMult;

	int max = F_MaxRun;

	if (INT_ABS(CHAR_ent->vel_x) > max && INT_SIGN(CHAR_ent->vel_x) == horz)
		CHAR_ent->vel_x = FIXED_APPROACH(CHAR_ent->vel_x, FIXED_MULT(max, horz), FIXED_MULT(F_RunReduce, mult)); // Reduce back from beyond the max speed
	else
		CHAR_ent->vel_x = FIXED_APPROACH(CHAR_ent->vel_x, max * horz, FIXED_MULT(F_RunAccel, mult)); // Approach the max speed

	// Vertical
	int maxFall = 0;
	// Calculate current max fall speed
	maxFall		= F_MaxFall;
	if (vert == 1)
		maxFall = F_FastMaxFall;

	// Gravity
	if (!ON_GROUND) {
		int wallSlideDir = 0;

		// Wall Slide
		int facingLeft = FACING_DIRECTION;

		if ((horz != 0 || (horz == 0 && pixtro_key_check(KEY_SHOULDER))) && vert < 1) {
			if (CHAR_ent->vel_y >= 0 && (facingLeft == -1 ? wallL : wallR)) {
				SET_DUCKING(0);
				wallSlideDir = (int)facingLeft;
			}

			if (wallSlideDir != 0) {
				maxFall = FIXED_MULT(maxFall, 0x70);
				CHAR_ent->flags[0] |= WALL_SLIDING;
			}
		}
		if ((IS_RAINY) && !(CHAR_ent->flags[0] & WALL_SLIDING) && vert < 1) {
			maxFall = FIXED_MULT(maxFall, 0x70);
		}
	}

	mult = (INT_ABS(CHAR_ent->vel_y) < F_HalfGravThreshold && pixtro_key_check(KEY_A)) ? 0x80 : 0x100;

	CHAR_ent->vel_y = FIXED_APPROACH(CHAR_ent->vel_y, maxFall, FIXED_MULT(F_Gravity, mult));

	// Variable Jumping
	if (varJumpTimer > 0) {
		--varJumpTimer;

		if (pixtro_key_check(KEY_A))
			CHAR_ent->vel_y = varJumpSpeed;
		else
			varJumpTimer = 0;
	}

	// Jumping - (Normal state)
	if (key_pressed(KEY_A, JUMP_BUFFER)) {

		if (CAN_JUMP) {
			clear_press(KEY_A);

			if (CHAR_dashTimer > DASH_BUFFER && CHAR_ent->vel_y >= 0 && CHAR_ent->vel_x != 0)
				SuperJump(horz);
			else
				Jump(horz);
		} else {

			if (wallL || wallR) {
				clear_press(KEY_A);

				int facingLeft = FACING_DIRECTION;
				int jumpDir	   = (wallL && wallR) ? -facingLeft : (wallL ? 1 : -1);

				if (pixtro_key_check(KEY_SHOULDER) && stamina > 0 && (facingLeft == -1 ? wallL : wallR) && !(IS_RAINY && INT_ABS(CHAR_ent->vel_x) < 0x40)) {
					ClimbJump();
				} else if (CHAR_dashTimer > 0 && (IS_DASH_DIR(DASHING_UP))) {
					SuperWallJump(jumpDir);
				} else {
					WallJump(jumpDir, horz);
				}
			}
		}
	}

	// if (ON_GROUND && CHAR_ent->vel_x == 0 && CHAR_ent->vel_y == 0 /*&& (width > 32 || height > 22)*/ && key_hit(KEY_SELECT)) {
	// 	// set_statemachine(&CHAR_state, ST_VIEWER);
	// 	CHAR_ent->vel_x = 0;
	// 	CHAR_ent->vel_y = 0;
	// 	forceMoveX		= 0;
	// }

	return -1;
}
void DASH_begin() {

	game_freeze	   = 3;
	speedHorz	   = 0;
	speedVert	   = 0;
	CHAR_dashTimer = Fr_DashInit + 1;
	varJumpTimer   = 0;
	coyoteTimer	   = 0;

	squish_x = 0x100;
	squish_y = 0x100;

	// CHAR_ent->vel_x = 0;
	// CHAR_ent->vel_y = 0;

	if (vert > 0) {
		SET_DUCKING(1);
	} else {
		SET_DUCKING(0);
	}

	return;

	int i;

	for (i = 0; i < ENTITY_LIMIT; ++i) {

		if (!ENT_FLAG(ACTIVE, i))
			continue;

		int id = ENT_TYPE(i);

		// switch (id) {
		// 	case 1:
		// 		STRAWB_ondash(&entities[i]);
		// 		break;
		// }
	}
}
int DASH_update() {

	if (CHAR_ent->vel_y > 0)
		SET_DUCKING(0);

	// initialize dash direction
	if (CHAR_dashTimer >= Fr_DashInit) {

		CHAR_ent->flags[0] &= ~0xF;

		if (horz) {
			CHAR_ent->flags[0] |= ((horz < 0) ? 0x2 : 0x0) | 0x1;
		}
		if (vert) {
			CHAR_ent->flags[0] |= ((vert < 0) ? 0x8 : 0x0) | 0x4;
		}

		// if no dash direction, set to horizontal
		if (!(CHAR_ent->flags[0] & 0xF)) {
			CHAR_ent->flags[0] |= 0x1 | ((CHAR_ent->flags[0] & FACE_LEFT)) >> 3;
		}

		// set speed
		set_dash_speed();

		// if (CHAR_ent->vel_y < 0)
		// 	SET_DUCKING(0);

		if (CHAR_ent->vel_x)
			SET_FACING(CHAR_ent->vel_x);
	}

	// Jumping - (Dash State)
	if (key_pressed(KEY_A, JUMP_BUFFER)) {

		unsigned int wallFace = GetWall(FACING_DIRECTION, WALL_HITBOX);
		unsigned int wallAway = GetWall(-FACING_DIRECTION, WALL_HITBOX);

		if (pixtro_key_check(KEY_SHOULDER) && stamina > 0 && wallFace && !IS_RAINY) {
			ClimbJump();
			clear_press(KEY_A);

			return CHARST_NORMAL;
		} else if (IS_DASH_DIR(DASHING_UP) && (wallFace || wallAway)) {
			clear_press(KEY_A);

			SuperWallJump(wallFace ? -FACING_DIRECTION : FACING_DIRECTION);
			return CHARST_NORMAL;

		} else if (CAN_JUMP) {
			clear_press(KEY_A);

			if (IS_DASH_DIR(DASHING_DOWN))
				Jump(0);
			else
				SuperJump(horz);

			return CHARST_NORMAL;
		} else if (wallFace || wallAway) {

			clear_press(KEY_A);

			WallJump(wallFace ? -FACING_DIRECTION : FACING_DIRECTION, horz);

			return CHARST_NORMAL;
		}
	}

	// End dash
	if (CHAR_dashTimer <= Fr_DashBuffer) {

		// Slow down velocity after dash
		int prevX = CHAR_ent->vel_x;

		CHAR_ent->vel_y = F_EndDashSpeed * INT_SIGN(CHAR_ent->vel_y);
		CHAR_ent->vel_x = F_EndDashSpeed * INT_SIGN(CHAR_ent->vel_x);

		if (CHAR_ent->vel_x && CHAR_ent->vel_y) {
			CHAR_ent->vel_x = FIXED_MULT(CHAR_ent->vel_x, 0xB5);
			CHAR_ent->vel_y = FIXED_MULT(CHAR_ent->vel_y, 0xB5);
		}
		if (CHAR_ent->vel_y < 0) {
			CHAR_ent->vel_y = FIXED_MULT(CHAR_ent->vel_y, 0xC0);
		}

		if (CHAR_ent->vel_y > 0 && prevX) {
			CHAR_ent->vel_x = prevX;
		}

		return CHARST_NORMAL;
	}

	return -1;
}
int CLIMB_update() {
	int facingLeft	  = FACING_DIRECTION;
	unsigned int wall = GetWall(facingLeft, WALL_HITBOX);

	if (climbNoMoveTimer >= F_ClimbNoMoveTime - 1) {
		CHAR_ent->vel_y = FIXED_MULT(CHAR_ent->vel_y, F_ClimbGrabYMult);
		CHAR_ent->vel_x = 0;
	}
	if (climbNoMoveTimer)
		--climbNoMoveTimer;

	// Refill stamina on ground
	if (ON_GROUND)
		CHAR_refill_stamina();

	// Wall Jump - (Climb state)
	if (key_pressed(KEY_A, JUMP_BUFFER)) {
		clear_press(KEY_A);

		if (horz == -facingLeft || stamina <= 0)
			WallJump(-facingLeft, horz);
		else
			ClimbJump();

		set_statemachine(&CHAR_state, CHARST_NORMAL);
		return -1;
	}

	// Let go
	if (!pixtro_key_check(KEY_SHOULDER)) {
		set_statemachine(&CHAR_state, CHARST_NORMAL);
		return -1;
	}

	// No wall to hold
	if (!wall) {
		// Climbed over ledge?
		if (CHAR_ent->vel_y < 0) {
			ClimbHop(facingLeft);
		}
		set_statemachine(&CHAR_state, CHARST_NORMAL);
		return -1;
	}

	// Climbing
	int target	 = 0;
	char trySlip = 0;
	if (climbNoMoveTimer <= 0) {
		if (vert == -1) {
			target	 = F_ClimbUpSpeed;
			int xPos = CHAR_ent->x + (facingLeft == 1 ? -INT2FIXED(WALL_HITBOX) : INT2FIXED(NORMAL_HITBOX_W));

			// Up Limit
			if ((collide_rect(xPos, (CHAR_ent->y) + 0x200, WALL_HITBOX, 1, BLOCKCLIMB_COLLISION)) &&
				(collide_rect(xPos, (CHAR_ent->y) + 0xA00, WALL_HITBOX, 1, SOLID_COLLISION))) {
				if (CHAR_ent->vel_y < 0)
					CHAR_ent->vel_y = 0;
				target	= 0;
				trySlip = 1;
			}
		} else if (vert == 1) {
			target = F_ClimbDownSpeed;

			if (ON_GROUND) {
				target = 0;
			}
		} else
			trySlip = 1;
	} else
		trySlip = 1;

	if (target == 0)
		lastClimbMove = 0;
	else
		lastClimbMove = INT_SIGN(target);

	{
		int xPos = CHAR_ent->x + (facingLeft == 1 ? -INT2FIXED(WALL_HITBOX) : INT2FIXED(NORMAL_HITBOX_W));

		// Slip down if hands above the ledge and no vertical input
		if (trySlip && !(collide_rect(xPos, CHAR_ent->y, WALL_HITBOX, NORMAL_HITBOX_H - 8, SOLID_COLLISION)))
			target = F_ClimbSlipSpeed;
	}

	// Set Speed
	CHAR_ent->vel_y = FIXED_APPROACH(CHAR_ent->vel_y, target, F_ClimbAccel);

	// Down Limit
	// if (vert != 1 && CHAR_ent->vel_y > 0)
	// CHAR_ent->vel_y = 0;

	// Stamina
	//*
	if (!ON_GROUND) {
		if (lastClimbMove == -1) {
			stamina -= F_ClimbUpCost;
		} else if (lastClimbMove == 0)
			stamina -= F_ClimbStillCost;
	}
	//*/

	// Too tired
	if (stamina <= 0) {
		stamina = 0;
		// Speed += LiftBoost;
		set_statemachine(&CHAR_state, CHARST_NORMAL);
		return -1;
	}
	//*/

	// [dash]
	// if ((!CHALLENGE_MODE || current_chapter != LEVELIDX_PROLOGUE) && dashBufferTimer && CURRENT_DASH_STATUS) // Start dash
	// {

	// 	RemoveDash();
	// 	set_statemachine(&CHAR_state, CHARST_DASH);
	// }

	return -1;
}
#pragma endregion

// void CHAR_render(OBJ_ATTR* buffer, int camX, int camY, int* count, int pal) {
// 	if (FIXED2INT(CHAR_ent->y) - camY < -0x40 ||
// 		FIXED2INT(CHAR_ent->y) - camY > 184 ||
// 		FIXED2INT(CHAR_ent->x) - camX < -0x40 ||
// 		FIXED2INT(CHAR_ent->x) - camX > 264)
// 		return;
// 	char displayHair = !(levelFlags & LEVELFLAG_TAS);
// 	deathAnimTimer -= deathAnimTimer > 60 && IS_FADING;
// 	// Death Animation
// 	if (deathAnimTimer && deathAnimTimer <= 60) {
// 		int currX, currY;
// 		deathAnimTimer -= (deathAnimTimer >= 30 && GAME_fading) ||
// 						  (CHAR_ent->x == saveX && CHAR_ent->y == saveY && deathAnimTimer <= 30);
// 		int offsetX = -camX - 4, offsetY = -camY - 5;
// 		int x = CHAR_ent->x - 0x400;
// 		int y = CHAR_ent->y - 0x400;
// 		if (FIXED2INT(CHAR_ent->y) - camY > 152)
// 			y = INT2FIXED(152 + camY);
// 		int offset = (deathAnimTimer > 30 ? (60 - deathAnimTimer) : deathAnimTimer) + 2;
// 		if (offset > 20 && offset & 1)
// 			return;
// 		offsetX = -camX;
// 		offsetY = -camY;
// 		currX	= 4;
// 		currY	= 0;
// 		int hairI = 0;
// 		for (; hairI < 6; ++hairI) {
// 			// Death anim part
// 			obj_set_attr((buffer + *count),
// 						 ATTR0_SQUARE | ATTR0_Y(FIXED2INT(y + (currY * offset * 0x70)) - camY),	 // ATTR0
// 						 ATTR1_SIZE_16 | ATTR1_X(FIXED2INT(x + (currX * offset * 0x70)) - camX), // ATTR1
// 						 ATTR2_PALBANK(0) | (HAIR_SPRITE_OFFSET) | ATTR2_PRIO(0));				 // ATTR2
// 			++*count;
// 			if (!currY) {
// 				currY = currX - INT_SIGN(currX);
// 				currX = currX / 2;
// 			} else if (INT_SIGN(currY) == INT_SIGN(currX)) {
// 				currX = -currX;
// 			} else {
// 				currX = currX * 2;
// 				currY = 0;
// 			}
// 		}
// 		for (hairI = 0; hairI < 6; ++hairI) {
// 			// death anim part
// 			obj_set_attr((buffer + *count),
// 						 ATTR0_SQUARE | ATTR0_Y(FIXED2INT(y + (currY * offset * 0x70)) - camY),				  // ATTR0
// 						 ATTR1_SIZE_16 | ATTR1_X(FIXED2INT(x + (currX * offset * 0x70)) - camX),			  // ATTR1
// 						 ATTR2_PALBANK(0) | (((HAIR_PART_COUNT) << 2) + HAIR_SPRITE_OFFSET) | ATTR2_PRIO(0)); // ATTR2
// 			++*count;
// 			if (!currY) {
// 				currY = currX - INT_SIGN(currX);
// 				currX = currX / 2;
// 			} else if (INT_SIGN(currY) == INT_SIGN(currX)) {
// 				currX = -currX;
// 			} else {
// 				currX = currX * 2;
// 				currY = 0;
// 			}
// 		}
// 		return;
// 	}
// 	int currX = 0, currY = -1;
// 	int offsetX = -camX - 4, offsetY = -camY - 5;
// 	// Animation
// #if 1
// 	switch (CHAR_state.state* (deathAnimTimer == 0)) {
// 		case CHARST_DREAMDASH:
// 			{
// 				displayHair = 0;
// 				break;
// 			}
// 		//
// 		case CHARST_CLIMB:
// 			{
// 				currX = -1 - (anim_timer >> 5);
// 				break;
// 			}
// 		//
// 		case CHARST_ZIPLINE:
// 			{
// 				offsetY += 2;
// 				break;
// 			}
// 		case ST_VIEWER:
// 			{
// 				currY -= 1;
// 				break;
// 			}
// 		//
// 		case CHARST_NORMAL:
// 			{
// 				if (ON_GROUND) {
// 					currY += (anim_state == ANIM_DUCK) * 4;
// 					currX -= (anim_state == ANIM_LOOK) * 2;
// 					currY += (anim_state >= ANIM_WALK) * (-1 + (anim_timer >> 4) % 3);
// 					currX += (anim_state >= ANIM_WALK);
// 					currY += (anim_timer >> 5) * (anim_state < ANIM_WALK);
// 				} else {
// 					currY -= (anim_state == ANIM_JUMP);
// 					currX += (anim_state == ANIM_JUMP);
// 					currX -= (anim_state == ANIM_CLIMB);
// 				}
// 				currX += (-currX - 1) * (anim_state == ANIM_DEATH);
// 				break;
// 				break;
// 			}
// 		//
// 		case CHARST_DASH:
// 			{
// 				currX = 2;
// 				currY = 0;
// 				break;
// 			}
// 	}
// #endif
// 	// todo: Only load sprite when frame has changed
// 	if (anim_state != anim_last) {
// 		memcpy(tile_mem[4], &madeline[anim_state << 5], SPRITE_16x16);
// 		anim_last = anim_state;
// 	}
// 	int prio = CHAR_PRIORITY;
// 	// Character
// 	obj_set_attr(buffer,
// 				 ATTR0_SQUARE | ATTR0_Y(FIXED2INT(CHAR_ent->y) + offsetY),													   // ATTR0
// 				 ATTR1_SIZE_16 | ATTR1_FLIP(FACING_DIRECTION) | ATTR1_X(FIXED2INT(CHAR_ent->x) + offsetX), // ATTR1
// 				 ATTR2_PALBANK(0) | ATTR2_PRIO(prio));																		   // ATTR2
// 	++*count;
// 	if (!displayHair)
// 		return;
// 	offsetY += currY;
// 	offsetX += currX * (1 - (((FACING_DIRECTION) > 0) << 1));
// 	// Umbrella
// 	if (IS_RAINY && displayHair && !deathAnimTimer) {
// 		obj_set_attr((buffer + *count),
// 					 ATTR0_SQUARE | ATTR0_Y(FIXED2INT(CHAR_ent->y) + offsetY - 10),												// ATTR0
// 					 ATTR1_SIZE_16 | ATTR1_X(FIXED2INT(CHAR_ent->x) + offsetX - ((FACING_DIRECTION) ? 2 : -1)), // ATTR1
// 					 ATTR2_PALBANK(0) | (CHAR_umbrellaOffset) | ATTR2_PRIO(prio));													// ATTR2
// 		if (key_released(KEY_DOWN)) {
// 			int i;
// 			int CHAR_ent->vel_y;
// 			for (i = 0; i < 4; ++i) {
// 				int rng = RNG();
// 				int offX = i << 5;
// 				int offY = (rng)&0x3F;
// 				offX -= (FACING_DIRECTION) ? 0x4F : 0x2F;
// 				offY -= 0x0F;
// 				offX = ((CHAR_ent->x >> 4) + offX) & 0xFFFF;
// 				offY = (((CHAR_ent->y - 0xF00) >> 4) + offY) & 0xFFFF;
// 				CHAR_ent->vel_y = (rng)&0x7;
// 				AddParticle(offX | (0x55000000 + (rng & 0x33000000)),
// 							offY | 0xC0000000 | ((0xE8 + CHAR_ent->vel_y) << 16),
// 							0x100 | 27 | ((rng & 0x3) << 10));
// 			}
// 		}
// 		memcpy(&tile_mem[4][CHAR_umbrellaOffset], &umbrella[32 * (pixtro_tri_vert() > 0)], umbrellaLen >> 1);
// 		++*count;
// 	}
// 	obj_set_attr((buffer + *count),
// 				 ATTR0_WIDE | ATTR0_Y(FIXED2INT(CHAR_ent->y) + offsetY),														 // ATTR0
// 				 ATTR1_SIZE_16x8 | ATTR1_FLIP(FACING_DIRECTION) | ATTR1_X(FIXED2INT(CHAR_ent->x) + offsetX), // ATTR1
// 				 ATTR2_PALBANK(0) | 4 | ATTR2_PRIO(prio));
// 	obj_set_attr((buffer + *count + HAIR_PART_COUNT + 1),
// 				 ATTR0_WIDE | ATTR0_Y(FIXED2INT(CHAR_ent->y) + offsetY),														 // ATTR0
// 				 ATTR1_SIZE_16x8 | ATTR1_FLIP(FACING_DIRECTION) | ATTR1_X(FIXED2INT(CHAR_ent->x) + offsetX), // ATTR1
// 				 ATTR2_PALBANK(0) | 6 | ATTR2_PRIO(prio));																		 // ATTR2
// 	*count += 2;
// 	int dist;
// 	int prevX = (CHAR_ent->x & ~0xFF) + 0x080, prevY = (CHAR_ent->y & ~0xFF) - 0x400 + ((CURRENT_DASH_STATUS >= 2) << 8);
// 	buffer += *count;
// 	//*
// 	int hairI = 0;
// 	for (; hairI < HAIR_PART_COUNT; ++hairI) {
// 		//hairPoints[(hairI << 1)] += FACING_DIRECTION * 0x2 * (hairI + 1) + 0x6;
// 		hairPoints[(hairI << 1) + (CURRENT_DASH_STATUS < 2)] += (0x120 - 0x0C * (hairI + 1)) *
// 																(1 - (!((FACING_DIRECTION) || (CURRENT_DASH_STATUS < 2)) << 1)) * !HEART_FREEZE;
// 		currX = hairPoints[(hairI << 1)] - prevX;
// 		currY = hairPoints[(hairI << 1) + 1] - prevY;
// 		dist = FIXED_MULT(currX, currX) + FIXED_MULT(currY, currY);
// 		dist = FIXED_DIV(0x5C0 - ((anim_state == ANIM_DUCK) * 0x400), dist);
// 		currX = hairPoints[(hairI << 1)];
// 		currY = hairPoints[(hairI << 1) + 1];
// 		currX += (FIXED_MULT(currX - prevX, dist + 0x50) + prevX - currX) * (dist < 0x100);
// 		currY += (FIXED_MULT(currY - prevY, dist + 0x50) + prevY - currY) * (dist < 0x100);
// 		prevX						 = currX;
// 		prevY						 = currY;
// 		hairPoints[(hairI << 1)]	 = currX;
// 		hairPoints[(hairI << 1) + 1] = currY;
// 		currY = ATTR0_SQUARE | ATTR0_Y(FIXED2INT(currY) + offsetY);
// 		currX = ATTR1_SIZE_16 | ATTR1_X(FIXED2INT(currX) + offsetX);
// 		// render hair part
// 		obj_set_attr((buffer - 1),
// 					 currY,														// ATTR0
// 					 currX,														// ATTR1
// 					 (((hairI) << 2) + HAIR_SPRITE_OFFSET) | ATTR2_PRIO(prio)); // ATTR2
// 		obj_set_attr((buffer + HAIR_PART_COUNT),
// 					 currY,																		  // ATTR0
// 					 currX,																		  // ATTR1
// 					 (((hairI + HAIR_PART_COUNT) << 2) + HAIR_SPRITE_OFFSET) | ATTR2_PRIO(prio)); // ATTR2
// 		++*count;
// 		++buffer;
// 	}
// 	*count += HAIR_PART_COUNT;
// }
// //
// void CHAR_LoadLevel() {
// 	set_statemachine(&CHAR_state, CHARST_NORMAL);
// 	forceMoveXTimer = 0;
// 	zipJumpBuffer	= 0;
// 	varJumpTimer	= 0;
// 	dashBufferTimer = 0;
// 	grabbed_entity	= 0;
// 	deathAnimTimer	= 0;
// 	anim_last		= anim_timer + 1;
// 	CHAR_ent->flags[0] &= ~(BACKGROUND_FLAG);
// 	int i = 0;
// 	for (; i < 8; i += 2) {
// 		hairPoints[i]	  = CHAR_ent->x + 0x400;
// 		hairPoints[i + 1] = CHAR_ent->y + 0x400;
// 	}
// 	CHAR_refill_dash(1);
// 	set_hair_color();
// }
void SET_DUCKING(bool value) {
	if (value == false && IS_DUCKING) {
		int hit = collide_rect(FIXED2INT(CHAR_ent->x), FIXED2INT(CHAR_ent->y) - DUCK_DIFF, NORMAL_HITBOX_W, NORMAL_HITBOX_H, SOLID_COLLISION);
		if (hit)
			return;
	}
	if (IS_DUCKING && !value) {
		if (ON_GROUND) {
			squish_x = 0xA0;
			squish_y = 0x160;
		}

		CHAR_ent->height = NORMAL_HITBOX_H;
		CHAR_ent->y -= INT2FIXED(DUCK_DIFF);
	}
	if (!IS_DUCKING && value) {
		if (ON_GROUND) {
			squish_x = 0x160;
			squish_y = 0xA0;
		}

		CHAR_ent->height -= DUCK_DIFF;
		CHAR_ent->y += INT2FIXED(DUCK_DIFF);
	}
}
// void CHAR_save_loc() {
// 	saveX = CHAR_ent->x;
// 	saveY = CHAR_ent->y;
// }
void CHAR_on_transition() {
	CHAR_ent->vel_x = 0;
	CHAR_ent->vel_y = 0;

	CHAR_state.state = CHARST_NORMAL;

	speedRetentionTimer = 0;
	CHAR_dashTimer		= 0;
	varJumpTimer		= 0;
	varJumpSpeed		= 0;
	forceMoveXTimer		= 0;
	speedVert			= 0;
	speedHorz			= 0;
	coyoteTimer			= COYOTE_MAX;
}
void set_hair_color() {

	// change hair color
	COLOR* palette = &pal_obj_mem[1];

	COLOR* lookUp = (COLOR*)&hairColor;

	lookUp += CURRENT_DASH_STATUS << 1;

	if (TOTAL_DASH_STATUS == 0)
		lookUp += 2;
	else if (TOTAL_DASH_STATUS == 3)
		lookUp -= 2;

	palette[0] = lookUp[0];
	palette[1] = lookUp[1];
}
void set_dash_speed() {

	CHAR_ent->vel_y = F_DashSpeed * ((CHAR_ent->flags[0] & 0x4) >> 2) * ((CHAR_ent->flags[0] & 0x8) ? -1 : 1);
	int newVelX		= F_DashSpeed * ((CHAR_ent->flags[0] & 0x1) >> 0) * ((CHAR_ent->flags[0] & 0x2) ? -1 : 1);

	// slow diagonal speed to minimize
	if (CHAR_ent->vel_y && newVelX) {
		CHAR_ent->vel_y = FIXED_MULT(CHAR_ent->vel_y, 0xB4);
		newVelX			= FIXED_MULT(newVelX, 0xB4);
	}

	if (INT_ABS(newVelX) > INT_ABS(CHAR_ent->vel_x) || newVelX == 0)
		CHAR_ent->vel_x = newVelX;
	else
		CHAR_ent->vel_x = INT_ABS(CHAR_ent->vel_x) * INT_SIGN(newVelX);
}
void CHAR_stop_zipline() {
	// if (!speedRetentionTimer) {
	// 	wallSpeedRetained	= CHAR_ent->vel_x;
	// 	speedRetentionTimer = ZIP_RETENTION_TIME;
	// }
}
void CHAR_refill_dash(char force) {
	if (force || (CAN_REFILL_DASH))
		CHAR_ent->flags[0] = (CHAR_ent->flags[0] & ~DASH_AMOUNT_MASK) | (TOTAL_DASH_STATUS << DASH_AMOUNT_SHIFT);

	set_hair_color();
}
void CHAR_refill_stamina() {
	stamina		 = F_ClimbMaxStamina;
	staminaBlink = 0;
}
void CHAR_dash_count(int value) {
	value &= 0x3;
	CHAR_ent->flags[0] = (CHAR_ent->flags[0] & ~TOTAL_DASH_MASK) | ((value << TOTAL_DASH_SHIFT) & TOTAL_DASH_MASK);
}
void CHAR_Die() {
	++death_total;
	if (START_OF_LEVEL) {
		++death_temp;
	} else {
		level_timer = 0;
	}
	CHAR_ent->vel_x = 0;
	CHAR_ent->vel_y = 0;
	// if (CHAR_state.state == CHARST_ZIPLINE) {
	// 	entities[grabbed_entity].flags |= ACTOR_COLLIDABLE_MASK;
	// }
	squish_x		= 0x100;
	squish_y		= 0x100;

	set_statemachine(&CHAR_state, CHARST_NORMAL);
	deathAnimTimer = 63;
	CHAR_ent->flags[0] |= ISDEAD_MASK;
}
unsigned int GetWall(int dir, int size) {
	if (dir == 0)
		return 0;

	int x = FIXED2INT(CHAR_ent->x) + (dir > 0 ? NORMAL_HITBOX_W : -size);

	unsigned int s = collide_rect(x, FIXED2INT(CHAR_ent->y), size, CHAR_ent->height, SOLID_COLLISION);

	if (IsDashingBuffered && IS_DASH_DIR(DASHING_UP)) {

		int tempVel		= CHAR_ent->vel_x;
		CHAR_ent->vel_x = dir;
		if (collide_rect(x, FIXED2INT(CHAR_ent->y), size, CHAR_ent->height, dir > 0 ? SPIKE_LEFT_COLLISION : SPIKE_RIGHT_COLLISION)) {
			CHAR_ent->vel_x = tempVel;
			return 0;
		}
		CHAR_ent->vel_x = tempVel;
	}

	return s;
}
// void CHAR_Restart() {
// 	CHAR_ent->x = saveX;
// 	CHAR_ent->y = saveY;
// 	CHAR_ent->flags[0] &= ~(BACKGROUND_FLAG);
// 	if ((GAME_music_beat & (NOTE_BLOCK_BEAT))) {
// 		memcpy(&pal_bg_mem[13 << 4], note_a1_pal, 32);
// 		memcpy(&pal_bg_mem[14 << 4], note_b2_pal, 32);
// 		GAME_music_beat += (NOTE_BLOCK_BEAT);
// 	}
// 	int i = 0;
// 	for (; i < 8; i += 2) {
// 		hairPoints[i]	  = CHAR_ent->x + 0x400;
// 		hairPoints[i + 1] = CHAR_ent->y + 0x400;
// 	}
// 	varJumpTimer		= 0;
// 	varJumpSpeed		= 0;
// 	speedRetentionTimer = 0;
// 	CHAR_dashTimer		= 0;
// 	forceMoveXTimer		= 0;
// 	speedHorz			= 0;
// 	speedVert			= 0;
// 	coyoteTimer = COYOTE_MAX;
// 	CHAR_ent->flags[0] &= ~ISDEAD_MASK;
// 	CHAR_refill_dash(1);
// 	CHAR_refill_stamina();
// }

// void LargeBounce(int actor) {
// 	int dirX = (CHAR_ent->x + 0x380) - (entities[actor].x + (entities[actor].width << 7));
// 	int dirY = (CHAR_ent->y + 0x550) - (entities[actor].y + (entities[actor].height << 7));
// 	int dist = FIXED_sqrt(FIXED_MULT(dirX, dirX) + FIXED_MULT(dirY, dirY));
// 	dirX = FIXED_DIV(dirX, dist);
// 	dirY = FIXED_DIV(dirY, dist);
// 	//dirY = FIXED_MULT(dirY, 0xD8);
// 	if (dirY < 0xA0 && dirY > -0x80) {
// 		forceMoveX		= INT_SIGN(dirX);
// 		forceMoveXTimer = 3;
// 		dirY			= -0x80;
// 		dirX			= INT_SIGN(dirX) * 0xD0;
// 	}
// 	set_statemachine(&CHAR_state, CHARST_NORMAL);
// 	CHAR_ent->vel_x = dirX * 3;
// 	CHAR_ent->vel_y = dirY * 3;
// }
// void SmallBounce(int actor) {
// 	int dirX = (CHAR_ent->x + 0x380) - (entities[actor].x + (entities[actor].width << 7));
// 	int dirY = (CHAR_ent->y + 0x550) - (entities[actor].y + (entities[actor].height << 7));
// 	int dist = FIXED_sqrt(FIXED_MULT(dirX, dirX) + FIXED_MULT(dirY, dirY));
// 	dirX = FIXED_DIV(dirX, dist);
// 	dirY = FIXED_DIV(dirY, dist);
// 	//set_statemachine(&CHAR_state, CHARST_NORMAL);
// 	CHAR_ent->vel_x = dirX * 2;
// 	CHAR_ent->vel_y = dirY * 2;
// }
void RemoveDash() {
	if (TOTAL_DASH_STATUS < 3 && CURRENT_DASH_STATUS >= 1)
		CHAR_ent->flags[0] = ((CHAR_ent->flags[0] & DASH_AMOUNT_MASK) - SINGLE_DASH_MASK) | (CHAR_ent->flags[0] & ~DASH_AMOUNT_MASK);
}

// // Update Methods
// void CHAR_change_state(int value) {
// 	switch (CHAR_state) {
// 		case CHARST_NORMAL:
// 			CHAR_end_normal();
// 			break;
// 	}
// 	switch (value) {
// 		case CHARST_CLIMB:
// 			updateMethod = CHAR_update_climb;
// 			CHAR_climb_start();
// 			break;
// 		case CHARST_DASH:
// 			updateMethod = CHAR_update_dash;
// 			CHAR_dash_start();
// 			break;
// 		case CHARST_DREAMDASH:
// 			updateMethod = CHAR_update_dreamdash;
// 			CHAR_dreamdash_start();
// 			break;
// 		case CHARST_ZIPLINE:
// 			updateMethod = CHAR_update_zip;
// 			CHAR_zip_start();
// 			break;
// 		case CHARST_NORMAL:
// 			updateMethod = CHAR_update_normal;
// 			break;
// 		case ST_VIEWER:
// 			updateMethod = CHAR_update_viewer;
// 			break;
// 	}
// 	CHAR_state.state= value;
// }
// void CHAR_end_normal() {
// 	speedRetentionTimer = 0;
// }
// void CHAR_dreamdash_start() {
// 	CHAR_dashTimer		= 0;
// 	dreamDashStayTimer	= 4;
// 	speedRetentionTimer = 0;
// 	int CHAR_ent->vel_y = F_DashSpeed * ((CHAR_ent->flags[0] & 0x8) >> 3) * ((CHAR_ent->flags[0] & 0x10) ? -1 : 1);
// 	int CHAR_ent->vel_x = F_DashSpeed * ((CHAR_ent->flags[0] & 0x2) >> 1) * ((CHAR_ent->flags[0] & 0x4) ? -1 : 1);
// 	if (CHAR_ent->vel_y && CHAR_ent->vel_x) {
// 		CHAR_ent->vel_x = FIXED_MULT(CHAR_ent->vel_x, 0xB4);
// 		CHAR_ent->vel_y = FIXED_MULT(CHAR_ent->vel_y, 0xB4);
// 	}
// 	CHAR_ent->x += CHAR_ent->vel_x;
// 	CHAR_ent->y += CHAR_ent->vel_y;
// 	CHAR_ent->vel_x = CHAR_ent->vel_x;
// 	CHAR_ent->vel_y = CHAR_ent->vel_y;
// }
// void CHAR_update_dreamdash() {
// 	if (dreamDashStayTimer) {
// 		--dreamDashStayTimer;
// 		return;
// 	}
// 	int CHAR_ent->vel_x = CHAR_ent->vel_x;
// 	int CHAR_ent->vel_y = CHAR_ent->vel_y;
// 	int oldX = CHAR_ent->x;
// 	int oldY = CHAR_ent->y;
// 	if (CHAR_ent->vel_x)
// 		coyoteTimer = COYOTE_MAX;
// 	int hit = collide_rect(CHAR_ent->x, CHAR_ent->y, NORMAL_HITBOX_W, NORMAL_HITBOX_H);
// 	if (!(hit & COLLISION_DREAM)) {
// 		int tempX = INT_SIGN(CHAR_ent->vel_x), tempY = INT_SIGN(CHAR_ent->vel_y);
// 		hit = collide_rect(CHAR_ent->x + 0x200, CHAR_ent->y + 0x400, NORMAL_HITBOX_W - 4, NORMAL_HITBOX_H - 8);
// 		if (hit & COLLISION_SOLID) {
// 			if (0) {
// 				CHAR_ent->vel_x *= -1;
// 				CHAR_ent->vel_y *= -1;
// 			} else {
// 				CHAR_ent->x = oldX;
// 				CHAR_ent->y = oldY;
// 				CHAR_Die();
// 				return;
// 			}
// 		} else {
// 			CHAR_refill_stamina();
// 			CHAR_refill_dash(0);
// 			game_freeze = 3;
// 			int i;
// 			for (; i < 8; i += 2) {
// 				hairPoints[i]	  = CHAR_ent->x + 0x400;
// 				hairPoints[i + 1] = CHAR_ent->y + 0x400;
// 			}
// 			if (key_pressed(KEY_A, JUMP_BUFFER) && CHAR_ent->vel_x && !(CHAR_ent->flags[0] & GROUND_JUMP_MASK)) {
// 				Jump(horz);
// 			} else {
// 				unsigned int wallL = GetWall(-1, WALL_HITBOX);
// 				unsigned int wallR = GetWall(1, WALL_HITBOX);
// 				if (pixtro_key_check(KEY_SHOULDER) && CHAR_ent->vel_x && INT_SIGN(CHAR_ent->vel_x) == -horz) {
// 					SET_FACING(horz);
// 					CHAR_ent->vel_x = horz * 0x4000;
// 					CHAR_ent->vel_y = 0;
// 					set_statemachine(&CHAR_state, CHARST_CLIMB);
// 					goto updateDreamdashReturn;
// 				}
// 			}
// 			for (i = 0; i < 8; i += 2) {
// 				hairPoints[i]	  = CHAR_ent->x + 0x400;
// 				hairPoints[i + 1] = CHAR_ent->y + 0x400;
// 			}
// 			set_statemachine(&CHAR_state, CHARST_NORMAL);
// 			goto updateDreamdashReturn;
// 		}
// 	}
// 	if (dashBufferTimer && CURRENT_DASH_STATUS) {
// 		dashBufferTimer = 0;
// 		CHAR_ent->vel_y			= F_DashSpeed * (pixtro_tri_vert());
// 		CHAR_ent->vel_x			= F_DashSpeed * (pixtro_tri_horz());
// 		if (CHAR_ent->vel_y && CHAR_ent->vel_x) {
// 			CHAR_ent->vel_x = FIXED_MULT(CHAR_ent->vel_x, 0xB4);
// 			CHAR_ent->vel_y = FIXED_MULT(CHAR_ent->vel_y, 0xB4);
// 		}
// 		if (CHAR_ent->vel_x)
// 			SET_FACING(CHAR_ent->vel_x);
// 		RemoveDash();
// 	}
// updateDreamdashReturn:
// 	CHAR_ent->vel_x = CHAR_ent->vel_x;
// 	CHAR_ent->vel_y = CHAR_ent->vel_y;
// }
// void CHAR_climb_start() {
// 	CHAR_dashTimer	 = 0;
// 	climbNoMoveTimer = F_ClimbNoMoveTime;
// 	lastClimbMove	 = 0;
// }
// void CHAR_update_viewer() {
// 	int ncamX = FIXED2INT(entities[0].x) - 120;
// 	if (ncamX < SCREEN_LEFT)
// 		ncamX = SCREEN_LEFT;
// 	else if (ncamX > SCREEN_RIGHT)
// 		ncamX = SCREEN_RIGHT;
// 	SET_FACING(CHAR_ent->flags[0], camX - ncamX);
// 	if (forceMoveX) {
// 		int ncamY = FIXED2INT(entities[0].y) - 90;
// 		if (ncamY < SCREEN_TOP)
// 			ncamY = SCREEN_TOP;
// 		else if (ncamY > SCREEN_BOTTOM)
// 			ncamY = SCREEN_BOTTOM;
// 		camX = FIXED_APPROACH(camX, ncamX, 4);
// 		camY = FIXED_APPROACH(camY, ncamY, 4);
// 		if (camX == ncamX && camY == ncamY)
// 			set_statemachine(&CHAR_state, CHARST_NORMAL);
// 	} else {
// 		camX += (pixtro_tri_horz() * 2) * !forceMoveX;
// 		camY += (pixtro_tri_vert() * 2) * !forceMoveX;
// 		if (key_hit(KEY_SELECT | KEY_START | KEY_B | KEY_A)) {
// 			forceMoveX = 1;
// 			key_mod2(KEY_START | KEY_B | KEY_A);
// 		}
// 	}
// }
// void CHAR_zip_start() {
// 	CHAR_ent->vel_y					 = 0;
// 	entities[grabbed_entity].x	 = CHAR_ent->x - ZIP_GRAB_OFFSET;
// 	entities[grabbed_entity].CHAR_ent->vel_x = 0;
// 	entities[grabbed_entity].flags &= ~ACTOR_COLLIDABLE_MASK;
// 	ZIP_update(&entities[grabbed_entity]);
// }
// void CHAR_update_zip() {
// 	if (speedRetentionTimer) {
// 		CHAR_ent->vel_x = wallSpeedRetained;
// 	}
// 	if (CHAR_ent->y != entities[grabbed_entity].y + ZIP_GRAB_HEIGHT) {
// 		if (!(collide_rect(CHAR_ent->x, entities[grabbed_entity].y + ZIP_GRAB_HEIGHT, NORMAL_HITBOX_W, NORMAL_HITBOX_H) & COLLISION_WALL)) {
// 			CHAR_ent->y = entities[grabbed_entity].y + ZIP_GRAB_HEIGHT;
// 		}
// 	}
// 	if (!pixtro_key_check(KEY_SHOULDER) || !stamina) {
// 		if (!stamina && key_pressed(KEY_A, JUMP_BUFFER)) {
// 			clear_press(KEY_A);
// 			zipJumpBuffer	= 10;
// 		}
// 		entities[grabbed_entity].CHAR_ent->vel_x = CHAR_ent->vel_x;
// 		set_statemachine(&CHAR_state, CHARST_NORMAL);
// 		entities[grabbed_entity].CHAR_ent->vel_x = FIXED_MULT(entities[grabbed_entity].CHAR_ent->vel_x, ZIP_SLOWDOWN);
// 		ZIP_ungrab(grabbed_entity);
// 		return;
// 	}
// 	if (dashBufferTimer && CURRENT_DASH_STATUS) {
// 		entities[grabbed_entity].CHAR_ent->vel_x = CHAR_ent->vel_x;
// 		zipJumpBuffer					 = 10;
// 		dashBufferTimer = 0;
// 		RemoveDash();
// 		set_statemachine(&CHAR_state, CHARST_DASH);
// 		ZIP_ungrab(grabbed_entity);
// 		return;
// 	}
// 	if (key_pressed(KEY_A, JUMP_BUFFER)) {
// 		clear_press(KEY_A);
// 		zipJumpBuffer	= 10;
// 		if (INT_ABS(CHAR_ent->vel_x) > ZIP_SPEED + 48)
// 			speedHorz = 12;
// 		entities[grabbed_entity].CHAR_ent->vel_x = CHAR_ent->vel_x;
// 		entities[grabbed_entity].CHAR_ent->vel_x = FIXED_MULT(entities[grabbed_entity].CHAR_ent->vel_x, ZIP_SLOWDOWN);
// 		Jump(horz);
// 		set_statemachine(&CHAR_state, CHARST_NORMAL);
// 		ZIP_ungrab(grabbed_entity);
// 		stamina -= F_ClimbJumpCost >> 1;
// 		if (stamina < 0)
// 			stamina = 0;
// 		return;
// 	}
// 	if (horz && !speedRetentionTimer) {
// 		if (INT_SIGN(CHAR_ent->vel_x) != horz || INT_ABS(CHAR_ent->vel_x) < ZIP_SPEED) {
// 			CHAR_ent->vel_x += horz * 0x20;
// 			stamina -= F_ClimbStillCost;
// 		}
// 	}
// }
// void CHAR_update_dash() {
//
// }
//

void STRAWB_playergrab(int);

// Physics
void check_entities(int id) {
	Entity* other = &entities[id];

	switch (ENT_TYPE(id)) {
		case 1:
			STRAWB_playergrab(id);
			break;
	}

	// if (value & 0xFFF) {
	// 	if (type != ENT_STRAWB && type != ENT_BACKGROUND && IN_BACKGROUND)
	// 		return;
	// 	switch (type) {
	// 		case ENT_STRAWB:
	// 			STRAWB_playergrab(ACTOR_ENTITY_ID(value));
	// 			break;
	// 		case ENT_DASH:
	// 			if (IS_TIRED || ((TOTAL_DASH_STATUS == 1 || TOTAL_DASH_STATUS == 2) && CURRENT_DASH_STATUS < TOTAL_DASH_STATUS)) {
	// 				DASHCR_playergrab(ACTOR_ENTITY_ID(value));
	// 				CHAR_refill_dash(1);
	// 				CHAR_refill_stamina();
	// 			}
	// 			break;
	// 		case ENT_SPRING:
	// 			CHAR_ent->vel_y = SPRING_bounce;
	// 			varJumpSpeed	= SPRING_bounce;
	// 			varJumpTimer	= SPRING_hold_time;
	// 			set_statemachine(&CHAR_state, CHARST_NORMAL);
	// 			entities[ACTOR_ENTITY_ID(value)].flags = entities[ACTOR_ENTITY_ID(value)].flags & (~0xF000) | 0x9000;
	// 			CHAR_refill_dash(0);
	// 			CHAR_refill_stamina();
	// 			break;
	// 		case ENT_HEART:
	// 			if (CHAR_dashTimer) {
	// 				int i;
	// 				for (i = 0; i < 10; ++i) {
	// 					int rng	 = RNG();
	// 					int offX = (rng)&0x7F;
	// 					int offY = (rng >> 8) & 0x7F;
	// 					offX -= 0x3F;
	// 					offY -= 0x3F;
	// 					offX				= (((entities[ACTOR_ENTITY_ID(value)].x + 0x300) >> 4) + offX) & 0xFFFF;
	// 					offY				= (((entities[ACTOR_ENTITY_ID(value)].y + 0x300) >> 4) + offY) & 0xFFFF;
	// 					int CHAR_ent->vel_x = (rng >> 16) & 0x1F;
	// 					offX |= ((CHAR_ent->vel_x - 0xF) & 0xFF) << 16;
	// 					CHAR_ent->vel_x = (rng >> 22) & 0x1F;
	// 					offY |= ((CHAR_ent->vel_x - 0xF) & 0xFF) << 16;
	// 					// Lifetime
	// 					offX |= (rng & 0xFF000000) + 0x00000000;
	// 					AddParticle(offX,
	// 								offY | 0x42000000,
	// 								Heart_S | ((rng & 0x3) << 10));
	// 				}
	// 				heart_freeze = 1;
	// 				entities[ACTOR_ENTITY_ID(value)].flags &= ~ACTOR_COLLIDABLE_MASK;
	// 				if (levelFlags & LEVELFLAG_BSIDE) {
	// 					GB_StopMusic();
	// 				}
	// 				break;
	// 			}
	// 			CHAR_refill_dash(1);
	// 			CHAR_refill_stamina();
	// 			SmallBounce(ACTOR_ENTITY_ID(value));
	// 			break;
	// 		case ENT_BUMPER:
	// 			if (current_chapter == LEVELIDX_CORE && IS_CORE_HOT) {
	// 				CHAR_Die();
	// 			} else {
	// 				CHAR_refill_dash(0);
	// 				CHAR_refill_stamina();
	// 				BUMPER_playerhit(ACTOR_ENTITY_ID(value));
	// 				LargeBounce(ACTOR_ENTITY_ID(value));
	// 				break;
	// 			}
	// 			break;
	// 		case ENT_ZIP:
	// 			if ((!zipJumpBuffer || grabbed_entity != ACTOR_ENTITY_ID(value)) && !IS_TIRED && CHAR_state.state != CHARST_ZIPLINE && CHAR_state.state != CHARST_CLIMB && pixtro_key_check(KEY_SHOULDER)) {
	// 				grabBufferTimer = 0;
	// 				grabbed_entity	= ACTOR_ENTITY_ID(value);
	// 				set_statemachine(&CHAR_state, CHARST_ZIPLINE);
	// 				CHAR_ent->y = entities[grabbed_entity].y + ZIP_GRAB_HEIGHT;
	// 				ZIP_grab(grabbed_entity);
	// 				speedRetentionTimer = 0;
	// 				forceMoveXTimer		= 0;
	// 			}
	// 			break;
	// 		case ENT_KEY:
	// 			KEY_playergrab(ACTOR_ENTITY_ID(value));
	// 			break;
	// 		case ENT_CASSETTE:
	// 			saveFile[CASSETTE_COLL_IDX] |= (1 << current_chapter);
	// 			levelFlags &= ~MUSICAL_LEVEL;
	// 			GAME_music_beat = 0;
	// 			STRAWB_SETTEMPCOLL(CASSETTE_TEMP);
	// 			entities[ACTOR_ENTITY_ID(value)].flags &= ~ACTOR_ACTIVE_MASK;
	// 			GB_StopMusic();
	// 			memcpy(&pal_bg_mem[13 << 4], note_a1_pal, 32);
	// 			memcpy(&pal_bg_mem[14 << 4], note_b2_pal, 32);
	// 			break;
	// 		case ENT_BACKGROUND:
	// 			DASHCR_playergrab(ACTOR_ENTITY_ID(value));
	// 			if ((CHAR_ent->flags[0] & BACKGROUND_FLAG) && (entities[ACTOR_ENTITY_ID(value)].flags & 0x100)) {
	// 				CHAR_ent->flags[0] &= ~BACKGROUND_FLAG;
	// 				stamina = 0;
	// 			} else if (!(entities[ACTOR_ENTITY_ID(value)].flags & 0x100)) {
	// 				CHAR_ent->flags[0] |= BACKGROUND_FLAG;
	// 			}
	// 			break;
	// 	}
	// } else {
	// }
}

void respawn(Routine* r) {

	rt_begin(r[0]);

	respawn_player();
	deathAnimTimer--;
	CHAR_ent->flags[0] &= ~ISDEAD_MASK;

	rt_end();
}

void CHAR_update(unsigned int _) {

	int nextLevel = 255;

	horz = pixtro_tri_horz();
	vert = pixtro_tri_vert();

	if (deathAnimTimer) {
		deathAnimTimer -= deathAnimTimer != 30;
		if (deathAnimTimer == 30 || (key_hit(KEY_A) && deathAnimTimer > 30)) {
			deathAnimTimer	= 30;
			onfade_function = &respawn;
			start_fading();
		} else if (!deathAnimTimer) {
		}
		return;
	}

#ifdef __DEBUG__
	if (key_hit(KEY_SELECT) && vert == 1)
		CHAR_dash_count(TOTAL_DASH_STATUS + 1);
#endif

	if (CHAR_ent->vel_y > F_MaxFall && CHAR_state.state != CHARST_DASH) {
		squish_x = FIXED_APPROACH(squish_x, 0x080, 0x8);
		squish_y = FIXED_APPROACH(squish_y, 0x180, 0x8);
	} else {
		squish_x = FIXED_APPROACH(squish_x, 0x100, 0x8);
		squish_y = FIXED_APPROACH(squish_y, 0x100, 0x8);
	}

	update_statemachine(&CHAR_state);

	if (CHAR_dashTimer > 0)
		CHAR_dashTimer--;

	zipJumpBuffer -= zipJumpBuffer > 0;
	zipJumpBuffer *= pixtro_key_check(KEY_SHOULDER) > 0;

	// check_entities();

	// Ticking timers
	if (CHAR_state.state == CHARST_DASH) {
	}
	if (climbJumpSave) {
		if (pixtro_tri_horz() == INT_SIGN(climbJumpSave)) {
			stamina += F_ClimbJumpCost;
			CHAR_ent->vel_x = INT_SIGN(climbJumpSave) * (int)(F_WallJumpHSpeed);
			CHAR_ent->vel_y = F_JumpSpeed;
			climbJumpSave	= 0;
		} else
			climbJumpSave -= INT_SIGN(climbJumpSave);
	}

	if (speedHorz) {
		speedHorz--;

		if (INT_ABS(CHAR_ent->vel_x) < 0x40)
			speedHorz = 0;
		if (!(speedHorz & 0x3)) {
		}
	} else if (speedVert) {
		--speedVert;
		if (!(speedVert & 0x3)) {
		}
	}

	if (CHAR_state.state == CHARST_DREAMDASH) {
		CHAR_ent->x += CHAR_ent->vel_x;
		CHAR_ent->y += CHAR_ent->vel_y;

	} else {

		// Keep speed after running into wall, but only for a short period
		if (speedRetentionTimer > 0) {
			if (CHAR_state.state != CHARST_ZIPLINE) {
				// Stop speed retention if moving away from wall
				if (CHAR_ent->vel_x && INT_SIGN(CHAR_ent->vel_x) == -INT_SIGN(wallSpeedRetained))
					speedRetentionTimer = 0;
				else if (!GetWall(INT_SIGN(wallSpeedRetained), 1)) {
					CHAR_ent->vel_x		= wallSpeedRetained;
					speedRetentionTimer = -1;

					// Show speed rings if corner boosting
					if (INT_ABS(wallSpeedRetained) >= F_DashSpeed - 0x40) {
						speedHorz = 12;
					}
				} else {

					--speedRetentionTimer;
				}

			} else {
				--speedRetentionTimer;
			}
		} else if (speedRetentionTimer == 0) {
			if (!GetWall(INT_SIGN(wallSpeedRetained), 1)) {

				speedRetentionTimer = -1;
			}
		}

		// velocity before collision
		int prevVelX = CHAR_ent->vel_x;
		int prevVelY = CHAR_ent->vel_y;
		int safeX	 = CHAR_ent->x + CHAR_ent->vel_x;
		int safeY	 = CHAR_ent->y + CHAR_ent->vel_y;

		int hit	   = entity_physics(CHAR_ent, SOLID_COLLISION);
		int ground = (prevVelY >= 0) ? collide_rect(FIXED2INT(CHAR_ent->x), FIXED2INT(CHAR_ent->y) + CHAR_ent->height, NORMAL_HITBOX_W, 1, SOLID_COLLISION) : 0;

		int X = (hit >> 16);
		int Y = (hit & 0xFFFF);

		if (collide_rect(FIXED2INT(CHAR_ent->x), FIXED2INT(CHAR_ent->y), NORMAL_HITBOX_W, CHAR_ent->height - 2, DANGER_COLLISION)) {
			CHAR_Die();
			return;
		} else if (X || Y) {

			safeX = FIXED2INT(safeX);
			safeY = FIXED2INT(safeY);

			if (prevVelX && !prevVelY && CHAR_state.state == CHARST_DASH) {

				int i;
				for (i = 0; i < DashCornerCorrection; ++i) {
					if (!collide_rect(safeX, safeY + i, NORMAL_HITBOX_W, CHAR_ent->height, SOLID_COLLISION)) {
						// TODO: allow this to happen when invincible?
						if (!collide_rect(safeX, safeY + i, NORMAL_HITBOX_W, CHAR_ent->height, DANGER_COLLISION)) {
							CHAR_ent->x			= INT2FIXED(safeX);
							CHAR_ent->y			= INT2FIXED(safeY + i);
							CHAR_ent->vel_x		= prevVelX;
							speedRetentionTimer = 0;
							X					= 0;
							break;
						}
					}
					if (!collide_rect(safeX, safeY - i, NORMAL_HITBOX_W, CHAR_ent->height, SOLID_COLLISION)) {
						// TODO: allow this to happen when invincible?
						if (!collide_rect(safeX, safeY - i, NORMAL_HITBOX_W, CHAR_ent->height, DANGER_COLLISION)) {
							CHAR_ent->x			= INT2FIXED(safeX);
							CHAR_ent->y			= INT2FIXED(safeY - i);
							CHAR_ent->vel_x		= prevVelX;
							speedRetentionTimer = 0;
							X					= 0;
							break;
						}
					}
				}

			} else if (prevVelY &&
					   ((CHAR_state.state == CHARST_DASH && (prevVelY < 0 || !prevVelX)) ||
						(CHAR_state.state != CHARST_DASH && prevVelY < 0))) {

				int i;
				for (i = 0; i < DashCornerCorrection; ++i) {
					if (!collide_rect(safeX + i, safeY, NORMAL_HITBOX_W, CHAR_ent->height, SOLID_COLLISION)) {
						// TODO: allow this to happen when invincible?
						if (!collide_rect(safeX + i, safeY, NORMAL_HITBOX_W, CHAR_ent->height, DANGER_COLLISION)) {
							CHAR_ent->x		= INT2FIXED(safeX + i);
							CHAR_ent->y		= INT2FIXED(safeY);
							CHAR_ent->vel_y = prevVelY;
							Y				= 0;
							ground			= 0;
							break;
						}
					}
					if (!collide_rect(safeX - i, safeY, NORMAL_HITBOX_W, CHAR_ent->height, SOLID_COLLISION)) {
						// TODO: allow this to happen when invincible?
						if (!collide_rect(safeX - i, safeY, NORMAL_HITBOX_W, CHAR_ent->height, DANGER_COLLISION)) {
							CHAR_ent->x		= INT2FIXED(safeX - i);
							CHAR_ent->y		= INT2FIXED(safeY);
							CHAR_ent->vel_y = prevVelY;
							Y				= 0;
							ground			= 0;
							break;
						}
					}
				}
			}
		}

		if (X && speedRetentionTimer < 0 && prevVelX) {
			speedRetentionTimer = F_WallSpeedRetentionTime;
			wallSpeedRetained	= prevVelX;
		}
		Y |= ground;

		CHAR_ent->flags[0] &= ~ONGROUND_MASK;
		if (prevVelY >= 0 && Y) {
			CHAR_ent->flags[0] |= ONGROUND_MASK;

			CHAR_refill_stamina();
			if (coyoteTimer == 0) {
				squish_x = 0x140;
				squish_y = 0xC0;
			}
			coyoteTimer = COYOTE_MAX;

			if ((CHAR_ent->flags[0] & DASH_VERT_MASK) == DASHING_DOWN) {
				SET_DUCKING(1);
				CHAR_ent->vel_x = FIXED_MULT(CHAR_ent->vel_x, 0x133);
				CHAR_ent->flags[0] &= ~DASH_VERT_MASK;
			}

			if (CHAR_dashTimer <= Fr_AllowDashBack)
				CHAR_refill_dash(0);
		} else // Not on ground
		{
			if (coyoteTimer > 0)
				coyoteTimer--;
		}

		// Enter dream dash state
		// if (CHAR_dashTimer && !game_freeze && (!prevVelY || (CHAR_ent->flags[0] & 0x8) >> 3)) {
		// 	set_statemachine(&CHAR_state, CHARST_DREAMDASH);
		// }
	}

	int id = collide_entity(_);

	if (id > 0) {
		check_entities(id);
	}

	if (CHAR_ent->y < 0x0000) {
		transition_direction(LVLDIR_Up);
		CHAR_on_transition();
	}
	if (CHAR_ent->y >= lvl_height << 11) {
		if (is_valid_transition(LVLDIR_Down)) {
			transition_direction(LVLDIR_Down);
			CHAR_on_transition();
		} else {
			respawn_player();
		}
	}

	if (CHAR_state.state == CHARST_ZIPLINE) {
		entities[grabbed_entity].x = CHAR_ent->x - ZIP_GRAB_OFFSET;
	}
	cam_x = FIXED2INT(CHAR_ent->x) + 6;
	cam_y = FIXED2INT(CHAR_ent->y) + CHAR_ent->height - 6;

	int i = 0;
	// for (; i < 8; ++i)
	//	memcpy(palette + i, lookUp + i, 2);
}