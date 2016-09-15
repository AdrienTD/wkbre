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

struct MapPart
{
	int x, y, w, h; float minhi, maxhi;

	IDirect3DVertexBuffer9 *vbuf;
	IDirect3DIndexBuffer9 *ibuf;

	float *glverts;
	GLushort *glindices;
};

extern int mapwidth, mapheight, mapedge, mapfogcolor; extern float maphiscale;

extern char lastmap[256];

extern float *himap;

extern uint mappartw, mapparth;

void InitMap();
void CloseMap();
void LoadMap(char *filename, int bitoff = 0);
void DrawMap();
float GetHeight(float ipx, float ipy);