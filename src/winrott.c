/* Routines from winrott needed for the highres support for the SDL port */
#include <stdlib.h>
#include <string.h>
#include "WinRott.h"
#include "modexlib.h"
#include "rt_cfg.h"
#include "rt_def.h"
#include "rt_util.h"
#include "rt_view.h"
#include "rt_draw.h"

// typedef unsigned char byte;

// width and height of the real framebuffer in pixels
// e.g. aspect ratio corrected 1920x1080 may be 1920x900 or 2304x1080 real FB
int FRAMEBUFFERWIDTH = 640;
int FRAMEBUFFERHEIGHT = 480;

// width and height of the displlay in pixels
// e.g. aspect ratio corrected 1920x1080 may be 1920x900 or 2304x1080 real fb.
// this is the native target resolution to rescale to
int VIRTUALWIDTH = 640; 
int VIRTUALHEIGHT = 480;

// width/height of monitor
int MONITORWIDTH = 640; 
int MONITORHEIGHT = 480;

int iGLOBAL_SCREENBWIDE;
int iG_SCREENWIDTH; // default screen width in bytes

int iGLOBAL_HEALTH_X;
int iGLOBAL_HEALTH_Y;
int iGLOBAL_AMMO_X;
int iGLOBAL_AMMO_Y;

int iG_X_center;
int iG_Y_center;

// int topBarCenterOffset = 0;

bool iG_aimCross = 0;

extern int viewheight;
extern int viewwidth;
extern int vfov;

//----------------------------------------------------------------------
#define FINEANGLES 2048

void RecalculateFocalLength(void)
{
	focallength = FOVToFocalLength(vfov);
	SetViewDelta();
}

extern int aspectRatioCorrection;
void GetMonitorResolution()
{
	SDL_DisplayMode DM;

	if (SDL_GetDesktopDisplayMode(0, &DM) != 0)
		Error("Can't get monitor resolution from SDL!\n%s\n", SDL_GetError());

	MONITORWIDTH = DM.w;
	MONITORHEIGHT = DM.h;
}

extern int hudRescaleFactor;

void SetRottScreenRes(int Width, int Height)
{
	int maxRescale = (int)floor((float)FRAMEBUFFERWIDTH / 320);

	FreeFramebuffer();

	constexpr int FAST = 1;
	constexpr int ACCURATE = 2;

	// prevent 320x200 at fast rendering as 320x160 and 3840x2160 as 4608x2160 at accurate
	// (essentially clamp buffer sizes to 320x200 < res < 3840x2160)
	if(Width >= 3840 && aspectRatioCorrection == ACCURATE)
	{
		aspectRatioCorrection = FAST;
	}
	if(Width <= 320 && aspectRatioCorrection == FAST)
	{
		aspectRatioCorrection = ACCURATE;
	}

	switch(aspectRatioCorrection)
	{
		case FAST:
			// upscale from width x (height/1.2)
			VIRTUALWIDTH = Width;
			VIRTUALHEIGHT = Height;
			FRAMEBUFFERWIDTH = Width;
			FRAMEBUFFERHEIGHT = (int)round(Height / 1.2f);
			break;
		case ACCURATE:
			// downscale from (width*1.2) x height
			VIRTUALWIDTH = Width;
			VIRTUALHEIGHT = Height;
			FRAMEBUFFERWIDTH = (int)round(Width * 1.2f);
			FRAMEBUFFERHEIGHT = Height;
			break;
		default: // none
			VIRTUALWIDTH = Width;
			VIRTUALHEIGHT = Height;
			FRAMEBUFFERWIDTH = Width;
			FRAMEBUFFERHEIGHT = Height;
			break;
	}

	focallength = FOVToFocalLength(vfov);
	SetViewDelta();

	int middleWidth = FRAMEBUFFERWIDTH / 2;

	iGLOBAL_AMMO_X = middleWidth + 160 - 20;

	iGLOBAL_AMMO_Y = FRAMEBUFFERHEIGHT - 16;

	iGLOBAL_HEALTH_X = middleWidth - 160 + 20;

	iGLOBAL_HEALTH_Y = iGLOBAL_AMMO_Y;

	iGLOBAL_SCREENBWIDE = FRAMEBUFFERWIDTH * (96 / 320);
	iG_SCREENWIDTH = FRAMEBUFFERWIDTH * (96 / 320);

	AllocateFramebuffer();
	VL_SetVGAPlaneMode();
	VL_SetPalette(origpal);
	GenerateSkyScalerTable();

	// hack hack hackky hack
	if(StretchScreen)
	{
		DisableScreenStretch();
		EnableScreenStretch();
	}
	else
	{
		EnableScreenStretch();
		DisableScreenStretch();
	}

	printf("framebuffer res %d x %d\ntarget res %d x %d\n", FRAMEBUFFERWIDTH, FRAMEBUFFERHEIGHT, VIRTUALWIDTH, VIRTUALHEIGHT);

	if(hudRescaleFactor > maxRescale)
		hudRescaleFactor = maxRescale;

	// default screen width in bytes

	// MinScreenWidth =  ((float)FRAMEBUFFERWIDTH * 0.01875);

	// MinScreenHeight = ((float)FRAMEBUFFERHEIGHT * 0.02);
}

