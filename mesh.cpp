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

GrowStringList strMaterials;
GrowStringList strAttachPntTags;

inline uchar readchar(FILE *file) {return fgetc(file);}
inline ushort readshort(FILE *file) {ushort a; fread(&a, 2, 1, file); return a;}
inline uint readint(FILE *file) {uint a; fread(&a, 4, 1, file); return a;}
inline float readfloat(FILE *file) {float a; fread(&a, 4, 1, file); return a;}
inline void ignorestr(FILE *file) {while(fgetc(file));}

//#define dbg(x) printf("%i, fp = %u\n", x, fp-fcnt)
#define dbg(x) 

void BeginMeshDrawing()
{
	renderer->BeginMeshDrawing();
}

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

GrowStringList alModelFn;
GrowList<Model*> alModel;

Model *GetModel(char *fno)
{
	// Remove "xxx\..\" in the paths.
	char *fn = strdup(fno);
	char *pd;
	while(pd = strstr(fn, "\\..\\"))
	{
		//char *dtr = (pd == fn) ? pd : (pd - 1);
		char *dtr = pd - 1;
		while((dtr > fn) && (*dtr != '\\'))
			dtr--;
		dtr++;
		strcpy(dtr, pd+4);
	}

	int x = alModelFn.find(fn);
	if(x != -1)
		return alModel[x];

	Model *m;
	char *ext = strrchr(fn, '.');
	if(!ext) ferr("Invalid model file name.");
	ext++;
	if(!stricmp(ext, "mesh3"))
		m = new Mesh(fn);
	else if(!stricmp(ext, "anim3"))
		m = new Anim(fn);
	else
		ferr("Invalid model file name extension.");
	alModelFn.add(fn);
	alModel.add(m);
	free(fn);
	return m;
}

Mesh::Mesh(char *fn)
{
	fname = strdup(fn);
	ready = 0;
	mesh = this;
	loadMin();
	if(preloadModels) prepare();
}

void Mesh::loadMin()
{
	char *fcnt, *fp; int fsize;
	LoadFile(fname, &fcnt, &fsize);
	fp = fcnt + 12;
	nAttachPnts = *(ushort*)fp; fp += 2;
	if(nAttachPnts) attachPnts = new AttachmentPoint[nAttachPnts];
	for(int i = 0; i < nAttachPnts; i++)
	{
		//attachPnts[i].tag = strdup(fp);
		int t = strAttachPntTags.find_cs(fp);
		if(t == -1)
			{t = strAttachPntTags.len; strAttachPntTags.add(fp);}
		attachPnts[i].tag = t;
		while(*(fp++));
		attachPnts[i].staticState.position.x = *(float*)fp; fp += 4;
		attachPnts[i].staticState.position.y = *(float*)fp; fp += 4;
		attachPnts[i].staticState.position.z = *(float*)fp; fp += 4;
		for(int j = 0; j < 4; j++)
			{attachPnts[i].staticState.orientation[j] = *(float*)fp; fp += 4;}
		attachPnts[i].staticState.on = *fp; fp++;
		if(*fp) attachPnts[i].path = strdup(fp);
		else attachPnts[i].path = 0;
		while(*(fp++));

		attachPnts[i].model = 0;
		if(attachPnts[i].path)
		{
			char *ext = strrchr(attachPnts[i].path, '.');
			if(ext)
				if(!stricmp(ext+1, "mesh3") || !stricmp(ext+1, "anim3"))
				{
					char mp[512];
					strcpy(mp, "Warrior Kings Game Set\\");
					strcat(mp, attachPnts[i].path);
					attachPnts[i].model = GetModel(mp);
				}
			//free(attachPnts[i].path);
		}
	}
	free(fcnt);
}

