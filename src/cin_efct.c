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

#include "cin_glob.h"
#include "cin_util.h"
#include "cin_def.h"
#include "cin_main.h"
#include "f_scale.h"
#include "rt_fixed.h"
#include "lumpy.h"
#include "w_wad.h"
#include "z_zone.h"
#include <string.h>
#include "rt_vid.h"

#include "modexlib.h"

static int cin_sprtopoffset;
static int cin_invscale;

void DrawFadeout(void);
void DrawBlankScreen(void);
void DrawClearBuffer(void);

/*
===============
=
= SpawnCinematicFlic
=
===============
*/

cine_event* SpawnCinematicFlic(char* name, bool loop, bool usefile)
{
	cine_event* f;

	f = SafeMalloc(sizeof(cine_event));

	// copy name of flic

	strcpy(f->name, name);

	f->loop = loop;

	f->usefile = usefile;

	return f;
}

/*
===============
=
= SpawnCinematicSprite
=
===============
*/

cinespr_event* SpawnCinematicSprite(char* name, int duration, int numframes,
								  int framedelay, int x, int y, int scale,
								  int endx, int endy, int endscale)
{
	cinespr_event* sprite;

	sprite = SafeMalloc(sizeof(cinespr_event));

	// copy name of sprite

	strcpy(sprite->name, name);

	// copy rest of sprite information

	sprite->duration = duration;
	sprite->numframes = numframes;
	sprite->framedelay = framedelay;
	sprite->frame = 0;
	sprite->frametime = framedelay;

	sprite->x = x << FRACTIONBITS;
	sprite->y = y << FRACTIONBITS;

	//   sprite->y+=(p->width-p->height)<<(FRACTIONBITS-1);

	sprite->scale = scale << FRACTIONBITS;
	sprite->dx = ((endx - x) << FRACTIONBITS) / duration;
	sprite->dy = ((endy - y) << FRACTIONBITS) / duration;
	sprite->dscale = ((endscale - scale) << FRACTIONBITS) / duration;

	return sprite;
}

/*
===============
=
= SpawnCinematicBack
=
===============
*/

cine_bgevent* SpawnCinematicBack(char* name, int duration, int width, int startx,
							  int endx, int yoffset)
{
	cine_bgevent* back;

	back = SafeMalloc(sizeof(cine_bgevent));

	// copy name of back

	strcpy(back->name, name);

	// copy rest of back information

	back->duration = duration;
	back->backdropwidth = width;
	back->dx = ((endx - startx) << FRACTIONBITS) / duration;
	back->currentoffset = startx << FRACTIONBITS;
	back->yoffset = yoffset;

	return back;
}

/*
===============
=
= SpawnCinematicMultiBack
=
===============
*/

cine_bgevent* SpawnCinematicMultiBack(char* name, char* name2, int duration,
								   int startx, int endx, int yoffset)
{
	cine_bgevent* back;
	lpic_t* pic1;
	lpic_t* pic2;

	pic1 = (lpic_t*)W_CacheLumpName(name, PU_CACHE, Cvt_lpic_t, 1);
	pic2 = (lpic_t*)W_CacheLumpName(name2, PU_CACHE, Cvt_lpic_t, 1);

	back = SafeMalloc(sizeof(cine_bgevent));

	// copy name of back

	strcpy(back->name, name);

	// copy rest of back information

	back->duration = duration;
	back->dx = ((endx - startx) << FRACTIONBITS) / duration;
	back->currentoffset = startx << FRACTIONBITS;
	back->yoffset = yoffset;
	if (pic1->height != pic2->height)
	{
		Error("SpawnCinematicMultiBack: heights are not the same\n"
			  "                         name1=%s name2=%s\n",
			  name, name2);
	}
	back->backdropwidth = pic1->width + pic2->width;
	back->height = pic1->height;
	back->data = SafeMalloc(back->backdropwidth * back->height);
	memcpy(back->data, &(pic1->data), pic1->width * pic1->height);
	memcpy(back->data + (pic1->width * pic1->height), &(pic2->data),
		   pic2->width * pic2->height);
	return back;
}

/*
===============
=
= SpawnCinematicPalette
=
===============
*/

cine_palevent* SpawnCinematicPalette(char* name)
{
	cine_palevent* p;

	p = SafeMalloc(sizeof(cine_palevent));

	// copy name of palette

	strcpy(p->name, name);

	return p;
}

