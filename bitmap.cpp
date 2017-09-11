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
	if(data[1] && ((data[2]&7) == 1) && (data[7] == 24) && (data[16] == 8))
		bm->form = BMFORMAT_P8;
	else if(((data[2]&7) == 2) && (data[16] == 24))
		bm->form = BMFORMAT_B8G8R8;
	else if(((data[2]&7) == 2) && (data[16] == 32))
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
	if(data[2] & 8) // RLE
	{
		int x = 0, y = (data[17]&32) ? 0 : (bm->h - 1);
		char *c = dp;
		while((y >= 0) || (y < bm->h))
		{
			uchar ua = *(c++);
			uint us = ((uchar)ua & 127) + 1;

			for(int i = 0; i < us; i++)
			{
			 	for(int j = 0; j < e; j++)	
					bm->pix[(y * bm->w + x) * e + j] = c[j];
				x++;
				if(!(ua & 128)) c += e;
				if(x >= bm->w)
				{
					x = 0;
					y += (data[17]&32) ? 1 : (-1);
					if((y < 0) || (y >= bm->h)) goto rleend;
				}
			}
			if(ua & 128) c += e;
		}
	rleend:	;
	}
	else
	{
		if(data[17] & 32)
			for(int y = 0; y < bm->h; y++)
				memcpy(bm->pix + (y * bm->w * e), dp + (y * bm->w * e), bm->w * e);
		else
			for(int y = 0; y < bm->h; y++)
				memcpy(bm->pix + ((bm->h-1-y) * bm->w * e), dp + (y * bm->w * e), bm->w * e);
	}

	return bm;
}

Bitmap *LoadPCX(char *data, int ds)
{
	Bitmap *bm = new Bitmap;
	int nplanes = (uchar)data[65];
	if((uchar)data[3] != 8) ferr("PCX files that don't have 8 bits per channel are not supported.");
	bm->w = *((ushort*)&(data[8])) - *((ushort*)&(data[4])) + 1;
	bm->h = *((ushort*)&(data[10])) - *((ushort*)&(data[6])) + 1;
	bm->form = (nplanes==3) ? BMFORMAT_R8G8B8 : BMFORMAT_P8;
	bm->pix = (uchar*)malloc(bm->w * bm->h * nplanes);
	int pitch = (bm->w & 1) ? (bm->w + 1) : bm->w;

	uchar *f = (uchar*)data + 128, *o = bm->pix; int pp = 0;
	while(pp < (pitch * bm->h * nplanes))
	{
		int a = *f++, c;
		if(a >= 192)
			{c = a & 63; a = *f++;}
		else	c = 1;
		for(; c > 0; c--)
			{if(bm->w & 1)
				if( (pp % (bm->w+1)) == bm->w)	
					{pp++; continue;}
			if(bm->form == BMFORMAT_P8)
			{
				*o++ = a;
			/*
				int oy = pp / pitch;
				int ox = pp % pitch;
				bm->pix[oy*bm->w + ox] = a;
			*/
			}
			else if(bm->form == BMFORMAT_R8G8B8)
			{
				int pl = (pp / pitch) % 3;
				int oy = (pp / pitch) / 3;
				int ox = pp % pitch;
				bm->pix[(oy*bm->w+ox)*3+pl] = a;
			}
			pp++;}
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

void write8(FILE *f, uchar a) {fwrite(&a, 1, 1, f);}
void write16(FILE *f, ushort a) {fwrite(&a, 2, 1, f);}
void write32(FILE *f, uint a) {fwrite(&a, 4, 1, f);}

void WritePCXHeader(FILE *f, int w, int h, int np)
{
	int i;
	write8(f, 10); write8(f, 5); write8(f, 1); write8(f, 8);
	write32(f, 0);
	write16(f, w-1); write16(f, h-1);
	write16(f, w); write16(f, h);
	for(i = 0; i < 12; i++) write32(f, 0);
	write8(f, 0);
	write8(f, np);
	write16(f, w + (w&1));
	write16(f, 1);
	write16(f, 0);
	for(i = 0; i < 14; i++) write32(f, 0);
}

void WritePCXData(FILE *f, uchar *pix, int w, int h, int np)
{
	int y, p, x;
	for(y = 0; y < h; y++)
	for(p = 0; p < np; p++)
	{
		uchar *sl = pix + y*w*np + p;
		uchar *d = sl;
		uchar o; // = *d;
		int cnt = 1;
		//d += np;
		for(x = 0; x < w-1; x++)
		{
			o = *d;
			d += np;
			if((*d != o) || (cnt >= 63))
			{
				if((cnt > 1) || (o >= 0xC0))
					write8(f, 0xC0 | cnt);
				write8(f, o);
				cnt = 1;
			}
			else cnt++;
		}
		if((cnt > 1) || (*d >= 0xC0))
			write8(f, 0xC0 | cnt);
		write8(f, *d);
		if(w & 1) write8(f, 0);
	}
}

void SaveBitmapPCX(Bitmap *bm, char *fname)
{
	FILE *f = fopen(fname, "wb"); if(!f) ferr("Cannot open \"%s\" for writing PCX file.", fname);
	switch(bm->form)
	{
		case BMFORMAT_P8:
			WritePCXHeader(f, bm->w, bm->h, 1);
			WritePCXData(f, bm->pix, bm->w, bm->h, 1);
			write8(f, 12);
			fwrite(bm->pal, 768, 1, f);
			break;
		case BMFORMAT_R8G8B8:
			WritePCXHeader(f, bm->w, bm->h, 3);
			WritePCXData(f, bm->pix, bm->w, bm->h, 3);
			break;
		default:
			ferr("SaveBitmapPCX doesn't support format %i.", bm->form);
	}
	fclose(f);
}
