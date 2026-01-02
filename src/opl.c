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

#include "SDL_mixer.h"

#include <adlmidi.h>
#include <ini.h>

#include "opl.h"
#include "rt_def.h"
#include "rt_util.h"
#include "rt_cfg.h"

static void OPLcallback(void *cbFunc, Uint8 *stream, int len);

static bool isPlaying = 0;
static bool isHooked = false;

extern bool useoplmusic;

static float volume = 0;

static struct ADLMIDI_AudioFormat s_audioFormat;

static Uint16                   obtained_format;
static struct ADL_MIDIPlayer    *midi_player = NULL;

static int OPL_FetchConfig(void* user, const char* section, 
                            const char* name, const char* value)
{
    oplCfg* cfg = (oplCfg*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if(MATCH("chip", "bank"))
    {
        cfg->bankNum = atoi(value);
    }
    else if(MATCH("chip", "count"))
    {
        cfg->oplChipNum = atoi(value);
    }
    else if(MATCH("chip", "emulator"))
    {
        cfg->emulator = atoi(value);
    }
    else
    {
        return 0;
    }
    return 1;
}

static bool OPL_WriteDefault(const char* path)
{
    FILE *ini = fopen(path, "w");
    
    if(ini == NULL)
    {
        return false;
    }

    // open chip section
    fputs("[chip]\n", ini);

    // kvp for bank
    fputs("; - notable banks - \n\n", ini);

    fputs("; 67 = ROTT v1.3\n", ini);
    fputs("; 70 = ROTT v1.0-1.2\n", ini);
    fputs("; 72 = DMXOPL (default)\n\n", ini);
    fputs("; for more: https://github.com/Wohlstand/libADLMIDI/blob/master/banks.ini\n", ini);

    fputs("bank=72\n\n", ini);

    // kvp for chip count
    fputs("; how many opl chips to emulate\n", ini);

    fputs("count=2\n\n", ini);

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

    fputs("emulator=2", ini);

    // clean up
    fclose(ini);

    return true;
}

void OPL_Init(void)
{
    char oplCfgPath[512];
    oplCfg cfg;

    GetPathFromEnvironment(oplCfgPath, ApogeePath, "opl.ini");

    if (access(oplCfgPath, F_OK) != 0)
    {
        printf("opl.ini doesn't exist!\n");

        if(!OPL_WriteDefault(oplCfgPath))
            printf("opl.ini creation failed.\n");
        else
            printf("opl.ini creation succeeded in %s.\n", ApogeePath);
    }
    
    if (ini_parse(oplCfgPath, OPL_FetchConfig, &cfg) < 0) 
    {
        printf("Can't load 'opl.ini'\n");
        exit(0);
    }

    midi_player = adl_init(44100);
    if (!midi_player)
    {
        fprintf(stderr, "Couldn't initialize ADLMIDI: %s\n", adl_errorString());
    }

    if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 2048) < 0)
    {
        fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
    }

    // get emulator count from ini - default to 2 (dosbox).
    int getEmu = cfg.emulator ? cfg.emulator : ADLMIDI_EMU_DOSBOX;
    adl_switchEmulator(midi_player, getEmu);

    // get chip count from ini - default to 2.
    int getChips = cfg.oplChipNum ? cfg.oplChipNum : 2;
    adl_setNumChips(midi_player, getChips);
    
    // banknum if set - default to DMXOPL
    // https://github.com/Wohlstand/libADLMIDI/blob/master/banks.ini
    int getBank = cfg.bankNum ? cfg.bankNum : 72;
    adl_setBank(midi_player, getBank);

    adl_setVolumeRangeModel(midi_player, ADLMIDI_VolumeModel_AUTO);

    Mix_QuerySpec(NULL, &obtained_format, NULL);

    switch(obtained_format)
    {
    case AUDIO_S8:
        s_audioFormat.type = ADLMIDI_SampleType_S8;
        s_audioFormat.containerSize = sizeof(int8_t);
        s_audioFormat.sampleOffset = sizeof(int8_t) * 2;
        break;
    case AUDIO_U8:
        s_audioFormat.type = ADLMIDI_SampleType_U8;
        s_audioFormat.containerSize = sizeof(uint8_t);
        s_audioFormat.sampleOffset = sizeof(uint8_t) * 2;
        break;
    case AUDIO_S16:
        s_audioFormat.type = ADLMIDI_SampleType_S16;
        s_audioFormat.containerSize = sizeof(int16_t);
        s_audioFormat.sampleOffset = sizeof(int16_t) * 2;
        break;
    case AUDIO_U16:
        s_audioFormat.type = ADLMIDI_SampleType_U16;
        s_audioFormat.containerSize = sizeof(uint16_t);
        s_audioFormat.sampleOffset = sizeof(uint16_t) * 2;
        break;
    case AUDIO_S32:
        s_audioFormat.type = ADLMIDI_SampleType_S32;
        s_audioFormat.containerSize = sizeof(int32_t);
        s_audioFormat.sampleOffset = sizeof(int32_t) * 2;
        break;
    case AUDIO_F32:
        s_audioFormat.type = ADLMIDI_SampleType_F32;
        s_audioFormat.containerSize = sizeof(float);
        s_audioFormat.sampleOffset = sizeof(float) * 2;
        break;
    }

    if(useoplmusic)
        OPL_RegisterHook();
}

