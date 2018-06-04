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

Vector3 camerapos(0.0f, 15.0f, 0.0f);
Matrix matrix, vpmatrix;
Vector3 nullvector(0.0f, 0.0f, 0.0f), onevector(1.0f, 1.0f, 1.0f);

Matrix matView, matProj, mWorld, camworld;
D3DVIEWPORT9 dvport;
float farzvalue = 250.0f, occlurate = 2.0f/3.0f, verticalfov = 52.5f * M_PI / 180.0f;
float camyaw = 0.0f, campitch = 0.0f;
Vector3 vLAD;
goref currentSelection, newSelection; float newSelZ;
Vector3 raystart, raydir;
Matrix mReal; //, mWorldMulView;
Matrix mIdentity;

int enableMap = 1, drawdebug = 1;
int fogenabled = 0, showrepresentations = 0;

struct OOBMTex
{
	int tid;
	GrowList<GameObject*> *objs;
};

boolean meshbatching = 0, animsEnabled = 0;
GrowList<OOBMTex> oobm[2];
RBatch *mshbatch;

void SetConstantMatrices()
{
	Vector3 vCamDir = Vector3(0.0f, 0.0f, 5.0f);

	//Matrix mCamRot; D3DXMatrixRotationYawPitchRoll(&mCamRot, camyaw, campitch, 0.0f);
	Matrix mCamRot, roty, rotx;
	CreateRotationYMatrix(&roty, -camyaw);
	CreateRotationXMatrix(&rotx, -campitch);
	MultiplyMatrices(&mCamRot, &rotx, &roty);

	TransformNormal3(&vLAD, &vCamDir, &mCamRot);

	Vector3 vLookatPt = camerapos + vLAD; //camerapos + Vector3(0.0f, 0.0f, 5.0f);
	Vector3 vUpVec( 0.0f, 1.0f, 0.0f );
	CreateLookAtLHViewMatrix(&matView, &camerapos, &vLookatPt, &vUpVec);
	//ddev->SetTransform( D3DTS_VIEW, &matView );

	CreatePerspectiveMatrix( &matProj, verticalfov, (float)scrw/(float)scrh, 1.0f, farzvalue );
	//ddev->SetTransform( D3DTS_PROJECTION, &matProj );

	MultiplyMatrices(&vpmatrix, &matView, &matProj);
	CreateTranslationMatrix(&camworld, camerapos.x, camerapos.y, camerapos.z);
	camworld *= mCamRot;
}

void SetMatrices(Vector3 is, Vector3 ir, Vector3 it)
{
	Matrix mscale, mrot, mtrans;
	CreateScaleMatrix(&mscale, is.x, is.y, is.z);
	CreateRotationYXZMatrix(&mrot, ir.y, ir.x, ir.z);
	CreateTranslationMatrix(&mtrans, it.x, it.y, it.z);

	mWorld = mscale*mrot*mtrans;
	//MultiplyMatrices(&mReal, &mWorld, &vpmatrix);
	//TransposeMatrix(&matrix, &mReal);
	MultiplyMatrices(&matrix, &mWorld, &vpmatrix);
	//ddev->SetTransform( D3DTS_WORLD, &matrix );

	//mWorldMulView = mWorld * matView;
}

void InitOOBMList()
{
	if(!meshbatching) return;
	//for(int i = 0; i < 2; i++)
	//	oobm[i] = new GrowList<GameObject*>[strMaterials.len];
	mshbatch = renderer->CreateBatch(16384, 25000);
	//mshbatch = renderer->CreateBatch(9000, 9000);
}

