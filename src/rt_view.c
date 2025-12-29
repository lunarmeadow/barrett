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
#include "rt_view.h"
#include "z_zone.h"
#include "w_wad.h"
#include "lumpy.h"
#include "rt_util.h"
#include "rt_vid.h"
#include "rt_game.h"
#include "rt_draw.h"
#include "rt_ted.h"
#include "isr.h"
#include "rt_rand.h"
#include "rt_sound.h"
#include "modexlib.h"
#include "rt_menu.h"

#include <stdlib.h>

#include "rt_main.h"
#include "rt_battl.h"
#include "rt_floor.h"
#include "rt_str.h"
#include "rt_fixed.h"
#include "develop.h"

#define LIGHTNINGLEVEL	  4
#define MINLIGHTNINGLEVEL 2
#define MAXLIGHTNINGLEVEL 10

/*
=============================================================================

							   GLOBALS

=============================================================================
*/

int StatusBar = 0;
int lightninglevel = 0;
boolean lightning = false;
int normalshade;
int darknesslevel;
int maxshade;
int minshade;
int baseminshade;
int basemaxshade;
int viewheight;
int viewwidth;
longword heightnumerator;
fixed scale;
int screenofs;
int centerx;
int centery;
int centeryfrac;
int fulllight;
int weaponscale;
int viewsize;
byte* colormap;
byte* blckmap;
byte* redmap;
byte* greenmap;
byte* playermaps[MAXPLAYERCOLORS];
// short  pixelangle[MAXVIEWWIDTH];
short pixelangle[MAXSCREENWIDTH];
byte gammatable[GAMMAENTRIES];
int gammaindex;
int focalwidth = 160;
int yzangleconverter;
byte uniformcolors[MAXPLAYERCOLORS] = {25,	222, 29, 206, 52, 6,
									   155, 16,	 90, 129, 109};

/*
=============================================================================

							   LOCAL

=============================================================================
*/

static char* YourComputerSucksString = "Buy a 486! :)";

static int viewsizes[MAXVIEWSIZES * 2] = {
	80,	 48,  128, 72,	160, 88,  192, 104, 224, 120, 256,
	136, 288, 152, 320, 168, 320, 184, 320, 200, 320, 200};

static int ColorMapLoaded = 0;

static int lightningtime = 0;
static int lightningdelta = 0;
static int lightningdistance = 0;
static int lightningsoundtime = 0;
static boolean periodic = false;
static int periodictime = 0;

void SetViewDelta(void);
void UpdatePeriodicLighting(void);

/*
====================
=
= ResetFocalWidth
=
====================
*/
void ResetFocalWidth(void)
{
	focalwidth = iGLOBAL_FOCALWIDTH; // FOCALWIDTH;
	SetViewDelta();
}

/*
====================
=
= ChangeFocalWidth
=
====================
*/
void ChangeFocalWidth(int amount)
{
	focalwidth = iGLOBAL_FOCALWIDTH + amount; // FOCALWIDTH+amount;
	SetViewDelta();
}

/*
====================
=
= SetViewDelta
=
====================
*/

void SetViewDelta(void)
{
	// iGLOBAL_SCREENHEIGHT
	// iGLOBAL_SCREENWIDTH
	//
	//  calculate scale value for vertical height calculations
	//  and sprite x calculations
	//

	scale = (centerx * focalwidth) / (160);
	//
	// divide heightnumerator by a posts distance to get the posts height for
	// the heightbuffer.  The pixel height is height>>HEIGHTFRACTION
	//
	heightnumerator = ((int)(((double)focalwidth / 10) * centerx * 4096) * 64);
}

/*
====================
=
= CalcProjection
=
====================
*/

void CalcProjection(void)
{
	int i;
	int frac;
	int intang;
	byte* table;
	byte* ptr;
	int length;
	int* pangle;

	// Already being called in ResetFocalWidth
	//    SetViewDelta();

	//
	// load in tables file
	//

	// Hey, isn't this stuff already loaded in?
	// Why don't we make this a lump?
	table = W_CacheLumpName("tables", PU_STATIC, CvtNull, 1);
	ptr = table;

	//
	// get size of table
	//

	memcpy(&length, ptr, sizeof(int));
	SwapIntelLong(&length);
	ptr += sizeof(int);
	pangle = SafeMalloc(length * sizeof(int));
	memcpy(pangle, ptr, length * sizeof(int));

	frac = ((length * 65536 / centerx)) >> 1;
	for (i = 0; i < centerx; i++)
	{
		// start 1/2 pixel over, so viewangle bisects two middle pixels
		intang = pangle[frac >> 16];
		SwapIntelLong(&intang);
		pixelangle[centerx - 1 - i] = (short)intang;
		pixelangle[centerx + i] = (short)-intang;
		frac += (length * 65536 / centerx);
	}
	table = W_CacheLumpName("tables", PU_CACHE, CvtNull, 1);
	SafeFree(pangle);
}

