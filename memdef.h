//
//  Memory map defines. All of them
//
//! \file tonc_memdef.h
//! \author J Vijn
//! \date 20060508 - 20060924
//
// === NOTES ===
// * This is a _small_ set of typedefs, #defines and inlines that can 
//   be found in tonclib, and might not represent the 
//   final forms.
// * '0'-defines are prefixed with '_', to indicate their zero-ness
//   and presents a safety from things like doing `if(x&0)'

#ifndef __MEMDEF__
#define __MEMDEF__

// --- Prefixes ---
// REG_DISPCNT		: DCNT
// REG_DISPSTAT		: DSTAT

// OAM attr 0		: OA0
// OAM attr 1		: OA1
// OAM attr 2		: OA2


// --- REG_DISPCNT -----------------------------------------------------

#define DCNT_MODE0				 0	//!< Mode 0; bg 0-4: reg
#define DCNT_MODE1			0x0001	//!< Mode 1; bg 0-1: reg; bg 2: affine
#define DCNT_MODE2			0x0002	//!< Mode 2; bg 2-2: affine
#define DCNT_MODE3			0x0003	//!< Mode 3; bg2: 240x160\@16 bitmap
#define DCNT_MODE4			0x0004	//!< Mode 4; bg2: 240x160\@8 bitmap
#define DCNT_MODE5			0x0005	//!< Mode 5; bg2: 160x128\@16 bitmap
#define DCNT_GB				0x0008	//!< (R) GBC indicator
#define DCNT_PAGE			0x0010	//!< Page indicator
#define DCNT_OAM_HBL		0x0020	//!< Allow OAM updates in HBlank
#define DCNT_OBJ_2D				 0	//!< OBJ-VRAM as matrix
#define DCNT_OBJ_1D			0x0040	//!< OBJ-VRAM as array
#define DCNT_BLANK			0x0080	//!< Force screen blank
#define DCNT_BG0			0x0100	//!< Enable bg 0
#define DCNT_BG1			0x0200	//!< Enable bg 1
#define DCNT_BG2			0x0400	//!< Enable bg 2
#define DCNT_BG3			0x0800	//!< Enable bg 3
#define DCNT_OBJ			0x1000	//!< Enable objects
#define DCNT_WIN0			0x2000	//!< Enable window 0
#define DCNT_WIN1			0x4000	//!< Enable window 1
#define DCNT_WINOBJ			0x8000	//!< Enable object window

#define DCNT_MODE_MASK		0x0007
#define DCNT_MODE_SHIFT			 0
#define DCNT_MODE(n)	   ((n)<<DCNT_MODE_SHIFT)

#define DCNT_LAYER_MASK		0x1F00
#define DCNT_LAYER_SHIFT		 8
#define DCNT_LAYER(n)	  ((n)<<DCNT_LAYER_SHIFT)

#define DCNT_WIN_MASK		0xE000
#define DCNT_WIN_SHIFT		  13
#define DCNT_WIN(n)			((n)<<DCNT_WIN_SHIFT)

#define BLD_BUILD(top, bot, mode)		\
	( (((bot)&63)<<8) | (((mode)&3)<<6) | ((top)&63) )
#define BLDA_BUILD(eva, evb)		\
	( ((eva)&31) | (((evb)&31)<<8) )
#define BLDY_BUILD(ey)				\
	( (ey)&31 )

#define DCNT_BUILD(mode, layer, win, obj1d, objhbl)				\
(																\
		(((win)&7)<<13) | (((layer)&31)<<8) | (((obj1d)&1)<<6)	\
	| (((oamhbl)&1)<<5) | ((mode)&7)							\
)


// --- REG_DISPSTAT ----------------------------------------------------

#define DSTAT_IN_VBL	0x0001	//!< Now in VBlank
#define DSTAT_IN_HBL	0x0002	//!< Now in HBlank
#define DSTAT_IN_VCT	0x0004	//!< Now in set VCount
#define DSTAT_VBL_IRQ	0x0008	//!< Enable VBlank irq
#define DSTAT_HBL_IRQ	0x0010	//!< Enable HBlank irq
#define DSTAT_VCT_IRQ	0x0020	//!< Enable VCount irq

#define DSTAT_VCT_MASK	0xFF00
#define DSTAT_VCT_SHIFT		 8
#define DSTAT_VCT(n)	((n)<<DSTAT_VCT_SHIFT)


// --- REG_KEYINPUT --------------------------------------------------------