/*
=================
=
= ScaleFilmPost
=
=================
*/
void ScaleFilmPost(byte* src, byte* buf)
{
	int offset;
	int length;
	int topscreen;
	int bottomscreen;

	offset = *(src++);
	for (; offset != 255;)
	{
		length = *(src++);
		topscreen = cin_sprtopoffset + (cin_invscale * offset);
		bottomscreen = topscreen + (cin_invscale * length);
		cin_yl = (topscreen + FRACTIONUNIT - 1) >> FRACTIONBITS;
		cin_yh = (bottomscreen - FRACTIONUNIT) >> FRACTIONBITS;
		if (cin_yh >= FRAMEBUFFERHEIGHT)
			cin_yh = FRAMEBUFFERHEIGHT - 1;
		if (cin_yl < 0)
			cin_yl = 0;
		if (cin_yl <= cin_yh)
		{
			cin_source = src - offset;
			R_DrawFilmColumn(buf);
		}
		src += length;
		offset = *(src++);
	}
}

/*
=================
=
= DrawFlic
=
=================
*/
void DrawFlic(cine_event* f)
{
	byte* curpal;
	char flicname[40];

	curpal = SafeMalloc(768);

	CinematicGetPalette(curpal);

	DrawFadeout();

	if (f->usefile == false)
	{
		strcpy(flicname, f->name);
	}
	else
	{
		strcpy(flicname, f->name);
		strcat(flicname, ".fli");
	}

	// med
	//   PlayFlic ( flicname, buf, flic->usefile, flic->loop);

	if (f->loop == true)
		ClearCinematicAbort();

	DrawFadeout();

	DrawBlankScreen();

	CinematicSetPalette(curpal);

	SafeFree(curpal);
	GetCinematicTics();
	GetCinematicTics();
}

/*
=================
=
= PrecacheFlic
=
=================
*/

void PrecacheFlic(cine_event* f)
{
	if (f->usefile == false)
	{
		W_CacheLumpName(f->name, PU_CACHE, CvtNull, 1);
	}
}

/*
===============
=
= DrawCinematicBackground
=
===============
*/

void DrawCinematicBackground(cine_bgevent* back)
{
	byte* src;
	byte* buf;
	lpic_t* pic;
	int i;
	int plane;
	int offset;
	int height;

	pic = (lpic_t*)W_CacheLumpName(back->name, PU_CACHE, Cvt_lpic_t, 1);

	height = pic->height;
	if (height + back->yoffset > FRAMEBUFFERHEIGHT)
		height = FRAMEBUFFERHEIGHT - back->yoffset;

	if (height != FRAMEBUFFERHEIGHT)
		DrawClearBuffer();

	plane = 0;

	{
		buf = (byte*)bufferofs + ylookup[back->yoffset];
		offset = (back->currentoffset >> FRACTIONBITS) + plane;

		for (i = 0; i < FRAMEBUFFERWIDTH; i++, offset++, buf++)
		{
			if (offset >= back->backdropwidth)
				src = &(pic->data) +
					  ((offset - back->backdropwidth) * (pic->height));
			else if (offset < 0)
				src = &(pic->data) +
					  ((offset + back->backdropwidth) * (pic->height));
			else
				src = &(pic->data) + (offset * (pic->height));
			DrawFilmPost(buf, src, height);
		}
	}
}

/*
===============
=
= DrawCinematicMultiBackground
=
===============
*/

void DrawCinematicMultiBackground(cine_bgevent* back)
{
	byte* src;
	byte* buf;
	int i;
	int plane;
	int offset;
	int height;

	height = back->height;
	if (height + back->yoffset > FRAMEBUFFERHEIGHT)
		height = FRAMEBUFFERHEIGHT - back->yoffset;

	if (height != FRAMEBUFFERHEIGHT)
		DrawClearBuffer();

	plane = 0;

	{
		buf = (byte*)bufferofs + ylookup[back->yoffset];
		offset = (back->currentoffset >> FRACTIONBITS) + plane;

		for (i = 0; i < FRAMEBUFFERWIDTH; i++, offset++, buf++)
		{
			if (offset >= back->backdropwidth)
				src = back->data +
					  ((offset - back->backdropwidth) * (back->height));
			else if (offset < 0)
				src = back->data +
					  ((offset + back->backdropwidth) * (back->height));
			else
				src = back->data + (offset * (back->height));
			DrawFilmPost(buf, src, height);
		}
	}
}

/*
===============
=
= DrawCinematicBackdrop
=
===============
*/

void DrawCinematicBackdrop(cine_bgevent* back)
{
	byte* src;
	byte* shape;
	byte* buf;
	patch_t* p;
	int i;
	int plane;
	int offset;
	int postoffset;
	int postlength;
	int toppost;

	shape = W_CacheLumpName(back->name, PU_CACHE, Cvt_patch_t, 1);
	p = (patch_t*)shape;

	toppost = -p->topoffset + back->yoffset;

	plane = 0;

	{
		buf = (byte*)bufferofs;
		offset = (back->currentoffset >> FRACTIONBITS) + plane;

		for (i = 0; i < FRAMEBUFFERWIDTH; i++, offset++, buf++)
		{
			if (offset >= back->backdropwidth)
				src = shape + p->collumnofs[offset - back->backdropwidth];
			else if (offset < 0)
				src = shape + p->collumnofs[offset + back->backdropwidth];
			else
				src = shape + p->collumnofs[offset];

			postoffset = *(src++);
			for (; postoffset != 255;)
			{
				postlength = *(src++);
				DrawFilmPost(buf + ylookup[toppost + postoffset], src,
							 postlength);
				src += postlength;
				postoffset = *(src++);
			}
		}
	}
}

