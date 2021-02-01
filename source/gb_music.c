#include "game_data.h"
#include "sprites.h"

#include "gb_music.h"

#include <maxmod.h>
#include "soundbank.h"
//#include "soundbank_bin.h"

#include "songs.h"
#include "physics.h"
#include "char.h"

int l1, l2, l3;
int wait_time1, wait_time2, wait_time3, meter_sq1, meter_sq2, meter_sq3;
unsigned short *notes_sq1, *notes_sq2, *notes_sq3;

int waitClicks = 0;

const unsigned short* songs[6] =
{
	(unsigned short*)&dream_song_sq1,    (unsigned short*)&dream_song_sq2,    (unsigned short*)&dream_song_sq3,
	(unsigned short*)&forsaken_song_sq1, (unsigned short*)&forsaken_song_sq2, (unsigned short*)&forsaken_song_sq3,
};

void set_soundvalues1()
{
	wait_time1 = 	notes_sq1[meter_sq1 + 3];
	
	REG_SND1CNT =	notes_sq1[meter_sq1];
	REG_SND1SWEEP = notes_sq1[meter_sq1 + 2];
	REG_SND1FREQ =	notes_sq1[meter_sq1 + 1] | 0x8000;
	
	meter_sq1 += 4;
}
void set_soundvalues2()
{
	wait_time2 = 	notes_sq2[meter_sq2 + 2];
	
	REG_SND2CNT =	notes_sq2[meter_sq2];
	REG_SND2FREQ =	notes_sq2[meter_sq2 + 1] | 0x8000;
	
	meter_sq2 += 3;
}

void set_soundvalues3()
{
	wait_time3 = 	notes_sq3[meter_sq3 + 2];
	
	REG_SND3CNT =	notes_sq3[meter_sq3];
	REG_SND3FREQ =	notes_sq3[meter_sq3 + 1] | 0x8000;
	
	meter_sq3 += 3;
}