#define KEY_A			0x0001	//!< Button A
#define KEY_B			0x0002	//!< Button B
#define KEY_SELECT		0x0004	//!< Select button
#define KEY_START		0x0008	//!< Start button
#define KEY_RIGHT		0x0010	//!< Right D-pad
#define KEY_LEFT		0x0020	//!< Left D-pad
#define KEY_UP			0x0040	//!< Up D-pad
#define KEY_DOWN		0x0080	//!< Down D-pad
#define KEY_R			0x0100	//!< Shoulder R
#define KEY_L			0x0200	//!< Shoulder L

#define KEY_ANY			0x03FF	//!< any key
#define KEY_DIR			0x00F0	//!< any-dpad
#define KEY_ACCEPT		0x0009	//!< A or start
#define KEY_CANCEL		0x0002	//!< B (well, it usually is)
#define KEY_SHOULDER	0x0300	//!< L or R

#define KEY_RESET		0x000F	//!< St+Se+A+B

#define KEY_MASK		0x03FF


// --- OAM attribute 0 -------------------------------------------------

#define ATTR0_REG				 0	//!< Regular object
#define ATTR0_AFF			0x0100	//!< Affine object
#define ATTR0_HIDE			0x0200	//!< Inactive object
#define ATTR0_AFF_DBL		0x0300	//!< Double-size affine object
#define ATTR0_AFF_DBL_BIT	0x0200
#define ATTR0_BLEND			0x0400	//!< Enable blend
#define ATTR0_WINDOW		0x0800	//!< Use for object window
#define ATTR0_MOSAIC		0x1000	//!< Enable mosaic
#define ATTR0_4BPP				 0	//!< Use 4bpp (16 color) tiles
#define ATTR0_8BPP			0x2000	//!< Use 8bpp (256 color) tiles
#define ATTR0_SQUARE			 0	//!< Square shape
#define ATTR0_WIDE			0x4000	//!< Tall shape (height &gt; width)
#define ATTR0_TALL			0x8000	//!< Wide shape (height &lt; width)

#define ATTR0_Y_MASK		0x00FF
#define ATTR0_Y_SHIFT			 0
#define ATTR0_Y(n)		((n&ATTR0_Y_MASK)<<ATTR0_Y_SHIFT)

#define ATTR0_MODE_MASK		0x0300
#define ATTR0_MODE_SHIFT		 8
#define ATTR0_MODE(n)		((n)<<ATTR0_MODE_SHIFT)

#define ATTR0_SHAPE_MASK	0xC000
#define ATTR0_SHAPE_SHIFT		14
#define ATTR0_SHAPE(n)	((n)<<ATTR0_SHAPE_SHIFT)


#define ATTR0_BUILD(y, shape, bpp, mode, mos, bld, win)				\
(																	\
	((y)&255) | (((mode)&3)<<8) | (((bld)&1)<<10) | (((win)&1)<<11) \
	| (((mos)&1)<<12) | (((bpp)&8)<<10)| (((shape)&3)<<14)			\
)


// --- OAM attribute 1 -------------------------------------------------

#define ATTR1_HFLIP			0x1000	//!< Horizontal flip (reg obj only)
#define ATTR1_VFLIP			0x2000	//!< Vertical flip (reg obj only)
// Base sizes
#define ATTR1_SIZE_8			 0
#define ATTR1_SIZE_16		0x4000
#define ATTR1_SIZE_32		0x8000
#define ATTR1_SIZE_64		0xC000
// Square sizes
#define ATTR1_SIZE_8x8			 0	//!< Size flag for  8x8 px object
#define ATTR1_SIZE_16x16	0x4000	//!< Size flag for 16x16 px object
#define ATTR1_SIZE_32x32	0x8000	//!< Size flag for 32x32 px object
#define ATTR1_SIZE_64x64	0xC000	//!< Size flag for 64x64 px object
// Tall sizes
#define ATTR1_SIZE_8x16			 0	//!< Size flag for  8x16 px object
#define ATTR1_SIZE_8x32		0x4000	//!< Size flag for  8x32 px object
#define ATTR1_SIZE_16x32	0x8000	//!< Size flag for 16x32 px object
#define ATTR1_SIZE_32x64	0xC000	//!< Size flag for 32x64 px object
// Wide sizes
#define ATTR1_SIZE_16x8			 0	//!< Size flag for 16x8 px object
#define ATTR1_SIZE_32x8		0x4000	//!< Size flag for 32x8 px object
#define ATTR1_SIZE_32x16	0x8000	//!< Size flag for 32x16 px object
#define ATTR1_SIZE_64x32	0xC000	//!< Size flag for 64x64 px object


