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

#include "rt_def.h"

#include <ini.h>

#include "rt_util.h"
#include "rt_cfg.h"

#include "rt_inicfg.h"

char iniPath[MAX_PATH];
boolean iniStarted = false;

int OPL_FetchConfig(void* user, const char* section, 
                            const char* name, const char* value)
{
    oplCfg* cfg = (oplCfg*)user;
    
    if(MATCH("OPL", "Bank"))
    {
        cfg->bankNum = atoi(value);
    }
    else if(MATCH("OPL", "Count"))
    {
        cfg->oplChipNum = atoi(value);
    }
    else if(MATCH("OPL", "Emulator"))
    {
        cfg->emulator = atoi(value);
    }
    else
    {
        return 0;
    }
    return 1;
}

int ModeX_FetchConfig(void* user, const char* section, 
                            const char* name, const char* value)
{
    modeXCfg* cfg = (modeXCfg*)user;

    if(MATCH("ModeX", "Backend"))
    {
        cfg->renderBackend = atoi(value);
    }
    if(MATCH("ModeX", "ScaleMode"))
    {
        cfg->scaleMode = strdup(value);
    }
    else
    {
        return 0;
    }
    return 1;
}

int Sound_FetchConfig(void* user, const char* section, 
                            const char* name, const char* value)
{
    sndCfg* cfg = (sndCfg*)user;

    if(MATCH("Audio", "MaxVoices"))
    {
        cfg->maxVoices = atoi(value);
    }
    if(MATCH("Audio", "SampleRate"))
    {
        cfg->sampleRate = atoi(value);
    }
    if(MATCH("Audio", "BitDepth"))
    {
        cfg->bitDepth = atoi(value);
    }
    if(MATCH("Audio", "Channels"))
    {
        cfg->channels = atoi(value);
    }
    else
    {
        return 0;
    }
    return 1;
}


void INI_Startup(void)
{
    GetPathFromEnvironment(iniPath, ApogeePath, "barrett.ini");

    if (access(iniPath, F_OK) != 0)
    {
        printf("barrett.ini doesn't exist!\n");

        if(!INI_WriteDefault())
            printf("barrett.ini creation failed.\n");
        else
            printf("barrett.ini creation succeeded in %s.\n", ApogeePath);
    }

    iniStarted = true;
}

boolean INI_CheckError(void)
{
    if(*iniPath == '\0')
    {
        printf("INI_CheckError: INI not found!\n");
        return false;
    }

    if(access(iniPath, F_OK) != 0)
    {
        printf("INI_CheckError: INI can't be accessed!\n");
        return false;
    }

    if(!iniStarted)
    {
        printf("INI_CheckError: INI subsystem not started!\n");
        return false;
    }

    return true;
}

boolean INI_WriteDefault(void)
{
    FILE *ini = fopen(iniPath, "w");
    
    if(ini == NULL)
    {
        return false;
    }

    // ! OPL SETTINGS !

    fputs("[OPL]\n", ini);

    // kvp for bank
    fputs("; - notable banks - \n\n", ini);

    fputs("; 67 = ROTT v1.3\n", ini);
    fputs("; 70 = ROTT v1.0-1.2\n", ini);
    fputs("; 72 = DMXOPL (default)\n\n", ini);
    fputs("; for more: https://github.com/Wohlstand/libADLMIDI/blob/master/banks.ini\n", ini);

    fputs("Bank=72\n\n", ini);

    // kvp for chip count
    fputs("; how many opl chips to emulate\n", ini);

    fputs("Count=2\n\n", ini);

    // kvp for OPL emulator
    fputs("; opl emulator choices: \n\n", ini);

    fputs("; 0 = Nuked\n", ini);
    fputs("; 1 = Nuked v1.7.4\n", ini);
    fputs("; 2 = DOSBox (default)\n", ini);
    fputs("; 3 = Opal\n", ini);
    fputs("; 4 = Java\n", ini);
    fputs("; 5 = ESFMu\n", ini);
    fputs("; 6 = MAME OPL2\n", ini);
    fputs("; 7 = YMFM OPL2\n", ini);
    fputs("; 8 = YMFM OPL3\n", ini);
    fputs("; 9 = Nuked OPL2 LLE\n", ini);
    fputs("; 10 = Nuked OPL3 LLE\n", ini);

    fputs("Emulator=2", ini);

    // ! MODEX SETTINGS !

    fputs("\n\n[ModeX]\n", ini);
    fputs("; backend is a bit field containing guarantees for how renderer is created. Simply add the numbers for your desired selection\n", ini);
    fputs("; 1 = Software\n", ini);
    fputs("; 2 = Accelerated (default)\n", ini);
    fputs("; 4 = Present w/ VSync\n", ini);
    fputs("; 8 = Use Target Texture\n", ini);
    fputs("Backend=2\n\n", ini);

    fputs("; this controls how SDL scales window content\n", ini);
    fputs("; options: \n", ini);
    fputs("; nearest (default)\n", ini);
    fputs("; linear\n", ini);
    fputs("; best\n", ini);
    fputs("ScaleMode=nearest\n\n", ini);

    // ! Audio SETTINGS !

    fputs("[Audio]\n", ini);
    fputs("; how many in-game sounds to mix\n", ini);
    fputs("MaxVoices=64\n", ini);

    fputs("\n; sample format\n", ini);
    fputs("SampleRate=44100\n", ini);
    fputs("Channels=2\n", ini);
    fputs("BitDepth=16\n", ini);

    // clean up
    fclose(ini);

    return true;
}