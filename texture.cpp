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
	Bitmap *c = bm;
	if(bm->form != BMFORMAT_B8G8R8A8)
		c = ConvertBitmapToB8G8R8A8(bm);

	// D3D9 specific
	IDirect3DTexture9 *dt; D3DLOCKED_RECT lore;
	if(FAILED(ddev->CreateTexture(c->w, c->h, mipmaps, (mipmaps!=1)?D3DUSAGE_AUTOGENMIPMAP:0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &dt, NULL)))
		ferr("Failed to create a D3D9 texture.");
	dt->LockRect(0, &lore, NULL, 0);
	for(int y = 0; y < c->h; y++)
		memcpy( ((char*)(lore.pBits)) + y * lore.Pitch, c->pix + y * c->w * 4, c->w * 4);
	dt->UnlockRect(0);
	texture t; t.dd = dt;

	if(bm->form != BMFORMAT_B8G8R8A8)
		FreeBitmap(c);
	return t;
}

texture LoadTexture(char *fn)
{
	Bitmap *bm;
	if(FileExists(fn))
		bm = LoadBitmap(fn);
	else
		bm = LoadBitmap("netexfb.tga");
	texture t = CreateTexture(bm);
	FreeBitmap(bm);
	return t;
}

texture GetTexture(char *fn, int tc)
{
	int t = alTexFn.find(fn);
	if(t != -1)
		return alTextures[t];

	texture x = LoadTexture(fn);
	alTextures.add(x);
	alTexFn.add(fn);
	return x;
}

void FreeTexture(texture t)
{
	t.dd->Release();
}