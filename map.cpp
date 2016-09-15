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
	if(bitpos >= 8) {bitpos = 0; bytepos++;}
	return a;
}
int readbyte()
{
	int a = 0, i;
	//for(i = 7; i >= 0; i--)
	for(i = 0; i < 8; i++)
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

	mapnverts = (mapwidth+1)*(mapheight+1);
	himap = (float*)malloc(mapnverts*sizeof(float));
	if(!himap) ferr("No mem. left for heightmap.");
	fp = fcnt + fsize - mapnverts;
	//bytepos = fp; bitpos = 0;
	//for(int i = 0; i < bitoff; i++) readbit();
	bytepos = fp + (bitoff / 8); bitpos = bitoff & 7;
	for(int i = 0; i < mapnverts; i++)
		himap[i] = (float)((uchar)readbyte()) * maphiscale;

	free(fcnt);

}

void LoadMapSNR(char *filename)
{
	char *fcnt, *fp; int fsize;
	char sfnpcx[256];

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
		if(!strcmp(word[0], "SCENARIO_EDGE_WIDTH"))
			mapedge = atoi(word[1]);
		if(!strcmp(word[0], "SCENARIO_HEIGHT_SCALE_FACTOR"))
			maphiscale = atof(word[1]);
		if(!strcmp(word[0], "SCENARIO_HEIGHTMAP"))
			strcpy_s(sfnpcx, 255, word[1]);
		if(!strcmp(word[0], "SCENARIO_FOG_COLOUR"))
			mapfogcolor = (atoi(word[1]) << 16) | (atoi(word[2]) << 8) | atoi(word[3]);
	}
	free(fcnt);

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

	renderer->CreateMapPart(p, x, y, w, h);
}

void LoadMap(char *filename, int bitoff)
{
	if(himap) CloseMap();
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

void CheckAndDrawPart(int px, int py)
{
	if((px >= 0) && (px < npartsw) && (py >= 0) && (py < npartsh))
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

	for(int dy = -sh; dy <= sh; dy++)
	for(int dx = -sw; dx <= sw; dx++)
		CheckAndDrawPart(px + dx, py + dy);
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