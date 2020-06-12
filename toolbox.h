//
// toolbox.h: 
// 
// Tools header for obj_demo
// 
// (20060211-20060924, cearn)
//
// === NOTES ===
// * This is a _small_ set of typedefs, #defines and inlines that can 
//   be found in tonclib, and might not represent the 
//   final forms.


#ifndef TOOLBOX_H
#define TOOLBOX_H

#include "types.h"		// (tonc_types.h)
#include "memmap.h"		// (tonc_memmap.h)
#include "memdef.h"		// (tonc_memdef.h)
#include "input.h"		// (tonc_input.h)

// === (tonc_core.h) ==================================================

// tribool: 1 if {plus} on, -1 if {minus} on, 0 if {plus}=={minus}
INLINE int bit_tribool(u32 x, int plus, int minus);


extern COLOR *vid_page;
extern u16 __key_curr, __key_prev;


// === (tonc_video.h) =================================================

// --- sizes ---
#define SCREEN_WIDTH	240
#define SCREEN_HEIGHT	160

#define M3_WIDTH	SCREEN_WIDTH
#define M3_HEIGHT	SCREEN_HEIGHT
#define M4_WIDTH	SCREEN_WIDTH
#define M4_HEIGHT	SCREEN_HEIGHT
#define M5_WIDTH	160
#define M5_HEIGHT	128

// --- colors ---

#define CLR_BLACK	0x0000
#define CLR_RED		0x001F
#define CLR_LIME	0x03E0
#define CLR_YELLOW	0x03FF
#define CLR_BLUE	0x7C00
#define CLR_MAG		0x7C1F
#define CLR_CYAN	0x7FE0
#define CLR_WHITE	0x7FFF

INLINE COLOR RGB15(u32 red, u32 green, u32 blue);

INLINE void vid_vsync();
u16 *vid_flip();


// --- Objects  ---

void oam_init(OBJ_ATTR *obj, u32 count);
void oam_copy(OBJ_ATTR *dst, const OBJ_ATTR *src, u32 count);

INLINE OBJ_ATTR *obj_set_attr(OBJ_ATTR *obj, u16 a0, u16 a1, u16 a2);
INLINE void obj_set_pos(OBJ_ATTR *obj, int x, int y);
INLINE void obj_hide(OBJ_ATTR *oatr);
INLINE void obj_unhide(OBJ_ATTR *obj, u16 mode);
void obj_copy(OBJ_ATTR *dst, const OBJ_ATTR *src, u32 count);


// === INLINES ========================================================

// --- (tonc_core.h) --------------------------------------------------

// --- Simple bit macros ---
#define BIT(n)					( 1<<(n) )
#define BIT_SHIFT(a, n)			( (a)<<(n) )
#define BIT_SET(word, flag)		( word |=  (flag) )
#define BIT_CLEAR(word, flag)	( word &= ~(flag) )
#define BIT_FLIP(word, flag)	( word ^=  (flag) )
#define BIT_EQ(word, flag)		( ((word)&(flag)) == (flag) )

// some EVIL bit-field operations, >:)
// _x needs shifting
#define BFN_PREP(x, name)		( ((x)<<name##_SHIFT) & name##_MASK )
#define BFN_GET(y, name)			( ((y) & name##_MASK)>>name##_SHIFT )
#define BFN_SET(y, x, name)		(y = ((y)&~name##_MASK) | BFN_PREP(x,name) )

// x already shifted
#define BFN_PREP2(x, name)		( (x) & name##_MASK )
#define BFN_GET2(y, name)		( (y) & name##_MASK )
#define BFN_SET2(y, x, name)		(y = ((y)&~name##_MASK) | BFN_PREP2(x,name) )

//! Gives a tribool (-1, 0, or +1) depending on the state of some bits.
/*! Looks at the \a plus and \a minus bits of \a flags, and subtracts 
*	  their status to give a +1, -1 or 0 result. Useful for direction flags.
*	\param plus		Bit number for positive result
*	\param minus	Bit number for negative result
*	\return	<b>+1</b> if \a plus bit is set but \a minus bit isn't<br>
*	  <b>-1</b> if \a minus bit is set and \a plus bit isn't</br>
*	  <b>0</b> if neither or both are set.
*/
INLINE int bit_tribool(u32 flags, int plus, int minus)
{	return ((flags>>plus)&1) - ((flags>>minus)&1);	}


// --- (tonc_video.h) -------------------------------------------------

//! Wait for next VBlank
INLINE void vid_vsync()
{
	while(REG_VCOUNT >= 160);	// wait till VDraw
	while(REG_VCOUNT < 160);	// wait till VBlank
}

//! Create a 15bit BGR color.
INLINE COLOR RGB15(u32 red, u32 green, u32 blue)
{	return red | (green<<5) | (blue<<10);	}


// --- Objects ---


//! Set the attributes of an object.
INLINE OBJ_ATTR *obj_set_attr(OBJ_ATTR *obj, u16 a0, u16 a1, u16 a2)
{
	obj->attr0= a0; obj->attr1= a1; obj->attr2= a2;
	return obj;
}

//! Set the position of \a obj
INLINE void obj_set_pos(OBJ_ATTR *obj, int x, int y)
{
	BFN_SET(obj->attr0, y, ATTR0_Y);
	BFN_SET(obj->attr1, x, ATTR1_X);
}

//! Hide an object.
INLINE void obj_hide(OBJ_ATTR *obj)
{	BFN_SET2(obj->attr0, ATTR0_HIDE, ATTR0_MODE);		}

//! Unhide an object.
/*! \param obj	Object to unhide.
*	\param mode	Object mode to unhide to. Necessary because this affects
*	  the affine-ness of the object.
*/
INLINE void obj_unhide(OBJ_ATTR *obj, u16 mode)
{	BFN_SET2(obj->attr0, mode, ATTR0_MODE);			}


#endif // TOOLBOX_H
