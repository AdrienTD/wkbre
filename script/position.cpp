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
		// depends on finder result's orientation?
		GameObject *o;
		f->begin(env);
		if(!(o = f->getnext())) {*po = nullpo; return;}
		valuetype a = u->get(env) * 2 * M_PI / 360, b = v->get(env);
		float x = cos(a), z = sin(a);
		po->pos.x = o->position.x + (x * b);
		po->pos.z = o->position.z + (z * b);
		po->pos.y = GetHeight(po->pos.x, po->pos.z);
		po->ori = nullVec3;
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
	}
	//ferr("Unknown position type."); return 0;
	int x = strUnknownPosition.find(word[0]);
	if(x == -1) {x = strUnknownPosition.len; strUnknownPosition.add(word[0]);}
	*wpnt += 1;
	return new PositionUnknown(x);
	//return new PositionUnknown();
}