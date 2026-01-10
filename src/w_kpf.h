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

#ifndef _KPF_H
#define _KPF_H

#include "lumpy.h"
#include <stddef.h>

// subsystem manager
bool KPF_Init(const char* path);
void KPF_Shutdown(void);
bool KPF_IsMounted(void);
bool KPF_IsCached(void);

// caching subsystem
// void KPF_CacheBetaWalls(void);
// void KPF_CacheAltSounds(void);

// use this to cache KPF data
bool KPF_MountAllResources(void);

// generic file indexing
void* KPF_GetEntryForNum(int entry);
size_t KPF_GetLengthForNum(int entry);

// ^-- audio indexing
void* KPF_GetAudioForEnum(int sndnum, int* len);

// ^-- wall indexing
void* KPF_GetWallForName(const char* name);
void* KPF_GetWallForNum(int tile);

#endif