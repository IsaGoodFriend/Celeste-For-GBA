#include "game_data.h"

char dialogueEnabled;
char *dialogue;
int cutsceneIndex;

void DLG_update();
void DLG_end();
void DLG_start();
void DLG_ShowBox();

void DLG_prologue();
void DLG_rainIntro();
void DLG_rainIntroEnd();
