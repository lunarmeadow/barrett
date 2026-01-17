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
// W_wad.c

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "rt_def.h"
#include "rt_util.h"
#include "_w_wad.h"
#include "w_wad.h"
#include "z_zone.h"
#include "isr.h"
#include "develop.h"

#include "rt_crc.h"
#include "rt_main.h"
#include "w_kpfdat.h"
#include "spng.h"

//=============
// GLOBALS
//=============

//=============
// STATICS
//=============

static lumpInfo_t *lumpcache = NULL; // location of each lump on disk
static int num_lumps = 0;
static wadInfo_t *wadcache = NULL;
static int num_wads = 0;

/*
============================================================================

												LUMP BASED ROUTINES

============================================================================
*/

/*
====================
=
= W_AddFile
=
= All files are optional, but at least one file must be found
= Files with a .wad extension are wadlink files with multiple lumps
= Other files are single lumps with the base filename for the lump name
=
====================
*/

void W_AddFile(char* _filename)
{
	wadLump_t *fileinfo = NULL, *fileinfo_ptr = NULL, singleinfo;
	wadHeader_t header;
	lumpInfo_t* lump_p;
	unsigned i;
	FILE *handle;
	int length;
	int startlump;
	wadType_t type = WAD_TYPE_NONE;
	mz_zip_archive zip;

	char filename[MAX_PATH];
	char buf[MAX_PATH + 100]; // bna++

	strncpy(filename, _filename, sizeof(filename));
	filename[sizeof(filename) - 1] = '\0';
	FixFilePath(filename);

	// bna section start
	if (access(filename, 0) != 0)
	{
		strcpy(buf, "Error, Could not find User file '");
		strcat(buf, filename);
		strcat(buf, "', ignoring file");
		printf("%s", buf);
	}
	// bna section end

	//
	// read the entire file in
	//      FIXME: shared opens

	if ((handle = fopen(filename, "rb")) == NULL)
		return;

	startlump = num_lumps;

	if ((strcmpi(filename + strlen(filename) - 3, "wad")) &&
		(strcmpi(filename + strlen(filename) - 3, "rts")) &&
		(strcmpi(filename + strlen(filename) - 3, "kpf")))
	{
		// single lump file
		if (!quiet)
			printf("    Adding single file %s.\n", filename);
		fileinfo_ptr = &singleinfo;
		singleinfo.filepos = 0;
		singleinfo.size = file_size(handle);
		ExtractFileBase(filename, singleinfo.name);
		num_lumps++;
		type = WAD_TYPE_LUMP;
	}
	else if (!strcmpi(filename + strlen(filename) - 10, "RottEX.kpf"))
	{
		// KPF file
		if (!quiet)
			printf("    Adding %s.\n", filename);
		type = WAD_TYPE_KPF;

		// open archive as zip
		if (!mz_zip_reader_init_cfile(&zip, handle, 0, 0))
			Error("Failed to open %s as zip file!\n", filename);

		// manually add WALBs
		fileinfo_ptr = fileinfo = malloc((ARRAY_COUNT(betaWalls) + 1) * sizeof(wadLump_t));

		memset(fileinfo[0].name, '\0', sizeof(fileinfo[0].name));
		strncpy(fileinfo[0].name, "WALBSTRT", sizeof(fileinfo[0].name));
		fileinfo[0].filepos = 0;
		fileinfo[0].size = 0;

		for (int i = 1; i <= ARRAY_COUNT(betaWalls); i++)
		{
			char path[MAX_PATH];
			snprintf(path, sizeof(path), "wad/wall/%s.png", betaWalls[i - 1]);
			memset(fileinfo[i].name, '\0', sizeof(fileinfo[i].name));
			strncpy(fileinfo[i].name, betaWalls[i - 1], sizeof(fileinfo[i].name));
			fileinfo[i].filepos = IntelLong(mz_zip_reader_locate_file(&zip, path, NULL, 0)); // NOTE: reusing l->filepos to act as the zip file index
			fileinfo[i].size = IntelLong(4096); // NOTE: this will byteswap it incorrectly at first, because it gets byteswapped back later
		}

		num_lumps += ARRAY_COUNT(betaWalls) + 1;
	}
	else
	{
		// WAD file
		if (!quiet)
			printf("    Adding %s.\n", filename);
		fread(&header, 1, sizeof(header), handle);

		if (strncmp(header.identification, "IWAD", 4) &&
			strncmp(header.identification, "PWAD", 4))
			Error("Input file %s doesn't have WAD magic!\n", filename);

		if (!strncmp(header.identification, "IWAD", 4))
			type = WAD_TYPE_IWAD;
		if (!strncmp(header.identification, "PWAD", 4))
			type = WAD_TYPE_PWAD;

		header.num_lumps = IntelLong(header.num_lumps);
		header.ofs_lumps = IntelLong(header.ofs_lumps);
		length = header.num_lumps * sizeof(wadLump_t);
		fileinfo_ptr = fileinfo = malloc(length);
		if (!fileinfo)
			Error("Wad file could not allocate header");
		fseek(handle, header.ofs_lumps, SEEK_SET);
		fread(fileinfo, 1, length, handle);

		num_lumps += header.num_lumps;
	}

	//
	// Fill in lumpcache
	//
	Z_Realloc((void **)&lumpcache, num_lumps * sizeof(lumpInfo_t));
	lump_p = &lumpcache[startlump];

	for (i = startlump; i < (unsigned int)num_lumps; i++, lump_p++, fileinfo_ptr++)
	{
		fileinfo_ptr->filepos = IntelLong(fileinfo_ptr->filepos);
		fileinfo_ptr->size = IntelLong(fileinfo_ptr->size);
		lump_p->handle = num_wads;
		lump_p->position = fileinfo_ptr->filepos;
		lump_p->size = fileinfo_ptr->size;
		lump_p->data = NULL;
		lump_p->byteswapped = false;
		strncpy(lump_p->name, fileinfo_ptr->name, 8);
	}

	if (fileinfo)
		free(fileinfo);

	wadcache = realloc(wadcache, sizeof(wadInfo_t) * (num_wads + 1));
	strncpy(wadcache[num_wads].path, filename, sizeof(filename));
	wadcache[num_wads].handle = handle;
	wadcache[num_wads].type = type;
	memcpy(&wadcache[num_wads].zip, &zip, sizeof(zip));
	num_wads++;
}

