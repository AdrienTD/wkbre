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

#define dbg(a) 

struct Mesh;
struct RBatch;
struct Model;

struct AttachmentPointState
{
	Vector3 position;
	float orientation[4];
	char on;
};

struct AttachmentPoint
{
	int tag;
	AttachmentPointState staticState;
	char *path;
	Model *model;
	//ParticleSystem *psys;
};

struct Model
{
	Mesh *mesh;
	char *fname; boolean ready;
	virtual void prepare() = 0;
	virtual void draw(int iwtcolor = 0) = 0;
	virtual void drawInBatch(RBatch *batch, int grp, int uvl = 0, int dif = 0, int tm = 0) = 0;
	virtual void getAttachPointPos(Vector3 *vout, int apindex, int tm = 0) = 0;
	virtual bool isAttachPointOn(int apindex, int tm = 0) = 0;
	virtual int hasAttachPointTurnedOn(int apindex, int tma, int tmb) = 0;
};

class Mesh : public Model
{
public:
	float *lstverts;
	int *lstmatflags;
	texture *lstmattex;
	float **lstuvlist;
	//GrowList<GrowList<int>*> lstgroup;
	int *lstmattid;

	GrowList<float> mverts;
	GrowList<uint> iverts;
	GrowList<GrowList<float>*> muvlist;
	uint *mgrpindex;
	GrowList<ushort> mindices;
	uint *mstartix;
	int ngrp;
	float sphere[4];
	uint *remapper;

	IDirect3DVertexBuffer9 *dvbverts;
	IDirect3DIndexBuffer9 *dixbuf;
	GrowList<IDirect3DVertexBuffer9*> dvbtexc;

	int nAttachPnts;
	AttachmentPoint *attachPnts;
	int nNormals;
	char *normals;
	uint *normalRemapper;

	Mesh(char *fn);

	void loadMin();
	void prepare();
	void draw(int iwtcolor = 0);
	void drawInBatch(RBatch *batch, int grp, int uvl = 0, int dif = 0, int tm = 0);
	void getAttachPointPos(Vector3 *vout, int apindex, int tm = 0);
	bool isAttachPointOn(int apindex, int tm = 0);
	int hasAttachPointTurnedOn(int apindex, int tma, int tmb);
};

extern GrowStringList strMaterials;
extern GrowStringList alModelFn;
extern GrowList<Model*> alModel;
extern GrowStringList strAttachPntTags;

void InitMeshDrawing();
void BeginMeshDrawing();
Mesh *LoadAnim(char *fn);
Model *GetModel(char *fn);
