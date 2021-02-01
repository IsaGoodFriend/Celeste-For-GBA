/*
 * Krawall, XM/S3M Modplayer Library
 * Copyright (C) 2001-2005, 2013 Sebastian Kienzl
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License in COPYING for more details.
 */

#ifndef __TOOLS_H__
#define __TOOLS_H__

#include "types.h"

#if !defined(__thumb__) && !defined(__arm__)
#error  "neither __thumb__ nor __arm__ defined!"
#endif

static inline void FastIntDivide( int Numer, int Denom, int *Result )
{
	asm volatile (
		" mov	r0, %1	\n"
#ifdef __thumb__
		" mov	r1, %2	\n"
		" beq	0f		\n"
		" swi	6		\n"
#else
		" movs	r1, %2	\n"
		" swine	0x60000	\n"
#endif
		"0: ldr   r2,	%0	\n"
		" str   r0, [r2]\n"
      	: "=m" ( Result )
      	: "r" ( Numer ), "r" ( Denom )
      	: "r0", "r1", "r2", "r3"
	);
}

static inline void FastIntDivideRem( int Numer, int Denom, int *Result, int *Remainder )
{
	asm volatile (
		" mov   r0,%2   \n"
#ifdef __thumb__
		" mov   r1,%3   \n"
		" beq	0f	\n"
		" swi   6       \n"
#else
		" movs	r1,%3	\n"
		" swine	0x60000	\n"
#endif
		" ldr   r2,%0   \n"
		" str   r0,[r2] \n"
		"0: ldr   r2,%1   \n"
		" str   r1,[r2] \n"
      	: "=m" ( Result ), "=m" ( Remainder ) // Outputs
      	: "r" ( Numer ), "r" ( Denom )        // Inputs
      	: "r0", "r1", "r2", "r3"              // Regs crushed & smushed
	);
}

static inline void FastIntRem( int Numer, int Denom, int *Remainder )
{
	asm volatile (
		" mov   r0,%1   \n"
#ifdef __thumb__
		" mov   r1,%2   \n"
		" beq	0f	\n"
		" swi   6       \n"
#else
		" movs   r1,%2   \n"
		" swine	0x60000\n"
#endif
		"0: ldr   r2,%0   \n"
		" str   r1,[r2] \n"
      	: "=m" ( Remainder ) 				  // Outputs
      	: "r" ( Numer ), "r" ( Denom )        // Inputs
      	: "r0", "r1", "r2", "r3"              // Regs crushed & smushed
	);
}

#endif

