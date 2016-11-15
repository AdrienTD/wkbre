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

GrowList<texture> alTextures;
GrowStringList alTexFn;
int TXCenabled = 1;

void ResetTexList()
{
	for(int i = 0; i < alTextures.len; i++)
		FreeTexture(alTextures[i]);
	alTextures.clear(); alTexFn.clear();
}

texture CreateTexture(Bitmap *bm, int mipmaps)
{
	return renderer->CreateTexture(bm, mipmaps);
}

texture LoadTexture(char *fn, int tc, int mipmaps)
{
	Bitmap *bm;
	if(FileExists(fn))
		bm = LoadBitmap(fn);
	else
		bm = LoadBitmap("netexfb.tga");
	texture t = CreateTexture(bm, mipmaps);
	FreeBitmap(bm);
	return t;
}

texture GetTexture(char *fn, int tc, int mipmaps)
{
	int t = alTexFn.find(fn);
	if(t != -1)
		return alTextures[t];

	texture x = LoadTexture(fn, tc, mipmaps);
	alTextures.add(x);
	alTexFn.add(fn);
	return x;
}

void FreeTexture(texture t)
{
	renderer->FreeTexture(t);
}