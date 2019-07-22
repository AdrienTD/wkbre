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

// void LoadMapBCM(char *filename);
// void DrawMap();

#include "global.h"

int mapwidth, mapheight, mapedge, mapnverts, mapfogcolor; float maphiscale;
float *himap = 0; uchar *himap_byte = 0;
MapTile *maptiles;
int mapsuncolor; Vector3 mapsunvector;
DynList<Vector3> maplakes;
char mapskytexdir[256];

char lastmap[256];
texture grass, gridtex; RBatch *mapbatch;
bool usemaptexdb = 1;
bool showMapGrid = 0;

GrowList<MapPart> mparts;
uint mappartw = 32, mapparth = 32;
uint npartsw, npartsh;

texture skyboxtex[5];

///// Bit Reading /////
char *bytepos; unsigned int bitpos = 0;
int readbit()
{
	int a;
	if(bitpos >= 8) {bitpos = 0; bytepos++;}
	a = ((*bytepos) & (1<<bitpos))?1:0;
	bitpos++;
	//if(bitpos >= 8) {bitpos = 0; bytepos++;}
	return a;
}
int readnb(int b)
{
	int a = 0, i;
	for(i = 0; i < b; i++)
		a |= readbit() << i;
	return a;
}
///////////////////////

void InitMap()
{
	grass = GetTexture("grass.tga");
	gridtex = GetTexture("grid.tga");

	if(usemaptexdb)
	{
		//mapbatch = renderer->CreateBatch(2*8192, 3*8192);
		//mapbatch = renderer->CreateBatch(2*768, 3*768);
		//mapbatch = renderer->CreateBatch(2*400, 3*400);
		//mapbatch = renderer->CreateBatch(2*1024, 3*1024);
		mapbatch = renderer->CreateBatch(4*1024, 6*1024);
	}
}

void CloseMap()
{
	if(!himap) return;
	free(himap); himap = 0;
	free(himap_byte); himap_byte = 0;
	delete [] maptiles; maptiles = 0;
	maplakes.clear();

	for(int i = 0; i < mparts.len; i++)
	{
		MapPart *p = mparts.getpnt(i);
		renderer->FreeMapPart(p);
		delete [] p->tpt;
	}
	mparts.clear();

	FreeMapTextures();
}

int GetMaxBits(int x)
{
	for(int i = 1; i < 32; i++)
		if((1 << i) > x) // > sign (not >=) intentional!
			return i;
	//assert(0 & (int)"no max bits found");
	return 0;
}

void LoadMapBCM(char *filename, int bitoff)
{
	char *fcnt, *fp; int fsize, strs;

	LoadFile(filename, &fcnt, &fsize);
	fp = fcnt + 12;
	mapwidth = *(int*)fp; fp += 4;
	mapheight = *(int*)fp; fp += 4;
	mapedge = *(int*)fp; fp += 4;
	strs = *(ushort*)fp; fp += 2 + strs;
	maphiscale = *(float*)fp; fp += 4;
	mapsuncolor = (*(uchar*)fp<<16) | ((*(uchar*)(fp+4))<<8) | (*(uchar*)(fp+8)); fp += 12;
	mapsunvector.x = *(float*)fp; mapsunvector.y = *(float*)(fp+4); mapsunvector.z = *(float*)(fp+8); fp += 12;
	mapfogcolor = (*(uchar*)fp<<16) | ((*(uchar*)(fp+4))<<8) | (*(uchar*)(fp+8)); fp += 12;
	strs = *(ushort*)fp / 2; fp += 2;
	if(strs > sizeof(mapskytexdir)) ferr("Sky texture directory path in BCM is too long.");
	for(int i = 0; i < strs; i++)
		{mapskytexdir[i] = *fp; fp += 2;}

	mapnverts = (mapwidth+1)*(mapheight+1);
	himap = (float*)malloc(mapnverts*sizeof(float));
	himap_byte = (uchar*)malloc(mapnverts);
	if(!(himap && himap_byte)) ferr("No mem. left for heightmap.");

	bytepos = fp + 0xC000; bitpos = 0;

	// Lakes
	int mapnlakes = readnb(6);
	for(int i = 0; i < mapnlakes; i++)
	{
		maplakes.add();
		Vector3 *v = &maplakes.last->value; int j;
		j = readnb(32); v->x = *(float*)&j;
		j = readnb(32); v->y = *(float*)&j;
		j = readnb(32); v->z = *(float*)&j;
		readnb(2);
	}

	// Group name table
	int nnames = readnb(16);
	MapTextureGroup **grptable = new MapTextureGroup*[nnames];
	for(int i = 0; i < nnames; i++)
	{
		int n = readnb(8);
		if(!usemaptexdb)
			for(int j = 0; j < n; j++) readnb(8);
		else
		{
			char *a = new char[n+1];
			for(int j = 0; j < n; j++) a[j] = readnb(8);
			a[n] = 0;
			for(int j = 0; j < maptexgroup.len; j++)
				if(!strcmp(a, maptexgroup.getpnt(j)->name))
					{grptable[i] = maptexgroup.getpnt(j); goto grpfnd;}
			ferr("BCM file uses unknown texture group.");
		grpfnd:	delete [] a;
		}
	}

	// ID table
	int nids = readnb(16);
	int *idtable = new int[nids];
	for(int i = 0; i < nids; i++)
		idtable[i] = readnb(32);

	// Tiles
	maptiles = new MapTile[mapwidth*mapheight];
	MapTile *t = maptiles;
	int ngrpbits = GetMaxBits(nnames);
	int nidbits = GetMaxBits(nids);
	//for(int z = mapheight-1; z >= 0; z--)
	for(int z = 0; z < mapheight; z++)
	for(int x = 0; x < mapwidth; x++)
	{
		int tg = readnb(ngrpbits); int ti = readnb(nidbits);
		int tr = readnb(2); int tx = readbit(); int tz = readbit();
		if(!usemaptexdb) continue;
		MapTextureGroup *g = grptable[tg];
		int id = idtable[ti];
		for(int i = 0; i < g->tex->len; i++)
			if(g->tex->getpnt(i)->id == id)
				{t->mt = g->tex->getpnt(i); goto mtfnd;}
		ferr("BCM file uses unknown texture ID.");
	mtfnd:	t->rot = tr; t->xflip = tx; t->zflip = tz;
		t->x = x; t->z = z;
		t++;
	}
	delete [] grptable; delete [] idtable;

	// ?
	int nunk = readnb(32);
	readnb(32); // this should return 0x62
	for(int i = 0; i < nunk; i++)
		{readnb(ngrpbits); readnb(nidbits);}

	// Heightmap
	for(int i = 0; i < mapnverts; i++)
	{
		himap_byte[i] = (uchar)readnb(8);
		himap[i] = (float)himap_byte[i] * maphiscale;
	}

	free(fcnt);
}

