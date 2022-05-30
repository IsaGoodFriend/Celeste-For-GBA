#include "level_handler.h"

#include "core.h"
#include "coroutine.h"
#include "entities.h"
#include "global_vars.h"
#include "graphics.h"
#include "level_data.h"
#include "levels.h"
#include "load_data.h"
#include "math.h"
#include "save_handler.h"
#include "sprites.h"

int moveto_dir;

extern int cam_x, cam_y;

extern int STRAWB_berry_sp, STRAWB_wing_sp;

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

bool coll_spikeup(int x, int y, int width, int height) {
	if (CHAR_ent->vel_y < 0 || y + height <= 5)
		return false;
	return true;
}
bool coll_spikedown(int x, int y, int width, int height) {
	if (CHAR_ent->vel_y > 0 || y >= 3)
		return false;
	return true;
}
bool coll_spikeright(int x, int y, int width, int height) {
	if (CHAR_ent->vel_x > 0 || x >= 3)
		return false;
	return true;
}
bool coll_spikeleft(int x, int y, int width, int height) {
	if (CHAR_ent->vel_x < 0 || x + width <= 5)
		return false;
	return true;
}
int phys_platform(int x, int y, int width, int height, int vel, bool move_vert) {
	if (!move_vert || vel < 0)
		return -1;
	if (y + height > 0)
		return -1;

	return 0;
}
bool coll_platform(int x, int y, int width, int height) {
	if (height > 1 || y != 0)
		return false;

	return true;
}

void start_chapter(int pack, int bside) {

	level_flags |= LEVELFLAG_BEGINNING;

	SET_DRAWING_FLAG(CAM_FOLLOW);

	LOAD_TILESET(Prologue);

	load_level_pack((bside ? levelsB : levels)[pack]);

	physics_code[0] = &phys_platform;

	collide_code[0] = &coll_platform;
	collide_code[1] = &coll_spikedown;
	collide_code[2] = &coll_spikeup;
	collide_code[3] = &coll_spikeleft;
	collide_code[4] = &coll_spikeright;

	STRAWB_berry_sp = load_anim_sprite(&SPR_strawberry, SPRITE16x16, 7, 8);

	load_obj_pal(&PAL_pickup_pal, 1);
	load_obj_pal(&PAL_pickupOld_pal, 2);

	start_playing();
}
void start_playing() {

	load_level(0);

	reset_cam();
}

void respawn_player() {

	int x = level_meta[0 + (moveto_dir << 1)] << 11;
	int y = level_meta[1 + (moveto_dir << 1)] << 11;

	CHAR_ent->x = x;
	CHAR_ent->y = y - 0xB00;

	set_cam();
	reset_cam();
}

void set_cam() {

	cam_x = FIXED2INT(CHAR_ent->x) + 6;
	cam_y = FIXED2INT(CHAR_ent->y) + CHAR_ent->height - 6;
}
void move_routine(Routine* ptr) {
	rt_begin(ptr[0]);

	load_level(level_meta[8 + moveto_dir]);

	respawn_player();

	celeste_savefile(false);

	rt_end();
}
int is_valid_transition(LevelDirection dir) {
	return level_meta[8 + dir] < 0x80;
}
void transition_direction(LevelDirection dir) {

	if (level_meta[8 + dir] < 0x80) {
		moveto_dir = dir;

		onfade_function = &move_routine;

		// Transitioning means level has properly started
		level_flags &= ~LEVELFLAG_BEGINNING;

		start_fading();
	}
}