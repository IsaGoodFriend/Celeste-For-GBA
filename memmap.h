//
//  GBA Memory map 
//
//! \file tonc_memmap.h
//! \author J Vijn
//! \date 20060508 - 20060508
// 
// === NOTES ===
// * This is a _small_ set of typedefs, #defines and inlines that can 
//   be found in tonclib, and might not represent the 
//   final forms.

#ifndef __MEMMAP__
#define __MEMMAP__


// === MEMORY SECTIONS ================================================

// basic sections
#define MEM_IO		0x04000000
#define MEM_PAL		0x05000000	// no 8bit write !!
#define MEM_VRAM	0x06000000	// no 8bit write !!
#define MEM_OAM		0x07000000	// no 8bit write !!

// basic sizes
#define PAL_SIZE	0x00400
#define VRAM_SIZE	0x18000
#define OAM_SIZE	0x00400

// sub sizes
#define PAL_BG_SIZE		0x00200
#define PAL_OBJ_SIZE	0x00200
#define VRAM_BG_SIZE	0x10000
#define VRAM_OBJ_SIZE	0x08000
#define M3_SIZE			0x12C00
#define M4_SIZE			0x09600
#define M5_SIZE			0x0A000
#define VRAM_PAGE_SIZE	0x0A000

// sub sections
#define REG_BASE	MEM_IO

#define MEM_PAL_OBJ		(MEM_PAL + PAL_BG_SIZE)
#define MEM_VRAM_BACK	(MEM_VRAM+ VRAM_PAGE_SIZE)
#define MEM_VRAM_OBJ	(MEM_VRAM+ VRAM_BG_SIZE)

#define REG_MOSAIC			*(vu32*)(REG_BASE+0x004C)	//!< Mosaic control
#define REG_BLDCNT			*(vu16*)(REG_BASE+0x0050)	//!< Alpha control
#define REG_BLDALPHA		*(vu16*)(REG_BASE+0x0052)	//!< Fade level
#define REG_BLDY			*(vu16*)(REG_BASE+0x0054)	//!< Blend levels


// === STRUCTURED MEMORY MAP ==========================================
// Heavily typedefed, watch your pointers
// Should simplify memory accesses


// --- PAL ---
// pal_bg_mem[y] = COLOR (color y)
#define pal_bg_mem		((COLOR*)MEM_PAL)
#define pal_obj_mem		((COLOR*)MEM_PAL_OBJ)

// pal_bg_bank[y] =	COLOR[] (bank y)
// pal_bg_bank[y][x] = COLOR (bank y, color x : color y*16+x)
#define pal_bg_bank		((PALBANK*)MEM_PAL)
#define pal_obj_bank	((PALBANK*)MEM_PAL_OBJ)


// --- VRAM ---
// tile_mem[y] = TILE[]   (char block y)
// tile_mem[y][x] = TILE (char block y, tile x)
#define tile_mem		( (CHARBLOCK*)MEM_VRAM)
#define tile8_mem		((CHARBLOCK8*)MEM_VRAM)

#define tile_mem_obj	( (CHARBLOCK*)MEM_VRAM_OBJ)
#define tile8_mem_obj	((CHARBLOCK8*)MEM_VRAM_OBJ)

// vid_mem[a] = COLOR
#define vid_mem			((COLOR*)MEM_VRAM)


// --- OAM ---
// oatr_mem[a] = OBJ_ATTR (oam entry a)
#define oam_mem			((OBJ_ATTR*)MEM_OAM)
#define obj_mem			((OBJ_ATTR*)MEM_OAM)
#define obj_aff_mem		((OBJ_AFFINE*)MEM_OAM)

// --- Save Data ---
#define save_data		((unsigned char*)0x0E000000)


// === REGISTER LIST ==================================================

#define MEM_IO		0x04000000
#define MEM_PAL		0x05000000		// no 8bit write !!
#define MEM_VRAM	0x06000000		// no 8bit write !!

#define PAL_SIZE	0x00400
#define VRAM_SIZE	0x18000

#define M3_SIZE			0x12C00
#define M4_SIZE			0x09600
#define M5_SIZE			0x0A000
#define CBB_SIZE		0x04000
#define SBB_SIZE		0x00800
#define VRAM_BG_SIZE	0x10000

// === SOUND REGISTERS ===
// sound regs, partially following pin8gba's nomenclature

//! \name Channel 1: Square wave with sweep
//\{
#define REG_SND1SWEEP		*(vu16*)(REG_BASE+0x0060)	//!< Channel 1 Sweep
#define REG_SND1CNT			*(vu16*)(REG_BASE+0x0062)	//!< Channel 1 Control
#define REG_SND1FREQ		*(vu16*)(REG_BASE+0x0064)	//!< Channel 1 frequency
//\}

//! \name Channel 2: Simple square wave
//\{
#define REG_SND2CNT			*(vu16*)(REG_BASE+0x0068)	//!< Channel 2 control
#define REG_SND2FREQ		*(vu16*)(REG_BASE+0x006C)	//!< Channel 2 frequency
//\}

//! \name Channel 3: Wave player
//\{
#define REG_SND3SEL			*(vu16*)(REG_BASE+0x0070)	//!< Channel 3 wave select
#define REG_SND3CNT			*(vu16*)(REG_BASE+0x0072)	//!< Channel 3 control
#define REG_SND3FREQ		*(vu16*)(REG_BASE+0x0074)	//!< Channel 3 frequency
//\}

