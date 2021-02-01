#include "game_data.h"

#define DIALOGUE_ENABLED	(dialogueFlags & 0x1)
#define END_CUTSCENE		(dialogueFlags & 0x2)

extern short dialogueFlags;
extern char *dialogue;
extern short cutsceneIndex, DLG_wait;

void DLG_update();
void DLG_end();
void DLG_start();
void DLG_ShowBox();
void DLG_dumbTest();

void DLG_prologue();
void DLG_rainIntro();
void DLG_rainIntroEnd();
