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

Matrix idmx = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};

void DrawRect(int x, int y, int w, int h, int c, float u, float v, float o, float p)
{
	renderer->DrawRect(x, y, w, h, c, u, v, o, p);
}

void DrawGradientRect(int x, int y, int w, int h, int c0, int c1, int c2, int c3)
{
	renderer->DrawGradientRect(x, y, w, h, c0, c1, c2, c3);
}

void InitRectDrawing()
{
	renderer->InitRectDrawing();
}

void BeginDrawing()
{
	renderer->BeginDrawing();
}

void EndDrawing()
{
	renderer->EndDrawing();
}

void SetTransformMatrix(Matrix *m)
{
	renderer->SetTransformMatrix(m);
}

void SetTexture(uint x, texture t)
{
	renderer->SetTexture(x, t);
}

void NoTexture(uint x)
{
	renderer->NoTexture(x);
}