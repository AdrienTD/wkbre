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

#include "../global.h"

Vector3 nullVec3(0, 0, 0);
PosOri nullpo = {Vector3(0,0,0),Vector3(0,0,0)};

int FinderToPosOri(PosOri *p, CFinder *f, SequenceEnv *env)
{
	int no = 0; GameObject *o;
	p->pos.x = p->pos.y = p->pos.z = p->ori.x = p->ori.y = p->ori.z = 0;
	f->begin(env);
	while(o = f->getnext())
	{
		p->pos += o->position;
		p->ori += o->orientation;
		no++;
	}
	if(!no) return 0;
	p->pos /= no;
	p->ori /= no;
	p->pos.y = GetHeight(p->pos.x, p->pos.z);
	return no;
}

GrowStringList strUnknownPosition;
struct PositionUnknown : public CPosition
{
	//void get(SequenceEnv *env, PosOri *po)
	//	{ferr("Cannot determinate a position of unknown type.");}
	int x;
	PositionUnknown(int a) : x(a) {}
	void get(SequenceEnv *env, PosOri *po)
	{
		char tb[1024];
		strcpy(tb, "Position determinator ");
		strcat(tb, strUnknownPosition.getdp(x));
		strcat(tb, " is unknown.");
		ferr(tb);
	}
};

struct PositionCentreOfMap : public CPosition
{
	PositionCentreOfMap() {}
	void get(SequenceEnv *env, PosOri *po)
	{
		po->pos.x = (mapwidth-2*mapedge)*5.0f/2.0f;
		po->pos.z = (mapheight-2*mapedge)*5.0f/2.0f;
		po->pos.y = GetHeight(po->pos.x, po->pos.z);
		po->ori = Vector3(0,0,0);
	}
};

struct PositionLocationOf : public CPosition
{
	CFinder *f;
	PositionLocationOf(CFinder *a) : f(a) {}
	void get(SequenceEnv *env, PosOri *po)
	{
		FinderToPosOri(po, f, env);
	}
};

//#define M_PI 3.1415

struct PositionOutAtAngle : public CPosition
{
	CFinder *f; CValue *u, *v;
	PositionOutAtAngle(CFinder *a, CValue *b, CValue *c) : f(a), u(b), v(c) {}
	void get(SequenceEnv *env, PosOri *po)
	{
		FinderToPosOri(po, f, env);
		po->ori.y -= u->get(env) * M_PI / 180;
		float l = v->get(env);
		po->pos.x += l * sin(po->ori.y);
		po->pos.z -= l * cos(po->ori.y);
	}
};

struct PositionNearestValidPositionFor : public CPosition
{
	CFinder *f; CPosition *p;
	PositionNearestValidPositionFor(CFinder *a, CPosition *b) : f(a), p(b) {}
	void get(SequenceEnv *env, PosOri *po)
	{
		// TO IMPLEMENT
		p->get(env, po);
	}
};

struct PositionNearestAttachmentPoint : public CPosition
{
	int t; CFinder *f; CPosition *p; CValue *v;
	PositionNearestAttachmentPoint(int a, CFinder *b, CPosition *c, CValue *d) : t(a), f(b), p(c), v(d) {}
	void get(SequenceEnv *env, PosOri *po)
	{
/*
		// TO IMPLEMENT
		GameObject *o = f->getfirst(env);
		po->pos = o->position;
		po->ori = o->orientation;
*/
		GameObject *o = f->getfirst(env);
		PosOri ip;
		p->get(env, &ip);
		int tag = strAttachPntTags.find(strAttachPointType.getdp(t));
		if(tag == -1)
		{
			printf("AP not found in model.\n");
			po->pos = o->position;
			po->ori = o->orientation;
			return;
		}
		Model *mdl = GetObjectModel(o);
		Mesh *msh = mdl->mesh;
		boolean first = 1; float lenscore;
		for(int i = 0; i < msh->nAttachPnts; i++)
		{
			AttachmentPoint *ap = &(msh->attachPnts[i]);
			if(ap->tag == tag)
			{
				Vector3 appos;
				mdl->getAttachPointPos(&appos, i, 0);
				appos += o->position;
				if(!first)
					if((appos - ip.pos).sqlen3() > lenscore)
						continue;
				else	first = 0;
				po->pos = appos;
				po->ori = o->orientation;
				lenscore = (appos - ip.pos).sqlen3();
			}
		}
	}
};