void DrawOOBM()
{
	uint tt = timeGetTime();
	renderer->BeginBatchDrawing();
	SetTransformMatrix(&vpmatrix);
	mshbatch->begin();
	for(int t = 0; t < strMaterials.len; t++)
	{
		int txset = 0;
		for(int a = 0; a < 2; a++)
		{
			int afset = 0;
			GrowList<GameObject*> *l = 0; // = &((oobm[a])[t]);
			for(int i = 0; i < oobm[a].len; i++)
				if(oobm[a].getpnt(i)->tid == t)
					{l = oobm[a].getpnt(i)->objs; break;}
			if(!l) continue;

			for(int i = 0; i < l->len; i++)
			{
				GameObject *o = l->get(i);

				int dif = -1;
				if((o->flags & FGO_SELECTED) || (currentSelection == o))
				{
					if((o->flags & FGO_SELECTED) && (currentSelection == o))
						dif = 0xFFFF00FF;
					else
						dif = (currentSelection==o)?0xFFFF0000:0xFF0000FF;
				}

				//Model *md = o->objdef->subtypes[o->subtype].appear[o->appearance].def;
				Model *md = GetObjectModel(o);
				if(showrepresentations && o->objdef->representation) md = o->objdef->representation;
				md->prepare(); md->mesh->prepare();
				Mesh *msh = md->mesh;
				// If md != md->mesh, this means that md is an Anim!
				SetMatrices(o->scale, -o->orientation, o->position);
				Matrix mWorldCopy = mWorld;

				uint tm = (int)(current_time*1000.0f) - o->animtimeref;
				// If the current task has SYNCH_ANIMATION_TO_FRACTION, use it.
				if(md != md->mesh) if(o->ordercfg.order.len)
				{
					STask *st = &o->ordercfg.order.first->value.task.first->value;
					if(st->type->satf)
					{
						SequenceEnv env; env.self = o;
						tm = st->type->satf->get(&env) * ((Anim*)md)->dur;
						tm = st->lastsatf + (tm - st->lastsatf) * elapsed_time * 0.5f;
						if(tm >= ((Anim*)md)->dur) tm = ((Anim*)md)->dur - 1;
						st->lastsatf = tm;
					}
				}
				if(o->animlooping) if(md != md->mesh)
					tm %= ((Anim*)md)->dur;

				for(int g = 0; g < msh->ngrp; g++)
					if((msh->lstmatflags[g] == a) && (msh->lstmattid[g] == t))
					{
						if(!txset) {txset = 1; SetTexture(0, msh->lstmattex[g]);}
						if(!afset) {afset = 1; if(a) renderer->EnableAlphaTest(); else renderer->DisableAlphaTest();}

						(animsEnabled?md:md->mesh)->drawInBatch(mshbatch, g, o->color, dif, tm);
					}

				for(int j = 0; j < msh->nAttachPnts; j++)
					if(msh->attachPnts[j].model)
					if(md->isAttachPointOn(j, tm))
					{
						Model *apmd = msh->attachPnts[j].model;
						Mesh *apms = apmd->mesh;
						for(int g = 0; g < apms->ngrp; g++)
							if((apms->lstmatflags[g] == a) && (apms->lstmattid[g] == t))
							{
								if(!txset) {txset = 1; SetTexture(0, apms->lstmattex[g]);}
								if(!afset) {afset = 1; if(a) renderer->EnableAlphaTest(); else renderer->DisableAlphaTest();}

								uint aptm = current_time * 1000;
								if(apmd != apmd->mesh)
									aptm %= ((Anim*)apmd)->dur;

								Vector3 p, s;
								md->getAttachPointPos(&s, j, tm);
								TransformVector3(&p, &s, &mWorldCopy);
								SetMatrices(Vector3(1,1,1), Vector3(0,0,0), p);

								(animsEnabled?apmd:apms)->drawInBatch(mshbatch, g, o->color, dif, aptm);
							}
					}
			}

			l->clear();
			mshbatch->flush();
		}
		mshbatch->flush();
	}
	mshbatch->end();
}

//GrowList<GameObject*> visobj;	// Visible objects
IDirect3DVertexShader9 *meshvsh;

void InitScene()
{
	InitMap(); //InitMeshDrawing();
	//meshvsh = LoadVertexShader("mesh.vsh");
	CreateIdentityMatrix(&mIdentity);
}

#define mabs(a) ((a>=0)?(a):(-a))

int IsPointOnScreen(Vector3 a, float *zp)
{
	Vector3 pc;
	TransformCoord3(&pc, &a, &vpmatrix);
	if(zp) *zp = pc.z;
	if( (mabs(pc.x) >= 2*occlurate) || (mabs(pc.y) >= 2*occlurate) || (pc.z >= 1.0f))
		return 0;
	return 1;
}

// Based on Pick() from the D3D9 Pick sample (in the latest DirectX SDK)
void CalcRay()
{
	// Lots of other possibilities in wkbre21

	Vector3 v;
	v.x = (2.0f * mouseX / scrw - 1.0f) / matProj._11;
	v.y = (2.0f * (scrh-1-mouseY) / scrh - 1.0f) / matProj._22;
	v.z = 1.0f;

	TransformBackFromViewMatrix(&raydir, &v, &matView);
	raystart = camerapos;
}

int InLevel(Vector3 &v)
{
	if( (v.x >= 0) && (v.x < ((mapwidth -2*mapedge)*5)) )
	if( (v.z >= 0) && (v.z < ((mapheight-2*mapedge)*5)) )
		return 1;
	return 0;
}

int InMap(Vector3 &v)
{
	if( (v.x >= -mapedge*5) && (v.x < ((mapwidth -mapedge)*5)) )
	if( (v.z >= -mapedge*5) && (v.z < ((mapheight-mapedge)*5)) )
		return 1;
	return 0;
}

