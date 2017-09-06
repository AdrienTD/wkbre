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

Anim::Anim(char *fn)
{
	fname = strdup(fn);
	ready = 0;
	if(preloadModels) prepare();
}

void Anim::prepare()
{
	if(ready) return;

	char *fcnt, *fp; int fsize; int i, ver, nparts, nverts, form;
	Anim *anim = this;

	LoadFile(fname, &fcnt, &fsize); fp = fcnt;
	form = *(uint*)fp; fp += 4;
	if(form != 'minA')
		ferr("Not an anim3 file.");

	fp += 4;
	char mshname[512], *mp;
	strcpy(mshname, fname);
	mp = strrchr(mshname, '\\');
	mp = mp ? (mp+1) : mshname;
	while(*(mp++) = *(fp++));
	anim->mesh = new Mesh(mshname);
	anim->dur = *(uint*)fp; fp += 4;
	fp += 16;
	nverts = *(uint*)fp; fp += 4;

	for(i = 0; i < 3; i++)
	{
		anim->coord[i].nframes = *(uint*)fp; fp += 4;
		anim->coord[i].ft = new uint[anim->coord[i].nframes];
		anim->coord[i].verts = new float*[anim->coord[i].nframes];
		for(int j = 0; j < anim->coord[i].nframes; j++)
			{anim->coord[i].ft[j] = *(uint*)fp; fp += 4;}
		for(int j = 0; j < anim->coord[i].nframes; j++)
		{
			float *v = anim->coord[i].verts[j] = new float[nverts];
			float ftrans = *(float*)fp; fp += 4;
			float fscale = *(float*)fp; fp += 4;
			for(int k = 0; k < nverts / 3; k++)
			{
				uint w = *(uint*)fp; fp += 4;
				for(int l = 0; l < 3; l++)
					*(v++) = (((w>>(l*11))&1023)/1023.0f) * fscale + ftrans;
			}
			if(nverts % 3)
			{
				uint w = *(uint*)fp; fp += 4;
				for(int l = 0; l < (nverts%3); l++)
					*(v++) = (((w>>(l*11))&1023)/1023.0f) * fscale + ftrans;
			}
		}
	}
	free(fcnt);
	ready = 1;
}

void Anim::CreateVertsFromTime(batchVertex *out, int tm, int grp)
{
	if(!dur) ferr("anim->dur == 0");
	//tm = tm % dur;
	if(tm >= dur) tm = dur-1;
	for(int c = 0; c < 3; c++)
	{
		int f;
		Anim3PosCoord *ac = &(coord[c]);
		for(f = 1; f < ac->nframes; f++)
			if(ac->ft[f] > tm)
				break;
		int fstart = f-1, fend = f;
		float ipol = (float)(tm-ac->ft[fstart]) / (float)(ac->ft[fend]-ac->ft[fstart]);
		batchVertex *o = out;
		for(int i = mesh->mgrpindex[grp]; i < mesh->mgrpindex[grp+1]; i++)
		{
			float v1 = (ac->verts[fstart])[mesh->remapper[mesh->iverts[i]]];
			float v2 = (ac->verts[fend])[mesh->remapper[mesh->iverts[i]]];
			(o++)->p[c] = v1 + (v2-v1) * ipol;
		}
	}
}

void Anim::draw(int iwtcolor)
{
	if(!ready) prepare();

	mesh->draw(iwtcolor);
}

void Anim::drawInBatch(RBatch *batch, int grp, int uvl, int dif, int tm)
{
	if(!ready) prepare();
	if(!mesh->ready) mesh->prepare();
	int cdif = renderer->ConvertColor(dif);

	if((uint)uvl >= mesh->muvlist.len) uvl = 0;
	int sv = mesh->mgrpindex[grp+1] - mesh->mgrpindex[grp];
	int si = mesh->mstartix[grp+1] - mesh->mstartix[grp];

	batchVertex *o = new batchVertex[sv], *p; p = o;
/*	batchVertex *vp; ushort *ip; uint fi;
	batch->next(sv, si, &vp, &ip, &fi);
	batchVertex *op = new batchVertex[sv]; batchVertex *o = op, *p = vp;
*/
	CreateVertsFromTime(o, tm, grp);
	for(int i = mesh->mgrpindex[grp]; i < mesh->mgrpindex[grp+1]; i++)
	{
		Vector3 v(p->x, p->y, p->z), t;
		//Vector3 v(o->x, o->y, o->z), t; o++;
		TransformVector3(&t, &v, &mWorld);
		p->x = t.x; p->y = t.y; p->z = t.z;
		p->color = cdif;
		p->u = mesh->muvlist[uvl]->get(i*2);
		p->v = mesh->muvlist[uvl]->get(i*2+1);
		p++;
	}
	batchVertex *vp; ushort *ip; uint fi;
	batch->next(sv, si, &vp, &ip, &fi);
	memcpy(vp, o, sv * sizeof(batchVertex));
	delete o;
//	delete op;

	for(int i = mesh->mstartix[grp]; i < mesh->mstartix[grp+1]; i++)
		*(ip++) = mesh->mindices[i] - mesh->mgrpindex[grp] + fi;
}
