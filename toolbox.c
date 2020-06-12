//
// toolbox.c
// 
// Tools source for obj_demo
// 
// (20060922-20060924, cearn)
//
// === NOTES ===
// * This is a _small_ set of typedefs, #defines and inlines that can 
//   be found in tonclib, and might not represent the 
//   final forms.

#include "toolbox.h"

// === (tonc_core.c) ==================================================

u16 __key_curr= 0, __key_prev= 0;

// === (tonc_oam.c) ===================================================

void oam_init(OBJ_ATTR *obj, u32 count)
{
	u32 nn= count;
	u32 *dst= (u32*)obj;

	// Hide each object
	while(nn--)
	{
		*dst++= ATTR0_HIDE;
		*dst++= 0;
	}
	// init oam
	oam_copy(oam_mem, obj, count);
}

void oam_copy(OBJ_ATTR *dst, const OBJ_ATTR *src, u32 count)
{

// NOTE: while struct-copying is the Right Thing to do here, 
//   there's a strange bug in DKP that sometimes makes it not work
//   If you see problems, just use the word-copy version.
#if 1
	while(count--)
		*dst++ = *src++;
#else
	u32 *dstw= (u32*)dst, *srcw= (u32*)src;
	while(count--)
	{
		*dstw++ = *srcw++;
		*dstw++ = *srcw++;
	}
#endif

}

void obj_copy(OBJ_ATTR *dst, const OBJ_ATTR *src, u32 count)
{
	int ii;
	for(ii=0; ii<count; ii++)
	{
		dst[ii].attr0= src[ii].attr0;
		dst[ii].attr1= src[ii].attr1;
		dst[ii].attr2= src[ii].attr2;
	}
}
