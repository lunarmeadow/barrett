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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "rt_def.h"

#include "lumpy.h"

#include "miniz.h"
#include "miniz_common.h"
#include "miniz_zip.h"

#include "rt_main.h"

#include "w_kpf.h"
#include "rt_util.h"
#include "spng.h"
#include "w_kpfdat.h"

static mz_zip_archive kpfArc;

// ensure we don't do anything stupid
static bool _isMounted;
static bool _areWallsCached = false;

// -- FILE TABLE --

int numWalls = ARRAY_COUNT(betaWalls);
int numSounds = ARRAY_COUNT(altSounds);

// wall entry range

constexpr int WALL_START = 0;
constexpr int WALL_END = WALL_START + numWalls;

// alternate sound range

constexpr int ALTSND_START = WALL_END + 1;
constexpr int ALTSND_END = ALTSND_START + numSounds;

// todo: stuff like widescreen graphics, hud etc.

// tally lengths of all sub-tables to get a number of entries
constexpr int NUM_ENTRIES = (WALL_END - WALL_START) + (ALTSND_END - ALTSND_START);

// KPF entry table
static uint8_t** fileCache;
uint8_t** entrySize;

// stores decoded png data, loaded into wall cache
static void* _decodeBuffer;

// -- KPF MANAGER --

void InitKPF(const char* path)
{
    mz_bool status = MZ_FALSE;
    mz_zip_error err;

    // initialize base pointers for wall cache tables
    wallCache = calloc(numWalls, patchSize);

    // allocate table of pointers for decoding stage
    _decodeBuffer = malloc(sizeof(void*));

    // initialize kpf shit
    if(mz_zip_validate_file_archive(path, 0, &err))
        status = mz_zip_reader_init_file(&kpfArc, path, 0);

    // shit don't work ! ! !
    if(!status)
    {
        printf("InitKPF: couldn't initialize KPF file %s with miniz error %s!\n", path, mz_zip_get_error_string(err));
        _isMounted = false;
        return;
    }

    // we loaded alright
    printf("Added %s\n", path);
    _isMounted = true;
}

void ShutdownKPF(void)
{
    // close and free miniz resources
    mz_zip_reader_end(&kpfArc);

    // free table base pointer and all indices if wall cache is loaded
    if (fileCache)
    {
        for (int i = 0; i < numWalls; i++)
        {
            // check if data is actually allocated, free if not.
            // this should avoid freeing on
            if (fileCache[i])
                free(fileCache[i]);
        }
        free(fileCache);
    }
}

bool IsKPFMounted(void)
{
    return _isMounted;
}

// -- PRECACHER SUBSYSTEM --

void KPF_CacheBetaWalls(void)
{
    mz_zip_archive_file_stat fileStat;
    char filePath[256];

    size_t len_png = 0;
    size_t len_decode;

    mz_bool status = MZ_FALSE;

    if(!_isMounted)
        return;

    if(_areWallsCached)
        return;
    
    for(int i = 0; i < numWalls; i++)
    {
        spng_ctx *ctx = spng_ctx_new(0);
        struct spng_ihdr ihdr = {0};

        snprintf(filePath, 256, "wad/wall/%s.png", betaWalls[i]);

        if (!ctx)
            Error("KPF_CacheBetaWalls: failed to create PNG context for LE wall decoding!\n");

        // -- LOCATE AND VALIDATE FILE --

        int fileIdx = mz_zip_reader_locate_file(&kpfArc, filePath, NULL, 0);

        if(fileIdx < 0)
            Error("KPF_CacheBetaWalls: failed to locate patch lump %s\n", filePath);

        if(mz_zip_reader_is_file_a_directory(&kpfArc, fileIdx))
            Error("KPF_CacheBetaWalls: attempted read %s is directory!\n", filePath);

        // -- EXTRACT PNG, LOAD IT INTO DECODING BUFFER --

        status = mz_zip_reader_file_stat(&kpfArc, fileIdx, &fileStat);
        
        // get size to alloc decoding buffers
        if(!status)
            Error("KPF_CacheBetaWalls: file %s has no info!\n", filePath);
        else 
            len_png = fileStat.m_uncomp_size;

        _decodeBuffer = malloc(len_png);
    
        // spng decodes from this decoded buffer using set_png_buffer
        status = mz_zip_reader_extract_file_to_mem(&kpfArc, filePath, _decodeBuffer, len_png, 0);

        if(!status)
            Error("KPF_CacheBetaWalls: file %s failed to decompress!\n", filePath);

        // -- GOOD TO DECODE! --
        // references https://github.com/fabiangreffrath/woof and https://github.com/fabiangreffrath/taradino/tree/kpf

        // give spng the buffer containing data, pass it along and decode into wall cache
        spng_set_png_buffer(ctx, _decodeBuffer, len_png);
		spng_decoded_image_size(ctx, SPNG_FMT_PNG, &len_decode);
		spng_set_image_limits(ctx, 64, 64);

        spng_get_ihdr(ctx, &ihdr);

         // ROTT walls must be 64x64, 8-bit indexed images! NOTE! Some LE textures are greyscale, and should be palette-matched.
        if(ihdr.bit_depth != 8 || ihdr.width != 64 || ihdr.height != 64 || len_decode != 4096)
            Error("KPF_CacheBetaWalls: invalid texture format for %s\nbit-depth = %d\nwidth = %d, height = %d, decoded length = %zu!", 
            filePath, ihdr.bit_depth, ihdr.width, ihdr.height, len_decode);

        // cache the decoded image from spng's buffer
        wallCache[i] = malloc(len_decode);
        int err = spng_decode_image(ctx, wallCache[i], len_decode, SPNG_FMT_PNG, 0);

        if(err)
            Error("KPF_CacheBetaWalls: decoding %s of size %zu to ROTT texture failed - %s!", filePath, len_decode, spng_strerror(err));

        free(_decodeBuffer);
        spng_ctx_free(ctx);
    }

    _areWallsCached = true;
}

// -- GENERIC KPF FILE INDEXING OPERATIONS --

void* KPF_GetEntryForNum(int entry)
{
    if(entry < 0 || entry > NUM_ENTRIES)
    {
        Error("KPF entry %d out of bounds!", entry);
    }
}

// -- BETA WALL INDEXERS --

void* KPF_GetWallForName(const char* name)
{
    int entryNum;

    if(!_isMounted)
    {
        Error("KPF_GetWallForName: kpf not mounted\n");
    }

    if(!_areWallsCached)
    {
        Error("KPF_GetWallForName: attempt to index %s but walls not cached\n", name);
    }

    // index in cache should match index in betaWalls, so this should just work.
    for(int i = WALL_START; i < numWalls; i++)
        if(strcmp(betaWalls[i], name) == 0)
            entryNum = i;
        else
            entryNum = -1;

    if(entryNum == -1)
    {
        Error("KPF_GetWallForName: no lump found for num %d - %s\n", entryNum, name);
    }

    return wallCache[entryNum];
}

void* KPF_GetWallForNum(int tile)
{
    if(!_isMounted)
    {
        Error("KPF_GetWallForNum: kpf not mounted\n");
    }

    if(!_areWallsCached)
    {
        Error("KPF_GetWallForNum: attempt to index %d but walls not cached\n", tile);
    }

    if(tile < WALL_START || tile > WALL_END)
    {
        Error("KPF_GetWallForNum: index %d out of range!", tile);
    }

    return wallCache[tile];
}