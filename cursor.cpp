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

int hardwareCursor = 1;
Cursor *currentCursor = 0;
HCURSOR invisibleCursor;

void InitCursor()
{
	if(!hardwareCursor)
	{
		int maskand[1] = {-1};
		int maskxor[1] = {0};
		invisibleCursor = CreateCursor(hInstance, 0, 0, 1, 1, maskand, maskxor);
		SetClassLongPtr(hWindow, GCL_HCURSOR, (LONG_PTR)invisibleCursor);
	}
}

Cursor *LoadCursor(char *str)
{
	Cursor *c = new Cursor;
	Bitmap *b = LoadBitmap(str);
	c->w = b->w; c->h = b->h;
	if(hardwareCursor)
	{
		Bitmap *n = ConvertBitmapToB8G8R8A8(b);
		HBITMAP hb = CreateBitmap(n->w, n->h, 1, 32, n->pix);
		ICONINFO ii = {0, 0, 0, hb, hb};
		c->hc = CreateIconIndirect(&ii);
		FreeBitmap(n);
	}
	else
		c->tx = CreateTexture(b, 1);
	FreeBitmap(b);
	return c;
}

void ChangeCursor(Cursor *c)
{
	currentCursor = c;
	if(hardwareCursor)
	{
		SetClassLongPtr(hWindow, GCL_HCURSOR, (LONG_PTR)currentCursor->hc);
		//SetCursor(c->hc);
	}
}

void DrawCursor()
{
	if(hardwareCursor) return;
	if(!currentCursor) return;
	// InitRectDrawing() must be called before.
	SetTexture(0, currentCursor->tx);
	DrawRect(mouseX, mouseY, currentCursor->w, currentCursor->h);
}