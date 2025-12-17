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
static boolean isMounted;
static boolean areWallsCached = false;

int numWalls;
void** wallCache;
int* wallSize;

void InitKPF(const char* path)
{
    mz_bool status;
    mz_zip_error err;

    if(mz_zip_validate_file_archive(path, NULL, &err))
        status = mz_zip_reader_init_file(&kpfArc, path, 0);

    if(!status)
    {
        printf("InitKPF: couldn't initialize KPF file %s!\n", path);
        ShutdownKPF();
        isMounted = false;
        return;
    }

    isMounted = true;
}

void ShutdownKPF(void)
{
    int cacheidx = 0;

    mz_zip_reader_end(&kpfArc);

    // for(int i = 0; i < numWalls; i++)
    //     Z_Free(wallCache[i]);

    // should be safer, this SHOULD free all allocated walls even if only cache is only partially filled.
    // still should be suspect of this a bit
    if(wallCache[0] != NULL)
        do free(wallCache[cacheidx]); 
            while (wallCache[++cacheidx] != NULL);

    // avoid freeing before malloc. this should never occur, but better safe than sorry.
    if(wallSize != NULL)
        free(wallSize);
}

void KPF_CacheBetaWalls(void)
{
    void *filePtr;
    size_t decompSize;
    mz_zip_archive_file_stat fileStat;
    char filePath[256];

    if(!isMounted)
        return;

    if(areWallsCached)
        return;

    if(!numWalls)
        numWalls = ARRAY_COUNT(betaWalls);

    if(!wallSize)
        wallSize = malloc(numWalls * sizeof(wallSize));
    
    for(int i = 0; i < numWalls; i++)
    {
        snprintf(filePath, 256, "/wad/wall/%s.png", betaWalls[i]);

        int fileIdx = mz_zip_reader_locate_file(&kpfArc, filePath, NULL, 
                MZ_ZIP_FLAG_CASE_SENSITIVE);

        mz_zip_reader_file_stat(&kpfArc, fileIdx, &fileStat);
        
        // this should roughly equate to the size of the wall. still freaks me out a bit.
        wallSize[i] = fileStat.m_uncomp_size;
        wallCache[i] = malloc(wallSize[i]);
    
        if(fileIdx < 0)
        {
            if(mz_zip_reader_is_file_a_directory(&kpfArc, fileIdx))
            {
                printf("KPF_CacheBetaWalls: attempted read %s is directory!", filePath);
                ShutdownKPF();
                ShutDown();
            }

            printf("KPF_CacheBetaWalls: failed to locate patch lump %s", filePath);
            ShutdownKPF();
            ShutDown();
        }

        filePtr = mz_zip_reader_extract_file_to_heap(&kpfArc, filePath, &decompSize, NULL);

        if(!decompSize)
        {
            printf("KPF_CacheBetaWalls: file %s failed to decompress!", filePath);
            ShutdownKPF();
            ShutDown();
        }

        mz_free(filePtr);
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
// cast this to a pointer of the required type. KPF subsystem manages resources on free
static void* KPF_GetWallFromCache(const char* name, int* outLength)
{
    if(!isMounted)
    {
        printf("KPF_GetLumpFromCache: kpf not mounted");
        goto err;
    }

    if(!areWallsCached)
    {
        printf("KPF_GetLumpFromCache: walls not cached");
        goto err;
    }

    int lumpNum = _KPF_GetWallForName(name);
    
    if(lumpNum == -1)
    {
        printf("KPF_GetLumpFromCache: no lump found for num %d - %s", lumpNum, name);
        goto err;
    }

    *outLength = wallSize[lumpNum];

    return wallCache[lumpNum];

err:
    ShutdownKPF();
    ShutDown();
}