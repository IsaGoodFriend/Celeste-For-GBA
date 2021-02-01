#include "game_data.h"
#include "physics.h"

#define PARTICLE_MAX			120 // 40 * 3
#define PARTICLE_DATA_SIZE		3

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

#define Dash_S			0
#define Dash_L			0xC0000000

#define RingV_S			4
#define RingV_L			0xC0000000

#define RingH_S			8
#define RingH_L			0xC0000000

#define Spark_S			12
#define Spark_L			0xC0000000

#define Cryst_S			16
#define Cryst_L			0xC0000000

#define Dream_S			20
#define Dream_L			0x80000000

#define MenuSnow_S		34
#define MenuSnow_L		0xC0000000

#define Heart_S		38
#define Heart_L		0xC0000000


extern unsigned int particle_data[PARTICLE_MAX];

void AddParticle(int att1, int att2, int att3);
void UpdateParticles(OBJ_ATTR* buffer, int camX, int camY, int *count);
