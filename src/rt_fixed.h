/*
Copyright (C) 1994-1995 Apogee Software, Ltd.
Copyright (C) 2002-2015 icculus.org, GNU/Linux port
Copyright (C) 2017-2018 Steven LeVesque
Copyright (C) 2025-2026 lunarmeadow (she/her)
Copyright (C) 2025-2026 erysdren (it/its)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef _rt_fixed_public
#define _rt_fixed_public

#include <stdint.h>

typedef int16_t fix16_t;
typedef uint16_t ufix16_t;

typedef int32_t fix32_t;
typedef uint32_t ufix32_t;

/*
FUNCTION:
	Fixed32 FixedSqrtHP(Fixed32 n);
DESCRIPTION:
	This does a high-precision square root of a Fixed32.  It has
	8.16 bit accuracy.
*/

fix32_t FixedSqrtHP(fix32_t n); // High Precision (8.16)

fix32_t FixedMul(fix32_t a, fix32_t b);
fix32_t FixedDiv2(fix32_t a, fix32_t b);
fix32_t FixedScale(fix32_t orig, fix32_t factor, fix32_t divisor);
fix32_t FixedMulShift(fix32_t a, fix32_t b, fix32_t shift);

#endif