/*
====================
=
= W_InitMultipleFiles
=
= Pass a null terminated list of files to use.
=
= All files are optional, but at least one file must be found
=
= Files with a .wad extension are idlink files with multiple lumps
=
= Other files are single lumps with the base filename for the lump name
=
= Lump names can appear multiple times. The name searcher looks backwards,
= so a later file can override an earlier one.
=
====================
*/

void W_InitMultipleFiles(char** filenames)
{
	//
	// open all the files, load headers, and count lumps
	//

	// init with a SafeMalloc (since Z_Realloc cant handle NULL)
	lumpcache = SafeMalloc(sizeof(lumpInfo_t));

	for (; *filenames; filenames++)
		W_AddFile(*filenames);

	if (!num_lumps)
		Error("W_InitFiles: no files found");

	if (!quiet)
		printf("W_Wad: Wad Manager Started NUMLUMPS=%ld\n", (long int)num_lumps);
}

/*
====================
=
= W_InitFile
=
= Just initialize from a single file
=
====================
*/

void W_InitFile(char* filename)
{
	char* names[2];

	names[0] = filename;
	names[1] = NULL;
	W_InitMultipleFiles(names);
}

/*
====================
=
= W_NumLumps
=
====================
*/

int W_NumLumps(void)
{
	return num_lumps;
}

/*
====================
=
= W_CheckNumForName
=
= Returns -1 if name not found
=
====================
*/

int W_CheckNumForName(char* name)
{
	char name8[9];
	int v1, v2;
	lumpInfo_t* lump_p;
	lumpInfo_t* endlump;

	// make the name into two integers for easy compares

	strncpy(name8, name, 8);
	name8[8] = 0;  // in case the name was a fill 8 chars
	strupr(name8); // case insensitive

	v1 = *(int*)name8;
	v2 = *(int*)&name8[4];

	// scan backwards so patch lump files take precedence

	lump_p = lumpcache;
	endlump = lumpcache + num_lumps;

	while (lump_p != endlump)
	{
		if (*(int*)lump_p->name == v1 && *(int*)&lump_p->name[4] == v2)
			return lump_p - lumpcache;
		lump_p++;
	}

	return -1;
}

/*
====================
=
= W_GetNumForName
=
= Calls W_CheckNumForName, but bombs out if not found
=
====================
*/

int W_GetNumForName(char* name)
{
	int i;

	i = W_CheckNumForName(name);
	if (i != -1)
		return i;

	Error("W_GetNumForName: %s not found!", name);
	return -1;
}

/*
====================
=
= W_LumpLength
=
= Returns the buffer size needed to load the given lump
=
====================
*/

int W_LumpLength(int lump)
{
	if (lump >= num_lumps)
		Error("W_LumpLength: %i >= num_lumps", lump);
	return lumpcache[lump].size;
}

/*
====================
=
= W_GetNameForNum
=
====================
*/

const char* W_GetNameForNum(int i)
{

	if (i >= num_lumps)
		Error("W_GetNameForNum: %i >= num_lumps", i);
	return &(lumpcache[i].name[0]);
}