void LoadMapSNR(char *filename)
{
	char *fcnt, *fp; int fsize;
	char sfnpcx[256], sfntrn[256];

	LoadFile(filename, &fcnt, &fsize, 1);
	fcnt[fsize] = 0; fp = fcnt;
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!strcmp(word[0], "SCENARIO_DIMENSIONS"))
			{mapwidth = atoi(word[1]); mapheight = atoi(word[2]);}
		else if(!strcmp(word[0], "SCENARIO_EDGE_WIDTH"))
			mapedge = atoi(word[1]);
		else if(!strcmp(word[0], "SCENARIO_HEIGHT_SCALE_FACTOR"))
			maphiscale = atof(word[1]);
		else if(!strcmp(word[0], "SCENARIO_HEIGHTMAP"))
			strcpy_s(sfnpcx, 255, word[1]);
		else if(!strcmp(word[0], "SCENARIO_FOG_COLOUR"))
			mapfogcolor = (atoi(word[1]) << 16) | (atoi(word[2]) << 8) | atoi(word[3]);
		else if(!strcmp(word[0], "SCENARIO_TERRAIN"))
			strcpy_s(sfntrn, 255, word[1]);
		else if(!strcmp(word[0], "SCENARIO_SUN_COLOUR"))
			mapsuncolor = (atoi(word[1]) << 16) | (atoi(word[2]) << 8) | atoi(word[3]);
		else if(!strcmp(word[0], "SCENARIO_SUN_VECTOR"))
			mapsunvector = Vector3(atof(word[1]), atof(word[2]), atof(word[3]));
		else if(!strcmp(word[0], "SCENARIO_SKY_TEXTURES_DIRECTORY"))
			strcpy_s(mapskytexdir, 255, word[1]);
		else if(!strcmp(word[0], "SCENARIO_LAKE"))
			{maplakes.add(); maplakes.last->value = Vector3(atof(word[1]), atof(word[2]), atof(word[3]));}
	}
	free(fcnt);

	// Tiles
	maptiles = new MapTile[mapwidth*mapheight];
	if(!usemaptexdb) goto notiles;
	LoadFile(sfntrn, &fcnt, &fsize, 1);
	fcnt[fsize] = 0; fp = fcnt;
	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		int x = atoi(word[1]) - 1, z = mapheight - atoi(word[3]);
		MapTile *t = maptiles + z * mapwidth + x;
		MapTextureGroup *g; int id = atoi(word[7]);
		for(int i = 0; i < maptexgroup.len; i++)
			if(!strcmp(maptexgroup.getpnt(i)->name, word[5]))
				{g = maptexgroup.getpnt(i); goto tgfnd;}
		ferr("TRN file uses unknown texture group.");
	tgfnd:	for(int i = 0; i < g->tex->len; i++)
			if(g->tex->getpnt(i)->id == id)
				{t->mt = g->tex->getpnt(i); goto idfnd;}
		ferr("TRN file uses unknown texture ID.");
	idfnd:	t->rot = atoi(word[9]);
		t->xflip = atoi(word[11]);
		t->zflip = atoi(word[13]);
		t->x = x; t->z = z;
	}
	free(fcnt);
notiles:
	// Heightmap
	mapnverts = (mapwidth+1)*(mapheight+1);
	himap = (float*)malloc(mapnverts*sizeof(float));
	himap_byte = (uchar*)malloc(mapnverts);
	if(!(himap && himap_byte)) ferr("No mem. left for heightmap.");

	Bitmap *bm = LoadBitmap(sfnpcx);
	if(bm->form != BMFORMAT_P8)
		ferr("SNR heightmap must be an 8 bpp bitmap file!");
	if((bm->w != mapwidth + 1) || (bm->h != mapheight + 1))
		ferr("SNR heightmap bitmap has size (%i, %i), but (%i, %i) is expected.", bm->w, bm->h, mapwidth+1, mapheight+1);
	memcpy(himap_byte, bm->pix, mapnverts);
	for(int i = 0; i < mapnverts; i++)
		himap[i] = (float)himap_byte[i] * maphiscale;
	FreeBitmap(bm);
}

void GetMapVertexNormal(Vector3 *out, int vx, int vz)
{
	Vector3 r(0,0,0);

	float a, b, c, d, h;
	h = himap[vz*(mapwidth+1)+vx];
	if(vz > 0)         a = himap[(vz-1)*(mapwidth+1) +vx  ] - h;
	if(vx > 0)         b = himap[ vz   *(mapwidth+1) +vx-1] - h;
	if(vz < mapheight) c = himap[(vz+1)*(mapwidth+1) +vx  ] - h;
	if(vx < mapwidth)  d = himap[ vz   *(mapwidth+1) +vx+1] - h;

	if(vx > 0 && vz > 0)                r += Vector3( b, 5, -a);
	if(vx > 0 && vz < mapheight)        r += Vector3( b, 5,  c);
	if(vx < mapwidth && vz < mapheight) r += Vector3(-d, 5,  c);
	if(vx < mapwidth && vz > 0)         r += Vector3(-d, 5, -a);

	*out = r.normal();
}

uint GetMapVertexColor(int vx, int vz)
{
	uint rc; Vector3 n, s;
	GetMapVertexNormal(&n, vx, vz);
	s = mapsunvector.normal();
	float dp = n.dot(s);
	//float i = (dp + 1) / 4 + 0.5f;
	float i = (dp + 1) / 2;
	//float i = 1 - acos(dp) / M_PI;
	//float i = (dp <= 0.28f) ? 0.28f : dp;
	//float i = (dp > 0) ? dp : 0;
	//float i = (dp+1) * 3 / 8 + 0.25f;
	if((vx < mapedge) || (vx > mapwidth-mapedge) ||
	   (vz < mapedge) || (vz > mapheight-mapedge) )
		i /= 2.0f;
	uchar b = i * 255;
	rc = 0xFF000000 | (b) | (b<<8) | (b<<16);
	return renderer->ConvertColor(rc);
}