/*
=================
=
= PrecacheBack
=
=================
*/
void PrecacheBack(cine_bgevent* back)
{
	W_CacheLumpName(back->name, PU_CACHE, CvtNull, 1);
}

/*
=================
=
= DrawCinematicSprite
=
=================
*/
void DrawCinematicSprite(cinespr_event* sprite)
{
	byte* shape;
	int frac;
	patch_t* p;
	int x1, x2;
	int tx;
	int xcent;
	byte* buf;
	int height;

	height = sprite->scale >> FRACTIONBITS;

	if (height < 2)
		return;

	shape = W_CacheLumpNum(W_GetNumForName(sprite->name) + sprite->frame,
						   PU_CACHE, Cvt_patch_t, 1);
	p = (patch_t*)shape;

	cin_ycenter = sprite->y >> FRACTIONBITS;
	cin_invscale = (height << FRACTIONBITS) / p->origsize;
	buf = (byte*)bufferofs;
	tx = -p->leftoffset;
	xcent = (sprite->x & 0xffff0000) - (height << (FRACTIONBITS - 1)) +
			(FRACTIONUNIT >> 1);

	//
	// calculate edges of the shape
	//
	x1 = (xcent + (tx * cin_invscale)) >> FRACTIONBITS;
	if (x1 >= FRAMEBUFFERWIDTH)
		return; // off the right side
	tx += p->width;
	x2 = ((xcent + (tx * cin_invscale)) >> FRACTIONBITS) - 1;
	if (x2 < 0)
		return; // off the left side

	cin_iscale = (p->origsize << FRACTIONBITS) / height;

	if (x1 < 0)
	{
		frac = cin_iscale * (-x1);
		x1 = 0;
	}
	else
		frac = 0;
	x2 = x2 >= FRAMEBUFFERWIDTH ? (FRAMEBUFFERWIDTH - 1) : x2;

	cin_texturemid = (((p->origsize >> 1) + p->topoffset) << FRACTIONBITS) +
					 (FRACTIONUNIT >> 1);
	cin_sprtopoffset =
		(cin_ycenter << 16) - FixedMul(cin_texturemid, cin_invscale);

	for (; x1 <= x2; x1++, frac += cin_iscale)
	{
		ScaleFilmPost(((p->collumnofs[frac >> FRACTIONBITS]) + shape),
					  buf + x1);
	}
}

/*
=================
=
= PrecacheCinematicSprite
=
=================
*/
void PrecacheCinematicSprite(cinespr_event* sprite)
{
	int i;

	for (i = 0; i < sprite->numframes; i++)
	{
		W_CacheLumpNum(W_GetNumForName(sprite->name) + i, PU_CACHE, Cvt_patch_t,
					   1);
	}
}

/*
=================
=
= DrawPalette
=
=================
*/

void DrawPalette(cine_palevent* event)
{
	byte* pal;

	pal = W_CacheLumpName(event->name, PU_CACHE, CvtNull, 1);
	VH_UpdateScreen();
	CinematicSetPalette(pal);
}

/*
=================
=
= PrecachePalette
=
=================
*/

void PrecachePalette(cine_palevent* event)
{
	W_CacheLumpName(event->name, PU_CACHE, CvtNull, 1);
}

/*
=================
=
= DrawFadeout
=
=================
*/
#define FADEOUTTIME 20

void DrawFadeout(void)
{
	byte opal[768];
	byte npal[768];
	int i, j;

	CinematicGetPalette(&opal[0]);
	for (j = 0; j < FADEOUTTIME; j++)
	{
		for (i = 0; i < 768; i++)
		{
			npal[i] = (opal[i] * (FADEOUTTIME - j - 1)) / FADEOUTTIME;
		}
		WaitVBL();
		CinematicSetPalette(&npal[0]);
		CinematicDelay();
	}
	VL_ClearVideo(0);
	GetCinematicTics();
	GetCinematicTics();
}

/*
=================
=
= DrawBlankScreen
=
=================
*/
void DrawBlankScreen(void)
{
	VL_ClearVideo(0);
}