/*
====================
=
= W_ReadLump
=
= Loads the lump into the given buffer, which must be >= W_LumpLength()
=
====================
*/
int readinglump;
byte* lumpdest;
void W_ReadLump(int lump, void* dest)
{
	int c;
	lumpInfo_t* l;

	readinglump = lump;
	lumpdest = dest;
	if (lump >= num_lumps)
		Error("W_ReadLump: %i >= num_lumps", lump);
	if (lump < 0)
		Error("W_ReadLump: %i < 0", lump);
	l = lumpcache + lump;

	if (wadcache[l->handle].type == WAD_TYPE_KPF)
	{
		if (strncmp(l->name, "WALB", 4) == 0)
		{
			mz_zip_archive_file_stat fileStat;
			void *pngBuffer;
			spng_ctx *pngCtx;
			size_t pngDecodeLen;
			struct spng_ihdr pngHdr = {0};

			// NOTE: reusing l->position to act as the zip file index
			mz_zip_reader_file_stat(&wadcache[l->handle].zip, l->position, &fileStat);

			pngBuffer = malloc(fileStat.m_uncomp_size);

			mz_zip_reader_extract_to_mem(&wadcache[l->handle].zip, l->position, pngBuffer, fileStat.m_uncomp_size, 0);

			pngCtx = spng_ctx_new(0);
			spng_set_png_buffer(pngCtx, pngBuffer, fileStat.m_uncomp_size);
			spng_decoded_image_size(pngCtx, SPNG_FMT_PNG, &pngDecodeLen);
			spng_set_image_limits(pngCtx, 64, 64);

			spng_get_ihdr(pngCtx, &pngHdr);

			// ROTT walls must be 64x64, 8-bit indexed images! NOTE! Some LE textures are greyscale, and should be palette-matched.
			if(pngHdr.bit_depth != 8 || pngHdr.width != 64 || pngHdr.height != 64 || pngDecodeLen != l->size)
				Error("W_ReadLump: invalid texture format for %.8s\nbit-depth = %d\nwidth = %d, height = %d, decoded length = %zu!", l->name, pngHdr.bit_depth, pngHdr.width, pngHdr.height, pngDecodeLen);

			if(pngHdr.color_type != SPNG_COLOR_TYPE_INDEXED)
				printf("W_ReadLump: Warning! Beta wall entry %.8s isn't using indexed colour!\n", l->name);

			spng_decode_image(pngCtx, dest, pngDecodeLen, SPNG_FMT_PNG, 0);

			free(pngBuffer);
			spng_ctx_free(pngCtx);
		}
		else
		{
			Error("W_ReadLump: unsupported KPF lump %.8s requested", l->name);
		}
	}
	else
	{
		fseek(wadcache[l->handle].handle, l->position, SEEK_SET);
		c = fread(dest, 1, l->size, wadcache[l->handle].handle);
		if (c < l->size)
			Error("W_ReadLump: only read %i of %i on lump %i", c, l->size, lump);
	}
}

/*
====================
=
= W_WriteLump
=
= Writes the lump into the given buffer, which must be >= W_LumpLength()
=
====================
*/

void W_WriteLump(int lump, void* src)
{
	int c;
	lumpInfo_t* l;

	if (lump >= num_lumps)
		Error("W_WriteLump: %i >= num_lumps", lump);
	if (lump < 0)
		Error("W_WriteLump: %i < 0", lump);
	l = lumpcache + lump;

	fseek(wadcache[l->handle].handle, l->position, SEEK_SET);
	c = fwrite(src, l->size, 1, wadcache[l->handle].handle);
	if (c < l->size)
		Error("W_WriteLump: only wrote %i of %i on lump %i", c, l->size, lump);
}

/*
====================
=
= W_CacheLumpNum
=
====================
*/

// ludicrous todo: add special case for caching LE beta walls from KPF.
void* W_CacheLumpNum(int lump, int tag, converter_t converter, int numrec)
{
	if (lump >= (int)num_lumps)
		Error("W_CacheLumpNum: %i >= num_lumps", lump);

	else if (lump < 0)
		Error("W_CacheLumpNum: %i < 0  Taglevel: %i", lump, tag);

	if (!lumpcache[lump].data)
	{
		// read the lump in
		Z_Malloc(W_LumpLength(lump), tag, &lumpcache[lump].data);
		W_ReadLump(lump, lumpcache[lump].data);
		Debug("Invoking endian converter on %p, %i records\n", lumpcache[lump].data, numrec);
		converter(lumpcache[lump].data, numrec);
		lumpcache[lump].byteswapped = true;
	}
	else
	{
		Z_ChangeTag(lumpcache[lump].data, tag);
	}

	return lumpcache[lump].data;
}

/*
====================
=
= W_CacheLumpName
=
====================
*/

void* W_CacheLumpName(char* name, int tag, converter_t converter, int numrec)
{
	return W_CacheLumpNum(W_GetNumForName(name), tag, converter, numrec);
}

/*
====================
=
= W_Shutdown
=
====================
*/

void W_Shutdown(void)
{
	if (wadcache)
	{
		for (int i = 0; i < num_wads; i++)
		{
			if (wadcache[i].type == WAD_TYPE_KPF)
				mz_zip_reader_end(&wadcache[i].zip);
			fclose(wadcache[i].handle);
		}
		free(wadcache);
		wadcache = NULL;
		num_wads = 0;
	}
}