/*
==========================
=
= SetViewSize
=
==========================
*/

extern int hudRescaleFactor;

void SetViewSize(int size)

{
	int height;
	int maxheight;
	int screenx;
	int screeny;
	int topy;
	/*
	if (size>=10){

	   SetTextMode (  );
	   screenx=screenx;
	   return;
	}
	*/

	height = 0;
	for (height = 0; height < 21;)
	{
		viewsizes[height++] = iGLOBAL_SCREENWIDTH;
		viewsizes[height++] = iGLOBAL_SCREENHEIGHT;
	}

	if ((size < 0) || (size >= MAXVIEWSIZES))
	{ // bna added
		printf("Illegal screen size = %d\n", size);
		size = 8; // set default value
		viewsize = 8;
	}

	// if ((size<0) || (size>=MAXVIEWSIZES))
	//    Error("Illegal screen size = %ld\n",size);

	viewwidth = viewsizes[size << 1];		 // must be divisable by 16
	viewheight = viewsizes[(size << 1) + 1]; // must be even

	maxheight = iGLOBAL_SCREENHEIGHT;
	topy = 0;

	// Only keep the kills flag
	StatusBar &= ~(BOTTOM_STATUS_BAR | TOP_STATUS_BAR | STATUS_PLAYER_STATS);

	if (SHOW_KILLS())
	{
		// Account for height of kill boxes
		maxheight -= 24;
	}

	if (size < 9)
	{
		StatusBar |= TOP_STATUS_BAR;

		// Account for height of top status bar
		maxheight -= 16 * hudRescaleFactor;
		topy += 16 * hudRescaleFactor;
	}

	//   if ( size == 7 ){maxheight -= 16;}//bna++
	//   if ( size <= 6 ){topy -= 8;}//bna++

	if (size < 8)
	{
		// Turn on health and ammo bar
		StatusBar |= BOTTOM_STATUS_BAR;

		maxheight -= 16 * hudRescaleFactor;
	}
	else if (size < 10)
	{
		// Turn on transparent health and ammo bar
		StatusBar |= STATUS_PLAYER_STATS;
	}
	//   SetTextMode (  );
	//   viewheight=viewheight;
	height = viewheight;
	if (height > 168 * iGLOBAL_SCREENHEIGHT / 200)
	{
		// Prevent weapon from being scaled too big
		height = 168 * iGLOBAL_SCREENHEIGHT / 200;
	}

	weaponscale = (height << 16) / 168; //( height << 16 ) = 170 * 65536

	centerx = viewwidth >> 1;
	centery = viewheight >> 1;
	centeryfrac = (centery << 16);

	// 0xE82A = tan(30deg) * (65536 / 2*pi)
	// scale it by ratio of focal width to default
	// 320 seems to work better than 200, 300, and 360. can't find a better value atm.
	// this could probably be further refined.
	yzangleconverter = (int)((0xA000 * ((double)focalwidth / 160)) * ((double)viewheight / 200));

	// Center the view horizontally
	screenx = (iGLOBAL_SCREENWIDTH - viewwidth) >> 1;

	if (viewheight >= maxheight)
	{
		screeny = topy;
		viewheight = maxheight;
	}
	else
	{
		// Center the view vertically
		screeny = ((maxheight - viewheight) >> 1) + topy;
	}

	// Calculate offset of view window
	screenofs = screenx + ylookup[screeny];

	//
	// calculate trace angles and projection constants
	//

	ResetFocalWidth();

	// Already being called in ResetFocalWidth
	//   SetViewDelta();

	CalcProjection();
}

//******************************************************************************
//
// DrawCPUJape ()
//
//******************************************************************************

void DrawCPUJape(void)

{
	int width;
	int height;

	CurrentFont = tinyfont;
	VW_MeasurePropString(YourComputerSucksString, &width, &height);

	DrawGameString(160 - width / 2, 100 + 48 / 2 + 2, YourComputerSucksString,
				   true);
}

/*
==========================
=
= SetupScreen
=
==========================
*/