Vector3 stdownpos; int stdownvalid = 0;
Vector3 mapstdownpos; int mapstdownvalid = 0;

void CalcStampdownPos()
{
	float lv = 4;

	//if(!InMap(raystart)) {stdownvalid = mapstdownvalid = 0; return;}

	Vector3 ptt = raystart, vta;
	NormalizeVector3(&vta, &raydir);
	vta *= lv;

	float h;
	int nlp = farzvalue * 1.5f / lv;
	int m = (ptt.y < GetHeight(ptt.x, ptt.z)) ? 0 : 1;

	for(int i = 0; i < nlp; i++)
	{
		ptt += vta;
		if(!InMap(ptt)) continue; //{stdownvalid = mapstdownvalid = 0; return;}
		h = GetHeight(ptt.x, ptt.z);

		if(ptt.y == h)
			break;

		//if(  (!m && (ptt.y > h))
		//  || (m && (ptt.y < h)) )
		if(m ^ ((ptt.y > h)?1:0))
			{vta *= -0.5f; m = 1 - m;}
	}

	if(InMap(ptt))
		{mapstdownpos = ptt; mapstdownpos.y = h; mapstdownvalid = 1;}
	else	mapstdownvalid = 0;
	if(mapstdownvalid && InLevel(mapstdownpos))
		{stdownpos = mapstdownpos; stdownvalid = 1;}
	else	stdownvalid = 0;
}

void DrawObj(GameObject *o)
{
	float pntz;
	if((o->renderable && o->objdef->subtypes[o->subtype].appear[o->appearance].def)
	  || (showrepresentations && o->objdef->representation))
	{
		if(IsPointOnScreen(o->position, &pntz))
		{
			//Model *md = o->objdef->subtypes[o->subtype].appear[o->appearance].def;
			Model *md = GetObjectModel(o);
			if(showrepresentations && o->objdef->representation) md = o->objdef->representation;
			md->prepare(); md->mesh->prepare();
			Mesh *msh = md->mesh;
			objsdrawn++;
			SetMatrices(o->scale, -o->orientation, o->position);

			Vector3 sphPos = o->position;
			sphPos.y += msh->sphere[1] * o->scale.y;
			if((newSelZ == -1) || (pntz < newSelZ))
			if(SphereIntersectsRay(&sphPos, msh->sphere[3]*o->scale.y/2.0f, &raystart, &raydir))
				{newSelection = o; newSelZ = pntz;}

			if(multiSel)
			{
				Vector3 tdp;
				TransformCoord3(&tdp, &nullvector, &matrix);
				float ba = (float)((mselx<mouseX)?mselx:mouseX) * 2.0f / scrw - 1;
				float bb = (float)((msely<mouseY)?msely:mouseY) * 2.0f / scrh - 1;
				float bc = (float)((mselx>mouseX)?mselx:mouseX) * 2.0f / scrw - 1;
				float bd = (float)((msely>mouseY)?msely:mouseY) * 2.0f / scrh - 1;
				if((tdp.x >= ba) && (tdp.x <= bc) &&
				   (-tdp.y >= bb) && (-tdp.y <= bd))
					msellist.add(o);
			}

			if(!meshbatching)
			{
				if((o->flags & FGO_SELECTED) || (currentSelection == o))
				{
					renderer->EnableColorBlend();
					if((o->flags & FGO_SELECTED) && (currentSelection == o))
						renderer->SetBlendColor(0xFFFF00FF);
					else
						renderer->SetBlendColor((currentSelection==o)?0xFFFF0000:0xFF0000FF);
				}
				SetTransformMatrix(&matrix);
				msh->draw(o->color);
				if((o->flags & FGO_SELECTED) || (currentSelection == o))
					renderer->DisableColorBlend();
			}
			else
			{
			auto f = [o](Mesh *msh)
			{
				msh->prepare();
				for(int i = 0; i < msh->ngrp; i++)
				{
					OOBMTex *ot;
					int f = msh->lstmatflags[i]&1;
					int t = msh->lstmattid[i];
					for(int j = 0; j < oobm[f].len; j++)
					{
						ot = oobm[f].getpnt(j);
						if(ot->tid == t)
							{ot->objs->add(o); goto nextgrp;}
					}
					ot = oobm[f].addp();
					ot->tid = t;
					ot->objs = new GrowList<GameObject*>;
					ot->objs->add(o);
				nextgrp: ;
				}
			};
				f(msh);
				for(int i = 0; i < msh->nAttachPnts; i++)
					if(msh->attachPnts[i].model)
						f(msh->attachPnts[i].model->mesh);
			}
		}
	}

	for(DynListEntry<GameObject> *e = o->children.first; e; e = e->next)
		DrawObj(&e->value);
}

