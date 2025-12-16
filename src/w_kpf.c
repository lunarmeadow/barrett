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

#include "rt_def.h"

#include "lumpy.h"

#include "miniz.h"
#include "miniz_common.h"
#include "miniz_zip.h"

#include "rt_main.h"

#include "rt_png.h"

#include "w_kpf.h"
#include "w_kpfdat.h"

static mz_zip_archive kpfArc;
static boolean isMounted;

int numWalls;
void** wallCache;
int* wallSize;

// TODO: search for kpf in LE install
const char* GetKPFPath(void)
{
    return "RottEX.kpf";
}

void InitKPF(void)
{
    mz_bool status;
    mz_zip_error err;

    if(mz_zip_validate_file_archive(GetKPFPath(), NULL, &err))
        status = mz_zip_reader_init_file(&kpfArc, GetKPFPath(), 0);

    if(!status)
    {
        printf("Couldn't initialize KPF file!\n");
        ShutdownKPF();
        isMounted = false;
        return;
    }

    isMounted = true;
}

void ShutdownKPF(void)
{
    mz_zip_reader_end(&kpfArc);

    for(int i = 0; i < numWalls; i++)
        free(wallCache[i]);

    free(wallSize);
}

// void KPF_DecodePatch(const char* name)
// {

// }

// void KPF_DecodeTransPatch(const char* name)
// {
    
// }

// void KPF_GetPatchType(void* jsonPtr, size_t* jsonSize)
// {

// }

void KPF_CacheBetaWalls(void)
{
    void *filePtr;
    size_t decompSize;
    mz_zip_archive_file_stat fileStat;
    char filePath[256];

    if(!isMounted)
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
        
        // this should roughly equate to the size of the wall
        wallSize[i] = fileStat.m_uncomp_size + sizeof(patch_t);

        if(fileIdx < 0)
        {
            if(mz_zip_reader_is_file_a_directory(&kpfArc, fileIdx))
            {
                printf("Attempted read %s is directory!", filePath);
                ShutdownKPF();
                ShutDown();
            }

            printf("Failed to locate patch lump %s", filePath);
            ShutdownKPF();
            ShutDown();
        }

        filePtr = mz_zip_reader_extract_file_to_heap(&kpfArc, filePath, &decompSize, NULL);

        if(!decompSize)
        {
            printf("File %s failed to decompress!", filePath);
            ShutdownKPF();
            ShutDown();
        }

        PNGDecode(filePtr, decompSize);

        mz_free(filePtr);
    }
}

// byte* KPF_JSONPatchByName(const char* name)
// {
//     void *filePtr;
//     void *jsonPtr;
//     size_t decompFileSize, decompJsonSize;

//     char filePath[256];
//     char jsonPath[256];

//     snprintf(filePath, 256, "/wad/wall/%s.png", name);
//     snprintf(jsonPath, 256, "/wad/wall/%s.png.json", name);

//     int fileIdx = mz_zip_reader_locate_file(&kpfArc, filePath, NULL, 
//             MZ_ZIP_FLAG_CASE_SENSITIVE);
    
//     int jsonIdx = mz_zip_reader_locate_file(&kpfArc, jsonPath, NULL, 
//             MZ_ZIP_FLAG_CASE_SENSITIVE);

//     if(fileIdx < 0 || jsonIdx < 0)
//     {
//         if(mz_zip_reader_is_file_a_directory(&kpfArc, fileIdx))
//         {
//             printf("Attempted read %s is directory!", name);
//             ShutdownKPF();
//             ShutDown();
//         }

//         printf("Failed to locate beta wall lump %s", name);
//         ShutdownKPF();
//         ShutDown();
//     }

//     filePtr = mz_zip_reader_extract_file_to_heap(&kpfArc, filePath, &decompFileSize, NULL);
//     jsonPtr = mz_zip_reader_extract_file_to_heap(&kpfArc, jsonPath, &decompJsonSize, NULL);

//     if(!decompFileSize || !decompJsonSize)
//     {
//         printf("File %s failed to decompress!", name);
//         ShutdownKPF();
//         ShutDown();
//     }

//     // KPF_GetPatchType(void *jsonPtr, size_t *jsonSize);
//     // PNGDecode(filePtr, decompSize);

//     mz_free(filePtr);
// }