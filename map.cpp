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
float *himap = 0;
texture grass;
char lastmap[256];
boolean usemaptexdb = 1;
MapTile *maptiles;

GrowList<MapPart> mparts;
uint mappartw = 32, mapparth = 32;
uint npartsw, npartsh;

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
}

void CloseMap()
{
	if(!himap) return;
	free(himap); himap = 0;
	for(int i = 0; i < mparts.len; i++)
	{
		MapPart *p = mparts.getpnt(i);
		renderer->FreeMapPart(p);
	}
	mparts.clear();
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
	maphiscale = *(float*)fp; fp += 28;
	mapfogcolor = (*(uchar*)fp<<16) | ((*(uchar*)(fp+4))<<8) | (*(uchar*)(fp+8)); fp += 12;
	strs = *(ushort*)fp; fp += 2 + strs;

	mapnverts = (mapwidth+1)*(mapheight+1);
	himap = (float*)malloc(mapnverts*sizeof(float));
	if(!himap) ferr("No mem. left for heightmap.");

	bytepos = fp + 0xC000; bitpos = 0;

	// Lakes
	int mapnlakes = readnb(6);
	for(int i = 0; i < mapnlakes; i++)
		{readnb(32); readnb(32); readnb(32); readnb(2);}

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
		himap[i] = (float)((uchar)readnb(8)) * maphiscale;

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
	LoadFile(sfnpcx, &fcnt, &fsize);
	mapnverts = (mapwidth+1)*(mapheight+1);
	himap = (float*)malloc(mapnverts*sizeof(float));
	if(!himap) ferr("No mem. left for heightmap.");
	fp = fcnt + 128;
	int o = 0, q = 0;
	//printf("--- PCX MAP LOADING ---\n");
	while(1)
	{
		uchar l, b;
		if(((uchar)(*fp)) >= 0xC0)
			l = (uchar)(*(fp++))&0x3F;
		else
			l = 1;
		b = *(fp++);

		//printf("l = %i; b = %i\n", l, b);
		for(int j = 0; j < l; j++)
		{
			if((mapwidth+1) & 1)
				if((q % (mapwidth+2)) == (mapwidth+1))
					{q++; continue;}
			himap[o++] = (float)b * maphiscale; q++;
			if(o >= mapnverts) goto pcxreadend;
		}
	}
pcxreadend:
	//printf("--- END ---\n");
	free(fcnt);
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
	}
}
	renderer->CreateMapPart(p, x, y, w, h);
}