void SetFog()
{
	renderer->SetFog();
}

void DisableFog()
{
	renderer->DisableFog();
}

Model *GetObjTypeDefaultModel(CObjectDefinition *cod)
{
	Model *m = 0;
	int s = (cod->numsubtypes >= 2) ? 1 : 0;
	for(int i = 0; i < strAppearTag.len; i++)
		if(cod->subtypes[s].appear[i].def)
			{m = cod->subtypes[s].appear[i].def; break;}
	if(!m) if(cod->representation)
		m = cod->representation;
	return m;
}

extern CObjectDefinition *objtypeToStampdown;
extern goref playerToGiveStampdownObj;
extern float stampdownRot;
extern int mousetool;
extern goref newmanorplayer;

void DrawScene()
{
	BeginMeshDrawing();
	if(fogenabled) SetFog();
	SetConstantMatrices();

	newSelection = -1; SetMatrices(onevector, nullvector, nullvector); CalcRay();
	newSelZ = -1;
	DrawObj(levelobj);
	if(currentSelection.get() != newSelection.get())
	{
		currentSelection = newSelection;
		//printf("New selection: %i\n", newSelection);
	}

	if(meshbatching) DrawOOBM();

if(experimentalKeys) {
	// Stampdown
	CalcStampdownPos();
/*
	int od;
	if((od = FindObjDef(CLASS_CHARACTER, "Archer")) != -1)
	{
		if(stdownvalid)
		{
			renderer->BeginMeshDrawing();
			SetMatrices(onevector, nullvector, stdownpos);
			SetTransformMatrix(&matrix);
			objdef[od].subtypes[0].appear[0].def->draw(0);
			//sprintf(statustextbuf, "stdownpos = (%f, %f, %f)", stdownpos.x, stdownpos.y, stdownpos.z);
			//sprintf(statustextbuf, "raydir = (%f, %f, %f)", raydir.x, raydir.y, raydir.z);
			//sprintf(statustextbuf, "raystart = (%f, %f, %f)", raystart.x, raystart.y, raystart.z);
		}
		//else	strcpy(statustextbuf, "Invalid stampdown position.");
		//statustext = statustextbuf;
	}
*/
	if(objtypeToStampdown)
	if(stdownvalid)
	if(playerToGiveStampdownObj.valid())
	{
		// Align buildings in the grid.
		if(objtypeToStampdown->type == CLASS_BUILDING)
		{
			stdownpos.x = (int)(stdownpos.x / 5) * 5.0f + 2.5f;
			stdownpos.z = (int)(stdownpos.z / 5) * 5.0f + 2.5f;
			if(objtypeToStampdown->footprint)
			{
				int ogx = objtypeToStampdown->footprint->origin_x;
				int ogy = objtypeToStampdown->footprint->origin_z;
				stdownpos.x += ogx * cos(stampdownRot) - ogy * sin(stampdownRot);
				stdownpos.z += ogx * sin(stampdownRot) + ogy * cos(stampdownRot);
			}
			stdownpos.y = GetHeight(stdownpos.x, stdownpos.z);
		}
		Model *m = GetObjTypeDefaultModel(objtypeToStampdown);
		if(m)
		{
			renderer->BeginMeshDrawing();
			SetMatrices(objtypeToStampdown->scale, Vector3(0,-stampdownRot,0), stdownpos);
			SetTransformMatrix(&matrix);
			m->draw(playerToGiveStampdownObj->color);
		}
	}
	int moti;
	if(mousetool == 7)
	if(stdownvalid)
	if(newmanorplayer.valid())
	if((moti = FindObjDef(CLASS_BUILDING, "Manor")) != -1)
	{
		CObjectDefinition *cod = &(objdef[moti]);
		// Align buildings in the grid.
		stdownpos.x = (int)(stdownpos.x / 5) * 5.0f + 2.5f;
		stdownpos.z = (int)(stdownpos.z / 5) * 5.0f + 2.5f;
		stdownpos.y = GetHeight(stdownpos.x, stdownpos.z);
		Model *m = GetObjTypeDefaultModel(cod);
		if(m)
		{
			renderer->BeginMeshDrawing();
			SetMatrices(cod->scale, Vector3(0,-stampdownRot,0), stdownpos);
			SetTransformMatrix(&matrix);
			m->draw(newmanorplayer->color);
		}
	}
}

	if(enableMap)
	{
		SetMatrices(Vector3(5.0f, 1.0f, -5.0f), nullvector, Vector3(-mapedge*5,0,+mapheight*5-mapedge*5));
		SetTransformMatrix(&matrix);
		DrawMap();
		DrawTileHighlights();
		DrawLakes();
	}
	drawdebug = 0;
	if(fogenabled) DisableFog();
}