//
//  Basic structs and typedefs
//
//! \file tonc_types.h
//! \author J Vijn
//! \date 20060508 - 20060508
//
// === NOTES ===
// * This is a _small_ set of typedefs, #defines and inlines that can 
//   be found in tonclib, and might not represent the 
//   final forms.


#ifndef __TYPES__
#define __TYPES__


// === GCC ATTRIBUTES =================================================

// alignment boundary
#define ALIGN(_n)   __attribute__((aligned(_n)))
#define ALIGN4      __attribute__((aligned(4)))

// pack aggregate members
#define PACKED      __attribute__((packed))


// === TYPES: =========================================================

// --- primary typedefs -----------------------------------------------
typedef unsigned char  u8,  byte;
typedef unsigned short u16, hword;
typedef unsigned int   u32, word;
typedef unsigned long long u64;

typedef signed char  s8;
typedef signed short s16; 
typedef signed int   s32;
typedef signed long long s64;

// and volatiles for registers 'n stuff
typedef volatile u8  vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;

typedef volatile s8  vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;

// and consts too for parameters *sigh*
typedef const u8  cu8;
typedef const u16 cu16;
typedef const u32 cu32;
typedef const u64 cu64;

typedef const s8  cs8;
typedef const s16 cs16;
typedef const s32 cs32;
typedef const s64 cs64;

typedef struct { u32 data[8];  } BLOCK;

// --- secondary typedefs ---------------------------------------------

typedef u16 COLOR;
typedef u16 SCR_ENTRY, SE;

// NOTE, u32[]!
typedef struct { u32 data[8];  } TILE, TILE4;
typedef struct { u32 data[16]; } TILE8;

// --- memory map structs  --------------------------------------------

// --- PAL types --- 
typedef COLOR PALBANK[16];

// --- VRAM types ---

typedef COLOR    M3LINE[240];
typedef u8       M4LINE[240];	// NOTE u8, not u16!!
typedef COLOR    M5LINE[160];

typedef TILE     CHARBLOCK[512];
typedef TILE8    CHARBLOCK8[256];
typedef SCR_ENTRY	SCREENBLOCK[1024];

// --- OAM structs ---
// NOTE: OATR and OAFF are interlaced; when using affine objs, 
// struct/DMA/mem copies will give bad results
typedef struct OBJ_ATTR
{
	u16 attr0;
	u16 attr1;
	u16 attr2;
	s16 fill;
} ALIGN4 OBJ_ATTR;

typedef struct OBJ_AFFINE
{
	u16 fill0[3];	s16 pa;
	u16 fill1[3];	s16 pb;
	u16 fill2[3];	s16 pc;
	u16 fill3[3];	s16 pd;
} ALIGN4 OBJ_AFFINE;


// === DEFINES ========================================================

#ifndef NULL
#define NULL 0
#endif

// `inline' inlines the function when -O > 0 when called, 
// but also creates a body for the function itself
// `static' removes the body as well
#define INLINE static inline


#endif // __TYPES__