struct PositionAbsolutePosition : public CPosition
{
	float x, y, z, o, p;
	PositionAbsolutePosition(float a, float b, float c, float d, float e) : x(a), y(b), z(c), o(d), p(e) {}
	void get(SequenceEnv *env, PosOri *po)
	{
		po->pos.x = x; po->pos.y = y; po->pos.z = z;
		po->ori.x = o; po->ori.y = p;
	}
};

struct PositionSpawnTilePosition : public CPosition
{
	CFinder *f;
	PositionSpawnTilePosition(CFinder *a) : f(a) {}
	void get(SequenceEnv *env, PosOri *po)
	{
		// TO IMPLEMENT
		GameObject *o = f->getfirst(env);
		po->pos = o->position;
		po->ori = o->orientation;
	}
};

struct PositionThisSideOf : public CPosition
{
	CFinder *x, *y; CValue *v;
	PositionThisSideOf(CFinder *a, CFinder *b, CValue *c) : x(a), y(b), v(c) {}
	void get(SequenceEnv *env, PosOri *po)
	{
		PosOri o, p; FinderToPosOri(&o, x, env); FinderToPosOri(&p, y, env);
		Vector3 d = (o.pos - p.pos).normal();
		po->pos = p.pos + (d * v->get(env));
		po->ori.x = po->ori.z = 0; po->ori.y = atan2(d.x, -d.z);
	}
};

struct PositionTheOtherSideOf : public CPosition
{
	CFinder *x, *y; CValue *v;
	PositionTheOtherSideOf(CFinder *a, CFinder *b, CValue *c) : x(a), y(b), v(c) {}
	void get(SequenceEnv *env, PosOri *po)
	{
		PosOri o, p; FinderToPosOri(&o, x, env); FinderToPosOri(&p, y, env);
		Vector3 d = (p.pos - o.pos).normal();
		po->pos = p.pos + (d * v->get(env));
		po->ori.x = po->ori.z = 0; po->ori.y = atan2(d.x, -d.z);
	}
};

struct PositionTowards : public CPosition
{
	CFinder *x, *y; CValue *v;
	PositionTowards(CFinder *a, CFinder *b, CValue *c) : x(a), y(b), v(c) {}
	void get(SequenceEnv *env, PosOri *po)
	{
		PosOri o, p; FinderToPosOri(&o, x, env); FinderToPosOri(&p, y, env);
		Vector3 d = (p.pos - o.pos).normal();
		po->pos = o.pos + (d * v->get(env));
		po->ori.x = po->ori.z = 0; po->ori.y = atan2(d.x, -d.z);
	}
};

struct PositionAwayFrom : public CPosition
{
	CFinder *x, *y; CValue *v;
	PositionAwayFrom(CFinder *a, CFinder *b, CValue *c) : x(a), y(b), v(c) {}
	void get(SequenceEnv *env, PosOri *po)
	{
		PosOri o, p; FinderToPosOri(&o, x, env); FinderToPosOri(&p, y, env);
		Vector3 d = (o.pos - p.pos).normal();
		po->pos = o.pos + (d * v->get(env));
		po->ori.x = po->ori.z = 0; po->ori.y = atan2(d.x, -d.z);
	}
};

struct PositionInFrontOf : public CPosition
{
	CFinder *f; CValue *v;
	PositionInFrontOf(CFinder *a, CValue *b) : f(a), v(b) {}
	void get(SequenceEnv *env, PosOri *po)
	{
		FinderToPosOri(po, f, env);
		Vector3 d; d.x = sin(po->ori.y); d.y = 0; d.z = -cos(po->ori.y);
		po->pos += d * v->get(env);
	}
};

