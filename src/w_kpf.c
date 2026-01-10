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

#include <stddef.h>
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
static bool _isMounted = false;
static bool _areWallsCached = false;
static bool _areSoundsCached = false;

// -- FILE TABLE --

constexpr int numWalls = ARRAY_COUNT(betaWalls);
constexpr int numSounds = ARRAY_COUNT(altSounds);

// wall entry range

constexpr int WALL_START = 0;
constexpr int WALL_END = WALL_START + numWalls - 1;

// alternate sound range

constexpr int ALTSND_START = WALL_END + 1;
constexpr int ALTSND_END = ALTSND_START + numSounds - 1;

// todo: stuff like widescreen graphics, hud etc.

// just use the endpoint of last section
constexpr int NUM_ENTRIES = ALTSND_END;

// KPF entry table
static uint8_t** fileCache;
size_t* entrySize;

// stores decoded png data, loaded into wall cache
static void* _decodeBuffer;

// -- KPF MANAGER --

bool KPF_Init(const char* path)
{
    mz_bool status = MZ_FALSE;
    mz_zip_error err;

    // initialize base pointers for cache table
    fileCache = calloc(NUM_ENTRIES, sizeof(uint8_t*));
    entrySize = calloc(NUM_ENTRIES, sizeof(size_t*));

    // allocate table of pointers for decoding stage of PNG loader
    _decodeBuffer = malloc(sizeof(void*));

    // initialize kpf shit
    if(mz_zip_validate_file_archive(path, 0, &err))
        status = mz_zip_reader_init_file(&kpfArc, path, 0);

    // shit don't work ! ! !
    if(!status)
    {
        printf("InitKPF: couldn't initialize KPF file %s with miniz error %s!\n", path, mz_zip_get_error_string(err));
        _isMounted = false;
        return false;
    }

    // we loaded alright
    printf("Added %s\n", path);
    _isMounted = true;
    return true;
}

void KPF_Shutdown(void)
{
    // close and free miniz resources
    mz_zip_reader_end(&kpfArc);

    if (fileCache)
    {
        for (int i = 0; i < NUM_ENTRIES; i++)
        {
            // check if data is actually allocated, free if not.
            // this should avoid freeing on
            if (fileCache[i])
                free(fileCache[i]);
        }
        free(fileCache);
    }

    if (entrySize)
    {
        free(entrySize);
    }

    // reset state variables
    _isMounted = false;
    _areSoundsCached = _areWallsCached = false;
}

bool KPF_IsMounted(void)
{
    return _isMounted;
}

bool KPF_IsCached(void)
{
    return _areSoundsCached && _areWallsCached;
}

// -- PRECACHER SUBSYSTEM --

void KPF_CacheBetaWalls(void)
{
    mz_zip_archive_file_stat fileStat;
    char filePath[256];

    size_t len_png = 0;
    size_t len_decode;

    mz_bool status = MZ_FALSE;
    
    for(int i = WALL_START; i < WALL_END; i++)
    {
        spng_ctx *ctx = spng_ctx_new(0);
        struct spng_ihdr ihdr = {0};

        snprintf(filePath, 256, "wad/wall/%s.png", betaWalls[i - WALL_START]);

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

        printf("Warning! Beta wall entry %s isn't using indexed colour!\n", betaWalls[i]);

        // cache the decoded image from spng's buffer
        fileCache[i] = malloc(len_decode);
        entrySize[i] = len_decode;
        int err = spng_decode_image(ctx, fileCache[i], len_decode, SPNG_FMT_PNG, 0);

        if(err)
            Error("KPF_CacheBetaWalls: decoding %s of size %zu to ROTT texture failed - %s!", filePath, len_decode, spng_strerror(err));

        // todo: rotate and recolor non-indexed images

        free(_decodeBuffer);
        spng_ctx_free(ctx);
    }

    _areWallsCached = true;
}

void KPF_CacheAltSounds(void)
{
    mz_zip_archive_file_stat fileStat;
    char filePath[256];

    size_t len_wav = 0;

    mz_bool status = MZ_FALSE;

    for(int i = ALTSND_START; i < ALTSND_END; i++)
    {
        snprintf(filePath, 256, "tactile/alt/%s.wav", altSounds[i - ALTSND_START]);

        // -- LOCATE AND VALIDATE FILE --

        int fileIdx = mz_zip_reader_locate_file(&kpfArc, filePath, NULL, 0);

        if(fileIdx < 0)
            Error("KPF_CacheAltSounds: failed to locate audio lump %s\n", filePath);

        if(mz_zip_reader_is_file_a_directory(&kpfArc, fileIdx))
            Error("KPF_CacheAltSounds: attempted read %s is directory!\n", filePath);

        // -- GET FILE INFO TO ALLOC BUFFERS --

        status = mz_zip_reader_file_stat(&kpfArc, fileIdx, &fileStat);
        
        // get size to alloc decoding buffers
        if(!status)
            Error("KPF_CacheAltSound: file %s has no info!\n", filePath);
        else 
            len_wav = fileStat.m_uncomp_size;

        // -- STORE IT BACK IN FILE CACHE --

        // allocate space in table for the audio entry
        fileCache[i] = malloc(len_wav);
        entrySize[i] = len_wav;

        // store to new memory block
        status = mz_zip_reader_extract_file_to_mem(&kpfArc, filePath, fileCache[i], len_wav, 0);

        if(!status)
            Error("KPF_CacheBetaWalls: file %s failed to decompress!\n", filePath);
    }

    _areSoundsCached = true;
}

bool KPF_MountAllResources(void)
{
    // skip if
    if(!KPF_IsMounted())
        return false;

    // create and store all precachers here
    KPF_CacheBetaWalls();
    KPF_CacheAltSounds();

    return true;
}

// -- GENERIC KPF FILE INDEXING OPERATIONS --

void* KPF_GetEntryForNum(int entry)
{
    if(entry < 0 || entry > NUM_ENTRIES)
    {
        Error("KPF entry %d out of bounds!", entry);
    }

    return fileCache[entry];
}

size_t KPF_GetLengthForNum(int entry)
{
    if(entry < 0 || entry > NUM_ENTRIES)
    {
        Error("KPF entry %d out of bounds!", entry);
    }

    return entrySize[entry];
}

// -- AUDIO INDEXER --

void* KPF_GetAudioForEnum(int sndnum)
{
    for(int i = 0; i < (int)ARRAY_COUNT(altSoundMapping); i++)
    {
        if(altSoundMapping[i] == sndnum)
        {
            return fileCache[i + ALTSND_START];
        }
    }

    Error("KPF_GetAudioForEnum: no audio mapping found for sndnum %d!\n", sndnum);
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
    for(int i = WALL_START; i < WALL_END; i++)
        if(strcmp(betaWalls[i], name) == 0)
            entryNum = i;
        else
            entryNum = -1;

    if(entryNum == -1)
    {
        Error("KPF_GetWallForName: no lump found for num %d - %s\n", entryNum, name);
    }

    return fileCache[entryNum];
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

    return fileCache[tile];
}