/*
=================
=
= DrawClearBuffer
=
=================
*/
void DrawClearBuffer(void)
{
	memset((byte*)bufferofs, 0, FRAMEBUFFERWIDTH * FRAMEBUFFERHEIGHT);
}

/*
===============
=
= UpdateCinematicBack
=
===============
*/

bool UpdateCinematicBack(cine_bgevent* back)
{
	back->duration--;

	if (back->duration < 0)
		return false;

	back->currentoffset += back->dx;

	return true;
}

/*
=================
=
= UpdateCinematicSprite
=
=================
*/
bool UpdateCinematicSprite(cinespr_event* sprite)
{
	sprite->duration--;

	if (sprite->duration < 0)
		return false;

	sprite->framedelay--;

	if (sprite->framedelay == 0)
	{
		sprite->frame++;
		if (sprite->frame == sprite->numframes)
			sprite->frame = 0;
		sprite->framedelay = sprite->frametime;
	}

	sprite->x += sprite->dx;
	sprite->y += sprite->dy;
	sprite->scale += sprite->dscale;

	return true;
}

/*
=================
=
= UpdateCinematicEffect
=
=================
*/
bool UpdateCinematicEffect(e_cinefxevent_t type, void* effect)
{
	switch (type)
	{
	case background_noscrolling:
	case background_scrolling:
	case backdrop_scrolling:
	case backdrop_noscrolling:
	case background_multi:
		return UpdateCinematicBack((cine_bgevent*)effect);
		break;
	case sprite_background:
	case sprite_foreground:
		return UpdateCinematicSprite((cinespr_event*)effect);
		break;
	case flic:
		return true;
		break;
	case palette:
	case fadeout:
	case blankscreen:
	case clearbuffer:
		return true;
		break;
	case cinematicend:
		cinematicdone = true;
		return true;
		break;
	}
	return true;
}
/*
=================
=
= DrawCinematicEffect
=
=================
*/
bool DrawCinematicEffect(e_cinefxevent_t type, void* effect)
{
	switch (type)
	{
	case background_noscrolling:
	case background_scrolling:
		DrawCinematicBackground((cine_bgevent*)effect);
		return true;
		break;
	case background_multi:
		DrawCinematicMultiBackground((cine_bgevent*)effect);
		return true;
		break;
	case backdrop_scrolling:
	case backdrop_noscrolling:
		DrawCinematicBackdrop((cine_bgevent*)effect);
		return true;
		break;
	case sprite_background:
	case sprite_foreground:
		DrawCinematicSprite((cinespr_event*)effect);
		return true;
		break;
	case flic:
		DrawFlic((cine_event*)effect);
		return false;
		break;
	case palette:
		DrawPalette((cine_palevent*)effect);
		return false;
		break;
	case fadeout:
		DrawFadeout();
		return false;
		break;
	case blankscreen:
		DrawBlankScreen();
		return false;
		break;
	case clearbuffer:
		DrawClearBuffer();
		return false;
		break;
	case cinematicend:
		return true;
		break;
	}
	return true;
}

/*
=================
=
= PrecacheCinematicEffect
=
=================
*/
void PrecacheCinematicEffect(e_cinefxevent_t type, void* effect)
{
	switch (type)
	{
	case background_noscrolling:
	case background_scrolling:
	case backdrop_scrolling:
	case backdrop_noscrolling:
		PrecacheBack((cine_bgevent*)effect);
		break;
	case sprite_background:
	case sprite_foreground:
		PrecacheCinematicSprite((cinespr_event*)effect);
		break;
	case palette:
		PrecachePalette((cine_palevent*)effect);
		break;
	case flic:
		PrecacheFlic((cine_event*)effect);
		break;
	default:;
	}
}

/*
===============
=
= ProfileDisplay
=
===============
*/

void ProfileDisplay(void)
{
	byte* buf;
	int i;
	byte src[200];
	int width = StretchScreen ? 320 : FRAMEBUFFERWIDTH;

	DrawClearBuffer();

	{
		buf = (byte*)bufferofs;

		for (i = 0; i < width; i++, buf++)
		{
			DrawFilmPost(buf, &src[0], 200);
		}
	}
}

/*
===============
=
= DrawPostPic
=
===============
*/

void DrawPostPic(int lumpnum)
{
	byte* src;
	byte* buf;
	lpic_t* pic;
	int i;
	int plane;
	int height;
	int width = StretchScreen ? 320 : FRAMEBUFFERWIDTH;

	pic = (lpic_t*)W_CacheLumpNum(lumpnum, PU_CACHE, Cvt_lpic_t, 1);

	height = pic->height;

	plane = 0;

	{
		buf = (byte*)bufferofs;

		src = &(pic->data) + (plane * pic->height);

		for (i = 0; i < width; i++, src += pic->height, buf++)
		{
			DrawFilmPost(buf, src, height);
		}
	}
}
