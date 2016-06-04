// wkbre - WK (Battles) recreated game engine
// Copyright (C) 2015-2016 Adrien Geets
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "global.h"

Bitmap *LoadBitmap(char *fn)
{
	char *d; int s;
	char *e = strrchr(fn, '.');
	LoadFile(fn, &d, &s);
	if(!stricmp(e, ".tga"))
		return LoadTGA(d, s);
	if(!stricmp(e, ".pcx"))
		return LoadPCX(d, s);
	ferr("Unknown bitmap file extension."); return 0;
}

Bitmap *LoadTGA(char *data, int ds)
{
	Bitmap *bm = new Bitmap;
	bm->w = *(ushort*)(data+12);
	bm->h = *(ushort*)(data+14);
	if(data[1] && (data[2] == 1) && (data[7] == 24) && (data[16] == 8))
		bm->form = BMFORMAT_P8;
	else if((data[2] == 2) && (data[16] == 24))
		bm->form = BMFORMAT_B8G8R8;
	else if((data[2] == 2) && (data[16] == 32))
		bm->form = BMFORMAT_B8G8R8A8;
	else
		ferr("Unknown TGA image format.");
	char *dp = data + 18 + (uchar)data[0];

	// 24-bits palette: BB GG RR    (!)
	// 24-bits bitmap:  BB GG RR    (!)
	// 32-bits bitmap:  BB GG RR AA (!)
	if(data[1])
	{
		bm->pal = (uchar*)malloc(768);
		for(int i = 0; i < 256; i++)
		{
			bm->pal[i*3+0] = dp[i*3+2];
			bm->pal[i*3+1] = dp[i*3+1];
			bm->pal[i*3+2] = dp[i*3+0];
		}
		dp += 768;
	} else
		bm->pal = 0;

	int e = data[16] / 8;
	e = ((data[16]&7)?(e+1):e);
	//printf("e = %i\n", e);
	int s = bm->w * bm->h * e;
	bm->pix = (uchar*)malloc(s);
	//memcpy(bm->pix, dp, s);
	for(int y = 0; y < bm->h; y++)
		memcpy(bm->pix + ((bm->h-1-y) * bm->w * e), dp + (y * bm->w * e), bm->w * e);

	return bm;
}

Bitmap *LoadPCX(char *data, int ds)
{
	Bitmap *bm = new Bitmap;
	bm->w = *((ushort*)&(data[8])) - *((ushort*)&(data[4])) + 1;
	bm->h = *((ushort*)&(data[10])) - *((ushort*)&(data[6])) + 1;
	bm->form = BMFORMAT_P8;
	bm->pix = (uchar*)malloc(bm->w * bm->h);
	int p = (bm->w & 1) ? (bm->w + 1) : bm->w;
	//for(int y = 0; y < bm->h; y++)
	//	memcpy(bm->pix + y * bm->w, data + 128 + y * p, bm->w);

	uchar *f = (uchar*)data + 128, *o = bm->pix; int pp = 0;
	while(pp < (bm->w * bm->h))
	{
		int a = *f++, c;
		if(a >= 192)
			{c = a & 63; a = *f++;}
		else	c = 1;
		for(; c > 0; c--)
			{if(bm->w & 1)
				if( ((uint)(o-bm->pix) % (bm->w+1)) == bm->w)	
					continue;
			*o++ = a; pp++;}
	}

	bm->pal = (uchar*)malloc(768);
	memcpy(bm->pal, data + ds - 768, 768);

	return bm;
}

