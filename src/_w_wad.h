/*
Copyright (C) 1994-1995 Apogee Software, Ltd.
Copyright (C) 2002-2015 icculus.org, GNU/Linux port
Copyright (C) 2017-2018 Steven LeVesque
Copyright (C) 2025 lunarmeadow (she/her)
Copyright (C) 2025 erysdren (it/its)

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
#ifndef _w_wad_private
#define _w_wad_private

#include "develop.h"

//===============
//   TYPES
//===============

// wad lump in memory
typedef struct
{
	char name[8];
	FILE *handle;
	int position, size;
} lumpinfo_t;

// wad header on disk
typedef struct
{
	char identification[4]; // should be IWAD
	int numlumps;
	int infotableofs;
} dwadheader_t;

// wad lump on disk
typedef struct
{
	int filepos;
	int size;
	char name[8];
} dwadlump_t;

#endif