void SetupScreen(boolean flip)
{
	pic_t* shape;

	SetViewSize(viewsize);

	if (viewsize <= 7)
	{
		shape = (pic_t*)W_CacheLumpName("backtile", PU_CACHE, Cvt_pic_t, 1);
		// DrawTiledRegion( 0, 16, 320, 200 - 32, 0, 16, shape );
		DrawTiledRegion(0, 16 * hudRescaleFactor, iGLOBAL_SCREENWIDTH,
						iGLOBAL_SCREENHEIGHT - 16 * hudRescaleFactor, 0, 16,
						shape); // bna++
	}

	if (viewsize == 0)
	{
		DrawCPUJape();
	}
	if (flip == true)
	{
		boolean wasStretched = false;

		if (StretchScreen)
		{
			wasStretched = true;
		}
		DisableScreenStretch();
		DrawPlayScreen(true);
		ThreeDRefresh();
		if (wasStretched == true)
		{
			EnableScreenStretch();
		}
	}
}

// ashley added: this table remaps blue palette indexes to another palette index for weapon recolours.
// blue palette is 26 entries long, thus the length
byte bluRemapper[26] = {35, 16, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 35, 36, 36, 0};

void LoadColorMap(void)
{
	int i, j;
	int lump, length;
	byte colour;

	if (ColorMapLoaded == 1)
		Error("Called LoadColorMap twice\n");
	ColorMapLoaded = 1;
	//
	//   load in the light tables
	//   256 byte align tables
	//

	lump = W_GetNumForName("colormap");
	length = W_LumpLength(lump) + 255;
	colormap = SafeMalloc(length);

	// this pattern is pretty suspect, after evaluating what this operation does it appears to align pointers to the nearest 256 boundary.
	// for instance, (925 & ~0xFF) = 768. the cast itself is pretty strange,
	// and I worry is shifting colormap outside of its allocated boundary or corrupting the pointer.
	// Windows builds crash in this function in the W_ReadLump call,
	// and investigating with -fsanitize=undefined after removing this line shows no unaligned accesses or anything of the sort
	// in addition to colormap still being perfectly functional.

	// colormap = (byte*)(((long)colormap + 255) & ~0xff);

	W_ReadLump(lump, colormap);

	// Fix fire colors in colormap

	for (i = 31; i >= 16; i--)
		for (j = 0xea; j < 0xf9; j++)
			colormap[i * 256 + j] = colormap[(((i - 16) / 4 + 16)) * 256 + j];

	// -- ashley added: get black colourmap for weapon recolouring

	blckmap = SafeMalloc(length);

	// first and last blu palette entries in PAL
	int firstblu = 142;
	int lastblu = 168;

	int blusize = lastblu - firstblu;

	for(int idx = 0; idx < length; idx++)
	{
		colour = colormap[idx];

		// copy colormap palette index without remapping for all non-blu colours
		if(colour < firstblu || colour > lastblu)
		{
			blckmap[idx] = colormap[idx];
		}

		// remappin time (colour is blue)
		if(colour >= firstblu && colour <= lastblu)
		{
			// grab wrapped index into blu palette table (colour - firstblu). index is relative to base pointer of blue table
			// modulo by size to wrap relative pointer back into table in case of overflows.
			colour = bluRemapper[(colour - firstblu) % blusize];
			
			blckmap[idx] = colour;			
		}
	}

	// -- ashley end

	// Get special maps

	lump = W_GetNumForName("specmaps");
	length = W_LumpLength(lump + 1) + 255;
	redmap = SafeMalloc(length);
	// redmap = (byte*)(((long)redmap + 255) & ~0xff);
	W_ReadLump(lump + 1, redmap);
	greenmap = redmap + (16 * 256);

	// Get player colormaps

	//   if (modemgame==true)
	{
		lump = W_GetNumForName("playmaps") + 1;
		for (i = 0; i < MAXPLAYERCOLORS; i++)
		{
			length = W_LumpLength(lump + i) + 255;
			playermaps[i] = SafeMalloc(length);
			// playermaps[i] = (byte*)(((long)playermaps[i] + 255) & ~0xff);
			W_ReadLump(lump + i, playermaps[i]);
		}
	}

	if (!quiet)
		printf("RT_VIEW: Colormaps Initialized\n");
}

