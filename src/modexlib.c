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

#include <SDL_stdinc.h>
#include <SDL_video.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "modexlib.h"
#include "isr.h"
#include "rt_draw.h"
#include "rt_util.h"
#include "rt_net.h" // for GamePaused
#include "rt_view.h"
#include "rt_vid.h"
#include "queue.h"
#include "lumpy.h"
#include "exeicon.c"

static void StretchMemPicture();
// GLOBAL VARIABLES

bool StretchScreen = 0; // bn�++
extern bool iG_aimCross;
extern bool sdl_fullscreen;
extern int iG_X_center;
extern int iG_Y_center;
char* iG_buf_center;

SDL_Surface* sdl_surface = NULL;

SDL_Window* window = NULL;

static SDL_Renderer* renderer = NULL;

SDL_Surface* unstretch_sdl_surface = NULL;

static SDL_Texture* sdl_texture = NULL;

SDL_Surface* temp = NULL;

// Queue *sdl_draw_obj_queue = NULL;

bool doRescaling = false;

int linewidth;
// int    ylookup[MAXSCREENHEIGHT];
int ylookup[MAXSCREENHEIGHT]; // just set to max res
byte* page1start;
byte* page2start;
byte* page3start;
int screensize;
byte* bufferofs;
byte* displayofs;
bool graphicsmode = false;
char* bufofsTopLimit;
char* bufofsBottomLimit;

void DrawCenterAim();

/*
====================
=
= GraphicsMode
=
====================
*/

// thank you - https://blog.gibson.sh/2015/04/13/how-to-integrate-your-sdl2-window-icon-or-any-image-into-your-executable/
static void GetIconMasks(Uint32* r, Uint32* g, Uint32* b, Uint32* a)
{
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		*shift = (my_icon.bytes_per_pixel == 3) ? 8 : 0;
		*r = 0xff000000 >> shift;
		*g = 0x00ff0000 >> shift;
		*b = 0x0000ff00 >> shift;
		*a = 0x000000ff >> shift;
	#else
		*r = 0x000000ff;
		*g = 0x0000ff00;
		*b = 0x00ff0000;
		*a = (barrett_icon.bytes_per_pixel == 3) ? 0 : 0xff000000;
	#endif
}

void GraphicsMode(void)
{
	Uint32 flags = 0;

	if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		Error("Could not initialize SDL\n");
	}

	// SDL_SetRelativeMouseMode(SDL_TRUE);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	if (sdl_fullscreen)
		flags = SDL_WINDOW_FULLSCREEN_DESKTOP;

	window = SDL_CreateWindow("Barrett", SDL_WINDOWPOS_UNDEFINED,
							  SDL_WINDOWPOS_UNDEFINED, iGLOBAL_SCREENWIDTH,
							  iGLOBAL_SCREENHEIGHT, flags);

	if (window == NULL)
	{
		Error("Could not set video mode\n");
		exit(1);
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	sdl_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
									SDL_TEXTUREACCESS_STREAMING,
									iGLOBAL_SCREENWIDTH, iGLOBAL_SCREENHEIGHT);

	sdl_surface = SDL_CreateRGBSurface(0, iGLOBAL_SCREENWIDTH,
									   iGLOBAL_SCREENHEIGHT, 8, 0, 0, 0, 0);

	SDL_SetSurfaceRLE(sdl_surface, 1);

	SDL_RenderSetLogicalSize(renderer, iGLOBAL_SCREENWIDTH,
							 iGLOBAL_SCREENHEIGHT);

	Uint32 r,g,b,a = 0;

	GetIconMasks(&r, &g, &b, &a);

	SDL_Surface* icon = SDL_CreateRGBSurfaceFrom((void*)barrett_icon.pixel_data,
		barrett_icon.width, barrett_icon.height, barrett_icon.bytes_per_pixel*8,
		barrett_icon.bytes_per_pixel*barrett_icon.width, r, g, b, a);

	SDL_SetWindowIcon(window, icon);
	
	SDL_FreeSurface(icon);
}

/*
====================
=
= SetTextMode
=
====================
*/
void SetTextMode(void)
{
	if (SDL_WasInit(SDL_INIT_VIDEO) == SDL_INIT_VIDEO)
	{
		if (sdl_surface != NULL)
		{
			SDL_FreeSurface(sdl_surface);

			sdl_surface = NULL;
		}

		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	}
}

/*
====================
=
= WaitVBL
=
====================
*/

