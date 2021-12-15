#pragma once

// The size of each individual save file.  Can be any size, but it is recommended it be a multiple of 16
// The readable data in the save file will be one byte less than this number here.
// The first byte is used to determine if a save file has been created or not
#define SAVEFILE_LEN 256

// The size of the settings file.  Can be any size, but it is recommended it be a multiple of 16
#define SETTING_LEN 32

// Amount of audio channels in maxmod audio engine
#define AUDIO_CHANNELS 16

// The max amount of entities in the game at one time
#define ENTITY_LIMIT 32

#define RNG_SEED_1 0xFA12B4
#define RNG_SEED_2 0x2B5C72
#define RNG_SEED_3 0x14F4D2