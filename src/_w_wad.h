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

typedef enum : int {
	WAD_TYPE_NONE, // invalid type
	WAD_TYPE_LUMP, // single lump file
	WAD_TYPE_IWAD, // main wad
	WAD_TYPE_PWAD, // patch wad
	WAD_TYPE_KPF // kex pack file
} wadType_t;

// mounted wad handle info
typedef struct wadInfo {
	char path[MAX_PATH];
	FILE *handle;
	wadType_t type;
} wadInfo_t;

// cached lump
typedef struct lumpInfo {
	char name[8]; // lump name (may not be null terminated)
	int handle; // source wadInfo_t index
	int position; // absolute position in source file in bytes
	int size; // total size in source file in bytes
	void *data; // cached data
	bool byteswapped; // true if has been byteswapped
} lumpInfo_t;

// wad header on disk
typedef struct wadHeader {
	char identification[4]; // should be IWAD
	int32_t num_lumps;
	int32_t ofs_lumps;
} wadHeader_t;

// wad lump on disk
typedef struct wadLump {
	int32_t filepos;
	int32_t size;
	char name[8];
} wadLump_t;

#endif
