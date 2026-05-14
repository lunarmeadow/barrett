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
//    Z_Zone Memory Manager
//
//***************************************************************************

/* erysdren NOTE: i borrowed this from Yamagi Quake II */

#include "z_zone.h"
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "rt_def.h"
#include "_z_zone.h"
#include "z_zone.h"
#include "rt_util.h"
#include "develop.h"
#include "rt_net.h"

bool zonememorystarted = false;
bool lowmemory = false;

#define Z_MAGIC 0x1d1d

typedef struct zhead_s
{
	struct zhead_s *prev, *next;
	size_t size;
	unsigned short magic;
	unsigned short tag; /* for group free */
} zhead_t;

static zhead_t z_chain;
static size_t z_count, z_bytes;

void
Z_Init(void)
{
	memset(&z_chain, 0, sizeof(z_chain));

	z_chain.prev = &z_chain;
	z_chain.next = &z_chain;

	z_count = 0;
	z_bytes = 0;

	zonememorystarted = true;
}

void Z_ShutDown(void)
{
	zhead_t *z, *next;

	for (z = z_chain.next; z != &z_chain; z = next)
	{
		next = z->next;
		Z_Free((void *)(z + 1));
	}

	zonememorystarted = false;
}

void
Z_Free(void *ptr)
{
	zhead_t *z;

	if (!ptr)
	{
		return;
	}

	z = ((zhead_t *)ptr) - 1;

	if (z->magic != Z_MAGIC)
	{
		Error("%s: not a valid memory block: %p", __func__, ptr);
		return;
	}

	z->prev->next = z->next;
	z->next->prev = z->prev;

	z_count--;
	z_bytes -= z->size;

	z->magic = 0; /* can avoid possible double free with check above */
	free(z);
}

void
Z_FreeTags(unsigned short lowtag, unsigned short hightag)
{
	zhead_t *z, *next;
	unsigned short tag;

	if (hightag < lowtag)
		return;

	for (tag = lowtag; tag <= hightag; tag++)
	{
		for (z = z_chain.next; z != &z_chain; z = next)
		{
			next = z->next;

			if (z->tag == tag)
			{
				Z_Free((void *)(z + 1));
			}
		}
	}
}

void *
Z_TagMalloc(size_t size, unsigned short tag)
{
	zhead_t *z;

	if (!size || ((SIZE_MAX - size) < sizeof(zhead_t)))
	{
		Error("%s: bad allocation size: %zu", __func__, size);
		return NULL;
	}

	size = size + sizeof(zhead_t);
	z = calloc(1, size);

	if (!z)
	{
		Error("%s: failed to allocate %zu bytes", __func__, size);
		return NULL;
	}

	z_count++;
	z_bytes += size;
	z->magic = Z_MAGIC;
	z->tag = tag;
	z->size = size;

	z->next = z_chain.next;
	z->prev = &z_chain;
	z_chain.next->prev = z;
	z_chain.next = z;

	return (void *)(z + 1);
}

void *
Z_Malloc(size_t size)
{
	return Z_TagMalloc(size, 0);
}

void *
Z_TagRealloc(void *ptr, size_t size, unsigned short tag)
{
	zhead_t *z, *zr;

	if (!size || ((SIZE_MAX - size) < sizeof(zhead_t)))
	{
		Error("%s: bad allocation size: %zu", __func__, size);
		return NULL;
	}

	if (!ptr)
	{
		return Z_TagMalloc(size, tag);
	}

	z = (zhead_t *)ptr - 1;

	if (z->magic != Z_MAGIC)
	{
		Error("%s: not a valid memory block: %p", __func__, ptr);
		return NULL;
	}

	size = size + sizeof(zhead_t);
	zr = realloc(z, size);

	if (!zr)
	{
		Error("%s: failed to allocate %zu bytes", __func__, size);
		return NULL;
	}

	if (size > zr->size)
	{
		memset((byte *)zr + zr->size, 0, size - zr->size);
	}

	z_bytes -= zr->size;
	z_bytes += size;

	zr->tag = tag;
	zr->size = size;
	zr->prev->next = zr;
	zr->next->prev = zr;

	return zr + 1;
}

void *Z_LevelMalloc(int size, int tag, void *user)
{
	return Z_TagMalloc(size, tag);
}

void Z_ChangeTag(void *ptr, int tag)
{
	zhead_t *z;
	z = (zhead_t *)ptr - 1;

	if (z->magic != Z_MAGIC)
	{
		Error("%s: not a valid memory block: %p", __func__, ptr);
	}

	z->tag = tag;
}

int Z_HeapSize(void)
{
	return INT_MAX; /* FIXME */
}

int Z_UsedHeap(void)
{
	zhead_t *z, *next;
	int sz = 0;

	for (z = z_chain.next; z != &z_chain; z = next)
	{
		next = z->next;
		sz += z->size;
	}

	return sz;
}

int Z_UsedLevelHeap(void)
{
	zhead_t *z, *next;
	int sz = 0;

	for (z = z_chain.next; z != &z_chain; z = next)
	{
		next = z->next;
		if (z->tag >= PU_LEVELSTRUCT && z->tag <= PU_LEVELEND)
			sz += z->size;
	}

	return sz;
}

void *
Z_Realloc(void *ptr, size_t size)
{
	return Z_TagRealloc(ptr, size, 0);
}

char *
Z_StrDup(const char *s)
{
	char *ptr;
	size_t len = strlen(s) + 1;

	ptr = (char *)Z_Malloc(len + 1);

	ptr[len] = '\0';

	strncpy(ptr, s, len);

	return ptr;
}
