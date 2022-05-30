#include "tonc_vscode.h"
#include <string.h>

typedef struct {
	unsigned int points[3];

} FlatPolygon;
typedef struct {
	unsigned int points[3];

} Polygon3D;

unsigned char *color_buffer, *back_buffer, *depth_buffer;
int needs_swap;

void mesh_render_interrupt() {

	int vcount = REG_VCOUNT;
	if ((vcount >= 0 && vcount < 159) || vcount == 227) { // 159, because we're one hblank behind
		REG_BG2Y = 0x000;

		unsigned short *src, *dst = MEM_VRAM;
		if (vcount == 227) {
			src = &color_buffer[240];
		} else {
			src = &color_buffer[((vcount + 2) * 240) % 160];
		}

		memcpy16(dst, src, 120);
	}
}
void vblankIntr() {
	if (needs_swap) {
		void* temp	 = back_buffer;
		back_buffer	 = color_buffer;
		color_buffer = temp;

		memcpy16(&tile8_mem[0][0], &color_buffer[0], 120 * 160);
	}
}

void init_mesh_rendering() {

	color_buffer = (unsigned char*)(0x02040000 - (240 * 160));
	back_buffer	 = color_buffer - (240 * 160);
	depth_buffer = back_buffer - (240 * 160);

	memset(color_buffer, 0x00, 240 * 160);
	// memset(back_buffer, 0x00, 240 * 160);
	// memset(depth_buffer, 0x00, 240 * 160);

	REG_DISPCNT = DCNT_MODE1 | DCNT_BG2;
	REG_BG2CNT	= BG_CBB(0) | BG_SBB(31) | BG_AFF_16x16 | BG_WRAP;

	irq_add(II_HBLANK, mesh_render_interrupt);
	irq_add(II_VBLANK, vblankIntr);

	// Test palette colors
	pal_bg_mem[0x0] = 0x0000;
	pal_bg_mem[0x1] = 0x1111;
	pal_bg_mem[0x2] = 0x2222;
	pal_bg_mem[0x3] = 0x3333;
	pal_bg_mem[0x4] = 0x4444;
	pal_bg_mem[0x5] = 0x5555;
	pal_bg_mem[0x6] = 0x6666;
	pal_bg_mem[0x7] = 0x7777;
	pal_bg_mem[0x8] = 0x8888;
	pal_bg_mem[0x9] = 0x9999;
	pal_bg_mem[0xA] = 0xAAAA;
	pal_bg_mem[0xB] = 0xBBBB;
	pal_bg_mem[0xC] = 0xCCCC;
	pal_bg_mem[0xD] = 0xDDDD;
	pal_bg_mem[0xE] = 0xEEEE;
	pal_bg_mem[0xF] = 0xFFFF;

	// Affine background shaped
	REG_BG2PA = 0x100;
	REG_BG2PC = 0x020;
	REG_BG2PD = 0x100;

	// Copy tiles to affine background
	unsigned int* ptr		 = &tile8_mem[0][0];
	unsigned short* tile_ptr = &se_mem[31][0];

	// Setting tiles for rendering
	int i, j;
	for (i = 0; i < 0x80; i += 8) {
		int diff = i >> 3;
		diff |= diff << 8;

		for (j = 0; j < 8; ++j)
			tile_ptr[i + j] = diff;
	}
}
void mesh_on_update() {

	REG_BG2Y = 0x000;

	if (needs_swap) {
		memset(back_buffer, 0x00, 240 * 160);
		needs_swap = 0;
	} else {
		render_tri(0x0000, 0x0010, 0x1000);
		needs_swap = 1;
	}

	// const int left = 1, right = 50;
	// const int up = 1, down = 120;

	// int y;
	// for (y = up; y <= down; ++y)
	// 	memset(&color_buffer[(y * 240) + left], 0x01, right - left);
}

#pragma region // Rendering code (will be converted to asm after)

void render_line(char* c_buff, int len) {
	while (len) {
		*c_buff = 1;
		//*d_buff = 0;

		c_buff++;
		// d_buff++;
		len--;
	}
}

#define VERT_MASK  0xFF
#define VERT_SHIFT 8
#define swap(a, b) \
	int t = a;     \
	a	  = b;     \
	b	  = t

void render_tri(int a, int b, int c) {
	int ax = a & 0xFF,
		ay = (a >> 8) & 0xFF;
	int bx = b & 0xFF,
		by = (b >> 8) & 0xFF;
	int cx = c & 0xFF,
		cy = (c >> 8) & 0xFF;

	if (ay > by) {
		swap(a, b);
	}
	if (by > cy) {
		swap(b, c);
		if (ay > by) {
			swap(a, b);
		}
	}

	if (ax < bx) {
		char* ptr = &back_buffer[ax + (ay * 240)];

		for (; ay < cy; ++ay) {
			render_line(ptr, bx - ax);
			ptr += 240;
		}

	} else {
	}
}

#pragma endregion