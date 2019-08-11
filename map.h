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

struct MapTile;
struct MapTexture;

struct MapPart
{
	int x, y, w, h; float minhi, maxhi;
	DynList<MapTile*> *tpt;

	void *vbuf;
	void *ibuf;

	float *glverts;
	ushort *glindices;
};

struct MapTile
{
	MapTexture *mt;
	uint rot, xflip, zflip;
	uint x, z;
	DynList<MapTile*> *pl; DynListEntry<MapTile*> *ple;
	float waterlev; DynListEntry<Vector3> *lake; uint fullWater;
};

extern int mapwidth, mapheight, mapedge, mapfogcolor; extern float maphiscale;
extern int mapsuncolor; extern Vector3 mapsunvector;
extern DynList<Vector3> maplakes;
extern char mapskytexdir[256];
extern char lastmap[256];

extern float *himap; extern uchar *himap_byte;
extern MapTile *maptiles;

extern uint mappartw, mapparth;

extern bool usemaptexdb;
extern bool showMapGrid;

void InitMap();
void CloseMap();
void LoadMap(char *filename, int bitoff = 0);
void DrawMap();
float GetHeight(float ipx, float ipy);
void DrawLakes();
void FloodfillWater();
void SaveMapSNR(char *filename);
void CreateEmptyMap(int w, int h);
void SaveMapBCM(char *filename);
void DrawTileHighlights();
void CreateMinimap(Bitmap &bhm, int bmpwh = 128, bool drawedge = false);
void DrawSkyBox();