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
#ifndef _rt_ted_private
#define _rt_ted_private

#define MAXPRECACHE 3500

#include "rt_actor.h"
#include "develop.h"

#define SHAREWARE_TAG	  0x4d4b
#define REGISTERED_TAG	  0x4344
#define RTL_VERSION		  (0x0101)
#define RXL_VERSION		  (0x0200)
#define COMMBAT_SIGNATURE ("RTC")
#define NORMAL_SIGNATURE  ("RTL")
#define NORMAL_SIGNATURE_EX  ("RXL")
#define COMMBAT_SIGNATURE_EX  ("RXC")
#define RTL_HEADER_OFFSET 8

typedef struct
{
	int lump;
	int cachelevel;
	int type; // To make precaching possible on big endian machines
} cache_t;

//========================================

typedef struct
{
	short RLEWtag;
	int headeroffsets[100];
	byte tileinfo[1];
} map_file_t;

typedef struct
{
	int planestart[3];
	word planelength[3];
	word width, height;
	char name[16];
} map_t;

#define ActorIsPushWall(xx, yy)                                                \
	((actorat[xx][yy]) && (((objtype*)actorat[xx][yy])->which == PWALL))
#define ActorIsWall(xx, yy)                                                    \
	((actorat[xx][yy]) && (((objtype*)actorat[xx][yy])->which == WALL))
#define ActorIsSpring(xx, yy)                                                  \
	((actorat[xx][yy]) && (((objtype*)actorat[xx][yy])->obclass == springobj))
#define StaticUndefined(xx, yy)                                                \
	((sprites[xx][yy]) && (((statobj_t*)sprites[xx][yy])->z < -64))

#define PRECACHEASTRINGX 141
#define PRECACHEASTRINGY 8

#define PRECACHEESTRINGX 16
#define PRECACHEESTRINGY 8

#define PRECACHESTRINGX 16
#define PRECACHESTRINGY 144

#define PRECACHEBARX 28
#define PRECACHEBARY 178

#define PRECACHELED1X 9
#define PRECACHELED1Y 8

#define PRECACHELED2X 9
#define PRECACHELED2Y 12

#define MAXLEDS 57

#define MAXSILLYSTRINGS 32

#endif