#define ATTR1_X_MASK		0x01FF
#define ATTR1_X_SHIFT			 0
#define ATTR1_X(n)			((n&ATTR1_X_MASK)<<ATTR1_X_SHIFT)

#define ATTR1_AFF_ID_MASK		0x3E00
#define ATTR1_AFF_ID_SHIFT		 9
#define ATTR1_AFF_ID(n)		((n)<<ATTR1_AFF_ID_SHIFT)

#define ATTR1_FLIP_MASK		0x3000
#define ATTR1_FLIP_SHIFT		12
#define ATTR1_FLIP(n)		((n)<<ATTR1_FLIP_SHIFT)

#define ATTR1_SIZE_MASK		0xC000
#define ATTR1_SIZE_SHIFT		14
#define ATTR1_SIZE(n)		((n)<<ATTR1_SIZE_SHIFT)


#define ATTR1_BUILDR(x, size, hflip, vflip)	\
( ((x)&511) | (((hflip)&1)<<12) | (((vflip)&1)<<13) | (((size)&3)<<14) )

#define ATTR1_BUILDA(x, size, affid)			\
( ((x)&511) | (((affid)&31)<<9) | (((size)&3)<<14) )


// --- OAM attribute 2 -------------------------------------------------

#define ATTR2_ID_MASK		0x03FF
#define ATTR2_ID_SHIFT			 0
#define ATTR2_ID(n)			((n)<<ATTR2_ID_SHIFT)

#define ATTR2_PRIO_MASK		0x0C00
#define ATTR2_PRIO_SHIFT		10
#define ATTR2_PRIO(n)		((n)<<ATTR2_PRIO_SHIFT)

#define ATTR2_PALBANK_MASK	0xF000
#define ATTR2_PALBANK_SHIFT		12
#define ATTR2_PALBANK(n)	((n)<<ATTR2_PALBANK_SHIFT)


#define ATTR2_BUILD(id, pbank, prio)			\
( ((id)&0x3FF) | (((pbank)&15)<<12) | (((prio)&3)<<10) )

// --- REG_DISPCNT ---

#define DCNT_MODE0				 0	//!< Mode 0; bg 0-4: reg
#define DCNT_MODE1			0x0001	//!< Mode 1; bg 0-1: reg; bg 2: affine
#define DCNT_MODE2			0x0002	//!< Mode 2; bg 2-2: affine
#define DCNT_MODE3			0x0003	//!< Mode 3; bg2: 240x160\@16 bitmap
#define DCNT_MODE4			0x0004	//!< Mode 4; bg2: 240x160\@8 bitmap
#define DCNT_MODE5			0x0005	//!< Mode 5; bg2: 160x128\@16 bitmap
#define DCNT_GB				0x0008	//!< (R) GBC indicator
#define DCNT_PAGE			0x0010	//!< Page indicator
#define DCNT_OAM_HBL		0x0020	//!< Allow OAM updates in HBlank
#define DCNT_OBJ_2D				 0	//!< OBJ-VRAM as matrix
#define DCNT_OBJ_1D			0x0040	//!< OBJ-VRAM as array
#define DCNT_BLANK			0x0080	//!< Force screen blank
#define DCNT_BG0			0x0100	//!< Enable bg 0
#define DCNT_BG1			0x0200	//!< Enable bg 1
#define DCNT_BG2			0x0400	//!< Enable bg 2
#define DCNT_BG3			0x0800	//!< Enable bg 3
#define DCNT_OBJ			0x1000	//!< Enable objects
#define DCNT_WIN0			0x2000	//!< Enable window 0
#define DCNT_WIN1			0x4000	//!< Enable window 1
#define DCNT_WINOBJ			0x8000	//!< Enable object window


// --- REG_BGxCNT ---

#define BG_MOSAIC			0x0040	//!< Enable Mosaic
#define BG_4BPP					 0	//!< 4bpp (16 color) bg (no effect on affine bg)
#define BG_8BPP				0x0080	//!< 8bpp (256 color) bg (no effect on affine bg)
#define BG_WRAP				0x2000	//!< Wrap around edges of affine bgs
#define BG_SIZE0				 0
#define BG_SIZE1			0x4000
#define BG_SIZE2			0x8000
#define BG_SIZE3			0xC000
#define BG_REG_32x32			 0	//!< reg bg, 32x32 (256x256 px)
#define BG_REG_64x32		0x4000	//!< reg bg, 64x32 (512x256 px)
#define BG_REG_32x64		0x8000	//!< reg bg, 32x64 (256x512 px)
#define BG_REG_64x64		0xC000	//!< reg bg, 64x64 (512x512 px)
#define BG_AFF_16x16			 0	//!< affine bg, 16x16 (128x128 px)
#define BG_AFF_32x32		0x4000	//!< affine bg, 32x32 (256x256 px)
#define BG_AFF_64x64		0x8000	//!< affine bg, 64x64 (512x512 px)
#define BG_AFF_128x128		0xC000	//!< affine bg, 128x128 (1024x1024 px)