void SetTileWaterLevel(int sx, int sz, float wl, DynListEntry<Vector3> *lake)
{
	MapTile *t = &(maptiles[sz*mapwidth+sx]);
	t->waterlev = wl;
	t->lake = lake;

	if(wl > himap[sz*(mapwidth+1)+sx])
	if(wl > himap[sz*(mapwidth+1)+sx+1])
	if(wl > himap[(sz+1)*(mapwidth+1)+sx])
	if(wl > himap[(sz+1)*(mapwidth+1)+sx+1])
		t->fullWater = 1;
}

bool IsFillableWithWater(int sx, int sz, float wl)
{
	if((sx < 0) || (sx >= mapwidth) || (sz < 0) || (sz >= mapheight)) return 0;
	MapTile *t = &(maptiles[sz*mapwidth+sx]);
	if(t->lake) if(wl <= t->waterlev)
		return 0;
	if(wl <= himap[sz*(mapwidth+1)+sx])
	if(wl <= himap[sz*(mapwidth+1)+sx+1])
	if(wl <= himap[(sz+1)*(mapwidth+1)+sx])
	if(wl <= himap[(sz+1)*(mapwidth+1)+sx+1])
		return 0;
	return 1;
}

void FillLineWithWater(int sx, int sz, float wl, DynListEntry<Vector3> *lake)
{
	//if((sx < 0) || (sx >= mapwidth) || (sz < 0) || (sz >= mapheight)) return;
	if(!IsFillableWithWater(sx, sz, wl)) return;
	SetTileWaterLevel(sx, sz, wl, lake);

	// 1. Fill the line.
	int x, xmin, xmax;
	for(x = sx - 1; IsFillableWithWater(x, sz, wl); x--)
		SetTileWaterLevel(x, sz, wl, lake);
	xmin = x + 1;
	for(x = sx + 1; IsFillableWithWater(x, sz, wl); x++)
		SetTileWaterLevel(x, sz, wl, lake);
	xmax = x - 1;

	// 2. Check above and below the line to fill additional lines.
	for(x = xmin; x <= xmax; x++)
	{
		FillLineWithWater(x, sz-1, wl, lake);
		FillLineWithWater(x, sz+1, wl, lake);
	}
}

void FloodfillWater()
{
	// Reset water height for every tile
	for(int i = 0; i < mapwidth*mapheight; i++)
	{
		maptiles[i].waterlev = -1;
		maptiles[i].lake = 0;
		maptiles[i].fullWater = 0;
	}

	for(DynListEntry<Vector3> *e = maplakes.first; e; e = e->next)
	{
		Vector3 *lake = &e->value;
		FillLineWithWater(lake->x / 5, mapheight - lake->z / 5, lake->y, e);
	}
}

void CreateMapPart(int x, int y, int w, int h)
{
	MapPart *p = mparts.addp();
	p->x = x; p->y = y; p->w = w; p->h = h;
	p->minhi = 255*maphiscale; p->maxhi = 0;

	// Determinate minimum and maximum height.
	for(int b = 0; b < h+1; b++)
	for(int a = 0; a < w+1; a++)
	{
		float m = himap[(b+y)*(mapwidth+1)+(a+x)];
		if(m > p->maxhi) p->maxhi = m;
		if(m < p->minhi) p->minhi = m;
	}

if(usemaptexdb)
{
	// Group tiles in corresponding texture list
	p->tpt = new DynList<MapTile*>[maptexfilenames.len];
	for(int b = 0; b < h; b++)
	for(int a = 0; a < w; a++)
	{
		MapTile *t = &maptiles[(b+y)*mapwidth+a+x];
		p->tpt[t->mt->tfid].add(t);

		t->pl = p->tpt;
		t->ple = p->tpt[t->mt->tfid].last;
	}
}
	renderer->CreateMapPart(p, x, y, w, h);
}

void InitMapParts()
{
	int mqx = mapwidth / mappartw, mrx = mapwidth % mappartw;
	int mqy = mapheight / mapparth, mry = mapheight % mapparth;
	npartsw = mqx + (mrx ? 1 : 0);
	npartsh = mqy + (mry ? 1 : 0);
	for(int y = 0; y < mqy; y++)
	{
		for(int x = 0; x < mqx; x++)
			CreateMapPart(x*mappartw, y*mapparth, mappartw, mapparth);
		if(mrx)
			CreateMapPart(mqx*mappartw, y*mapparth, mrx, mapparth);
	}
	if(mry)
	{
		for(int x = 0; x < mqx; x++)
			CreateMapPart(x*mappartw, mqy*mapparth, mappartw, mry);
		if(mrx)
			CreateMapPart(mqx*mappartw, mqy*mapparth, mrx, mry);
	}
}

extern char newmapname[128];

void LoadMap(char *filename, int bitoff)
{
	if(himap) CloseMap();

	char *nwp = strrchr(filename, '\\');
	if(!nwp) nwp = filename; else nwp++;
	if(usemaptexdb)
	{
		char ln[512];
		strcpy(ln, "Maps\\Map_Textures\\");
		strcat(ln, nwp);
		strcpy(strrchr(ln, '.')+1, "lttt");
		//printf("LTTT filename is: %s\n", ln);
		if(FileExists(ln)) LoadLTTT(ln);
		LoadMapTextures();
		FreeLTTT();
	}

	strcpy_s(lastmap, 255, filename);
	strcpy_s(newmapname, 127, nwp);
	char *nmne = strrchr(newmapname, '.');
	if(nmne) *nmne = 0;
	char *fe = strrchr(filename, '.');
	if((tolower(fe[1]) == 'b') && (tolower(fe[2]) == 'c') && (tolower(fe[3]) == 'm'))
		LoadMapBCM(filename, bitoff);
	else	LoadMapSNR(filename);

	InitMapParts();
	FloodfillWater();

	// Load skybox textures
	static char* skyfilenames[5] = { "sky_front.tga", "sky_right.tga", "sky_back.tga", "sky_left.tga", "sky_top.tga" };
	char stn[512];
	strcpy(stn, mapskytexdir);
	char *pp = stn + strlen(stn);
	for (int i = 0; i < 5; i++) {
		strcpy(pp, skyfilenames[i]);
		printf(stn);
		skyboxtex[i] = LoadTexture(stn, 1, 1);
	}
}

void DrawPart(MapPart *p)
{
	renderer->DrawPart(p);
}