//----------------------------------------------------------------------
// luckey for me that I am not programmin a 386 or the next
// 4 function would never have worked. bna++
extern int viewsize;
void MoveScreenUpLeft()
{
	int startX, startY, startoffset;
	byte *Ycnt, *b;
	//   SetTextMode (  );
	b = (byte*)bufferofs;
	b += (((FRAMEBUFFERHEIGHT - viewheight) / 2) * FRAMEBUFFERWIDTH) +
		 ((FRAMEBUFFERWIDTH - viewwidth) / 2);
	if (viewsize == 8)
	{
		b += 8 * FRAMEBUFFERWIDTH;
	}
	startX = 3; // take 3 pixels to the right
	startY = 3; // take 3 lines down
	startoffset = (startY * FRAMEBUFFERWIDTH) + startX;

	for (Ycnt = b; Ycnt < b + ((viewheight - startY) * FRAMEBUFFERWIDTH);
		 Ycnt += FRAMEBUFFERWIDTH)
	{
		memcpy(Ycnt, Ycnt + startoffset, viewwidth - startX);
	}
}
//----------------------------------------------------------------------
void MoveScreenDownLeft()
{
	int startX, startY, startoffset;
	byte *Ycnt, *b;
	//   SetTextMode (  );
	b = (byte*)bufferofs;
	b += (((FRAMEBUFFERHEIGHT - viewheight) / 2) * FRAMEBUFFERWIDTH) +
		 ((FRAMEBUFFERWIDTH - viewwidth) / 2);
	if (viewsize == 8)
	{
		b += 8 * FRAMEBUFFERWIDTH;
	}
	startX = 3;									  // take 3 pixels to the right
	startY = 3;									  // take 3 lines down
	startoffset = (startY * FRAMEBUFFERWIDTH); //+startX;

	// Ycnt starts in botton of screen and copys lines upwards
	for (Ycnt = b + ((viewheight - startY - 1) * FRAMEBUFFERWIDTH); Ycnt > b;
		 Ycnt -= FRAMEBUFFERWIDTH)
	{
		memcpy(Ycnt + startoffset, Ycnt + startX, viewwidth - startX);
	}
}
//----------------------------------------------------------------------
void MoveScreenUpRight()
{
	int startX, startY, startoffset;
	byte *Ycnt, *b;
	//   SetTextMode (  );
	b = (byte*)bufferofs;

	b += (((FRAMEBUFFERHEIGHT - viewheight) / 2) * FRAMEBUFFERWIDTH) +
		 ((FRAMEBUFFERWIDTH - viewwidth) / 2);
	if (viewsize == 8)
	{
		b += 8 * FRAMEBUFFERWIDTH;
	}
	startX = 3;									  // take 3 pixels to the right
	startY = 3;									  // take 3 lines down
	startoffset = (startY * FRAMEBUFFERWIDTH); //+startX;

	for (Ycnt = b; Ycnt < b + ((viewheight - startY) * FRAMEBUFFERWIDTH);
		 Ycnt += FRAMEBUFFERWIDTH)
	{
		memcpy(Ycnt + startX, Ycnt + startoffset, viewwidth - startX);
	}
}
//----------------------------------------------------------------------
void MoveScreenDownRight()
{
	int startX, startY, startoffset;
	byte *Ycnt, *b;
	//   SetTextMode (  );
	b = (byte*)bufferofs;

	b += (((FRAMEBUFFERHEIGHT - viewheight) / 2) * FRAMEBUFFERWIDTH) +
		 ((FRAMEBUFFERWIDTH - viewwidth) / 2);
	if (viewsize == 8)
	{
		b += 8 * FRAMEBUFFERWIDTH;
	}
	startX = 3; // take 3 pixels to the right
	startY = 3; // take 3 lines down
	startoffset = (startY * FRAMEBUFFERWIDTH) + startX;

	// Ycnt starts in botton of screen and copys lines upwards
	for (Ycnt = b + ((viewheight - startY - 1) * FRAMEBUFFERWIDTH); Ycnt > b;
		 Ycnt -= FRAMEBUFFERWIDTH)
	{
		memcpy(Ycnt + startoffset, Ycnt, viewwidth - startX);
	}
}