void Mesh::prepare()
{
	if(ready) return;

	FILE *file; int i, ver, nparts, nverts;
	char *fcnt; int fsize; char *fp;

	LoadFile(fname, &fcnt, &fsize);
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

	// Position remapper
	int nremap;
	if(ver < 4)
	{
		nremap = *(ushort*)fp; fp += 2;
		remapper = new uint[nremap];
		for(int i = 0; i < nremap; i++)
			{remapper[*(ushort*)fp] = i; fp += 2;}
	}
	else
	{
		nremap = nverts;
		remapper = new uint[nremap];
		for(int i = 0; i < nremap; i++)
			remapper[i] = i;
	}

	// Normals construction from triangles (unused)
	int nnorm = *(ushort*)fp; fp += 2;
	fp += (*(uint*)fp) * 2 + 4;
	dbg(3);

	// Materials
	int nmat = *(ushort*)fp; fp += 2;
	lstmatflags = (int*)malloc(nmat*sizeof(int));
	lstmattex = (texture*)malloc(nmat*sizeof(texture));
	lstmattid = (int*)malloc(nmat*sizeof(int));
	for(i = 0; i < nmat; i++)
	{
		char *s = new char[256]; char *p = s;
		lstmatflags[i] = *(uchar*)fp; fp++;

		int tid = strMaterials.find(fp);
		if(tid != -1)
			lstmattid[i] = tid;
		else
			{lstmattid[i] = strMaterials.len;
			strMaterials.add(fp);}

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
			iverts.add(v);
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

	// Ignore next polygon lists (at the moment)
	for(int a = 1; a < ntrilist; a++)
	{
		fp += 2 + 2 + 2 + 4;
		for(int n = 0; n < nmat; n++)
		{
			int sgrp = *(ushort*)fp; fp += 2;
			fp += 4 + sgrp * 3 * 2;
		}
	}

	// Normals
	nNormals = nnorm;
	normals = new char[nNormals];
	memcpy(normals, fp, nNormals);
	fp += nNormals;

	// Normal remapper
	int nnormremap;
	if(ver < 4)
	{
		nnormremap = *(ushort*)fp; fp += 2;
		normalRemapper = new uint[nnormremap];
		for(int i = 0; i < nnormremap; i++)
			{normalRemapper[*(ushort*)fp] = i; fp += 2;}
	}
	else
	{
		nnormremap = nnorm;
		normalRemapper = new uint[nnormremap];
		for(int i = 0; i < nnormremap; i++)
			normalRemapper[i] = i;
	}

	free(fcnt);

	renderer->CreateMesh(this);

	free(lstverts); free(lstuvlist);

	ready = 1;
}

void Mesh::draw(int iwtcolor)
{
	if(!ready) prepare();

	if((uint)iwtcolor >= muvlist.len) iwtcolor = 0;
	renderer->DrawMesh(this, iwtcolor);
}

void Mesh::drawInBatch(RBatch *batch, int grp, int uvl, int dif, int tm)
{
	if(!ready) prepare();
	int cdif = renderer->ConvertColor(dif);

	if((uint)uvl >= muvlist.len) uvl = 0;
	int sv = mgrpindex[grp+1] - mgrpindex[grp];
	int si = mstartix[grp+1] - mstartix[grp];
	batchVertex *vp; ushort *ip; uint fi;
	batch->next(sv, si, &vp, &ip, &fi);

	for(int i = mgrpindex[grp]; i < mgrpindex[grp+1]; i++)
	{
		Vector3 p(mverts[i*3], mverts[i*3+1], mverts[i*3+2]), t;
		TransformVector3(&t, &p, &mWorld);
		vp->x = t.x; vp->y = t.y; vp->z = t.z;
		//vp->x = 0; vp->y = 0; vp->z = 0;
		vp->color = cdif;
		vp->u = muvlist[uvl]->get(i*2); vp->v = muvlist[uvl]->get(i*2+1);
		vp++;
	}

	for(int i = mstartix[grp]; i < mstartix[grp+1]; i++)
		*(ip++) = mindices[i] - mgrpindex[grp] + fi;
}

void Mesh::getAttachPointPos(Vector3 *vout, int apindex, int tm)
{
	*vout = attachPnts[apindex].staticState.position;
}

bool Mesh::isAttachPointOn(int apindex, int tm)
{
	return attachPnts[apindex].staticState.on;
}

int Mesh::hasAttachPointTurnedOn(int apindex, int tma, int tmb)
{
	return 0;
}