bool IsPartInFrontOfCam(int px, int py)
{
	Vector3 v1((px+0)*mappartw, 0, (py+0)*mapparth),
		v2((px+1)*mappartw, 0, (py+0)*mapparth),
		v3((px+1)*mappartw, 0, (py+1)*mapparth),
		v4((px+0)*mappartw, 0, (py+1)*mapparth),
		t1, t2, t3, t4;
	//Matrix m = mWorld * matView;
	TransformVector3(&t1, &v1, &matrix);
	TransformVector3(&t2, &v2, &matrix);
	TransformVector3(&t3, &v3, &matrix);
	TransformVector3(&t4, &v4, &matrix);
	return (t1.z > 0) || (t2.z > 0) || (t3.z > 0) || (t4.z > 0);
}

bool IsTPInScreen(Vector3 &v)
{
	Vector3 t;
	TransformVector3(&t, &v, &matrix);
	t.x /= t.z; t.y /= t.z;
	if(abs(t.x) >= 1.2) return 0;
	if(abs(t.y) >= 1.15) return 0;
	if(t.z <= -1.5 || t.z >= farzvalue) return 0;
	return 1;
}

bool IsTileInScreen(MapTile *c)
{
	Vector3 v(c->x, himap[c->z*(mapwidth+1)+c->x], c->z);
	if(IsTPInScreen(v)) return 1;
	v = Vector3(c->x+1, himap[c->z*(mapwidth+1)+c->x+1], c->z);
	if(IsTPInScreen(v)) return 1;
	v = Vector3(c->x+1, himap[(c->z+1)*(mapwidth+1)+c->x+1], c->z+1);
	if(IsTPInScreen(v)) return 1;
	v = Vector3(c->x, himap[(c->z+1)*(mapwidth+1)+c->x], c->z+1);
	if(IsTPInScreen(v)) return 1;
	return 0;
}

bool IsWaterTileInScreen(MapTile *c)
{
	Vector3 v(c->x, c->waterlev, c->z);
	if(IsTPInScreen(v)) return 1;
	v = Vector3(c->x+1, c->waterlev, c->z);
	if(IsTPInScreen(v)) return 1;
	v = Vector3(c->x+1, c->waterlev, c->z+1);
	if(IsTPInScreen(v)) return 1;
	v = Vector3(c->x, c->waterlev, c->z+1);
	if(IsTPInScreen(v)) return 1;
	return 0;
}

void CheckAndDrawPart(int px, int py)
{
	if((px >= 0) && (px < npartsw) && (py >= 0) && (py < npartsh))
		if(IsPartInFrontOfCam(px, py))
			DrawPart(mparts.getpnt(py*npartsw+px));
}

void DrawMap()
{
	renderer->BeginMapDrawing();

	int cx = (int)camerapos.x / 5 + mapedge, cy = mapheight-2*mapedge-((int)camerapos.z / 5) + mapedge;

	int px = cx / (int)mappartw, py = cy / (int)mapparth;

	int sw = (farzvalue / (mappartw*5)) + 1;
	int sh = (farzvalue / (mapparth*5)) + 1;

if(!usemaptexdb)
{
	SetTexture(0, showMapGrid ? gridtex : grass);
	for(int dy = -sh; dy <= sh; dy++)
	for(int dx = -sw; dx <= sw; dx++)
		CheckAndDrawPart(px + dx, py + dy);
}
else
{
	renderer->BeginBatchDrawing();
	mapbatch->begin();
	if(showMapGrid) SetTexture(0, gridtex);

	for(int i = 0; i < maptexfilenames.len; i++)
	{int tns = 1;
	for(int dy = -sh; dy <= sh; dy++)
	for(int dx = -sw; dx <= sw; dx++)
	{
		int tx = px + dx, ty = py + dy;
		if(!((tx >= 0) && (tx < npartsw) && (ty >= 0) && (ty < npartsh)))
			continue;
		if(!IsPartInFrontOfCam(tx, ty))
			continue;
		MapPart *mp = mparts.getpnt(ty*npartsw+tx);
		for(DynListEntry<MapTile*> *e = mp->tpt[i].first; e; e = e->next)
		{
				MapTile *c = e->value;
				if(!IsTileInScreen(c)) continue;
				MapTexture *t = c->mt;
				if(tns)
				{
					if(!showMapGrid)
						SetTexture(0, t->t);
					tns = 0;
				}

				batchVertex *bv; ushort *bi; uint fi;
				mapbatch->next(4, 6, &bv, &bi, &fi);
				int a = c->rot, f = ((c->zflip&1)?1:0) ^ ((c->xflip&1)?3:0);
			/*
				int difcol = -1;
				if((c->x < mapedge) || (c->x >= mapwidth-mapedge) ||
				   (c->z < mapedge) || (c->z >= mapwidth-mapedge) )
					difcol = 0xFF808080;
				difcol = renderer->ConvertColor(difcol);
			*/

				float tu[4], tv[4];

				tu[((0^f)+a)&3] = (t->x+0.5f) / 256.f;		tv[((0^f)+a)&3] = (t->y+0.5f) / 256.f;
				tu[((1^f)+a)&3] = (t->x+t->w-0.5f) / 256.f;	tv[((1^f)+a)&3] = (t->y+0.5f) / 256.f;
				tu[((2^f)+a)&3] = (t->x+t->w-0.5f) / 256.f;	tv[((2^f)+a)&3] = (t->y+t->h-0.5f) / 256.f;
				tu[((3^f)+a)&3] = (t->x+0.5f) / 256.f;		tv[((3^f)+a)&3] = (t->y+t->h-0.5f) / 256.f;

				bv[0].x = c->x;
				bv[0].y = himap[(c->z+0)*(mapwidth+1)+c->x+0];
				bv[0].z = c->z;
				bv[0].color = GetMapVertexColor(c->x, c->z); //difcol;
				if(showMapGrid) {bv[0].u = 0; bv[0].v = 0;}
				else {bv[0].u = tu[0]; bv[0].v = tv[0];}

				bv[1].x = c->x + 1;
				bv[1].y = himap[(c->z+0)*(mapwidth+1)+c->x+1];
				bv[1].z = c->z;
				bv[1].color = GetMapVertexColor(c->x+1, c->z); //difcol;
				if(showMapGrid) {bv[1].u = 1; bv[1].v = 0;}
				else {bv[1].u = tu[1]; bv[1].v = tv[1];}

				bv[2].x = c->x + 1;
				bv[2].y = himap[(c->z+1)*(mapwidth+1)+c->x+1];
				bv[2].z = c->z + 1;
				bv[2].color = GetMapVertexColor(c->x+1, c->z+1); //difcol;
				if(showMapGrid) {bv[2].u = 1; bv[2].v = 1;}
				else {bv[2].u = tu[2]; bv[2].v = tv[2];}

				bv[3].x = c->x;
				bv[3].y = himap[(c->z+1)*(mapwidth+1)+c->x+0];
				bv[3].z = c->z + 1;
				bv[3].color = GetMapVertexColor(c->x, c->z+1); //difcol;
				if(showMapGrid) {bv[3].u = 0; bv[3].v = 1;}
				else {bv[3].u = tu[3]; bv[3].v = tv[3];}

				bi[0] = fi+0; bi[1] = fi+3; bi[2] = fi+1;
				bi[3] = fi+1; bi[4] = fi+3; bi[5] = fi+2;
		}
	} mapbatch->flush();}
	mapbatch->end();
}
}

