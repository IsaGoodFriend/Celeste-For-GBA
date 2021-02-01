#include "game_data.h"
#include "physics.h"

#define ZIP_OG_XPOS			0x0000FF00
#define ZIP_OG_XPOS_SHIFT	8
#define ZIP_LENGTH			0x00FF0000
#define ZIP_LENGTH_SHIFT	16
#define ZIP_OFFSET			0xFF000000
#define ZIP_OFFSET_SHIFT	24

extern int ZIP_index, ZIP_pal;

void ZIP_update(Actor *physics);
void ZIP_grab();
void ZIP_reset(Actor *physics);