// duplicate in-game frame sync method
void WaitVBL(void)
{
	int tc, oldtime;

	tc = oldtime = GetTicCount();
	while (tc == oldtime)
	{
		tc = GetTicCount();
	}
}

/*
=======================
=
= VL_SetVGAPlaneMode
=
=======================
*/

void VL_SetVGAPlaneMode(void)
{
	int i, offset;

	GraphicsMode();

	//
	// set up lookup tables
	//
	// bna--   linewidth = 320;
	linewidth = iGLOBAL_SCREENWIDTH;

	offset = 0;

	for (i = 0; i < iGLOBAL_SCREENHEIGHT; i++)
	{
		ylookup[i] = offset;
		offset += linewidth;
	}

	//    screensize=MAXSCREENHEIGHT*MAXSCREENWIDTH;
	screensize = iGLOBAL_SCREENHEIGHT * iGLOBAL_SCREENWIDTH;

	page1start = sdl_surface->pixels;
	page2start = sdl_surface->pixels;
	page3start = sdl_surface->pixels;
	displayofs = page1start;
	bufferofs = page2start;

	iG_X_center = iGLOBAL_SCREENWIDTH / 2;
	iG_Y_center = (iGLOBAL_SCREENHEIGHT / 2) + 10; //+10 = move aim down a bit

	iG_buf_center =
		(char*)(bufferofs +
				(screensize /
				 2)); //(iG_Y_center*iGLOBAL_SCREENWIDTH);//+iG_X_center;

	bufofsTopLimit = (char*)(bufferofs + screensize - iGLOBAL_SCREENWIDTH);
	bufofsBottomLimit = (char*)(bufferofs + iGLOBAL_SCREENWIDTH);

	// start stretched
	EnableScreenStretch();
	VH_UpdateScreen();
}

/*
=======================
=
= VL_CopyPlanarPage
=
=======================
*/
void VL_CopyPlanarPage(byte* src, byte* dest)
{
	memcpy(dest, src, screensize);
}

/*
=================
=
= VL_ClearBuffer
=
= Fill the entire video buffer with a given color
=
=================
*/

void VL_ClearBuffer(byte* buf, byte color)
{
	memset((byte*)buf, color, screensize);
}

/*
=================
=
= VL_ClearVideo
=
= Fill the entire video buffer with a given color
=
=================
*/

void VL_ClearVideo(byte color)
{
	memset(sdl_surface->pixels, color,
		   iGLOBAL_SCREENWIDTH * iGLOBAL_SCREENHEIGHT);
}

void RescaleAreaOfTexture(SDL_Renderer* render, SDL_Texture* source,
						  SDL_Rect src, SDL_Rect dest)
{
	SDL_Texture* sourceToResize =
		SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888,
						  SDL_TEXTUREACCESS_TARGET, src.w, src.h);
	SDL_SetRenderTarget(render, sourceToResize);
	SDL_RenderCopy(render, source, &src, NULL);
	// the folowing line should reset the target to default(the screen)
	SDL_SetRenderTarget(render, NULL);

	SDL_RenderCopy(render, sourceToResize, NULL, &dest);
	SDL_DestroyTexture(sourceToResize);
}

int hudRescaleFactor = 1;

