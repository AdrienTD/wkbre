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

//#define mpgetint(p) *(((int*)(p))++)

struct MapPart
{
	int x, y, w, h; float minhi, maxhi;
	IDirect3DVertexBuffer9 *vbuf;
	IDirect3DIndexBuffer9 *ibuf;
};

D3DVERTEXELEMENT9 mapvdecli[] = {
 {0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
 {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
 D3DDECL_END()
};

int mapwidth, mapheight, mapedge, mapnverts, mapfogcolor; float maphiscale;
float *himap = 0;
IDirect3DVertexDeclaration9 *dvdmap; IDirect3DVertexShader9 *dvsmap;
//IDirect3DVertexBuffer9 *dvbmap;
//IDirect3DIndexBuffer9 *dibmap;
texture grass;
char lastmap[256];

GrowList<MapPart> mparts;
uint mappartw = 32, mapparth = 32;
uint npartsw, npartsh;

//Matrix idmx = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};

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
	ddev->CreateVertexDeclaration(mapvdecli, &dvdmap);
	//dvsmap = LoadVertexShader("map.vsh");
	grass = GetTexture("grass.tga");
}

void CloseMap()
{
	if(!himap) return;
	free(himap); himap = 0;
	for(int i = 0; i < mparts.len; i++)
	{
		MapPart *p = mparts.getpnt(i);
		p->vbuf->Release(); p->ibuf->Release();
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
	{
		//himap[i] = (float)((uchar)(*(fp++))) * maphiscale; // / 16.0f;
		////himap[i] = (float)(( (((uchar)*fp)>>bitoff) | (((uchar)(*(fp-1)))<<(8-bitoff)) )&255) * maphiscale;
		////fp++;
		himap[i] = (float)((uchar)readbyte()) * maphiscale;
	}

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
	MapPart p; float *fb; ushort *sb;
	p.x = x; p.y = y; p.w = w; p.h = h;
	p.minhi = 255*maphiscale; p.maxhi = 0;

	ddev->CreateVertexBuffer((w+1)*(h+1)*5*4, 0, 0, D3DPOOL_MANAGED, &p.vbuf, NULL);
	p.vbuf->Lock(0, 0, (void**)&fb, 0);
	float *fp = fb, m;
	for(int b = 0; b < h+1; b++)
	for(int a = 0; a < w+1; a++)
	{
		*(fp++) = a+x;
		m = *(fp++) = himap[(b+y)*(mapwidth+1)+(a+x)];
		if(m > p.maxhi) p.maxhi = m;
		if(m < p.minhi) p.minhi = m;
		*(fp++) = b+y;

		*(fp++) = a+x; *(fp++) = b+y;
	}
	p.vbuf->Unlock();

	ddev->CreateIndexBuffer(( (2*(w+1)+1)*h ) * 2, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &p.ibuf, NULL);
	p.ibuf->Lock(0, 0, (void**)&sb, 0);
	ushort *os = sb;

	for(int cy = 0; cy < h; cy++)
	{
		if(!(cy&1))
		{
			for(int cx = 0; cx < w+1; cx++)
			{
				*(os++) = cy*(w+1)+cx;
				*(os++) = (cy+1)*(w+1)+cx;
				if(cx == w) *(os++) = (cy+1)*(w+1)+cx;
			}
		} else {
			for(int cx = w+1-1; cx >= 0; cx--)
			{
				*(os++) = cy*(w+1)+cx;
				*(os++) = (cy+1)*(w+1)+cx;
				if(cx == 0) *(os++) = (cy+1)*(w+1)+cx;
			}
		}
	}
	//if(h & 1)
	//	*(os++) = h*(w+1)+w;
	//else
	//	*(os++) = h*(w+1);
	p.ibuf->Unlock();

	mparts.add(p);
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

// Between wkbre13 and wkbre19 both included, there was some old out-commented
// code for terrain drawing at this place. Removed in wkbre20.

void DrawPart(MapPart *p)
{
	ddev->SetStreamSource(0, p->vbuf, 0, 20);
	ddev->SetIndices(p->ibuf);
	ddev->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, (p->w+1)*(p->h+1), 0, (2*(p->w+1)+1)*p->h-2); //p->w*p->h*2+3*p->h);
}

void CheckAndDrawPart(int px, int py)
{
	if((px >= 0) && (px < npartsw) && (py >= 0) && (py < npartsh))
		DrawPart(mparts.getpnt(py*npartsw+px));
}

void DrawMap()
{
	ddev->SetRenderState(D3DRS_LIGHTING, FALSE);
	ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	ddev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	ddev->SetRenderState(D3DRS_ZENABLE, TRUE);
	ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	ddev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

	ddev->SetVertexDeclaration(dvdmap);
	//ddev->SetVertexShader(dvsmap);
	//ddev->SetVertexShaderConstantF(0, (float*)&matrix, 4);
	ddev->SetVertexShader(0);
	SetTransformMatrix(&matrix);
	ddev->SetPixelShader(0);
	SetTexture(0, grass);

	int cx = (int)camerapos.x / 5 + mapedge, cy = mapheight-2*mapedge-((int)camerapos.z / 5) + mapedge;

	int px = cx / mappartw, py = cy / mapparth;

	int sw = (farzvalue / (mappartw*5)) + 1;
	int sh = (farzvalue / (mapparth*5)) + 1;
	//_snprintf(statustextbuf, 128, "sw = %i, sh = %i", sw, sh);
	//statustext = statustextbuf;

	for(int dy = -sh; dy <= sh; dy++)
	for(int dx = -sw; dx <= sw; dx++)
		CheckAndDrawPart(px + dx, py + dy);
}

//int IsInFirstTri(float x, floay y) // x, y between 0 and 1
//{
//	return (x < (1.0f-y));
//}

/*void GetHeight(float x, float y)
{
	float a, b, ao, c, d, co, m; int u, v;
	u = x; v = y;
	a = x - u; b = 1.0f - a;
	c = y - v; d = 1.0f - c;
	
}*/

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