void OPL_Free(void)
{
    adl_close(midi_player);
}


void OPL_RegisterHook(void)
{
    Mix_HookMusic(OPLcallback, midi_player);
    isHooked = true;
}

void OPL_DeregisterHook(void)
{
    Mix_HookMusic(NULL, NULL);
    isHooked = false;
}

void OPL_CheckForStateChange(void)
{
    if(!OPL_IsHooked() && useoplmusic)
	{
		OPL_RegisterHook();
	}
	if(OPL_IsHooked() && !useoplmusic)
	{
		OPL_DeregisterHook();
	}
}

// ! these seem to be broken, removing for now.
// int OPL_GetPosition(void)
// {
//     return (int)(adl_positionTell(midi_player) * 1000);
// }

// void OPL_SetPosition(int ms)
// {
//     adl_positionSeek(midi_player, (float)ms / 1000);
// }


void OPL_SetVolume(float newVol)
{
    // newvol/127 = normalize to 0.0-1.0
    // y = 10 * (newVol / 127) ^ 2
    // essentially, 0.0-10.0f scaled to square of volumescale
    float volumescale = newVol / 127;
    volume = 10 * pow(volumescale, 2);
}

int OPL_IsPlaying(void)
{
    return isPlaying;
}

int OPL_IsHooked(void)
{
    return isHooked;
}



bool OPL_Play(char* buffer, int size, int loopflag)
{
    if(adl_openData(midi_player, buffer, size) < 0)
    {
        fprintf(stderr, "Couldn't open music file: %s\n", adl_errorInfo(midi_player));

        return false;
    }

    adl_setLoopEnabled(midi_player, loopflag);

    isPlaying = true;
    SDL_PauseAudio(0);

    return true;
}


void OPL_Stop(void)
{
    isPlaying = false;
}

void OPL_Pause(void)
{
    isPlaying = false;
}


static void OPLcallback(void *cbFunc, Uint8 *stream, int len)
{
    if (!isPlaying)
      return;

    int samples_count = len / s_audioFormat.containerSize;

    samples_count = adl_playFormat(cbFunc, samples_count,
                                   stream,
                                   stream + s_audioFormat.containerSize,
                                   &s_audioFormat);

    // assuming signed 16-bit due to that being used for Mix_OpenAudio
    int16_t* sampleBuf = (int16_t*)stream;

    for(int i = 0; i < samples_count; i++)
    {
        float clampVal = (float)sampleBuf[i] * volume;

        clampVal = clampVal > INT16_MAX ? INT16_MAX : 
                   clampVal < INT16_MIN ? INT16_MIN 
                   : clampVal;
        
        sampleBuf[i] = (int16_t)clampVal;
    }

    if(samples_count <= 0)
    {
        isPlaying = false;
        SDL_memset(stream, 0, len);
    }
}