float GetHeight(float ipx, float ipy)
{
	float x = mapedge + ipx / 5.0f, y = mapheight - (mapedge + ipy / 5.0f);
	if(x < 0) x = 0; if(x >= mapwidth) x = mapwidth - 0.001f;
	if(y < 0) y = 0; if(y >= mapheight) y = mapheight - 0.001f;
	float h1 = himap[(int)y*(mapwidth+1)+(int)x], h2 = himap[(int)y*(mapwidth+1)+(int)x+1];
	float h3 = himap[((int)y+1)*(mapwidth+1)+(int)x], h4 = himap[((int)y+1)*(mapwidth+1)+(int)x+1];
	int rx = x; int ry = y;
	float qx = x - rx, qy = y - ry;
	float h5 = h1 * (1.0f-qx) + h2 * qx;
	float h6 = h3 * (1.0f-qx) + h4 * qx;
	float hr = h5 * (1.0f-qy) + h6 * qy;
	return hr;
}

int lakedifcolor = 0x80FFFFFF;

void DrawLakes()
{
	if(!usemaptexdb) return;

	renderer->BeginLakeDrawing();

	int cx = (int)camerapos.x / 5 + mapedge, cy = mapheight-2*mapedge-((int)camerapos.z / 5) + mapedge;

	int px = cx / (int)mappartw, py = cy / (int)mapparth;

	int sw = (farzvalue / (mappartw*5)) + 1;
	int sh = (farzvalue / (mapparth*5)) + 1;


	renderer->BeginBatchDrawing();
	mapbatch->begin();
	//SetTexture(0, gridtex);
	NoTexture(0);

	lakedifcolor = renderer->ConvertColor((mapfogcolor & 0xFFFFFF) | 0x80000000);

	for(int dy = -sh; dy <= sh; dy++)
	for(int dx = -sw; dx <= sw; dx++)
	{
		int tx = px + dx, ty = py + dy;
		if(!((tx >= 0) && (tx < npartsw) && (ty >= 0) && (ty < npartsh)))
			continue;
		if(!IsPartInFrontOfCam(tx, ty))
			continue;
		MapPart *mp = mparts.getpnt(ty*npartsw+tx);
		for(int i = 0; i < maptexfilenames.len; i++)
		for(DynListEntry<MapTile*> *e = mp->tpt[i].first; e; e = e->next)
		{
			MapTile *c = e->value;
			if(!c->lake) continue;
			if(!IsWaterTileInScreen(c)) continue;

			batchVertex *bv; ushort *bi; uint fi;
			mapbatch->next(4, 6, &bv, &bi, &fi);

			bv[0].x = c->x;
			bv[0].y = c->waterlev;
			bv[0].z = c->z;
			bv[0].color = lakedifcolor;
			bv[0].u = 0; bv[0].v = 0;

			bv[1].x = c->x + 1;
			bv[1].y = c->waterlev;
			bv[1].z = c->z;
			bv[1].color = lakedifcolor;
			bv[1].u = 1; bv[1].v = 0;

			bv[2].x = c->x + 1;
			bv[2].y = c->waterlev;
			bv[2].z = c->z + 1;
			bv[2].color = lakedifcolor;
			bv[2].u = 1; bv[2].v = 1;

			bv[3].x = c->x;
			bv[3].y = c->waterlev;
			bv[3].z = c->z + 1;
			bv[3].color = lakedifcolor;
			bv[3].u = 0; bv[3].v = 1;

			bi[0] = fi+0; bi[1] = fi+3; bi[2] = fi+1;
			bi[3] = fi+1; bi[4] = fi+3; bi[5] = fi+2;
		}
	}
	mapbatch->flush();
	mapbatch->end();
}

void CreateMinimap(Bitmap &bhm, int bmpwh, bool drawedge)
{
	bhm.w = bhm.h = bmpwh; bhm.form = BMFORMAT_R8G8B8;
	bhm.pix = (uchar*)malloc(bmpwh*bmpwh*3); bhm.pal = 0;
	memset(bhm.pix, 0, bmpwh*bmpwh*3);
	int ltw = drawedge ? mapwidth : (mapwidth - 2*mapedge);   int lvw = ltw + 1;
	int lth = drawedge ? mapheight : (mapheight - 2*mapedge); int lvh = lth + 1;
	int mmw, mmh;
	if(ltw > lth) {mmw = bmpwh; mmh = lth * bmpwh / ltw;}
	else          {mmh = bmpwh; mmw = ltw * bmpwh / lth;}
	int sx = bmpwh/2-mmw/2, sy = bmpwh/2-mmh/2;
	int mmedge = drawedge ? 0 : mapedge;
	for(int y = sy; y < sy+mmh; y++)
	for(int x = sx; x < sx+mmw; x++)
	{
		int v = ((y-sy) * lvh / mmh + mmedge) * (mapwidth+1) + (x-sx) * lvw / mmw + mmedge;
		int t = ((y-sy) * lth / mmh + mmedge) * mapwidth     + (x-sx) * ltw / mmw + mmedge;
		bhm.pix[3*(y*bmpwh+x)+0] = 0;
		bhm.pix[3*(y*bmpwh+x)+1] = himap_byte[v] * 3 / 4 + 64;
		bhm.pix[3*(y*bmpwh+x)+2] = maptiles[t].lake ? 255 /*bhm.pix[3*(y*bmpwh+x)+1]*/ : 0;
	}
}