void GB_init_soundchip(int number){

	waitClicks = 5;
	mmStop();
	
	--number;
	GAME_music_beat = -10;
	
	// Start audio engine
	REG_SNDSTAT = SSTAT_ENABLE;
	
	// Enable all audio channels
	REG_SNDDMGCNT = SDMG_BUILD_LR(SDMG_SQR1, 7);
	REG_SNDDMGCNT |= SDMG_BUILD_LR(SDMG_SQR2, 7);
	REG_SNDDMGCNT |= SDMG_BUILD_LR(SDMG_WAVE, 7);
	REG_SNDDMGCNT |= SDMG_BUILD_LR(SDMG_NOISE, 1);
    // DMG ratio to 100%
    REG_SNDDSCNT |= SDS_DMG100;
    // DMG ratio to 100%
    //REG_SNDDSCNT &= ~(SDS_AR | SDS_AL | SDS_BR | SDS_BL);
	
	// Load wave 3 sound
	REG_SND3SEL = 0x40;
	REG_WAVE_RAM0=0x10325476;
	REG_WAVE_RAM1=0x98badcfe;
	REG_WAVE_RAM2=0x10325476;
	REG_WAVE_RAM3=0x98badcfe;
	REG_SND3SEL = 0;
	REG_SND3SEL |= 0x80;
	
	if (number == -1) {
		notes_sq1 = (unsigned short*)strawb_jingle_song_sq1;
		notes_sq2 = (unsigned short*)strawb_jingle_song_sq2;
		notes_sq3 = (unsigned short*)strawb_jingle_song_sq3;
		l1 = strawb_jingle_song_sq1Len >> 1;
		l2 = strawb_jingle_song_sq2Len >> 1;
		l3 = strawb_jingle_song_sq3Len >> 1;
	}
	else {
		notes_sq1 = (unsigned short*)songs[number * 3    ];
		notes_sq2 = (unsigned short*)songs[number * 3 + 1];
		notes_sq3 = (unsigned short*)songs[number * 3 + 2];
	
		switch (number) {
		default:
			l1 = dream_song_sq1Len >> 1;
			l2 = dream_song_sq2Len >> 1;
			l3 = dream_song_sq3Len >> 1;
			break;
		case 1:
			l1 = forsaken_song_sq1Len >> 1;
			l2 = forsaken_song_sq2Len >> 1;
			l3 = forsaken_song_sq3Len >> 1;
			break;
		}
	}
	
	meter_sq1 = 0;
	meter_sq2 = 0;
	meter_sq3 = 0;
	
	wait_time1 = 0;
	wait_time2 = 0;
	wait_time3 = 0;
	
	//set_soundvalues1();
	//set_soundvalues2();
	//set_soundvalues3();
	
}
void GB_StopMusic()
{
	REG_SND1CNT = 0;
	REG_SND2CNT = 0;
	REG_SND3CNT = 0;
	meter_sq1 = -1;
	
	mmStart( MOD_FLATOUTLIES, MM_PLAY_LOOP );
}
void GB_PauseMusic()
{
	REG_SND1CNT = 0;
	REG_SND2CNT = 0;
	REG_SND3CNT = 0;
}
void GB_PlayMusic(){
	
	// Get music rhythm for note blocks
	if (meter_sq1 < 0)
		return;
	
	REG_SNDSTAT = SSTAT_ENABLE;
	
	char inBlock = 0;
	++GAME_music_beat;
	
	if (!(GAME_music_beat & NOTE_BLOCK_DIR))
	{
		if (!deathAnimTimer && collide_rect(PHYS_actors[0].x, PHYS_actors[0].y, NORMAL_HITBOX_W, NORMAL_HITBOX_H) & COLLISION_WALL){
			inBlock = 1;
			--GAME_music_beat;
		}
		
		if (GAME_music_beat & NOTE_BLOCK_BEAT) {
			memcpy(&pal_bg_mem[13 << 4], note_a2_pal, 32);
			memcpy(&pal_bg_mem[14 << 4], note_b1_pal, 32);
		}
		else{
			memcpy(&pal_bg_mem[13 << 4], note_a1_pal, 32);
			memcpy(&pal_bg_mem[14 << 4], note_b2_pal, 32);
		}
	}
	if (!inBlock && !waitClicks)
	{
		if (!wait_time1)
		{
			if (meter_sq1 >= l1) {
				if (NOTE_BLOCKS_ACTIVE)
					meter_sq1 = 0;
				else {
					GB_StopMusic();
					return;
				}
			}
			
			set_soundvalues1();
		}
		if (!wait_time2)
		{
			if ( meter_sq2 >= l2)
				if (NOTE_BLOCKS_ACTIVE)
					meter_sq2 = 0;
				else
					GB_StopMusic();
			
			set_soundvalues2();
		}
		if (!wait_time3)
		{
			if (meter_sq3 >= l3)
				if (NOTE_BLOCKS_ACTIVE)
					meter_sq3 = 0;
				else
					GB_StopMusic();
			
			set_soundvalues3();
		}
		--wait_time1;
		--wait_time2;
		--wait_time3;
	}
	if (!NOTE_BLOCKS_ACTIVE)
		return;
	int offset = GAME_music_beat - 2;

	// Note block clicks
	if (!(offset & 0x1F)) {
		
		waitClicks -= waitClicks > 0;
		
		if (!(offset & 0x20)){ // 	base kick (on beat)
			REG_SND4CNT = SSQR_ENV_BUILD(8, 0, 2) | SSQR_DUTY1_4;
			REG_SND4FREQ = 0x8056;
		}
		else{ //							snare or whatever (off beat)
			REG_SND4CNT = SSQR_ENV_BUILD(8, 0, 1) | SSQR_DUTY1_4;
			REG_SND4FREQ = 0x8012;
		}
	}
}