void RenderSurface(void)
{
	SDL_Texture* newTex = SDL_CreateTextureFromSurface(renderer, sdl_surface);

	if (newTex == NULL)
	{
		Error("CreateTextureFromSurface failed: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_RenderClear(renderer);

	SDL_RenderCopy(renderer, newTex, NULL, NULL);

	if (!StretchScreen && hudRescaleFactor > 1 && doRescaling)
	{
		if (SHOW_TOP_STATUS_BAR())
			RescaleAreaOfTexture(
				renderer, newTex,
				(SDL_Rect){(iGLOBAL_SCREENWIDTH - 320) >> 1, 0, 320, 16},
				(SDL_Rect){(iGLOBAL_SCREENWIDTH - (320 * hudRescaleFactor)) >>
							   1,
						   0, 320 * hudRescaleFactor,
						   16 * hudRescaleFactor}); // Status Bar
		if (SHOW_BOTTOM_STATUS_BAR())
			RescaleAreaOfTexture(
				renderer, newTex,
				(SDL_Rect){(iGLOBAL_SCREENWIDTH - 320) >> 1,
						   iGLOBAL_SCREENHEIGHT - 16, 320, 16},
				(SDL_Rect){(iGLOBAL_SCREENWIDTH - (320 * hudRescaleFactor)) >>
							   1,
						   iGLOBAL_SCREENHEIGHT - 16 * hudRescaleFactor,
						   320 * hudRescaleFactor,
						   16 * hudRescaleFactor}); // Bottom Bar
	}

	SDL_RenderPresent(renderer);

	SDL_DestroyTexture(newTex);
}

/* C version of rt_vh_a.asm */

void VH_UpdateScreen(void)
{

	if (StretchScreen)
	{ // bna++
		StretchMemPicture();
	}
	else
	{
		DrawCenterAim();
	}

	RenderSurface();
}

void EnableScreenStretch(void)
{
	if (iGLOBAL_SCREENWIDTH <= 320 || StretchScreen)
		return;

	if (unstretch_sdl_surface == NULL)
	{
		/* should really be just 320x200, but there is code all over the
		   places which crashes then */
		unstretch_sdl_surface =
			SDL_CreateRGBSurface(SDL_SWSURFACE, iGLOBAL_SCREENWIDTH,
								 iGLOBAL_SCREENHEIGHT, 8, 0, 0, 0, 0);
	}

	displayofs = (byte *)unstretch_sdl_surface->pixels +
					(displayofs - (byte*)sdl_surface->pixels);
	bufferofs = unstretch_sdl_surface->pixels;
	page1start = unstretch_sdl_surface->pixels;
	page2start = unstretch_sdl_surface->pixels;
	page3start = unstretch_sdl_surface->pixels;
	StretchScreen = 1;
}

void DisableScreenStretch(void)
{
	if (iGLOBAL_SCREENWIDTH <= 320 || !StretchScreen)
		return;

	displayofs = (byte*)sdl_surface->pixels +
				 (displayofs - (byte*)unstretch_sdl_surface->pixels);
	bufferofs = sdl_surface->pixels;
	page1start = sdl_surface->pixels;
	page2start = sdl_surface->pixels;
	page3start = sdl_surface->pixels;
	StretchScreen = 0;
	SDL_RenderSetLogicalSize(renderer, iGLOBAL_SCREENWIDTH,
							 iGLOBAL_SCREENHEIGHT);
	// SDL_RenderSetLogicalSize(renderer, 320, 200);
}

void EnableHudStretch(void)
{
	doRescaling = 1;
}

void DisableHudStretch(void)
{
	doRescaling = 0;
}

// bna section -------------------------------------------
static void StretchMemPicture()
{
	SDL_Rect src;
	SDL_Rect dest;

	src.x = 0;
	src.y = 0;
	src.w = 320;
	src.h = 200;

	dest.x = 0;
	dest.y = 0;
	dest.w = iGLOBAL_SCREENWIDTH;
	dest.h = iGLOBAL_SCREENHEIGHT;
	SDL_SoftStretch(unstretch_sdl_surface, &src, sdl_surface, &dest);
	SDL_RenderSetLogicalSize(renderer, 320,
							 240); // help keep aspect ratio of menus so that
								   // the game doesn't look stretched
}

// bna function added start
extern bool ingame;
extern exit_t playstate;
int iG_playerTilt;

void DrawCenterAim()
{
	bool drawDot = true;
	bool usePercentHealth = true;
	int colour;
	int gap = 4, length = 6;

	int thickness = 3, startY = 0;

	int x;

	int percenthealth = (locplayerstate->health * 10) /
						MaxHitpointsForCharacter(locplayerstate);
	int hpcolour = percenthealth < 3	? egacolor[RED]
				: percenthealth < 4 ? egacolor[YELLOW]
									: egacolor[GREEN];


	// re-center crosshair to compensate for thickness.
	// e.g. a thickness of 3 will result in a starting position of -1, thus the pixel range will be -1 to 1.
	// as is the generally case with crosshairs, even numbered thicknesses are a pain, but still follow this rule for consistency.
	// this will truncate, and thus is always the floor of this operation.
	if(thickness > 1)
		startY = 0 - thickness / 2;

	if (iG_aimCross && !GamePaused && 
		playstate != ex_died && ingame == true &&
		iGLOBAL_SCREENWIDTH > 320)
	{
		if(usePercentHealth)
			colour = hpcolour;

		// get center of back buffer as char pointer
		iG_buf_center = (char*)(bufferofs + ((iG_Y_center) * iGLOBAL_SCREENWIDTH));

		// draw center dot
		if(drawDot)
			*(iG_buf_center + iG_X_center) = colour;


		// left line
		for (x = iG_X_center - (gap + length); x <= iG_X_center - gap; x++)
		{
			for(int y = startY; y < thickness; y++)
			{
				// add nothing when current y thickness is zero, to avoid shifting down by screenwidth.
				int ycoord = y != 0 ? iGLOBAL_SCREENWIDTH * y : 0;

				if ((iG_buf_center + x + ycoord < bufofsTopLimit) &&
					(iG_buf_center + x + ycoord > bufofsBottomLimit))
				{
					*(iG_buf_center + x + ycoord) = colour;
				}
			}
		}
		
		// right line
		for (x = iG_X_center + gap; x <= iG_X_center + (gap + length); x++)
		{
			for(int y = startY; y < thickness; y++)
			{
				// add nothing when current y thickness is zero, to avoid shifting down by screenwidth.
				int ycoord = y != 0 ? iGLOBAL_SCREENWIDTH * y : 0;

				if ((iG_buf_center + x + ycoord < bufofsTopLimit) &&
					(iG_buf_center + x + ycoord > bufofsBottomLimit))
				{
					*(iG_buf_center + x + ycoord) = colour;
				}
			}
		}
		
		// top line
		for (x = (gap + length); x >= gap; x--)
		{
			if (((iG_buf_center - (x * iGLOBAL_SCREENWIDTH) + iG_X_center) <
					bufofsTopLimit) &&
				((iG_buf_center - (x * iGLOBAL_SCREENWIDTH) + iG_X_center) >
					bufofsBottomLimit))
			{
				*(iG_buf_center - (x * iGLOBAL_SCREENWIDTH) + iG_X_center) = colour;
			}
		}

		// bottom line
		for (x = gap; x <= (gap + length); x++)
		{
			if (((iG_buf_center + (x * iGLOBAL_SCREENWIDTH) + iG_X_center) <
					bufofsTopLimit) &&
				((iG_buf_center + (x * iGLOBAL_SCREENWIDTH) + iG_X_center) >
					bufofsBottomLimit))
			{
				*(iG_buf_center + (x * iGLOBAL_SCREENWIDTH) + iG_X_center) = colour;
			}
		}
}
}
// bna function added end

// bna section -------------------------------------------

void sdl_handle_window_events(void)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.window.type)
		{
			case SDL_WINDOWEVENT_FOCUS_LOST:
				SDL_SetRelativeMouseMode(SDL_FALSE);
				SDL_SetWindowMouseGrab(window, SDL_FALSE);
				break;
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				SDL_SetRelativeMouseMode(SDL_TRUE);
				SDL_SetWindowMouseGrab(window, SDL_TRUE);
				break;
			case SDL_WINDOWEVENT_MINIMIZED:
				SDL_SetRelativeMouseMode(SDL_FALSE);
				SDL_SetWindowMouseGrab(window, SDL_FALSE);
				break;
			case SDL_WINDOWEVENT_RESTORED:
				SDL_SetRelativeMouseMode(SDL_TRUE);
				SDL_SetWindowMouseGrab(window, SDL_TRUE);
				break;
			case SDL_WINDOWEVENT_CLOSE:
				event.type = SDL_QUIT;
				SDL_PushEvent(&event);
				break;
		}
	}
}

// extern int tics;

// void CalcTics (void);

// void DrawRotatedScreen(int cx, int cy, byte *destscreen, int angle, int
// scale, int masked)

void DoScreenRotateScale(int w, int h, SDL_Texture* tex, float angle,
						 float scrScale)
{

	SDL_RenderClear(renderer);

	SDL_Rect output;

	output.w = abs((int)((float)w * scrScale));

	output.h = abs((int)((float)h * scrScale));

	// if (output.w < MinScreenWidth)
	// output.w = MinScreenWidth;
	// if (output.h < MinScreenHeight)
	// output.h = MinScreenHeight;

	output.x = (iGLOBAL_SCREENWIDTH - output.w) >> 1;

	output.y = (iGLOBAL_SCREENHEIGHT - output.h) >> 1;

	SDL_RenderCopyEx(renderer, tex, NULL, &output, angle, NULL, SDL_FLIP_NONE);

	SDL_RenderPresent(renderer);
}

SDL_Texture* GetMainSurfaceAsTexture(void)
{
	return SDL_CreateTextureFromSurface(renderer, sdl_surface);
}

const SDL_Renderer* GetRenderer(void)
{
	return renderer;
}
