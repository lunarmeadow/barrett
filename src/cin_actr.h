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
#ifndef _cin_actr_public
#define _cin_actr_public

#include "cin_glob.h"
#include "cin_def.h"

extern cineactor_t* firstcinematicactor;
extern cineactor_t* lastcinematicactor;

void AddCinematicActor(cineactor_t* actor);
void DeleteCinematicActor(cineactor_t* actor);

cineactor_t* GetNewCinematicActor(void);
void StartupCinematicActors(void);
void ShutdownCinematicActors(void);
void SpawnCinematicActor(e_cinefxevent_t type, void* effect);
void DrawCinematicActors(void);
void UpdateCinematicActors(void);

#endif
