#include "particle.h"
#include "game_data.h"
#include "sprites.h"

// Life time			(4 bits)
// Default Life Time 	(4 bits)
// X vel				(8 bits  (X.X))
// X Coor				(16 bits (XXX.X))

// Animation frame		(2 bits)
// Priority				(2 bits)
// Color index			(4 bits)
// Y vel				(8 bits  (X.X))
// Y Coor				(16 bits (XXX.X))

// Flip Data			(2 bits)
// Gravity				(2 bits)
// Particle Frame start	(8 bits)


#define ATT1_LIFE				0xF0000000
#define ATT1_LIFE_1				0x10000000
#define ATT1_DEF_LIFE			0x0F000000
#define ATT1_DEF_LIFE_SHIFT		4

#define ATT2_ANIMFRAME			0xC0000000
#define ATT2_ANIMFRAME_TICK		0x40000000
#define ATT2_PRIO				0x30000000
#define ATT2_PRIO_S				28
#define ATT2_PAL				0x0F000000
#define ATT2_PAL_S				24

#define ATT3_START_FRAME		0x000000FF
#define ATT3_GRAV1				0x00000100
#define ATT3_GRAVS				0x00000200
#define ATT3_FLIP				0x00000C00

unsigned int particle_data[PARTICLE_MAX];
int lastParticle;

#define PARTICLE_FRAME(i) (((particle_data[i + 1] & ATT2_ANIMFRAME) >> 30) + (particle_data[i + 2] & ATT3_START_FRAME)) << 3

void AddParticle(int att1, int att2, int att3) {
	if (levelFlags & LEVELFLAG_TAS)
		return;
	
	particle_data[lastParticle] = att1;
	particle_data[lastParticle + 1] = att2;
	particle_data[lastParticle + 2] = att3;
	
	memcpy(&tile_mem[4][PARTICLE_OFFSET + (lastParticle / 3)], &particles[PARTICLE_FRAME(lastParticle)], SPRITE_8x8);
	
	lastParticle += 3;
	lastParticle %= PARTICLE_MAX;
}

void UpdateParticles(OBJ_ATTR* buffer, int camX, int camY, int *count) {
	int index, moveParticles = !(HEART_FREEZE || (GAME_fadeAmount == TRANSITION_CAP));
	
	
	for (index = 0; index < PARTICLE_MAX; index += PARTICLE_DATA_SIZE) {
		
		if (!(particle_data[index] & ATT1_LIFE)){
			if (particle_data[index + 1] & ATT2_ANIMFRAME){
				
				particle_data[index + 1] -= ATT2_ANIMFRAME_TICK;
				particle_data[index] |= ((particle_data[index] & ATT1_DEF_LIFE) << ATT1_DEF_LIFE_SHIFT);
				
				memcpy(&tile_mem[4][PARTICLE_OFFSET + (index / 3)], &particles[PARTICLE_FRAME(index)], SPRITE_8x8);
			} 
			else
				continue;
		}
		
		particle_data[index] -= ATT1_LIFE_1 * moveParticles;
		
		// X Velocity
		int pos = particle_data[index] & 0xFFFF;
		int vel = ((particle_data[index] & 0x7F0000) >> 16) | ((particle_data[index] & 0x800000) ? 0xFFFFFF80 : 0);
		pos += vel * moveParticles;
		pos &= 0xFFFF;
		particle_data[index] = pos | (particle_data[index] & 0xFFFF0000);
		
		pos = (pos >> 4) - camX;
		if (pos < -8 || pos > 240) {
			particle_data[index] &= ~ATT1_LIFE;
			continue;
		}
		
		pos = particle_data[index + 1] & 0xFFFF;
		vel = ((particle_data[index + 1] & 0x7F0000) >> 16) | ((particle_data[index + 1] & 0x800000) ? 0xFFFFFF80 : 0);
		vel += ((particle_data[index + 2] & ATT3_GRAV1) >> 8) | (particle_data[index + 2] & ATT3_GRAVS ? 0xFFFFFFFE : 0);
		pos += vel * moveParticles;
		pos &= 0xFFFF;
		particle_data[index + 1] = pos | (particle_data[index + 1] & 0xFF000000) | ((vel & 0xFF) << 16);
		
		pos = (pos >> 4) - camY;
		if (pos < -8 || pos > 160) {
			particle_data[index] &= ~ATT1_LIFE;
			continue;
		}
		
		vel = particle_data[index] & 0xFFFF;
		
		obj_set_attr((buffer + *count),
		ATTR0_SQUARE | ATTR0_Y(pos),	// ATTR0
		ATTR1_SIZE_8 | ATTR1_X((vel >> 4) - camX) | ((particle_data[index + 2] & ATT3_FLIP) << 2),	// ATTR1
		ATTR2_PALBANK((particle_data[index + 1] & ATT2_PAL) >> ATT2_PAL_S) | (PARTICLE_OFFSET + (index / 3)) | ATTR2_PRIO((particle_data[index + 1] & ATT2_PRIO) >> ATT2_PRIO_S));		// ATTR2
		
		++*count;
	}
}
void ClearParticles() {
	int index;
	for (index = 0; index < PARTICLE_MAX; index += PARTICLE_DATA_SIZE) {
		particle_data[index] = 0;
		particle_data[index + 1] = 0;
	}
}