struct PositionFiringAttachmentPoint : public CPosition
{
	void get(SequenceEnv *env, PosOri *po)
	{
		if(!env->self.valid())
			{*po = nullpo; return;}
		po->pos = env->self->position + Vector3(0, 1, 0);
		po->ori = env->self->orientation;
	}
};

struct PositionNearestValidStampdownPos : public CPosition
{
	CObjectDefinition *od; CPosition *p;
	PositionNearestValidStampdownPos(CObjectDefinition *a, CPosition *b) : od(a), p(b) {}
	void get(SequenceEnv *env, PosOri *po)
	{
		// TO IMPLEMENT
		p->get(env, po);
	}
};

struct PositionOffsetFrom : public CPosition
{
	CFinder *f; CValue *x, *y, *z;
	PositionOffsetFrom(CFinder *a, CValue *b, CValue *c, CValue *d) : f(a), x(b), y(c), z(d) {}
	void get(SequenceEnv *env, PosOri *po)
	{
		FinderToPosOri(po, f, env);
		po->pos.x += x->get(env);
		po->pos.y += y->get(env);
		po->pos.z += z->get(env);
	}
};

struct PositionDestinationOf : public CPosition
{
	CFinder *f;
	PositionDestinationOf(CFinder *a) : f(a) {}
	void get(SequenceEnv *env, PosOri *po)
	{
		FinderToPosOri(po, f, env);
		GameObject *o = f->getfirst(env);
		if(!o) return;
		if(!o->ordercfg.order.len) return;
		SOrder *s = &o->ordercfg.order.first->value;
		STask *t = &s->task.getEntry(s->currentTask)->value;
		if(t->type->type == ORDTSKTYPE_MOVE)
		{
			if(t->destinations.len)
			{
				po->pos.x = t->destinations.first->value.x;
				po->pos.z = t->destinations.first->value.y;
				po->pos.y = GetHeight(po->pos.x, po->pos.z);
			}
		}
		else
		{
			if(t->target.valid())
				po->pos = t->target->position;
		}
	}
};

struct PositionMatchingOffset : public CPosition
{
	CPosition *cp, *cr, *co;
	PositionMatchingOffset(CPosition *a, CPosition *b, CPosition *c) : cp(a), cr(b), co(c) {}
	void get(SequenceEnv *env, PosOri *ret)
	{
		PosOri pp, pr, po;
		cp->get(env, &pp); cr->get(env, &pr); co->get(env, &po);

		float ro, rl, vo;
		Vector3 v = po.pos - pr.pos;
		vo = atan2(v.x, -v.z);
		ro = vo - pr.ori.y;
		rl = v.len2xz();

		ro += pp.ori.y;
		Vector3 nv = Vector3(sin(ro) * rl, 0, cos(ro) * rl);
		ret->pos = pp.pos + nv;
		ret->ori = Vector3(0,0,0); // TODO (?)
	}
};