/*
==========================
=
= SetupLightLevels
=
==========================
*/
#define LIGHTRATEBASE  252
#define LIGHTRATEEND   267
#define LIGHTLEVELBASE 216
#define LIGHTLEVELEND  223
void SetupLightLevels(void)
{
	int glevel;

	periodic = false;
	fog = 0;
	lightsource = 0;

	// Set up light level for level

	if (((word)MAPSPOT(2, 0, 1) >= 104) && ((word)MAPSPOT(2, 0, 1) <= 105))
		fog = (word)MAPSPOT(2, 0, 1) - 104;
	else
		Error("There is no Fog icon on map %d\n", gamestate.mapon);
	if ((word)MAPSPOT(3, 0, 1) == 139)
	{
		if (fog == 0)
		{
			lightsource = 1;
			lights = Z_Malloc(MAPSIZE * MAPSIZE * (sizeof(unsigned long)),
							  PU_LEVEL, NULL);
			memset(lights, 0, MAPSIZE * MAPSIZE * (sizeof(unsigned long)));
		}
		else
			Error(
				"You cannot use light sourcing on a level with fog on map %d\n",
				gamestate.mapon);
	}
	else if ((word)MAPSPOT(3, 0, 1))
		Error("You must use the lightsource icon or nothing at all at (3,0) in "
			  "plane 1 on map %d\n",
			  gamestate.mapon);
	if (((word)MAPSPOT(2, 0, 0) >= LIGHTLEVELBASE) &&
		((word)MAPSPOT(2, 0, 0) <= LIGHTLEVELEND))
		glevel = (MAPSPOT(2, 0, 0) - LIGHTLEVELBASE);
	else
		Error(
			"You must specify a valid darkness level icon at (2,0) on map %d\n",
			gamestate.mapon);

	SetLightLevels(glevel);

	if (((word)MAPSPOT(3, 0, 0) >= LIGHTRATEBASE) &&
		((word)MAPSPOT(3, 0, 0) <= LIGHTRATEEND))
		glevel = (MAPSPOT(3, 0, 0) - LIGHTRATEBASE);
	else
	{
		//      Error("You must specify a valid darkness rate icon at (3,0) on
		//      map %ld\n",gamestate.mapon);
		glevel = 4;
	}

	SetLightRate(glevel);
	lightningtime = 0;
	lightningdistance = 0;
	lightninglevel = 0;
	lightningdelta = 0;
	lightningsoundtime = 0;
}

/*
==========================
=
= SetLightLevels
=
==========================
*/
void SetLightLevels(int darkness)
{
	if (fog == 0)
	{
		baseminshade = 0x10 + ((7 - darkness) >> 1);
		basemaxshade = 0x1f - (darkness >> 1);
	}
	else
	{
		baseminshade = darkness;
		basemaxshade = 0x10;
	}
	minshade = baseminshade;
	maxshade = basemaxshade;
	darknesslevel = darkness;
}

/*
==========================
=
= GetLightLevelTile
=
==========================
*/
int GetLightLevelTile(void)
{
	if (fog == 0)
	{
		return ((7 - ((baseminshade - 0x10) << 1)) + LIGHTLEVELBASE);
	}
	else
	{
		return (baseminshade + LIGHTLEVELBASE);
	}
}

/*
==========================
=
= SetLightRate
=
==========================
*/
void SetLightRate(int rate)
{
	normalshade = (HEIGHTFRACTION + 8) - rate;
	if (normalshade > 14)
		normalshade = 14;
	if (normalshade < 3)
		normalshade = 3;
}

/*
==========================
=
= GetLightRate
=
==========================
*/
int GetLightRate(void)
{
	return ((HEIGHTFRACTION + 8) - normalshade);
}

/*
==========================
=
= GetLightRateTile
=
==========================
*/
int GetLightRateTile(void)
{
	return ((HEIGHTFRACTION + 8) - normalshade + LIGHTRATEBASE);
}

/*
==========================
=
= UpdateLightLevel
=
==========================
*/
void UpdateLightLevel(int area)
{
	int numlights;
	int targetmin;
	int targetmax;
	int numtiles;

	if (fog == true)
		return;

	numtiles = (numareatiles[area] >> 5) - 2;
	numlights = (LightsInArea[area] - numtiles) >> 1;

	if (numlights < 0)
		numlights = 0;
	if (numlights > GENERALNUMLIGHTS)
		numlights = GENERALNUMLIGHTS;
	targetmin = baseminshade + (GENERALNUMLIGHTS - numlights);
	targetmax = basemaxshade - numlights;
	if (targetmax < baseminshade)
		targetmax = baseminshade;
	if (targetmin > targetmax)
		targetmin = targetmax;

	if (minshade > targetmin)
		minshade -= 1;
	else if (minshade < targetmin)
		minshade += 1;

	if (maxshade > targetmax)
		maxshade -= 1;
	else if (maxshade < targetmax)
		maxshade += 1;
}