void SaveMapSNR(char *filename)
{
	char tbuf[512];
	char *fnwoext = strdup(filename);
	char *ext = strrchr(fnwoext, '.');
	if(ext) *ext = 0;

	char *bs = strrchr(fnwoext, '\\'), *fs = strrchr(fnwoext, '/');
	char *namewop;
	if((bs) && (bs > fs)) namewop = bs + 1;
	else if(fs) namewop = fs + 1;
	else namewop = fnwoext;

	FILE *fsnr = fopen(filename, "w"); if(!fsnr) ferr("Cannot open \"%s\" for writing scenario.", filename);
	fprintf(fsnr, "SCENARIO_VERSION 4.00\n");
	fprintf(fsnr, "SCENARIO_DIMENSIONS %u %u\n", mapwidth, mapheight);
	fprintf(fsnr, "SCENARIO_EDGE_WIDTH %u\n", mapedge);
	fprintf(fsnr, "SCENARIO_TEXTURE_DATABASE \"Maps\\Map_Textures\\All_Textures.dat\"\n");
	fprintf(fsnr, "SCENARIO_TERRAIN \"Maps\\%s\\%s.trn\"\n", namewop, namewop);
	fprintf(fsnr, "SCENARIO_HEIGHTMAP \"Maps\\%s\\%s_heightmap.pcx\"\n", namewop, namewop);
	fprintf(fsnr, "SCENARIO_HEIGHT_SCALE_FACTOR %f\n", maphiscale);
	fprintf(fsnr, "SCENARIO_SUN_COLOUR %u %u %u\n", (mapsuncolor>>16)&255, (mapsuncolor>>8)&255, mapsuncolor&255);
	fprintf(fsnr, "SCENARIO_SUN_VECTOR %f %f %f\n", mapsunvector.x, mapsunvector.y, mapsunvector.z);
	fprintf(fsnr, "SCENARIO_FOG_COLOUR %u %u %u\n", (mapfogcolor>>16)&255, (mapfogcolor>>8)&255, mapfogcolor&255);
	fprintf(fsnr, "SCENARIO_SKY_TEXTURES_DIRECTORY \"%s\"\n", mapskytexdir);
	fprintf(fsnr, "SCENARIO_MINIMAP \"Maps\\%s\\%s_minimap.pcx\"\n", namewop, namewop);
	for(DynListEntry<Vector3> *e = maplakes.first; e; e = e->next)
	{
		Vector3 *l = &e->value;
		fprintf(fsnr, "SCENARIO_LAKE %f %f %f 0.0000\n", l->x, l->y, l->z);
	}
	fclose(fsnr);

	sprintf(tbuf, "%s.trn", fnwoext);
	FILE *ftrn = fopen(tbuf, "w"); if(!ftrn) ferr("Cannot open \"%s\" for writing terrain.", tbuf);
	for(int i = 0; i < mapwidth*mapheight; i++)
	{
		MapTile *t = &(maptiles[i]);
		fprintf(ftrn, "X\t%u\tZ\t%u\tGROUP\t\"%s\"\tID\t%i\tROTATION\t%u\tXFLIP\t%u\tZFLIP\t%u\n", 
			t->x + 1, mapheight - t->z, t->mt->grp->name, t->mt->id, t->rot, t->xflip, t->zflip);
	}
	fclose(ftrn);

	Bitmap bhm;
	bhm.w = mapwidth + 1; bhm.h = mapheight + 1; bhm.form = BMFORMAT_P8;
	bhm.pix = himap_byte;
	bhm.pal = (uchar*)malloc(768);
	for(int i = 0; i < 256; i++)
		for(int j = 0; j < 3; j++)
			bhm.pal[i*3+j] = i;
	sprintf(tbuf, "%s_heightmap.pcx", fnwoext);
	SaveBitmapPCX(&bhm, tbuf);
	free(bhm.pal);

	CreateMinimap(bhm);
	sprintf(tbuf, "%s_minimap.pcx", fnwoext);
	SaveBitmapPCX(&bhm, tbuf);
	free(bhm.pix);

	free(fnwoext);
}

void CreateEmptyMap(int w, int h)
{
	if(himap) CloseMap();
	lastmap[0] = 0;

	if(usemaptexdb) LoadMapTextures();

	mapwidth = w; mapheight = h;
	mapnverts = (mapwidth+1)*(mapheight+1);
	mapedge = 0;
	mapfogcolor = 0x9FC5E6;
	maphiscale = 0.25f;
	mapsuncolor = 0xFFFFFF; mapsunvector = Vector3(1,1,1);
	strcpy(mapskytexdir, "Maps\\Sky Textures\\Bright Cloudy Blue Sky\\");

	himap = (float*)malloc(mapnverts*sizeof(float));
	himap_byte = (uchar*)malloc(mapnverts);
	if(!(himap && himap_byte)) ferr("No mem. left for heightmap.");
	memset(himap, 0, mapnverts*sizeof(float));
	memset(himap_byte, 0, mapnverts);

if(usemaptexdb)
{
	maptiles = new MapTile[mapwidth*mapheight];
	MapTile *t = maptiles;
	MapTexture *mtx = maptexgroup.getpnt(0)->tex->getpnt(0);
	for(int z = 0; z < mapheight; z++)
	for(int x = 0; x < mapwidth; x++)
	{
		t->mt = mtx;
		t->rot = 0; t->xflip = 0; t->zflip = 0;
		t->x = x; t->z = z;
		t++;
	}
}

	InitMapParts();
	FloodfillWater();
}

int bwcurbyte = 0; int bwnextbit = 0;
FILE *bwfile;

void writebit(bool b)
{
	if(b) bwcurbyte |= 1 << bwnextbit;
	bwnextbit++;
	if(bwnextbit == 8)
	{
		fputc(bwcurbyte, bwfile);
		bwcurbyte = 0;
		bwnextbit = 0;
	}
}

void writenb(int nb, int a)
{
	for(int i = 0; i < nb; i++)
		writebit(a & (1 << i));
}

void writefloat(float a) {writenb(32, *(int*)&a);}

void awrite8(FILE *f, uchar c)     {fwrite(&c, 1, 1, f);}
void awrite16(FILE *f, ushort c)   {fwrite(&c, 2, 1, f);}
void awrite32(FILE *f, uint c)     {fwrite(&c, 4, 1, f);}
void awritefloat(FILE *f, float c) {fwrite(&c, 4, 1, f);}
void awritestr(FILE *f, char *s)
{
	int l = strlen(s);
	awrite16(f, l*2 + 2);
	for(int i = 0; i <= l; i++)
		awrite16(f, s[i]);
}

