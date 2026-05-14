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
//***************************************************************************
//
//    Z_ZONE.C - Carmack's Memory manager for protected mode
//
//***************************************************************************

/* erysdren NOTE: i borrowed this from Yamagi Quake II */

#ifndef _Z_ZONE_H_
#define _Z_ZONE_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// tags < 100 are not overwritten until freed
#define PU_STATIC	   1  // static entire execution time
#define PU_GAME		   20 // static until game completed
#define PU_LEVELSTRUCT 49 // start of static until level exited
#define PU_LEVEL	   50 // start of static until level exited
#define PU_LEVELEND	   51 // end of static until level exited

// tags >= 100 are purgable whenever needed
#define PU_PURGELEVEL	100
#define PU_CACHE		101
#define PU_CACHEWALLS	155
#define PU_CACHESPRITES 154
#define PU_CACHEBJWEAP	153
#define PU_CACHEACTORS	152
#define PU_CACHESOUNDS	120
#define PU_FLAT			102
#define PU_PATCH		103
#define PU_TEXTURE		104

#define URGENTLEVELSTART PU_LEVEL

extern bool zonememorystarted;
extern bool lowmemory;

void Z_Init(void);
void Z_ShutDown(void);

void Z_Free(void *ptr);
void Z_FreeTags(unsigned short lowtag, unsigned short hightag);

void *Z_Malloc(size_t size);           /* returns 0 filled memory */
void *Z_TagMalloc(size_t size, unsigned short tag);
void *Z_Realloc(void *ptr, size_t size);
void *Z_TagRealloc(void *ptr, size_t size, unsigned short tag);

void *Z_LevelMalloc(int size, int tag, void *user);
void Z_ChangeTag(void *ptr, int tag);
int Z_HeapSize(void);
int Z_UsedHeap(void);
int Z_UsedLevelHeap(void);

char *Z_StrDup(const char *s);

#endif /* _Z_ZONE_H_ */
