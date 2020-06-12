//
//  Input header
//
//! \file tonc_input.h
//! \author J Vijn
//! \date 20060508 - 20060924
//
// === NOTES ===
// * This is a _small_ set of typedefs, #defines and inlines that can 
//   be found in tonclib, and might not represent the 
//   final forms.


#ifndef __INPUT__
#define __INPUT__

#include "memmap.h"
#include "memdef.h"


INLINE int bit_tribool(u32 x, int plus, int minus);

// === CONSTANTS ======================================================

typedef enum eKeyIndex
{
	KI_A=0, KI_B, KI_SELECT, KI_START, 
	KI_RIGHT, KI_LEFT, KI_UP, KI_DOWN,
	KI_R, KI_L, KI_MAX
} eKeyIndex;

// === MACROS =========================================================

// check which of the specified keys are down or up right now
#define KEY_DOWN_NOW(key)	(~(REG_KEYINPUT) & key)
#define KEY_UP_NOW(key)		( (REG_KEYINPUT) & key)	

// test whether all keys are pressed, released, whatever.
// Example use:
//   KEY_EQ(key_hit, KEY_L | KEY_R)
// will be true if and only if KEY_L and KEY_R are _both_ being pressed
#define KEY_EQ(key_fun, keys)	( key_fun(keys) == (keys) )

// === CLASSES ========================================================
// === GLOBALS ========================================================

extern u16 __key_curr, __key_prev;	// in tonc_core.c

// === PROTOTYPES =====================================================

// --- synchronous key states ---
INLINE void key_poll();
INLINE void key_mod(u32 key);
INLINE u32 key_curr_state();
INLINE u32 key_prev_state();

INLINE u32 key_is_down(u32 key);	// any of key currently down?
INLINE u32 key_is_up(u32 key);		// any of key currently up?

INLINE u32 key_was_down(u32 key);	// any of key previously down?
INLINE u32 key_was_up(u32 key);		// any of key previously up?

INLINE u32 key_transit(u32 key);	// any of key changing?
INLINE u32 key_held(u32 key);		// any of key held down?
INLINE u32 key_hit(u32 key);		// any of key being hit (going down)?
INLINE u32 key_released(u32 key);	// any of key being released?

// --- tribools ---
INLINE int key_tri_horz();
INLINE int key_tri_vert();
INLINE int key_tri_shoulder();
INLINE int key_tri_fire();

// === INLINES=========================================================

// --- keys -----------------------------------------------------------

//! Poll for keystates
INLINE void key_poll()
{	__key_prev= __key_curr;	__key_curr= ~REG_KEYINPUT & KEY_MASK;	}

INLINE void key_mod2(u32 key)
{	__key_prev= key & KEY_MASK;	}

INLINE void key_mod(u32 key)
{	__key_curr= key & KEY_MASK;	}

//! Get current keystate
INLINE u32 key_curr_state()			{	return __key_curr;			}

//! Get previous key state
INLINE u32 key_prev_state()			{	return __key_prev;			}

//! Gives the keys of \a key that are currently down
INLINE u32 key_is_down(u32 key)		{	return  __key_curr & key;	}

//! Gives the keys of \a key that are currently up
INLINE u32 key_is_up(u32 key)		{	return ~__key_curr & key;	}

//! Gives the keys of \a key that were previously down
INLINE u32 key_was_down(u32 key)	{	return  __key_prev & key;	}

//! Gives the keys of \a key that were previously down
INLINE u32 key_was_up(u32 key)		{	return ~__key_prev & key;	}

//! Gives the keys of \a key that are different from before
INLINE u32 key_transit(u32 key)
{	return (__key_curr^__key_prev) & key;	}

//! Gives the keys of \a key that are being held down
INLINE u32 key_held(u32 key)
{	return (__key_curr& __key_prev) & key;	}

//! Gives the keys of \a key that are pressed (down now but not before)
INLINE u32 key_hit(u32 key)
{	return (__key_curr&~__key_prev) & key;	}

//! Gives the keys of \a key that are being released
INLINE u32 key_released(u32 key)
{	return (~__key_curr&__key_prev) & key;	}

// --- tribools ---

//! Horizontal tribool (right,left)=(+,-)
INLINE int key_tri_horz()		
{	return bit_tribool(__key_curr, KI_RIGHT, KI_LEFT);	}

//! Vertical tribool (down,up)=(+,-)
INLINE int key_tri_vert()		
{	return bit_tribool(__key_curr, KI_DOWN, KI_UP);		}

//! Shoulder-button tribool (R,L)=(+,-)
INLINE int key_tri_shoulder()	
{	return bit_tribool(__key_curr, KI_R, KI_L);			}

//! Fire-button tribool (A,B)=(+,-)
INLINE int key_tri_fire()		
{	return bit_tribool(__key_curr, KI_A, KI_B);			}


#endif // __INPUT__

