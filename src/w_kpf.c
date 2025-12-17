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

#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "rt_def.h"

#include "lumpy.h"

#include "miniz.h"
#include "miniz_common.h"
#include "miniz_zip.h"

#include "rt_main.h"

#include "w_kpf.h"
#include "w_kpfdat.h"

static mz_zip_archive kpfArc;

// ensure we don't do anything stupid
static boolean isMounted;
static boolean areWallsCached = false;

// numWalls is a compile-time constant
int numWalls = ARRAY_COUNT(betaWalls);

// wallCache is a table of pointers to memory blocks containing decoded patches. 
// each index correponds to the entry in betaWalls[i], and same with wallSize
// a hash table would likely be better here but that's complex.
// this is sort of structured like the pre-existing wad code but very specific.
// Access through KPF_GetWallFromCache which uses a linear search in betaWalls to index by name.
void** wallCache;
int* wallSize;

void InitKPF(const char* path)
{
    mz_bool status = MZ_FALSE;
    mz_zip_error err;

    // initialize base pointers for wall cache tables
    wallSize = calloc(numWalls, sizeof(*wallSize));
    wallCache = calloc(numWalls, sizeof(*wallCache));

    // initialize kpf shit
    if(mz_zip_validate_file_archive(path, 0, &err))
        status = mz_zip_reader_init_file(&kpfArc, path, 0);

    // shit don't work ! ! !
    if(!status)
    {
        printf("InitKPF: couldn't initialize KPF file %s with miniz error %s!\n", path, mz_zip_get_error_string(err));
        ShutdownKPF();
        isMounted = false;
        return;
    }

    // we loaded alright
    printf("Added %s\n", path);
    isMounted = true;
}

void ShutdownKPF(void)
{
    // close and free miniz resources
    mz_zip_reader_end(&kpfArc);

    // free table base pointer and all indices if wall cache is loaded
    if (wallCache)
    {
        for (int i = 0; i < numWalls; i++)
        {
            // check if data is actually allocated, free if not.
            // this should avoid freeing on
            if (wallCache[i])
                free(wallCache[i]);
        }
        free(wallCache);
    }

    if (wallSize)
        free(wallSize);
}

void KPF_CacheBetaWalls(void)
{
    mz_zip_archive_file_stat fileStat;
    char filePath[256];

    mz_bool status = MZ_FALSE;

    if(!isMounted)
        return;

    if(areWallsCached)
        return;
    
    for(int i = 0; i < numWalls; i++)
    {
        snprintf(filePath, 256, "wad/wall/%s.png", betaWalls[i]);

        // locate and validate file

        int fileIdx = mz_zip_reader_locate_file(&kpfArc, filePath, NULL, 
                0);

        if(fileIdx < 0)
        {
            printf("KPF_CacheBetaWalls: failed to locate patch lump %s\n", filePath);
            ShutdownKPF();
            ShutDown();
        }

        if(mz_zip_reader_is_file_a_directory(&kpfArc, fileIdx))
        {
            printf("KPF_CacheBetaWalls: attempted read %s is directory!\n", filePath);
            ShutdownKPF();
            ShutDown();
        }

        // extract file info

        status = mz_zip_reader_file_stat(&kpfArc, fileIdx, &fileStat);
        
        if(!status)
        {
            printf("KPF_CacheBetaWalls: file %s has no info!\n", filePath);
            ShutdownKPF();
            ShutDown();
        }
        else 
        {
            wallSize[i] = fileStat.m_uncomp_size;
            wallCache[i] = malloc(fileStat.m_uncomp_size);
        }

        // load png into test cache buffer
         
        // todo: should load the png into a scratch buffer instead, and then use it to decode into a patch_t which is then loaded into wallCache.
    
        status = mz_zip_reader_extract_file_to_mem(&kpfArc, filePath, wallCache[i], wallSize[i], 0);

        if(!status)
        {
            printf("KPF_CacheBetaWalls: file %s failed to decompress!\n", filePath);
            ShutdownKPF();
            ShutDown();
        }
    }

    areWallsCached = true;
}

static int _KPF_GetWallForName(const char* name)
{
    // index in cache should match index in betaWalls, so this should just work.
    for(int i = 0; i < numWalls; i++)
        if(strcmp(betaWalls[i], name) == 0)
            return i;

    return -1;
}

// outLength should take a pointer to an integer that stores the total length of the malloc'd cache entry
// cast this to a pointer of the required type. KPF subsystem manages resources on free.
// returns an allocated block of memory managed by kpf subsystem containing patch data.
void* KPF_GetWallFromCache(const char* name, int* outLength)
{
    if(!isMounted)
    {
        printf("KPF_GetLumpFromCache: kpf not mounted\n");
        goto err;
    }

    if(!areWallsCached)
    {
        printf("KPF_GetLumpFromCache: attempt to index %s but walls not cached\n", name);
        goto err;
    }

    int lumpNum = _KPF_GetWallForName(name);
    
    if(lumpNum == -1)
    {
        printf("KPF_GetLumpFromCache: no lump found for num %d - %s\n", lumpNum, name);
        goto err;
    }

    *outLength = wallSize[lumpNum];
    return wallCache[lumpNum];

err:
    ShutdownKPF();
    ShutDown();
    return NULL;
}