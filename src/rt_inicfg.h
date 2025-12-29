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

#ifndef INICFG_H_
#define INICFG_H_

#include "rt_def.h"

extern char iniPath[MAX_PATH];
extern boolean iniStarted;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

typedef struct opl
{
    int oplChipNum;
    int bankNum;
    int emulator;
} oplCfg;

typedef struct modeX
{
    unsigned int renderBackend;
    char* scaleMode;
} modeXCfg;

typedef struct snd
{
    int maxVoices;
    unsigned int sampleRate;
    int bitDepth;
    int channels;
} sndCfg;

boolean INI_WriteDefault(void);
void INI_Startup(void);
boolean INI_CheckError(void);
int OPL_FetchConfig(void* user, const char* section, 
                            const char* name, const char* value);
int ModeX_FetchConfig(void* user, const char* section, 
                            const char* name, const char* value);
int Sound_FetchConfig(void* user, const char* section, 
                            const char* name, const char* value);                                                

#endif