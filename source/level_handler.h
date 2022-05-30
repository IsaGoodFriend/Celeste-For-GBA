#pragma once

typedef enum {

	LVLDIR_Up,
	LVLDIR_Down,
	LVLDIR_Left,
	LVLDIR_Right,
} LevelDirection;

extern int lvl_width, lvl_height;

void start_chapter(int pack, int bside);
void respawn_player();
void set_cam();
int is_valid_transition(LevelDirection dir);
void transition_direction(LevelDirection dir);