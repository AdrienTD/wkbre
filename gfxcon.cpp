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

int gfxconon = 0;
texture conbgnd; int cbav = 0;
char *conbuf[128]; int cbwp;

void InitGfxConsole()
{
	char bfn[128];
	strcpy(bfn, "Interface\\LOADING SCREEN ?.tga");
	bfn[25] = (rand() % 8) + '1';
	if(FileExists(bfn))
		{conbgnd = GetTexture(bfn, 1); cbav = 1;}
	for(uint i = 0; i < 128; i++)
		if(conbuf[i])
			{delete [] conbuf[i]; conbuf[i] = 0;}
	cbwp = 0;
	gfxconon = 1;
}

void DrawGfxConsoleInCurrentFrame()
{
	if(!gfxconon) return;
	InitRectDrawing();
	if(cbav) SetTexture(0, conbgnd);
	else     NoTexture(0);
	DrawRect(0, 0, scrw, scrh, 0xFF808080);
	for(uint i = 0; i < 128; i++)
		if(conbuf[i])
			DrawFont(0, i * 20, conbuf[i]);
}

void DrawGfxConsole()
{
	if(!gfxconon) return;
	BeginDrawing();
	DrawGfxConsoleInCurrentFrame();
	EndDrawing();
}

void WriteGfxConsole(char *s)
{
	if(!gfxconon) return;
	if(cbwp >= 128) return;
	conbuf[cbwp] = new char[strlen(s)+1];
	strcpy(conbuf[cbwp], s);
	cbwp++;
}

void WriteAndDrawGfxConsole(char *s)
{
	if(!gfxconon) return;
	WriteGfxConsole(s);
	DrawGfxConsole();
}