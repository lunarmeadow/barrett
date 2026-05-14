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
#ifndef _develop_public
#define _develop_public

#include <stdbool.h>
#include <stdint.h>

#define NOMEMCHECK
#define WEAPONCHEAT 1
#define SYNCCHECK	1

#define DELUXE	0
#define LOWCOST 0

#define BNACRASHPREVENT 1 // bna added
// Flavor selection (shareware, registered, cd version, site license) has moved
// to the Makefile
#ifndef SHAREWARE
#define SHAREWARE 0
#endif

// cute little dopefish thing, only works with special patch?
#define DOPEFISH 0

// okay?

#define TEDLAUNCH 0

#if defined(__GNUC__) || defined(__clang__)
#define ATTR_PRINTF(FMT, VARGS) __attribute__((format(printf, FMT , VARGS)))
#define ATTR_NORETURN  __attribute__((noreturn))
#else
#define ATTR_PRINTF(FMT, VARGS)
#define ATTR_NORETURN __declspec(noreturn)
#endif

#endif
