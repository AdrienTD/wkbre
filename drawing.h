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

void DoDrawing();

void SetRect(int x, int y, int w, int h);
void DrawRect(int x, int y, int w, int h, int c, float u = 0, float v = 0, float o = 1, float p = 1);
void DrawGradientRect(int x, int y, int w, int h, int c0, int c1, int c2, int c3);
void InitRectDrawing();
void BeginDrawing();
void EndDrawing();

IDirect3DVertexShader9 *LoadVertexShader(char *filename);
IDirect3DPixelShader9 *LoadPixelShader(char *filename);

void SetTransformMatrix(Matrix *m);
void SetTexture(uint x, texture t);
void NoTexture(uint x);