#define BG_PRIO_MASK	0x0003
#define BG_PRIO_SHIFT		0
#define BG_PRIO(n)		((n)<<BG_PRIO_SHIFT)

#define BG_CBB_MASK		0x000C
#define BG_CBB_SHIFT		 2
#define BG_CBB(n)		((n)<<BG_CBB_SHIFT)

#define BG_SBB_MASK		0x1F00
#define BG_SBB_SHIFT		 8
#define BG_SBB(n)		((n)<<BG_SBB_SHIFT)

#define BG_SIZE_MASK	0xC000
#define BG_SIZE_SHIFT		14
#define BG_SIZE(n)		((n)<<BG_SIZE_SHIFT)


#define BG_BUILD(cbb, sbb, size, bpp, prio, mos, wrap)		\
(															\
	   ((size)<<14)  | (((wrap)&1)<<13) | (((sbb)&31)<<8	\
	| (((bpp)&8)<<4) | (((mos)&1)<<6)   | (((cbb)&3)<<2)	\
	| ((prio)&3)											\
)

// --- REG_KEYINPUT ---

#define KEY_A			0x0001	//!< Button A
#define KEY_B			0x0002	//!< Button B
#define KEY_SELECT		0x0004	//!< Select button
#define KEY_START		0x0008	//!< Start button
#define KEY_RIGHT		0x0010	//!< Right D-pad
#define KEY_LEFT		0x0020	//!< Left D-pad
#define KEY_UP			0x0040	//!< Up D-pad
#define KEY_DOWN		0x0080	//!< Down D-pad
#define KEY_R			0x0100	//!< Shoulder R
#define KEY_L			0x0200	//!< Shoulder L

#define KEY_ANY			0x03FF	//!< any key
#define KEY_DIR			0x00F0	//!< any-dpad
#define KEY_ACCEPT		0x0009	//!< A or start
#define KEY_CANCEL		0x0002	//!< B (well, it usually is)
#define KEY_SHOULDER	0x0300	//!< L or R

#define KEY_RESET		0x000F	//!< St+Se+A+B

#define KEY_MASK		0x03FF

// --- Reg screen entries ----------------------------------------------

#define SE_HFLIP		0x0400	//!< Horizontal flip
#define SE_VFLIP		0x0800	//!< Vertical flip

#define SE_ID_MASK		0x03FF
#define SE_ID_SHIFT			 0
#define SE_ID(n)		((n)<<SE_ID_SHIFT)

#define SE_FLIP_MASK	0x0C00
#define SE_FLIP_SHIFT		10
#define SE_FLIP(n)	 ((n)<<SE_FLIP_SHIFT)

#define SE_PALBANK_MASK		0xF000
#define SE_PALBANK_SHIFT		12
#define SE_PALBANK(n)		((n)<<SE_PALBANK_SHIFT)

// === AUDIO ===

#define SSTAT_SQR1		0x0001	//!< (R) Channel 1 status
#define SSTAT_SQR2		0x0002	//!< (R) Channel 2 status
#define SSTAT_WAVE		0x0004	//!< (R) Channel 3 status
#define SSTAT_NOISE		0x0008	//!< (R) Channel 4 status
#define SSTAT_DISABLE		 0	//!< Disable sound
#define SSTAT_ENABLE	0x0080	//!< Enable sound. NOTE: enable before using any other sound regs

#define SDMG_SQR1		0x01
#define SDMG_SQR2		0x02
#define SDMG_WAVE		0x04
#define SDMG_NOISE		0x08// --- REG_SND1CNT, REG_SND2CNT, REG_SND4CNT ---------------------------

/*!	\defgroup grpAudioSSQR	Tone Generator, Square Flags
	\ingroup grpMemBits
	\brief	Bits for REG_SND{1,2,4}CNT
	(aka REG_SOUND1CNT_H, REG_SOUND2CNT_L, REG_SOUND4CNT_L, respectively)
*/
/*!	\{	*/

#define SSQR_DUTY1_8		 0	//!< 12.5% duty cycle (#-------)
#define SSQR_DUTY1_4	0x0040	//!< 25% duty cycle (##------)
#define SSQR_DUTY1_2	0x0080	//!< 50% duty cycle (####----)
#define SSQR_DUTY3_4	0x00C0	//!< 75% duty cycle (######--) Equivalent to 25%
#define SSQR_INC			 0	//!< Increasing volume
#define SSQR_DEC		0x0800	//!< Decreasing volume

#define SSQR_LEN_MASK	0x003F
#define SSQR_LEN_SHIFT		 0
#define SSQR_LEN(n)		((n)<<SSQR_LEN_SHIFT)

#define SSQR_DUTY_MASK	0x00C0
#define SSQR_DUTY_SHIFT		 6
#define SSQR_DUTY(n)	((n)<<SSQR_DUTY_SHIFT)

#define SSQR_TIME_MASK	0x0700
#define SSQR_TIME_SHIFT		 8
#define SSQR_TIME(n)	((n)<<SSQR_TIME_SHIFT)

#define SSQR_IVOL_MASK	0xF000
#define SSQR_IVOL_SHIFT		12
#define SSQR_IVOL(n)	((n)<<SSQR_IVOL_SHIFT)

#define SDS_DMG25			 0	//!< Tone generators at 25% volume
#define SDS_DMG50		0x0001	//!< Tone generators at 50% volume
#define SDS_DMG100		0x0002	//!< Tone generators at 100% volume
#define SDS_A50			 0		//!< Direct Sound A at 50% volume
#define SDS_A100		0x0004	//!< Direct Sound A at 100% volume
#define SDS_B50			 0		//!< Direct Sound B at 50% volume
#define SDS_B100		0x0008	//!< Direct Sound B at 100% volume
#define SDS_AR			0x0100	//!< Enable Direct Sound A on right
#define SDS_AL			0x0200	//!< Enable Direct Sound A on left
#define SDS_ATMR0			 0	//!< Direct Sound A to use timer 0
#define SDS_ATMR1		0x0400	//!< Direct Sound A to use timer 1
#define SDS_ARESET		0x0800	//!< Reset FIFO of Direct Sound A
#define SDS_BR			0x1000	//!< Enable Direct Sound B on right
#define SDS_BL			0x2000	//!< Enable Direct Sound B on left
#define SDS_BTMR0			 0	//!< Direct Sound B to use timer 0
#define SDS_BTMR1		0x4000	//!< Direct Sound B to use timer 1
#define SDS_BRESET		0x8000	//!< Reset FIFO of Direct Sound B


#define SSW_INC			 0		//!< Increasing sweep rate
#define SSW_DEC			0x0008	//!< Decreasing sweep rate
#define SSW_OFF         0x0008	//!< Disable sweep altogether


#define SSQR_ENV_BUILD(ivol, dir, time)				\
	(  ((ivol)<<12) | ((dir)<<11) | (((time)&7)<<8) )

#define SSQR_BUILD(_ivol, dir, step, duty, len)		\
	( SSQR_ENV_BUILD(ivol,dir,step) | (((duty)&3)<<6) | ((len)&63) )

#define SDMG_BUILD(_lmode, _rmode, _lvol, _rvol)	\
	( ((_rmode)<<12) | ((_lmode)<<8) | (((_rvol)&7)<<4) | ((_lvol)&7) )

#define SDMG_BUILD_LR(_mode, _vol) SDMG_BUILD(_mode, _mode, _vol, _vol)


// --- REG_SND1FREQ, REG_SND2FREQ, REG_SND3FREQ ------------------------

/*!	\defgroup grpAudioSFREQ	Tone Generator, Frequency Flags
	\ingroup grpMemBits
	\brief	Bits for REG_SND{1-3}FREQ 
	(aka REG_SOUND1CNT_X, REG_SOUND2CNT_H, REG_SOUND3CNT_X)
*/
/*!	\{	*/

#define SFREQ_HOLD				 0	//!< Continuous play
#define SFREQ_TIMED			0x4000	//!< Timed play
#define SFREQ_RESET			0x8000	//!< Reset sound

#define SFREQ_RATE_MASK		0x07FF
#define SFREQ_RATE_SHIFT		 0
#define SFREQ_RATE(n)		((n)<<SFREQ_RATE_SHIFT)

extern const unsigned int __snd_rates[12];

#define SND_RATE(note, oct) ( 2048-(__snd_rates[note]>>(4+(oct))) )


#define SFREQ_BUILD(rate, timed, reset)				\
	( ((rate)&0x7FF) | ((timed)<<14) | ((reset)<<15) )


#define SE_BUILD(id, pbank, hflip, vflip)	\
( ((id)&0x03FF) | (((hflip)&1)<<10) | (((vflip)&1)<<11) | ((pbank)<<12) )

#endif // __MEMDEF__

