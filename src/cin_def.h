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
#ifndef _cin_def_public
#define _cin_def_public

#define MAXCINEMATICEVENTS 100
#define MAXCINEMATICACTORS 30

#define FRACTIONBITS (16)
#define FRACTIONUNIT (1 << FRACTIONBITS)

typedef enum
{
	background_noscrolling,
	background_scrolling,
	background_multi,
	sprite_background,
	sprite_foreground,
	backdrop_scrolling,
	backdrop_noscrolling,
	palette,
	flic,
	fadeout,
	cinematicend,
	blankscreen,
	clearbuffer
} e_cinefxevent_t;

typedef struct e_cinefxevent_node_t
{
	int time;
	e_cinefxevent_t effecttype;
	void* effect;
	struct e_cinefxevent_node_t* next;
	struct e_cinefxevent_node_t* prev;
} e_cinefxevent_node_t;

typedef struct cineactor_t
{
	e_cinefxevent_t effecttype;
	void* effect;
	struct cineactor_t* next;
	struct cineactor_t* prev;
} cineactor_t;

typedef struct
{
	char name[10];
	bool loop;
	bool usefile;
} cine_event;

typedef struct
{
	char name[10];
	int duration;
	int frame;
	int frametime;
	int numframes;
	int framedelay;
	int x;
	int y;
	int scale;
	int dx;
	int dy;
	int dscale;
} cinespr_event;

typedef struct
{
	char name[10];
	int duration;
	int backdropwidth;
	int currentoffset;
	int dx;
	int yoffset;
	int height;
	byte* data;
} cine_bgevent;

typedef struct
{
	char name[10];
} cine_palevent;

#endif