CPosition *ReadCPosition(char ***wpnt)
{
	char **word = *wpnt; CPosition *cv;
	switch(stfind_cs(POSITION_str, POSITION_NUM, word[0]))
	{
		case POSITION_CENTRE_OF_MAP:
			*wpnt += 1;
			return new PositionCentreOfMap();
		case POSITION_LOCATION_OF:
			*wpnt += 1;
			return new PositionLocationOf(ReadFinder(wpnt));
		case POSITION_OUT_AT_ANGLE:
			{*wpnt += 1; CFinder *a = ReadFinder(wpnt);
			CValue *b = ReadValue(wpnt);
			return new PositionOutAtAngle(a, b, ReadValue(wpnt));}
		case POSITION_NEAREST_VALID_POSITION_FOR:
			{*wpnt += 1;
			CFinder *f = ReadFinder(wpnt);
			return new PositionNearestValidPositionFor(f, ReadCPosition(wpnt));}
		case POSITION_NEAREST_ATTACHMENT_POINT:
			{int t = strAttachPointType.find(word[1]);
			*wpnt += 2;
			CFinder *f = ReadFinder(wpnt);
			CPosition *p = ReadCPosition(wpnt);
			CValue *v = 0;
			if(**wpnt)
				if(!stricmp(**wpnt, "CHOOSE_FROM_NEAREST"))
					{*wpnt += 1; v = ReadValue(wpnt);}
			return new PositionNearestAttachmentPoint(t, f, p, v);}
		case POSITION_ABSOLUTE_POSITION:
			//*wpnt += 6;
			//return new PositionAbsolutePosition(atof(word[1]), atof(word[2]),
			//	atof(word[3]), atof(word[4]), atof(word[5]));
			{float a, b, c, d, e;
			a = atof(word[1]); b = atof(word[2]); c = atof(word[3]);
			if(word[4]) {d = atof(word[4]); e = atof(word[5]); *wpnt += 6;}
			else {d = e = 0; *wpnt += 4;}
			return new PositionAbsolutePosition(a, b, c, d, e);}
		case POSITION_SPAWN_TILE_POSITION:
			*wpnt += 1;
			return new PositionSpawnTilePosition(ReadFinder(wpnt));
		case POSITION_THIS_SIDE_OF:
			{*wpnt += 1;
			CFinder *x = ReadFinder(wpnt); CFinder *y = ReadFinder(wpnt);
			return new PositionThisSideOf(x, y, ReadValue(wpnt));}
		case POSITION_THE_OTHER_SIDE_OF:
			{*wpnt += 1;
			CFinder *x = ReadFinder(wpnt); CFinder *y = ReadFinder(wpnt);
			return new PositionTheOtherSideOf(x, y, ReadValue(wpnt));}
		case POSITION_TOWARDS:
			{*wpnt += 1;
			CFinder *x = ReadFinder(wpnt); CFinder *y = ReadFinder(wpnt);
			return new PositionTowards(x, y, ReadValue(wpnt));}
		case POSITION_AWAY_FROM:
			{*wpnt += 1;
			CFinder *x = ReadFinder(wpnt); CFinder *y = ReadFinder(wpnt);
			return new PositionAwayFrom(x, y, ReadValue(wpnt));}
		case POSITION_IN_FRONT_OF:
			{*wpnt += 1;
			CFinder *f = ReadFinder(wpnt);
			return new PositionInFrontOf(f, ReadValue(wpnt));}
		case POSITION_FIRING_ATTACHMENT_POINT:
			*wpnt += 1; return new PositionFiringAttachmentPoint();
		case POSITION_NEAREST_VALID_STAMPDOWN_POS:
			{*wpnt += 1;
			// Oddity in Auto Farm Peasant workaround.
			if(!strcmp(**wpnt, "BUILDING")) *wpnt += 1;
			int od = FindObjDef(CLASS_BUILDING, **wpnt); mustbefound(od);
			CObjectDefinition *a = &objdef[od];
			*wpnt += 1; return new PositionNearestValidStampdownPos(a, ReadCPosition(wpnt));}
		case POSITION_OFFSET_FROM:
			{*wpnt += 1;
			CFinder *f = ReadFinder(wpnt);
			CValue *x = ReadValue(wpnt);
			CValue *y = ReadValue(wpnt);
			CValue *z = ReadValue(wpnt);
			return new PositionOffsetFrom(f, x, y, z);}
		case POSITION_DESTINATION_OF:
			{*wpnt += 1;
			return new PositionDestinationOf(ReadFinder(wpnt));}
		case POSITION_MATCHING_OFFSET:
			{*wpnt += 1;
			CPosition *x = ReadCPosition(wpnt);
			CPosition *y = ReadCPosition(wpnt);
			CPosition *z = ReadCPosition(wpnt);
			return new PositionMatchingOffset(x, y, z);}
	}
	//ferr("Unknown position type."); return 0;
	int x = strUnknownPosition.find(word[0]);
	if(x == -1) {x = strUnknownPosition.len; strUnknownPosition.add(word[0]);}
	*wpnt += 1;
	return new PositionUnknown(x);
	//return new PositionUnknown();
}