//! \name Channel 4: Noise generator
//\{
#define REG_SND4CNT			*(vu16*)(REG_BASE+0x0078)	//!< Channel 4 control
#define REG_SND4FREQ		*(vu16*)(REG_BASE+0x007C)	//!< Channel 4 frequency
//\}

//! \name Sound control
//\{
#define REG_SNDCNT			*(vu32*)(REG_BASE+0x0080)	//!< Main sound control
#define REG_SNDDMGCNT		*(vu16*)(REG_BASE+0x0080)	//!< DMG channel control
#define REG_SNDDSCNT		*(vu16*)(REG_BASE+0x0082)	//!< Direct Sound control
#define REG_SNDSTAT			*(vu16*)(REG_BASE+0x0084)	//!< Sound status
#define REG_SNDBIAS			*(vu16*)(REG_BASE+0x0088)	//!< Sound bias
//\}

//! \name Sound buffers
//\{
#define REG_WAVE_RAM		 (vu32*)(REG_BASE+0x0090)	//!< Channel 3 wave buffer

#define REG_WAVE_RAM0		*(vu32*)(REG_BASE+0x0090)
#define REG_WAVE_RAM1		*(vu32*)(REG_BASE+0x0094)
#define REG_WAVE_RAM2		*(vu32*)(REG_BASE+0x0098)
#define REG_WAVE_RAM3		*(vu32*)(REG_BASE+0x009C)

#define REG_FIFO_A			*(vu32*)(REG_BASE+0x00A0)	//!< DSound A FIFO
#define REG_FIFO_B			*(vu32*)(REG_BASE+0x00A4)	//!< DSound B FIFO
//\}


// --- memmap ---

// pal_bg_mem[y] = COLOR (color y)
#define pal_bg_mem		((COLOR*)MEM_PAL)

// --- VRAM ---
// tile_mem[y] = TILE[]   (char block y)
// tile_mem[y][x] = TILE (char block y, tile x)
#define tile_mem		( (CHARBLOCK*)MEM_VRAM)
#define tile8_mem		((CHARBLOCK8*)MEM_VRAM)

// se_mem[y] = SB_ENTRY[] (screen block y)
// se_mem[y][x] = screen entry (screen block y, screen entry x)
#define se_mem			((SCREENBLOCK*)MEM_VRAM)


// --- registers ------------------------------------------------------

#define REG_BASE	MEM_IO

#define REG_DISPCNT			*(vu32*)(REG_BASE+0x0000)	// display control
#define REG_DISPSTAT		*(vu16*)(REG_BASE+0x0004)	// display interupt status
#define REG_VCOUNT			*(vu16*)(REG_BASE+0x0006)	// vertical count

// --- background ---
#define REG_BG0CNT			*(vu16*)(REG_BASE+0x0008)	// bg 0-3 control
#define REG_BG1CNT			*(vu16*)(REG_BASE+0x000A)
#define REG_BG2CNT			*(vu16*)(REG_BASE+0x000C)
#define REG_BG3CNT			*(vu16*)(REG_BASE+0x000E)

#define REG_BG0HOFS			*(vu16*)(REG_BASE+0x0010)		// bg 0-3 origins
#define REG_BG0VOFS			*(vu16*)(REG_BASE+0x0012)
#define REG_BG1HOFS			*(vu16*)(REG_BASE+0x0014)
#define REG_BG1VOFS			*(vu16*)(REG_BASE+0x0016)
#define REG_BG2HOFS			*(vu16*)(REG_BASE+0x0018)
#define REG_BG2VOFS			*(vu16*)(REG_BASE+0x001A)
#define REG_BG3HOFS			*(vu16*)(REG_BASE+0x001C)
#define REG_BG3VOFS			*(vu16*)(REG_BASE+0x001E)

#define REG_KEYINPUT		*(vu16*)(REG_BASE+0x0130)	// Key status

// === VIDEO REGISTERS ===
#define REG_DISPCNT			*(vu32*)(REG_BASE+0x0000)	// display control
#define REG_DISPSTAT		*(vu16*)(REG_BASE+0x0004)	// display interupt status
#define REG_VCOUNT			*(vu16*)(REG_BASE+0x0006)	// vertical count

// === KEYPAD ===
#define REG_KEYINPUT		*(vu16*)(REG_BASE+0x0130)	// Key status
#define REG_KEYCNT			*(vu16*)(REG_BASE+0x0132)


typedef void (*fnptr) (void);

#define IRQ_HBLANK			2

#define REG_IE				*(vu16*)(REG_BASE+0x0200)	//!< IRQ enable
#define REG_IF				*(vu16*)(REG_BASE+0x0202)	//!< IRQ status/acknowledge
#define REG_WAITCNT			*(vu16*)(REG_BASE+0x0204)	//!< Waitstate control
#define REG_IME				*(vu16*)(REG_BASE+0x0208)	//!< IRQ master enable
#define REG_PAUSE			*(vu16*)(REG_BASE+0x0300)	//!< Pause system (?)

#define REG_IFBIOS			*(vu16*)(REG_BASE-0x0008)	//!< IRQ ack for IntrWait functions
#define REG_RESET_DST		*(vu16*)(REG_BASE-0x0006)	//!< Destination for after SoftReset
#define REG_ISR_MAIN		*(fnptr*)(REG_BASE-0x0004)	//!< IRQ handler address

#endif // __MEMMAP__