Bitmap *ConvertBitmapToR8G8B8A8(Bitmap *sb)
{
	Bitmap *bm = new Bitmap;
	bm->w = sb->w; bm->h = sb->h;
	bm->form = BMFORMAT_R8G8B8A8;
	int l = bm->w * bm->h;
	bm->pix = (uchar*)malloc(l * 4);
	bm->pal = 0;
	if(sb->form == BMFORMAT_R8G8B8A8)
		memcpy(bm->pix, sb->pix, l * 4);
	else if(sb->form == BMFORMAT_P8)
	{
		for(int i = 0; i < l; i++)
			((uint*)bm->pix)[i] =	CM_RGBA(sb->pal[sb->pix[i]*3+0],
							sb->pal[sb->pix[i]*3+1],
							sb->pal[sb->pix[i]*3+2],
							255);
	}
	else if(sb->form == BMFORMAT_R8G8B8)
	{
		for(int i = 0; i < l; i++)
			((uint*)bm->pix)[i] =	CM_RGBA(sb->pix[i*3+0],
							sb->pix[i*3+1],
							sb->pix[i*3+2],
							255);
	}
	else if(sb->form == BMFORMAT_B8G8R8)
	{
		for(int i = 0; i < l; i++)
			((uint*)bm->pix)[i] =	CM_RGBA(sb->pix[i*3+2],
							sb->pix[i*3+1],
							sb->pix[i*3+0],
							255);
	}
	else if(sb->form == BMFORMAT_B8G8R8A8)
	{
		for(int i = 0; i < l; i++)
			((uint*)bm->pix)[i] =	CM_RGBA(sb->pix[i*4+2],
							sb->pix[i*4+1],
							sb->pix[i*4+0],
							sb->pix[i*4+3]);
	}
	else	ferr("Failed to convert a bitmap to R8G8B8A8");

	return bm;
}

Bitmap *ConvertBitmapToB8G8R8A8(Bitmap *sb)
{
	Bitmap *bm = new Bitmap;
	bm->w = sb->w; bm->h = sb->h;
	bm->form = BMFORMAT_B8G8R8A8;
	int l = bm->w * bm->h;
	bm->pix = (uchar*)malloc(l * 4);
	bm->pal = 0;
	if(sb->form == BMFORMAT_B8G8R8A8)
		memcpy(bm->pix, sb->pix, l * 4);
	else if(sb->form == BMFORMAT_P8)
	{
		for(int i = 0; i < l; i++)
			((uint*)bm->pix)[i] =	CM_BGRA(sb->pal[sb->pix[i]*3+0],
							sb->pal[sb->pix[i]*3+1],
							sb->pal[sb->pix[i]*3+2],
							255);
	}
	else if(sb->form == BMFORMAT_R8G8B8)
	{
		for(int i = 0; i < l; i++)
			((uint*)bm->pix)[i] =	CM_BGRA(sb->pix[i*3+0],
							sb->pix[i*3+1],
							sb->pix[i*3+2],
							255);
	}
	else if(sb->form == BMFORMAT_B8G8R8)
	{
		for(int i = 0; i < l; i++)
			((uint*)bm->pix)[i] =	CM_BGRA(sb->pix[i*3+2],
							sb->pix[i*3+1],
							sb->pix[i*3+0],
							255);
	}
	else if(sb->form == BMFORMAT_R8G8B8A8)
	{
		for(int i = 0; i < l; i++)
			((uint*)bm->pix)[i] =	CM_BGRA(sb->pix[i*4+0],
							sb->pix[i*4+1],
							sb->pix[i*4+2],
							sb->pix[i*4+3]);
	}
	else	ferr("Failed to convert a bitmap to B8G8R8A8");

	return bm;
}

void FreeBitmap(Bitmap *bm)
{
	free(bm->pix); free(bm->pal);
	delete bm;
}

void BitmapBlit32(Bitmap *db, int dx, int dy, Bitmap *sb, int sx, int sy, int sw, int sh)
{
	//printf("BitmapBlit32: dx = %i, dy = %i, sx = %i, sy = %i, sw = %i, sh = %i\n", dx, dy, sx, sy, sw, sh);
	for(int y = sy; y < (sy+sh); y++)
	for(int x = sx; x < (sx+sw); x++)
		((uint*)db->pix)[((y-sy+dy)*db->w+(x-sx+dx))] = ((uint*)sb->pix)[(y*sb->w+x)];
}