#include "gb_music.h"
#include "songs.h"
#include "game_data.h"
#include "physics.h"
#include "char.h"

int l1, l2, l3;
int wait_time1, wait_time2, wait_time3, meter_sq1, meter_sq2, meter_sq3;
unsigned short *notes_sq1, *notes_sq2, *notes_sq3;

const unsigned short* songs[6] =
{
	&dream_song_sq1, &dream_song_sq2, &dream_song_sq3,
	&forsaken_song_sq1, &forsaken_song_sq2, &forsaken_song_sq3,
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
	
	number = 0;
	
	// Start audio engine
	REG_SNDSTAT = SSTAT_ENABLE;
	
	// Enable all audio channels
	REG_SNDDMGCNT = SDMG_BUILD_LR(SDMG_SQR1, 7);
	REG_SNDDMGCNT |= SDMG_BUILD_LR(SDMG_SQR2, 7);
	REG_SNDDMGCNT |= SDMG_BUILD_LR(SDMG_WAVE, 7);
	REG_SNDDMGCNT |= SDMG_BUILD_LR(SDMG_NOISE, 1);
    // DMG ratio to 100%
    REG_SNDDSCNT = SDS_DMG100;
	
	// Load wave 3 sound
	REG_SND3SEL = 0x40;
	REG_WAVE_RAM0=0x10325476;
	REG_WAVE_RAM1=0x98badcfe;
	REG_WAVE_RAM2=0x10325476;
	REG_WAVE_RAM3=0x98badcfe;
	REG_SND3SEL = 0;
	REG_SND3SEL |= 0x80;
	
	notes_sq1 = songs[number * 3    ];
	notes_sq2 = songs[number * 3 + 1];
	notes_sq3 = songs[number * 3 + 2];
	l1 = forsaken_song_sq1Len >> 1;
	l2 = forsaken_song_sq2Len >> 1;
	l3 = forsaken_song_sq3Len >> 1;
	
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

void GB_PauseMusic()
{
	REG_SND1CNT = 0;
	REG_SND2CNT = 0;
	REG_SND3CNT  = 0;
}
void GB_PlayMusic(){
	if (gamestate == 0){
		REG_SND4CNT = 0x5700;
		return;
	}
	/*
	if (IS_RAINY && !(GAME_music_beat & 0xF))
	{
		if ((GAME_music_beat & 0x10) == 0){
			REG_SND4CNT = 0x5700;
			REG_SND4FREQ = 0x8006;
		}
		else if ((GAME_music_beat & 0x10) == 0x10){
			REG_SND4CNT = 0x5F00;
			REG_SND4FREQ = 0x0006;
		}
		if (GAME_music_beat < 10)
			REG_SND4FREQ |= 0x8000;
	}
	else if (!IS_RAINY)
		REG_SND4CNT = 0x5700;
	*/
	
	// Get music rhythm for note blocks
	if (!NOTE_BLOCKS_ACTIVE)
		return;
	
	char inBlock = 0;
	++GAME_music_beat;
	
	if (!(GAME_music_beat & NOTE_BLOCK_DIR))
	{
		if (collide_rect(PHYS_actors[0].x, PHYS_actors[0].y, NORMAL_HITBOX_W, NORMAL_HITBOX_H) & COLLISION_WALL){
			//CHAR_Die();
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
	if (!inBlock)
	{
		if (!wait_time1)
		{
			if (meter_sq1 >= l1)
				meter_sq1 = 0;
			
			set_soundvalues1();
		}
		if (!wait_time2)
		{
			if (meter_sq2 >= l2)
				meter_sq2 = 0;
			
			set_soundvalues2();
		}
		if (!wait_time3)
		{
			if (meter_sq3 >= l3)
				meter_sq3 = 0;
			
			set_soundvalues3();
		}
		--wait_time1;
		--wait_time2;
		--wait_time3;
	}
	int offset = GAME_music_beat - 2;

	// Note block clicks
	if (!(offset & 0x1F)) {
		
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
