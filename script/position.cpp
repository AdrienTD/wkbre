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

struct PositionUnknown : public CPosition
{
	void get(SequenceEnv *env, PosOri *po)
		{ferr("Cannot determinate a position of unknown type.");}
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
		float x = 0, z = 0; int n = 0; GameObject *o;
		f->begin(env);
		while(o = f->getnext())
		{
			x += o->position.x;
			z += o->position.z;
			n++;
		}
		if(!n) {*po = nullpo; return;}
		po->pos.x = x / n;
		po->pos.z = z / n;
		po->pos.y = GetHeight(po->pos.x, po->pos.z);
		po->ori = nullVec3;
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
	*wpnt += 1;
	return new PositionUnknown();
}