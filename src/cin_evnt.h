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
#ifndef _cin_evnt_public
#define _cin_evnt_public

#include "cin_glob.h"
#include "cin_def.h"

extern e_cinefxevent_node_t* firstevent;
extern e_cinefxevent_node_t* lastevent;

void AddEvent(e_cinefxevent_node_t* event);
void DeleteEvent(e_cinefxevent_node_t* event);

e_cinefxevent_node_t* GetNewEvent(void);
void StartupEvents(void);
void ShutdownEvents(void);
e_cinefxevent_node_t* CreateEvent(int time, int type);
void ParseEvent(int time);
void UpdateCinematicEvents(int time);
void PrecacheCinematic(void);

#endif
