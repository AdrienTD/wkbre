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

D3DVERTEXELEMENT9 meshddve[] = {
{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
{1, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
D3DDECL_END()
};
IDirect3DVertexDeclaration9 *meshddvd;

inline uchar readchar(FILE *file) {return fgetc(file);}
inline ushort readshort(FILE *file) {ushort a; fread(&a, 2, 1, file); return a;}
inline uint readint(FILE *file) {uint a; fread(&a, 4, 1, file); return a;}
inline float readfloat(FILE *file) {float a; fread(&a, 4, 1, file); return a;}
inline void ignorestr(FILE *file) {while(fgetc(file));}

void InitMeshDrawing()
{
	ddev->CreateVertexDeclaration(meshddve, &meshddvd);
}

//#define dbg(x) printf("%i, fp = %u\n", x, fp-fcnt)
#define dbg(x) 

void BeginMeshDrawing()
{
	ddev->SetVertexDeclaration(meshddvd);
	ddev->SetRenderState(D3DRS_LIGHTING, FALSE);
	ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	ddev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	ddev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	ddev->SetRenderState(D3DRS_ALPHAREF, 240);
	ddev->SetRenderState(D3DRS_ZENABLE, TRUE);
	ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	ddev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	ddev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
}

/*Mesh *LoadAnim(char *fn)
{
	char *fcnt; int fsize; char mf[512];

	strcpy(mf, fn);
	char *c = strrchr(mf, '\\');
	*(c+1) = 0;

	LoadFile(fn, &fcnt, &fsize);
	strcat(mf, fcnt+8);
	free(fcnt);
	return new Mesh(mf);
}*/

Mesh *LoadAnim(char *fn)
{
	char *fcnt; int fsize; Mesh *m;

	LoadFile(fn, &fcnt, &fsize);
	if(*(uint*)fcnt == 'hseM')
		m = new Mesh(fn);
	else if(*(uint*)fcnt == 'minA')
	{
		char mf[512];
		strcpy(mf, fn);
		char *c = strrchr(mf, '\\');
		*(c+1) = 0;
		strcat(mf, fcnt+8);
		m = new Mesh(mf);
	}
	else ferr("File to load is supposed to be a Mesh or Anim but it isn't.");
	free(fcnt);
	return m;
}

Mesh::Mesh(char *fn)
{
	FILE *file; int i, ver, nparts, nverts;
	char *fcnt; int fsize; char *fp;

	LoadFile(fn, &fcnt, &fsize);
	fp = fcnt + 4;
	ver = *(uint*)fp;

	// Points
	fp = fcnt + 12;
	nparts = *(ushort*)fp; fp += 2;
	for(i = 0; i < nparts; i++)
	{
		while(*(fp++));
		fp += 0x1D;
		while(*(fp++));
	}
	dbg(1);

	// Positions
	nverts = *(ushort*)fp; fp += 2;
	lstverts = (float*)malloc(nverts*3*sizeof(float));
	int x = 0;
	//fread(lstverts, nverts*3*sizeof(float), 1, file);
	memcpy(lstverts, fp, nverts*3*sizeof(float));
	fp += nverts*3*sizeof(float);
	dbg(2);

	// Sphere
	memcpy(&sphere, fp, 16);
	fp += 16;

	// Remapper
	if(ver < 4)
	{
		i = *(ushort*)fp; fp += 2;
		fp += 2 * i;
	}

	// Normals
	int nnorm = *(ushort*)fp; fp += 2;
	fp += (*(uint*)fp) * 2 + 4;
	dbg(3);

	// Materials
	int nmat = *(ushort*)fp; fp += 2;
	lstmatflags = (int*)malloc(nmat*sizeof(int));
	lstmattex = (texture*)malloc(nmat*sizeof(texture));
	for(i = 0; i < nmat; i++)
	{
		char *s = new char[256]; char *p = s;
		lstmatflags[i] = *(uchar*)fp; fp++;
		strcpy(s, "Warrior Kings Game Set\\Textures\\");
		p = s + strlen(s);
		while(*(p++) = *(fp++));
		lstmattex[i] = GetTexture(s);
		delete [] s;
	}

	// Texture coordinates
	int nuvlist = *(uint*)fp; fp += 4;
	lstuvlist = (float**)malloc(nuvlist * sizeof(float*));
	for(int l = 0; l < nuvlist; l++)
	{
		int ntexc = *(ushort*)fp; fp += 2;
		float *uvl = (float*)malloc(ntexc * 2 * sizeof(float));
		//fread(uvl, ntexc * 2 * sizeof(float), 1, file);
		memcpy(uvl, fp, ntexc * 2 * sizeof(float));
		fp += ntexc * 2 * sizeof(float);
		lstuvlist[l] = uvl;
		muvlist.add(new GrowList<float>);
	}
	dbg(4);

	// Groups
	ngrp = *(uint*)fp; fp += 4;
	mgrpindex = (uint*)malloc((ngrp+1) * sizeof(uint));
	mgrpindex[0] = 0;
	for(i = 0; i < ngrp; i++)
	{
		int sg = *(ushort*)fp; fp += 2;
		for(int j = 0; j < sg; j++)
		{
			int v = *(ushort*)fp; fp += 2;
			for(int k = 0; k < 3; k++)
				mverts.add(lstverts[v*3+k]);
			fp += 2; // Index to normal list
			v = *(ushort*)fp; fp += 2;
			for(int u = 0; u < nuvlist; u++)
				for(int k = 0; k < 2; k++)
					(muvlist[u])->add((lstuvlist[u])[v*2+k]);
		}
		mgrpindex[i+1] = mgrpindex[i]+sg;
	}

	// Polygon Lists
	// One used at the moment.
	int ntrilist = *(uint*)fp; fp += 4;
	//for(int a = 0; a < ntrilist; a++)

	fp += 2 + 2 + 2 + 4;
	mstartix = (uint*)malloc((nmat + 1) * sizeof(uint));
	mstartix[0] = 0;
	for(int n = 0; n < nmat; n++)
	{
		int sgrp = *(ushort*)fp; fp += 2;
		fp += 4;
		mstartix[n+1] = mstartix[n] + (sgrp * 3);
		for(i = 0; i < sgrp; i++)
			for(int j = 0; j < 3; j++)
				{mindices.add(*(ushort*)fp + mgrpindex[n]); fp += 2;}
	}

	free(fcnt);

	ddev->CreateVertexBuffer(mverts.len*4, 0, 0, D3DPOOL_MANAGED, &dvbverts, 0);
	ddev->CreateIndexBuffer(mindices.len*2, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &dixbuf, 0);

	void *buf;
	dvbverts->Lock(0, mverts.len*4, &buf, 0);
	memcpy(buf, mverts.gb.memory, mverts.len * 4);
	dvbverts->Unlock();
	dixbuf->Lock(0, mindices.len*2, &buf, 0);
	memcpy(buf, mindices.gb.memory, mindices.len*2);
	dixbuf->Unlock();

	for(int i = 0; i < muvlist.len; i++)
	{
		IDirect3DVertexBuffer9 *ul;
		ddev->CreateVertexBuffer(muvlist[i]->len*4, 0, 0, D3DPOOL_MANAGED, &ul, 0);
		ul->Lock(0, muvlist[i]->len*4, &buf, 0);
		memcpy(buf, muvlist[i]->gb.memory, muvlist[i]->len*4);
		ul->Unlock();
		dvbtexc.add(ul);
	}

	free(lstverts); free(lstuvlist);
}

void Mesh::draw(int iwtcolor)
{
	if((uint)iwtcolor >= dvbtexc.len) iwtcolor = 0;
	ddev->SetStreamSource(0, dvbverts, 0, 12);
	ddev->SetStreamSource(1, dvbtexc[iwtcolor], 0, 8);
	ddev->SetIndices(dixbuf);
	for(int g = 0; g < ngrp; g++)
	{
		ddev->SetRenderState(D3DRS_ALPHATESTENABLE, lstmatflags[g]?TRUE:FALSE);
		SetTexture(0, lstmattex[g]);
		ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, mgrpindex[g], mgrpindex[g+1]-mgrpindex[g]/*mstartix[g+1]-mstartix[g]*/, mstartix[g], (mstartix[g+1]-mstartix[g])/3);
	}
}