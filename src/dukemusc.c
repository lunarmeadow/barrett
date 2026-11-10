/*
 * A reimplementation of Jim Dose's FX_MAN routines, using  SDL_mixer 1.2.
 *   Whee. FX_MAN is also known as the "Apogee Sound System", or "ASS" for
 *   short. How strangely appropriate that seems.
 *
 * Written by Ryan C. Gordon. (icculus@clutteredmind.org)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#define ROTT

#ifdef DUKE3D
#include "duke3d.h"
#include "buildengine/cache1d.h"
#endif

#include "SDL.h"
#include "SDL_mixer.h"
#ifdef ROTT
#include "rt_def.h"	 // ROTT music hack
#include "rt_cfg.h"	 // ROTT music hack
#include "rt_util.h" // ROTT music hack
#endif
#include "music.h"

#include "opl.h"

#define __FX_TRUE  (1 == 1)
#define __FX_FALSE (!__FX_TRUE)

#define DUKESND_DEBUG "DUKESND_DEBUG"

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

int MUSIC_ErrorCode = MUSIC_Ok;

static char warningMessage[80];
static char errorMessage[80];
static FILE* debug_file = NULL;
static int initialized_debugging = 0;

static size_t music_songdatasize = 0;

extern char ApogeePath[256];

static char MUSIC_SoundFonts[2048];
static char soundfonts_fallback[] = "/usr/share/soundfonts/default.sf2;/usr/share/sounds/sf2/default-GM.sf2;/usr/share/sounds/sf2/FluidR3_GM.sf2";

// This gets called all over the place for information and debugging messages.
//  If the user set the DUKESND_DEBUG environment variable, the messages
//  go to the file that is specified in that variable. Otherwise, they
//  are ignored for the expense of the function call. If DUKESND_DEBUG is
//  set to "-" (without the quotes), then the output goes to stdout.
static void musdebug(const char* fmt, ...)
{
	va_list ap;

	if (debug_file)
	{
		fprintf(debug_file, "DUKEMUS: ");
		va_start(ap, fmt);
		vfprintf(debug_file, fmt, ap);
		va_end(ap);
		fprintf(debug_file, "\n");
		fflush(debug_file);
	} // if
} // musdebug

static void init_debugging(void)
{
	const char* envr;

	if (initialized_debugging)
		return;

	envr = getenv(DUKESND_DEBUG);
	if (envr != NULL)
	{
		if (strcmp(envr, "-") == 0)
			debug_file = stdout;
		else
			debug_file = fopen(envr, "w");

		if (debug_file == NULL)
			fprintf(stderr, "DUKESND: -WARNING- Could not open debug file!\n");
		else
			setbuf(debug_file, NULL);
	} // if

	initialized_debugging = 1;
} // init_debugging

static void setWarningMessage(const char* msg)
{
	strncpy(warningMessage, msg, sizeof(warningMessage));
	// strncpy() doesn't add the null char if there isn't room...
	warningMessage[sizeof(warningMessage) - 1] = '\0';
	musdebug("Warning message set to [%s].", warningMessage);
} // setErrorMessage

static void setErrorMessage(const char* msg)
{
	strncpy(errorMessage, msg, sizeof(errorMessage));
	// strncpy() doesn't add the null char if there isn't room...
	errorMessage[sizeof(errorMessage) - 1] = '\0';
	musdebug("Error message set to [%s].", errorMessage);
} // setErrorMessage

// The music functions...

char* MUSIC_ErrorString(int ErrorNumber)
{
	switch (ErrorNumber)
	{
	case MUSIC_Warning:
		return (warningMessage);

	case MUSIC_Error:
		return (errorMessage);

	case MUSIC_Ok:
		return ("OK; no error.");

	case MUSIC_ASSVersion:
		return ("Incorrect sound library version.");

	case MUSIC_SoundCardError:
		return ("General sound card error.");

	case MUSIC_InvalidCard:
		return ("Invalid sound card.");

	case MUSIC_MidiError:
		return ("MIDI error.");

	case MUSIC_MPU401Error:
		return ("MPU401 error.");

	case MUSIC_TaskManError:
		return ("Task Manager error.");

	case MUSIC_FMNotDetected:
		return ("FM not detected error.");

	case MUSIC_DPMI_Error:
		return ("DPMI error.");

	default:
		return ("Unknown error.");
	} // switch

	assert(0); // shouldn't hit this point.
	return (NULL);
} // MUSIC_ErrorString

static int music_initialized = 0;
static int music_context = 0;
static int music_loopflag = MUSIC_PlayOnce;
static char* music_songdata = NULL;
static Mix_Music* music_musicchunk = NULL;

extern bool useoplmusic;

int MUSIC_Init(void)
{
	init_debugging();

	char localsf2[256];

	GetPathFromEnvironment(localsf2, ApogeePath, "barrett.sf2");

	if (music_initialized)
	{
		setErrorMessage("Music system is already initialized.");
		return (MUSIC_Error);
	} // if

	// add soundfonts
    const char *soundfonts = Mix_GetSoundFonts();

	SDL_SetHint("SDL_HINT_AUDIO_RESAMPLING_MODE", "best");

	OPL_Init();

    if (soundfonts)
    {
        snprintf(MUSIC_SoundFonts, sizeof(MUSIC_SoundFonts), "%s", soundfonts);
    }
    else
    {
        snprintf(MUSIC_SoundFonts, sizeof(MUSIC_SoundFonts), "%s;%s", soundfonts_fallback, localsf2);
    }

    printf("Using soundfonts path string \"%s\"\n", MUSIC_SoundFonts);
    Mix_SetSoundFonts(MUSIC_SoundFonts);

	music_initialized = 1;
	return (MUSIC_Ok);
} // MUSIC_Init

int MUSIC_Shutdown(void)
{
	musdebug("shutting down sound subsystem.");

	if (!music_initialized)
	{
		setErrorMessage("Music system is not currently initialized.");
		return (MUSIC_Error);
	} // if

	MUSIC_StopSong();
	music_context = 0;
	music_initialized = 0;
	music_loopflag = MUSIC_PlayOnce;

	return (MUSIC_Ok);
} // MUSIC_Shutdown

void MUSIC_SetMaxFMMidiChannel(int channel)
{
	musdebug("STUB ... MUSIC_SetMaxFMMidiChannel(%d).\n", channel);
} // MUSIC_SetMaxFMMidiChannel

void MUSIC_SetVolume(int volume)
{
	if(useoplmusic && OPL_IsHooked())
		OPL_SetVolume(volume >> 1);
	else
		Mix_VolumeMusic(volume >> 1); // convert 0-255 to 0-128.
} // MUSIC_SetVolume

void MUSIC_SetMidiChannelVolume(int channel, int volume)
{
	musdebug("STUB ... MUSIC_SetMidiChannelVolume(%d, %d).\n", channel, volume);
} // MUSIC_SetMidiChannelVolume

void MUSIC_ResetMidiChannelVolumes(void)
{
	musdebug("STUB ... MUSIC_ResetMidiChannelVolumes().\n");
} // MUSIC_ResetMidiChannelVolumes

int MUSIC_GetVolume(void)
{
	return (Mix_VolumeMusic(-1) << 1); // convert 0-128 to 0-255.
} // MUSIC_GetVolume

void MUSIC_SetLoopFlag(int loopflag)
{
	music_loopflag = loopflag;
} // MUSIC_SetLoopFlag

int MUSIC_SongPlaying(void)
{
	if(useoplmusic && OPL_IsHooked())
		return OPL_IsPlaying();
	else
		return ((Mix_PlayingMusic()) ? __FX_TRUE : __FX_FALSE);
} // MUSIC_SongPlaying

void MUSIC_Continue(void)
{
	if (Mix_PausedMusic())
		Mix_ResumeMusic();
	else if (music_songdata)
		MUSIC_PlaySong(music_songdata, music_songdatasize, MUSIC_PlayOnce);
} // MUSIC_Continue

void MUSIC_Pause(void)
{
	Mix_PauseMusic();
} // MUSIC_Pause

int MUSIC_StopSong(void)
{
	// if (!fx_initialized)
	if (!Mix_QuerySpec(NULL, NULL, NULL))
	{
		setErrorMessage("Need FX system initialized, too. Sorry.");
		return (MUSIC_Error);
	} // if

	if(useoplmusic)
		OPL_Stop();

	if ((Mix_PlayingMusic()) || (Mix_PausedMusic()))
		Mix_HaltMusic();

	if (music_musicchunk)
		Mix_FreeMusic(music_musicchunk);

	music_songdata = NULL;
	music_musicchunk = NULL;

	return (MUSIC_Ok);
} // MUSIC_StopSong

int MUSIC_PlaySong(char* song, int size, int loopflag)
{
	OPL_CheckForStateChange();

	if(useoplmusic)
	{
		if(!OPL_Play(song, size, loopflag))
		{	
			OPL_DeregisterHook();
			goto oplskip;
		}
		else
		{
			OPL_RegisterHook();
			return (MUSIC_Ok);
		}
	}
	else
	{
oplskip:
		MUSIC_StopSong();

		if (size < 1) {
			return MUSIC_Error;
		}

		SDL_RWops *rw = SDL_RWFromConstMem(song, size);
		if (rw == NULL) 
		{
			return MUSIC_Error;
		}

		music_musicchunk = Mix_LoadMUS_RW(rw, SDL_TRUE);

		music_songdata = song;
		music_songdatasize = size;

		Mix_PlayMusic(music_musicchunk, (loopflag == MUSIC_PlayOnce) ? 0 : -1);
	}	
	return (MUSIC_Ok);
} // MUSIC_PlaySong

void MUSIC_SetContext(int context)
{
	musdebug("STUB ... MUSIC_SetContext().\n");
	music_context = context;
} // MUSIC_SetContext

int MUSIC_GetContext(void)
{
	return (music_context);
} // MUSIC_GetContext

void MUSIC_SetSongTick(unsigned long PositionInTicks)
{
	musdebug("STUB ... MUSIC_SetSongTick().\n");
} // MUSIC_SetSongTick

void MUSIC_SetSongTime(unsigned long milliseconds)
{
	musdebug("STUB ... MUSIC_SetSongTime().\n");
} // MUSIC_SetSongTime

void MUSIC_SetSongPosition(int measure, int beat, int tick)
{
	musdebug("STUB ... MUSIC_SetSongPosition().\n");
} // MUSIC_SetSongPosition

void MUSIC_GetSongPosition(songtic* pos)
{
	musdebug("STUB ... MUSIC_GetSongPosition().\n");
} // MUSIC_GetSongPosition

void MUSIC_GetSongLength(songtic* pos)
{
	musdebug("STUB ... MUSIC_GetSongLength().\n");
} // MUSIC_GetSongLength

int MUSIC_FadeVolume(int tovolume, int milliseconds)
{
	Mix_FadeOutMusic(milliseconds);
	return (MUSIC_Ok);
} // MUSIC_FadeVolume

int MUSIC_FadeActive(void)
{
	return ((Mix_FadingMusic() == MIX_FADING_OUT) ? __FX_TRUE : __FX_FALSE);
} // MUSIC_FadeActive

void MUSIC_StopFade(void)
{
	musdebug("STUB ... MUSIC_StopFade().\n");
} // MUSIC_StopFade

void MUSIC_RerouteMidiChannel(int channel,
							  int (*function)(int event, int c1, int c2))
{
	musdebug("STUB ... MUSIC_RerouteMidiChannel().\n");
} // MUSIC_RerouteMidiChannel

void MUSIC_RegisterTimbreBank(unsigned char* timbres)
{
	musdebug("STUB ... MUSIC_RegisterTimbreBank().\n");
} // MUSIC_RegisterTimbreBank

// end of fx_man.c ...