void LoadMap(char *filename, int bitoff)
{
	if(himap) CloseMap();

	if(usemaptexdb)
	{
		char ln[512];
		char *nwp = strrchr(filename, '\\');
		if(!nwp) nwp = filename; else nwp++;
		strcpy(ln, "Maps\\Map_Textures\\");
		strcat(ln, nwp);
		strcpy(strrchr(ln, '.')+1, "lttt");
		//printf("LTTT filename is: %s\n", ln);
		if(FileExists(ln)) LoadLTTT(ln);
		LoadMapTextures();
		FreeLTTT();
	}

	strcpy_s(lastmap, 255, filename);
	char *fe = strrchr(filename, '.');
	if((tolower(fe[1]) == 'b') && (tolower(fe[2]) == 'c') && (tolower(fe[3]) == 'm'))
		LoadMapBCM(filename, bitoff);
	else	LoadMapSNR(filename);

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

void DrawPart(MapPart *p)
{
	renderer->DrawPart(p);
}

boolean IsPartInFrontOfCam(int px, int py)
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

boolean IsTPInScreen(Vector3 &v)
{
	Vector3 t;
	TransformVector3(&t, &v, &matrix);
	t.x /= t.z; t.y /= t.z;
	if(abs(t.x) >= 1.2) return 0;
	if(abs(t.y) >= 1.15) return 0;
	if(t.z <= -1.5 || t.z >= farzvalue) return 0;
	return 1;
}

boolean IsTileInScreen(MapTile *c)
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

void CheckAndDrawPart(int px, int py)
{
	if((px >= 0) && (px < npartsw) && (py >= 0) && (py < npartsh))
		if(IsPartInFrontOfCam(px, py))
			DrawPart(mparts.getpnt(py*npartsw+px));
}

void DrawMap()
{
	renderer->BeginMapDrawing();
	SetTexture(0, grass);

	int cx = (int)camerapos.x / 5 + mapedge, cy = mapheight-2*mapedge-((int)camerapos.z / 5) + mapedge;

	int px = cx / mappartw, py = cy / mapparth;

	int sw = (farzvalue / (mappartw*5)) + 1;
	int sh = (farzvalue / (mapparth*5)) + 1;

if(!usemaptexdb)
{
	for(int dy = -sh; dy <= sh; dy++)
	for(int dx = -sw; dx <= sw; dx++)
		CheckAndDrawPart(px + dx, py + dy);
}
else
{
	//RBatch *batch = renderer->CreateBatch(2*8192, 3*8192);
	//RBatch *batch = renderer->CreateBatch(2*768, 3*768);
	//RBatch *batch = renderer->CreateBatch(2*400, 3*400);
	//RBatch *batch = renderer->CreateBatch(2*1024, 3*1024);
	RBatch *batch = renderer->CreateBatch(4*1024, 6*1024);
	renderer->BeginBatchDrawing();

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
				if(tns) {SetTexture(0, t->t); tns = 0;}

				batchVertex *bv; ushort *bi; uint fi;
				batch->next(4, 6, &bv, &bi, &fi);
				int a = c->rot, f = ((c->zflip&1)?1:0) ^ ((c->xflip&1)?3:0);

				bv[0].x = c->x;			bv[0].z = c->z;
				bv[1].x = c->x + 1;		bv[1].z = c->z;
				bv[2].x = c->x + 1;		bv[2].z = c->z + 1;
				bv[3].x = c->x;			bv[3].z = c->z + 1;

				bv[0].y = himap[(c->z+0)*(mapwidth+1)+c->x+0];
				bv[1].y = himap[(c->z+0)*(mapwidth+1)+c->x+1];
				bv[2].y = himap[(c->z+1)*(mapwidth+1)+c->x+1];
				bv[3].y = himap[(c->z+1)*(mapwidth+1)+c->x+0];
/*
				bv[((0^f)+a)&3].u = t->x / 256.f;		bv[((0^f)+a)&3].v = t->y / 256.f;
				bv[((1^f)+a)&3].u = (t->x+t->w) / 256.f;	bv[((1^f)+a)&3].v = t->y / 256.f;
				bv[((2^f)+a)&3].u = (t->x+t->w) / 256.f;	bv[((2^f)+a)&3].v = (t->y+t->h) / 256.f;
				bv[((3^f)+a)&3].u = t->x / 256.f;		bv[((3^f)+a)&3].v = (t->y+t->h) / 256.f;
*/
				bv[((0^f)+a)&3].u = (t->x+0.5f) / 256.f;		bv[((0^f)+a)&3].v = (t->y+0.5f) / 256.f;
				bv[((1^f)+a)&3].u = (t->x+t->w-0.5f) / 256.f;		bv[((1^f)+a)&3].v = (t->y+0.5f) / 256.f;
				bv[((2^f)+a)&3].u = (t->x+t->w-0.5f) / 256.f;		bv[((2^f)+a)&3].v = (t->y+t->h-0.5f) / 256.f;
				bv[((3^f)+a)&3].u = (t->x+0.5f) / 256.f;		bv[((3^f)+a)&3].v = (t->y+t->h-0.5f) / 256.f;

				for(int j = 0; j < 4; j++)
					{/*bv[j].y = 0;*/ bv[j].color = -1;
					}//bv[j].x -= 0.5f; bv[j].y -= 0.5f;}
					 // L-> this will drop FPS because it rereads the locked buffer!
				//bi[0] = fi+0; bi[1] = fi+1; bi[2] = fi+3;
				bi[0] = fi+0; bi[1] = fi+3; bi[2] = fi+1;
				bi[3] = fi+1; bi[4] = fi+3; bi[5] = fi+2;
				//bi[3] = fi+1; bi[4] = fi+2; bi[5] = fi+3;
		}
	} batch->flush();}

	delete batch;
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