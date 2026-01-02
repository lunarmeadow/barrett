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
#ifndef _cin_efct_public
#define _cin_efct_public

#include "cin_glob.h"
#include "cin_def.h"

cine_event* SpawnCinematicFlic(char* name, bool loop, bool usefile);
cinespr_event* SpawnCinematicSprite(char* name, int duration, int numframes,
								  int framedelay, int x, int y, int scale,
								  int endx, int endy, int endscale);
cine_bgevent* SpawnCinematicBack(char* name, int duration, int width, int startx,
							  int endx, int yoffset);

cine_bgevent* SpawnCinematicMultiBack(char* name, char* name2, int duration,
								   int startx, int endx, int yoffset);
cine_palevent* SpawnCinematicPalette(char* name);
void DrawFlic(cine_event* flic);
void DrawCinematicBackdrop(cine_bgevent* back);
void DrawCinematicBackground(cine_bgevent* back);
void DrawPalette(cine_palevent* event);
void DrawCinematicSprite(cinespr_event* sprite);
void DrawClearBuffer(void);
void DrawBlankScreen(void);
bool DrawCinematicEffect(e_cinefxevent_t type, void* effect);
bool UpdateCinematicBack(cine_bgevent* back);
bool UpdateCinematicSprite(cinespr_event* sprite);
bool UpdateCinematicEffect(e_cinefxevent_t type, void* effect);
void PrecacheCinematicEffect(e_cinefxevent_t type, void* effect);
void ProfileDisplay(void);
void DrawPostPic(int lumpnum);

#endif
