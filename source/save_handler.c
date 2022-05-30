#include "core.h"
#include "engine.h"
#include "entities.h"
#include "global_vars.h"

extern char save_data[SAVEFILE_LEN];

// 000-007 => save file timer.  only ticks up during any level.  It's a long, not int to allow for hours instead of minutes of timing
// 008-00B => death total count
// 00C-00C => strawb total count
// 00D-00? => ???
// 010-01F => strawberry collection binary

// Per level:
// - 00-03 => flags:
// ++ 0x00 => Level beaten
// ++ 0x01 => Heart collected
// ++ 0x02 => Cassette collected / B side unlocked
// ++ 0x03 => B side completed / Heart B collected
// ++ 0x04 => Golden collected
// ++ 0x05 => Winged golden collected
// ++ 0x3F =>
// - 08-0B => A side timer (personal bests only)
// - 0C-0F => B side timer
// - 10-13 => Golden wing timer?
// - 14-17 => ARB timer?
// - 18-1B => Full Clear timer?

void celeste_savefile(bool save_state) {

	long_to_file(0, file_timer);

	save_file();
}
void celeste_loadfile(int file) {
	open_file(file);

	file_timer = long_from_file(0);
}

bool is_strawb_collected(int index) {
	char val = save_data[0x10 + (index >> 4)];

	return (val >> (index & 0xF)) & 0x1;
}
void collect_strawb(int index) {
	save_data[0x10 + (index >> 4)] |= 1 << (index & 0xF);
}