/*
==========================
=
= SetIllumination
=
= Postive value lightens
= Negative value darkens
=
==========================
*/
void SetIllumination(int level)
{
	if (fog)
		return;
	maxshade -= level;
	if (maxshade > 31)
		maxshade = 31;
	if (maxshade < 0x10)
		maxshade = 0x10;
	minshade -= level;
	if (minshade < 0x10)
		minshade = 0x10;
	if (minshade > 31)
		minshade = 31;
}

/*
==========================
=
= GetIlluminationDelta
=
==========================
*/
int GetIlluminationDelta(void)
{
	if (fog)
		return 0;
	else
		return maxshade - basemaxshade;
}

/*
==========================
=
= UpdateLightning
=
==========================
*/
void UpdateLightning(void)
{
	if (periodic == true)
	{
		UpdatePeriodicLighting();
		return;
	}
	if ((fog == 1) || (lightning == false))
		return;

	if (lightningtime <= 0)
	{
		if (lightningsoundtime > 0)
			SD_Play3D(SD_LIGHTNINGSND, 0, lightningdistance);
		lightningtime = GameRandomNumber("UpdateLightning", 0) << 1;
		lightningdistance = GameRandomNumber("UpdateLightning", 0);
		lightninglevel = (255 - lightningdistance) >> LIGHTNINGLEVEL;
		if (lightninglevel < MINLIGHTNINGLEVEL)
			lightninglevel = MINLIGHTNINGLEVEL;
		if (lightninglevel > MAXLIGHTNINGLEVEL)
			lightninglevel = MAXLIGHTNINGLEVEL;
		lightningdelta = lightninglevel >> 1;
		lightningsoundtime = lightningdistance >> 1;
		if (lightningdistance < 100)
		{
			SetIllumination(lightninglevel);
		}
	}
	else if (lightninglevel > 0)
	{
		lightninglevel -= lightningdelta;
		if (lightninglevel <= 0)
		{
			lightninglevel = 0;
		}
	}
	else
		lightningtime--;
	if (lightningsoundtime)
	{
		lightningsoundtime--;
		if (lightningsoundtime <= 0)
		{
			int volume;

			volume = 255 - lightningdistance;
			if (volume > 170)
				volume = 170;
			SD_PlayPitchedSound(SD_LIGHTNINGSND, volume,
								-(lightningdistance << 2));
			lightningsoundtime = 0;
		}
	}
}

/*
==========================
=
= UpdatePeriodicLighting
=
==========================
*/
#define PERIODICMAG	 (6)
#define PERIODICSTEP (20)
#define PERIODICBASE (0x0f)
void UpdatePeriodicLighting(void)
{
	int val;

	val = FixedMul(PERIODICMAG, sintable[periodictime]);
	periodictime = (periodictime + PERIODICSTEP) & (FINEANGLES - 1);
	basemaxshade = PERIODICBASE + (PERIODICMAG) + val;
	baseminshade = basemaxshade - GENERALNUMLIGHTS - 1;
}

/*
==========================
=
= SetModemLightLevel
=
==========================
*/
void SetModemLightLevel(int type)
{
	periodic = false;
	fulllight = false;
	switch (type)
	{
	case bo_light_dark:
		MAPSPOT(2, 0, 0) = (word)216;
		MAPSPOT(3, 0, 0) = (word)255;
		MAPSPOT(3, 0, 1) = (word)139;
		MAPSPOT(2, 0, 1) = (word)104;
		SetupLightLevels();
		break;
	case bo_light_normal:
		break;
	case bo_light_bright:
		MAPSPOT(2, 0, 0) = (word)223;
		MAPSPOT(3, 0, 0) = (word)267;
		MAPSPOT(3, 0, 1) = (word)0;
		MAPSPOT(2, 0, 1) = (word)104;
		SetupLightLevels();
		break;
	case bo_light_fog:
		MAPSPOT(2, 0, 0) = (word)219;
		MAPSPOT(3, 0, 0) = (word)259;
		MAPSPOT(2, 0, 1) = (word)105;
		MAPSPOT(3, 0, 1) = (word)0;
		SetupLightLevels();
		break;
	case bo_light_periodic:
		fog = 0;
		MAPSPOT(2, 0, 1) = (word)104;
		MAPSPOT(3, 0, 1) = (word)139;
		SetupLightLevels();
		periodic = true;
		break;
	case bo_light_lightning:
		if (sky != 0)
		{
			MAPSPOT(2, 0, 0) = (word)222;
			MAPSPOT(3, 0, 0) = (word)255;
			MAPSPOT(3, 0, 1) = (word)139;
			MAPSPOT(2, 0, 1) = (word)104;
			SetupLightLevels();
			lightning = true;
		}
		break;
	}
}