void SaveMapBCM(char *filename)
{
	FILE *bcm = fopen(filename, "wb"); if(!bcm) ferr("Could not open \"%s\" for writing BCM.", filename);
	fputs("TEQUILA", bcm); fputc(0, bcm); // To consume with moderation.
	awrite32(bcm, 0x3F000000);
	awrite32(bcm, mapwidth); awrite32(bcm, mapheight); awrite32(bcm, mapedge);
	awritestr(bcm, "Maps\\Map_Textures\\All_Textures.dat");
	awritefloat(bcm, maphiscale);
	for(int i = 2; i >= 0; i--) awrite32(bcm, (mapsuncolor >> (i*8)) & 255);
	awritefloat(bcm, mapsunvector.x); awritefloat(bcm, mapsunvector.y); awritefloat(bcm, mapsunvector.z);
	for(int i = 2; i >= 0; i--) awrite32(bcm, (mapfogcolor >> (i*8)) & 255);
	awritestr(bcm, mapskytexdir);

	// Minimap
	Bitmap b;
	CreateMinimap(b);
	fwrite(b.pix, 128*128*3, 1, bcm);
	free(b.pix);

	bwcurbyte = 0; bwnextbit = 0; bwfile = bcm;

	// Lakes
	if (maplakes.len >= 64)
		MessageBox(hWindow, "The terrain has 64 or more lakes, but the BCM format only supports up to 63 lakes.\nOnly the first 63 lakes will be kept and saved.", appName, 48);
	writenb(6, (maplakes.len >= 63) ? 63 : maplakes.len);
	int nlkw = 0;
	for(DynListEntry<Vector3> *e = maplakes.first; e; e = e->next)
	{
		writefloat(e->value.x); writefloat(e->value.y); writefloat(e->value.z);
		writenb(2, 0);
		if (++nlkw >= 63) break;
	}

	// Make a list of used groups, IDs, and textures.
	GrowList<MapTextureGroup*> usedmtg;
	GrowList<int> usedids;
	GrowList<MapTexture*> usedtex;
	for(int i = 0; i < mapwidth*mapheight; i++)
	{
		MapTile *t = &(maptiles[i]);
		if(!usedmtg.has(t->mt->grp))
			usedmtg.add(t->mt->grp);
		if(!usedids.has(t->mt->id))
			usedids.add(t->mt->id);
		if(!usedtex.has(t->mt))
			usedtex.add(t->mt);
	}

	// List of used texture groups
	writenb(16, usedmtg.len);
	for(int i = 0; i < usedmtg.len; i++)
	{
		MapTextureGroup *g = usedmtg[i];
		int l = strlen(g->name);
		writenb(8, l);
		for(int i = 0; i < l; i++)
			writenb(8, g->name[i]);
	}

	// List of used texture IDs
	writenb(16, usedids.len);
	for(int i = 0; i < usedids.len; i++)
		writenb(32, usedids[i]);

	// Determine the number of bits required for texture groups and IDs
	int ngrpbits = GetMaxBits(usedmtg.len);
	int nidbits = GetMaxBits(usedids.len);

	// Tiles
	MapTile *t = maptiles;
	for(int z = 0; z < mapheight; z++)
	for(int x = 0; x < mapwidth; x++)
	{
		writenb(ngrpbits, usedmtg.find(t->mt->grp));
		writenb(nidbits, usedids.find(t->mt->id));
		writenb(2, t->rot);
		writenb(1, t->xflip);
		writenb(1, t->zflip);
		t++;
	}

	// Sort the used textures by group
	GrowList<MapTexture*> sortedtex;
	for(int i = 0; i < usedmtg.len; i++)
	{
		MapTextureGroup *g = usedmtg[i];
		for(int j = 0; j < usedtex.len; j++)
		{
			MapTexture *t = usedtex[j];
			if(t->grp == g)
				sortedtex.add(t);
		}
	}
	assert(usedtex.len == sortedtex.len);

	// Used textures list
	writenb(32, sortedtex.len); writenb(32, 0x62);
	for(int i = 0; i < sortedtex.len; i++)
	{
		writenb(ngrpbits, usedmtg.find(sortedtex[i]->grp));
		writenb(nidbits, usedids.find(sortedtex[i]->id));
	}

	// Heightmap
	for(int i = 0; i < (mapwidth+1)*(mapheight+1); i++)
		writenb(8, himap_byte[i]);

	if(bwnextbit != 0) fputc(bwcurbyte, bcm);
	fclose(bcm);
	bwfile = 0;
}

extern int brushsize, brushshape, mousetool, nmc_size;

bool IsTileCorcernedByBrush(int tx, int tz)
{
	int cx = mapstdownpos.x / 5 + mapedge, cz = mapheight - (mapstdownpos.z / 5 + mapedge);
	int m = brushsize/2;

	switch(mousetool)
	{
		case 1:
		case 2:
		case 6:
		case 10:
			if(tz >= cz-m && tz < cz-m+brushsize)
			if(tx >= cx-m && tx < cx-m+brushsize)
			{
				if(brushshape == 1)
					if( ((tx-cx)*(tx-cx) + (tz-cz)*(tz-cz)) > (m*m))
						return 0;
				return 1;
			}
			return 0;
		case 7:
		{
			int ctlx = cx-1-nmc_size-1, ctlz = cz-1-nmc_size-1;
			int cbrx = cx+1+nmc_size+1, cbrz = cz+1+nmc_size+1;
			if(tz >= ctlz && tz <= cbrz)
				if(tx == ctlx || tx == cbrx)
					return 1;
			if(tx >= ctlx && tx <= cbrx)
				if(tz == ctlz || tz == cbrz)
					return 1;
			return 0;
		}
		case 47:
			if(cx == tx && cz == tz) return 1;
			return 0;
	}
	return 0;
}

//bool smallTileHeightlights = 1;

