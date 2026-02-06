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
//                  RT_MAIN.H
//
//***************************************************************************

#ifndef _rt_main_public
#define _rt_main_public

#include "develop.h"
#include "rt_def.h"
#include "rottnet.h"
#include "rt_battl.h"

#define STANDARDGAMELEVELS		(DATADIR "DARKWAR.RTL")
#define STANDARDBATTLELEVELS	(DATADIR "DARKWAR.RTC")
#define SUPERROTTBATTLELEVELS	(DATADIR "ROTTCD.RTC")
#define SITELICENSEBATTLELEVELS (DATADIR "ROTTSITE.RTC")
#define SHAREWAREGAMELEVELS	 (DATADIR "HUNTBGIN.RTL")
#define SHAREWAREBATTLELEVELS (DATADIR "HUNTBGIN.RTC")

enum
{
	vl_low,
	vl_medium,
	vl_high,
	vl_excessive
};

// Enum for each version of the game
typedef enum
{
	ROTT_SHAREWARE,
	ROTT_REGISTERED,
	ROTT_SUPERCD,
	ROTT_SITELICENSE
} version_t;

typedef struct
{
	int GodModeTime;
	int DogModeTime;
	int ShroomsModeTime;
	int ElastoModeTime;
	int AsbestosVestTime;
	int BulletProofVestTime;
	int GasMaskTime;
	int MercuryModeTime;

	int GodModeRespawnTime;
	int DogModeRespawnTime;
	int ShroomsModeRespawnTime;
	int ElastoModeRespawnTime;
	int AsbestosVestRespawnTime;
	int BulletProofVestRespawnTime;
	int GasMaskRespawnTime;
	int MercuryModeRespawnTime;

} specials;

typedef struct
{
	unsigned Version;
	// Variable for which version of the game can be played
	version_t Product;

	int TimeCount;
	int frame;
	int secrettotal, treasuretotal, killtotal;
	int secretcount, treasurecount, killcount;
	int supertotal, healthtotal, missiletotal;
	int supercount, healthcount, missilecount;
	int democratictotal, planttotal;
	int democraticcount, plantcount;
	int dipballs;
	int difficulty;
	int violence;
	int mapon;
	int score;
	int episode;
	int battlemode;
	int battleoption;
	int randomseed;
	bool teamplay;
	bool DODEMOCRATICBONUS1;
	bool DOGROUNDZEROBONUS;
	int autorun;

	// Battle Options
	battle_cfg_t BattleOptions;

	bool SpawnCollectItems;
	bool SpawnEluder;
	bool SpawnDeluder;
	bool ShowScores;
	bool PlayerHasGun[MAXPLAYERS];
	specials SpecialsTimes;

} gametype;

extern int doublestep;
extern bool tedlevel;
extern int tedlevelnum;
extern int tedx;
extern int tedy;
extern bool fizzlein;
extern int pheight;
extern int NoSound;
extern int timelimit;
extern bool timelimitenabled;
extern bool noecho;
extern bool demoexit;
extern bool quiet;

extern gametype gamestate;
extern bool DebugOk;
extern bool newlevel;

#define IS_SHAREWARE (gamestate.Product == ROTT_SHAREWARE)
#define IS_NOT_SHAREWARE (!IS_SHAREWARE)

void QuitGame(void);
void PlayCinematic(void);
void InitCharacter(void);
void ShutDown(void);
void UpdateGameObjects(void);

extern int polltime;
extern int oldpolltime;
extern volatile int oldtime;
void PauseLoop(void);
#if SAVE_SCREEN
extern bool inhmenu;
void SaveScreen(bool saveLBM);
#endif
void SetupWads(void);

extern bool SCREENSHOTS;
extern bool COMPUTELEVELSIZE;
extern bool MONOPRESENT;
extern bool MAPSTATS;
extern bool TILESTATS;
extern bool HUD;

extern char CWD[40];

#endif