void DrawTileHighlights()
{
	if(!usemaptexdb) return;

	if(!(mousetool == 1 || mousetool == 2 || mousetool == 6 ||
	     mousetool == 7 || mousetool == 47 || mousetool == 10))
		return;

	renderer->BeginLakeDrawing();

	int cx = (int)camerapos.x / 5 + mapedge, cy = mapheight-2*mapedge-((int)camerapos.z / 5) + mapedge;

	int px = cx / (int)mappartw, py = cy / (int)mapparth;

	int sw = (farzvalue / (mappartw*5)) + 1;
	int sh = (farzvalue / (mapparth*5)) + 1;

	renderer->BeginBatchDrawing();
	mapbatch->begin();
	//SetTexture(0, gridtex);
	NoTexture(0);

	int difcolor = renderer->ConvertColor(0x80FFFFFF);

	for(int dy = -sh; dy <= sh; dy++)
	for(int dx = -sw; dx <= sw; dx++)
	{
		int tx = px + dx, ty = py + dy;
		if(!((tx >= 0) && (tx < npartsw) && (ty >= 0) && (ty < npartsh)))
			continue;
		if(!IsPartInFrontOfCam(tx, ty))
			continue;
		MapPart *mp = mparts.getpnt(ty*npartsw+tx);
		for(int i = 0; i < maptexfilenames.len; i++)
		for(DynListEntry<MapTile*> *e = mp->tpt[i].first; e; e = e->next)
		{
			MapTile *c = e->value;
			if(!IsTileCorcernedByBrush(c->x, c->z)) continue;
			if(!IsTileInScreen(c)) continue;

			batchVertex *bv; ushort *bi; uint fi;
			mapbatch->next(4, 6, &bv, &bi, &fi);

			if(!(mousetool == 2 || mousetool == 6 || mousetool == 10))
			{
				bv[0].x = c->x;
				bv[0].y = himap[(c->z+0)*(mapwidth+1)+c->x+0];
				bv[0].z = c->z;
				bv[0].color = difcolor;
				bv[0].u = 0; bv[0].v = 0;

				bv[1].x = c->x + 1;
				bv[1].y = himap[(c->z+0)*(mapwidth+1)+c->x+1];
				bv[1].z = c->z;
				bv[1].color = difcolor;
				bv[1].u = 1; bv[1].v = 0;

				bv[2].x = c->x + 1;
				bv[2].y = himap[(c->z+1)*(mapwidth+1)+c->x+1];
				bv[2].z = c->z + 1;
				bv[2].color = difcolor;
				bv[2].u = 1; bv[2].v = 1;

				bv[3].x = c->x;
				bv[3].y = himap[(c->z+1)*(mapwidth+1)+c->x+0];
				bv[3].z = c->z + 1;
				bv[3].color = difcolor;
				bv[3].u = 0; bv[3].v = 1;
			}
			else
			{
				float h0 = himap[(c->z+0)*(mapwidth+1)+c->x+0];
				float dx = himap[(c->z+0)*(mapwidth+1)+c->x+1] - h0;
				float dz = himap[(c->z+1)*(mapwidth+1)+c->x+0] - h0;
				const float elv = 0.05f;

				bv[0].x = c->x;
				bv[0].y = h0 + elv;
				bv[0].z = c->z;
				bv[0].color = difcolor;
				bv[0].u = 0; bv[0].v = 0;

				bv[1].x = c->x + 0.25f;
				bv[1].y = h0 + dx * 0.25f + elv;
				bv[1].z = c->z;
				bv[1].color = difcolor;
				bv[1].u = 1; bv[1].v = 0;

				bv[2].x = c->x + 0.25f;
				bv[2].y = h0 + (dx + dz) * 0.25f + elv;
				bv[2].z = c->z + 0.25f;
				bv[2].color = difcolor;
				bv[2].u = 1; bv[2].v = 1;

				bv[3].x = c->x;
				bv[3].y = h0 + dz * 0.25f + elv;
				bv[3].z = c->z + 0.25f;
				bv[3].color = difcolor;
				bv[3].u = 0; bv[3].v = 1;
			}

			bi[0] = fi+0; bi[1] = fi+3; bi[2] = fi+1;
			bi[3] = fi+1; bi[4] = fi+3; bi[5] = fi+2;
		}
	}
	mapbatch->flush();
	mapbatch->end();
}

extern Matrix vpmatrix;
extern void SetMatrices(Vector3 is, Vector3 ir, Vector3 it);

RVertexBuffer *skybox_vertexbuf = 0;
RIndexBuffer *skybox_indexbuf = 0;


void DrawSkyBox()
{
	static batchVertex cube[8] = {
		batchVertex(-1, 0, -1, 0xFF000000, 0, 0.5f),
		batchVertex(-1, 1, -1, 0xFFFF0000, 0, 0),
		batchVertex( 1, 0, -1, 0xFF00FF00, 1, 0.5f),
		batchVertex( 1, 1, -1, 0xFFFFFF00, 1, 0),
		batchVertex( 1, 0,  1, 0xFF0000FF, 0, 0.5f),
		batchVertex( 1, 1,  1, 0xFFFF00FF, 0, 0),
		batchVertex(-1, 0,  1, 0xFF00FFFF, 1, 0.5f),
		batchVertex(-1, 1,  1, 0xFFFFFFFF, 1, 0),
	};
	ushort indix[36] = { 0, 2, 1, 1, 2, 3,  2, 4, 3, 3, 4, 5,  4, 6, 5, 5, 6, 7,  6, 0, 7, 7, 0, 1,  0, 6, 2, 2, 6, 4,  1, 3, 7, 3, 5, 7 };
	batchVertex *outcube; ushort *outindix;  uint fi;
	SetMatrices(Vector3(5, 5, 5), Vector3(0, 0, 0), camerapos);
	renderer->BeginBatchDrawing();
	renderer->SetTransformMatrix(&matrix);
	renderer->DisableAlphaTest();
	renderer->DisableDepth();
	renderer->EnableAlphaBlend();

	static ushort quads[20] = { 0, 1, 2, 3,   2, 3, 4, 5,   4, 5, 6, 7,   6, 7, 0, 1,   1, 7, 3, 5 };
	static ushort ixix[6] = { 0, 2, 1, 1, 2, 3 };

	mapbatch->begin();
	for (int i = 0; i < 5; i++) {
		batchVertex *vpnt; ushort *ipnt; uint fi;
		mapbatch->next(4, 6, &vpnt, &ipnt, &fi);
		for (int j = 0; j < 4; j++) {
			batchVertex v = cube[quads[i * 4 + j]];
			if (i < 4)
				v.color = (j & 1) ? (-1) : 0x00; // FFFFFF;
			else
				v.color = -1;
			v.u = (j & 2) ? 1.0f : 0.0f;
			v.v = (j & 1) ? 0.0f : ((i==4)?1.0f:0.5f);
			vpnt[j] = v;
		}
		for (int j = 0; j < 6; j++) {
			ipnt[j] = ixix[j] + fi;
		}
		renderer->SetTexture(0, skyboxtex[i]);
		mapbatch->flush();
	}
	mapbatch->end();

	renderer->EnableDepth();
	renderer->DisableAlphaBlend();
}