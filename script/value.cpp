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

#define RANDZEROTOONE ((rand()%32768)/32768.0f)

GrowStringList strUnknownValue;
struct ValueUnknown : public CValue
{
	//valuetype get(SequenceEnv *env)
	//	{ferr("Cannot determinate a value of unknown type."); return 0;}
	int x;
	ValueUnknown(int a) : x(a) {}
	valuetype get(SequenceEnv *env)
	{
		char tb[1024];
		strcpy(tb, "Value determinator ");
		strcat(tb, strUnknownValue.getdp(x));
		strcat(tb, " is unknown.");
		ferr(tb);
		return 0;
	}
};

struct ValueConstant : public CValue
{
	valuetype c;
	ValueConstant(valuetype a) {c = a;}
	valuetype get(SequenceEnv *env) {return c;}
};

struct ValueItemValue : public CValue
{
	int index; CFinder *finder;
	ValueItemValue(int a, CFinder *b) : index(a), finder(b) {}
	valuetype get(SequenceEnv *env)
	{
		finder->begin(env);
		GameObject *o = finder->getnext();
		if(!o) return 0;
		return o->getItem(index);
	}
};

struct ValueNumObjects : public CValue
{
	CFinder *f;
	ValueNumObjects(CFinder *a) : f(a) {}
	valuetype get(SequenceEnv *env)
	{
		f->begin(env);
		int i = 0;
		while(f->getnext()) i++;
		return i;
	}
};

// NOTE: Only looks after one object in the finder!
struct ValueEquationResult : public CValue
{
	uint x; CFinder *f;
	ValueEquationResult(int a, CFinder *b) : x(a), f(b) {}
	valuetype get(SequenceEnv *env)
	{
		SequenceEnv n;
		env->copyAll(&n);
		GameObject *o = f->getfirst(env);
		if(o) n.self = o;
		else n.self.deref();
		return equation[x]->get(&n);
	}
};

struct ValueObjectClass : public CValue
{
	uint c; CFinder *f;
	ValueObjectClass(uint a, CFinder *b) : c(a), f(b) {}
	valuetype get(SequenceEnv *env)
	{
		f->begin(env);
		GameObject *o = f->getnext();
		if(!o) return 0;
		do {
			if(o->objdef->type != c)
				return 0;
		} while(o = f->getnext());
		return 1;
	}
};

struct ValueObjectType : public CValue
{
	CObjectDefinition *c; CFinder *f;
	ValueObjectType(CObjectDefinition *a, CFinder *b) : c(a), f(b) {}
	valuetype get(SequenceEnv *env)
	{
		f->begin(env);
		GameObject *o = f->getnext();
		if(!o) return 0;
		do {
			if(o->objdef != c)
				return 0;
		} while(o = f->getnext());
		return 1;
	}
};

struct ValueValueTagInterpretation : public CValue
{
	int x; CFinder *f;
	ValueValueTagInterpretation(int a, CFinder *b) : x(a), f(b) {}
	valuetype get(SequenceEnv *env)
	{
		GameObject *o = f->getfirst(env); if(!o) return 0;
		SequenceEnv c; env->copyAll(&c); c.self = o;
		if(o->objdef->valueTagInterpret[x])
			return o->objdef->valueTagInterpret[x]->get(&c);
		else if(defValueTag[x])
			return defValueTag[x]->get(&c);
		else
			return 0;
	}
};

struct ValueObjectID_WKB : public CValue
{
	CFinder *f;
	ValueObjectID_WKB(CFinder *a) : f(a) {}
	valuetype get(SequenceEnv *env)
	{
		GameObject *o = f->getfirst(env);	// Only first object.
		if(o)
			return o->id;
		else
			return 0;
	}
};

struct ValueObjectID_WKO : public CValue
{
	CFinder *f; CFinder *g;
	ValueObjectID_WKO(CFinder *a, CFinder *b) : f(a), g(b) {}
	valuetype get(SequenceEnv *env)
	{
		GameObject *o = f->getfirst(env); if(!o) return 0;
		GameObject *p = g->getfirst(env); if(!p) return 0;
		return o == p;
	}
};

struct ValueIsSubsetOf : public CValue
{
	CFinder *x, *y;
	ValueIsSubsetOf(CFinder *a, CFinder *b) : x(a), y(b) {}
	valuetype get(SequenceEnv *env)
	{
		GrowList<GameObject*> a, b; GameObject *o;
		x->begin(env); y->begin(env);
		while(o = x->getnext()) a.add(o);
		while(o = y->getnext()) b.add(o);

		// OGE returns 0 when the first finder returns 0 objects.
		if(!a.len) return 0;

		for(uint i = 0; i < a.len; i++)
		{
			for(uint j = 0; j < b.len; j++)
				if(a[i] == b[j])
					goto isoc;
			return 0;
		isoc:	;
		}
		return 1;
	}
};

struct ValueWaterBeneath : public CValue
{
	CFinder *f;
	ValueWaterBeneath(CFinder *a) : f(a) {}
	valuetype get(SequenceEnv *env)
	{
		// TODO: Rewrite when water implemented.
		return 0;
	}
};

struct ValueBlueprintItemValue : public CValue
{
	int item; CObjectDefinition *od;
	ValueBlueprintItemValue(int a, CObjectDefinition *b) : item(a), od(b) {}
	valuetype get(SequenceEnv *env)
	{
		return od->startItems[item];
	}
};

struct ValueDistanceBetween : public CValue
{
	int mode; CFinder *x, *y;
	ValueDistanceBetween(int a, CFinder *b, CFinder *c) : mode(a), x(b), y(c) {}
	valuetype get(SequenceEnv *env)
	{
		PosOri a, b;
		int n = FinderToPosOri(&a, x, env) && FinderToPosOri(&b, y, env);
		if(!n) return 0;
		switch(mode)
		{
			case DISTCALCMODE_3D:
				{Vector3 s = a.pos - b.pos;
				return sqrt(s.x*s.x + s.y*s.y + s.z*s.z);}
			case DISTCALCMODE_HORIZONTAL:
				{Vector3 s = a.pos - b.pos;
				return sqrt(s.x*s.x + s.z*s.z);}
			case DISTCALCMODE_VERTICAL:
				return abs(a.pos.y - b.pos.y);
		}
		ferr("ValueDistanceBetween::get Unknown distance calculation mode.");
		return 0;
	}
};

int DiplomaticStatusAtLeast(int s, GameObject *x, GameObject *y)
{
	GameObject *a = x->player, *b = y->player;
	if(a == b) return 1;
	return GetDiplomaticStatus(a, b) <= s;
}

struct ValueDiplomaticStatusAtLeast : public CValue
{
	int s; CFinder *x, *y;
	ValueDiplomaticStatusAtLeast(int a, CFinder *b, CFinder *c) : s(a), x(b), y(c) {}
	valuetype get(SequenceEnv *env)
	{
		GameObject *a, *b;
		x->begin(env);
		a = x->getnext(); if(!a) return 0;
		do
		{
			y->begin(env);
			b = y->getnext(); if(!b) return 0;
			do
			{
				if(!DiplomaticStatusAtLeast(s, a, b))
					return 0;
			} while(b = y->getnext());
		} while(a = x->getnext());
		return 1;
	}
};

struct ValueCurrentlyDoingOrder : public CValue
{
	int cat; CFinder *f;
	ValueCurrentlyDoingOrder(int a, CFinder *b) : cat(a), f(b) {}
	valuetype get(SequenceEnv *env)
	{
		f->begin(env);
		GameObject *o = f->getnext();
		if(!o) return 0;
		do {
			if(!o->ordercfg.order.len) return 0;
			SOrder *s = &o->ordercfg.order.first->value;
			if(s->processState >= 4) return 0;
			if(s->type->cat != cat) return 0;
		} while(o = f->getnext());
		return 1;
	}
};

struct ValueTotalItemValue : public CValue
{
	int x; CFinder *f;
	ValueTotalItemValue(int a, CFinder *b) : x(a), f(b) {}
	valuetype get(SequenceEnv *env)
	{
		f->begin(env);
		GameObject *o; valuetype s = 0;
		while(o = f->getnext())
			s += o->getItem(x);
		return s;
	}
};

struct ValueHasAppearance : public CValue
{
	CFinder *f; int ap;
	ValueHasAppearance(CFinder *a, int b) : f(a), ap(b) {}
	valuetype get(SequenceEnv *env)
	{
		// TODO: What happens with more than 1 object?
		GameObject *o = f->getfirst(env);
		if(!o) return 0;
		return o->appearance == ap;
	}
};

struct ValueCanReach : public CValue
{
	CFinder *f; CPosition *p;
	ValueCanReach(CFinder *a, CPosition *b) : f(a), p(b) {}
	valuetype get(SequenceEnv *env)
	{
		// TODO: Correct answer later.
		return 1;
	}
};

struct ValueSamePlayer : public CValue
{
	CFinder *x, *y;
	ValueSamePlayer(CFinder *b, CFinder *c) : x(b), y(c) {}
	valuetype get(SequenceEnv *env)
	{
		GameObject *a, *b;
		x->begin(env);
		a = x->getnext(); if(!a) return 0;
		do
		{
			y->begin(env);
			b = y->getnext(); if(!b) return 0;
			do
			{
				if(a->player != b->player)
					return 0;
			} while(b = y->getnext());
		} while(a = x->getnext());
		return 1;
	}
};

struct ValueAreAssociated : public CValue
{
	CFinder *x, *y; int t;
	ValueAreAssociated(CFinder *a, int b, CFinder *c) : x(a), t(b), y(c) {}
	valuetype get(SequenceEnv *env)
	{
		GameObject *a, *b;
		x->begin(env);
		a = x->getnext(); if(!a) return 0;
		do {
			y->begin(env);
			b = y->getnext(); if(!b) return 0;
			do {
				if(!AreObjsAssociated(b, t, a))
					return 0;
			} while(b = y->getnext());
		} while(a = x->getnext());
		return 1;
	}
};

// TODO: An object is also idle if it has only cancelled/terminated sequences ???
struct ValueIsIdle : public CValue
{
	CFinder *f;
	ValueIsIdle(CFinder *a) : f(a) {}
	valuetype get(SequenceEnv *env)
	{
		GameObject *o;
		f->begin(env);
		o = f->getnext(); if(!o) return 0;
		do {
			if(o->ordercfg.order.len)
				return 0;
		} while(o = f->getnext());
		return 1;
	}
};

struct ValueIsDisabled : public CValue
{
	CFinder *f;
	ValueIsDisabled(CFinder *a) : f(a) {}
	valuetype get(SequenceEnv *env)
	{
		GameObject *o;
		f->begin(env);
		o = f->getnext(); if(!o) return 0;
		do {
			if(o->disableCount <= 0)
				return 0;
		} while(o = f->getnext());
		return 1;
	}
};

struct ValueIsMusicPlaying : public CValue
{
	CFinder *f;
	ValueIsMusicPlaying(CFinder *a) : f(a) {}
	valuetype get(SequenceEnv *env)
	{
		// TO IMPLEMENT
		return 1;
	}
};

struct ValueTileItem : public CValue
{
	int x; CFinder *f;
	ValueTileItem(int a, CFinder *b) : x(a), f(b) {}
	valuetype get(SequenceEnv *env)
	{
		// TO IMPLEMENT
		return 1;
	}
};

// NOTE: I've only seen NUM_REFERENCERS with a single object.
struct ValueNumReferencers : public CValue
{
	int x; CFinder *f;
	ValueNumReferencers(int a, CFinder *b) : x(a), f(b) {}
	valuetype get(SequenceEnv *env)
	{
		GameObject *o = f->getfirst(env);
		if(!o) return 0;
		int n = 0;
		for(DynListEntry<GameObject*> *e = o->referencers.first; e; e = e->next)
		{
			if(o->ordercfg.order.len)
			{
				SOrder *s = &o->ordercfg.order.first->value;
				STask *t = &s->task.getEntry(s->currentTask)->value;
				if(t->type->cat == x)
					n++;
			}
		}
		return n;
	}
};

struct ValueIsAccessible : public CValue
{
	CFinder *f, *g;
	ValueIsAccessible(CFinder *a, CFinder *b) : f(a), g(b) {}
	valuetype get(SequenceEnv *env)
	{
		// TO IMPLEMENT
		return 1;
	}
};

struct ValueHasDirectLineOfSightTo : public CValue
{
	CFinder *f; CPosition *p; CFinder *g;
	ValueHasDirectLineOfSightTo(CFinder *a, CPosition *b, CFinder *c) : f(a), p(b), g(c) {}
	valuetype get(SequenceEnv *env)
	{
		// TO IMPLEMENT
		return 1;
	}
};

// NOTE: If the second finder argument returns multiple objects and that for
// every object the objfinddef returns the same object, this object is counted more than
// one time. (There's no need to do a(n) union, simply an addition!)
struct ValueFinderResultsCount : public CValue
{
	int x; CFinder *f;
	CFinder *r;
	ValueFinderResultsCount(int a, CFinder *b) : x(a), f(b), r(0) {}
	valuetype get(SequenceEnv *env)
	{
		if(!r) r = finderdef[x]->clone();

		int no = 0; GameObject *o;
		f->begin(env);
		SequenceEnv nc;
		env->copyAll(&nc);

		while(o = f->getnext())
		{
			nc.self = o;
			r->begin(&nc);
			while(r->getnext())
				no++;
		}
		return no;
	}
};

// Only works in WKO and as an ENODE in an EQUATION. (I could use an Enode0V struct.)
struct ValueNumAssociates : public CValue
{
	int x; CFinder *f;
	ValueNumAssociates(int a, CFinder *b) : x(a), f(b) {}
	valuetype get(SequenceEnv *env)
	{
		int n = 0;
		GameObject *o; f->begin(env);
		while(o = f->getnext())
		{
			for(DynListEntry<GOAssociation> *e = o->association.first; e; e = e->next)
				if(e->value.category == x)
				{
					for(DynListEntry<GameObjAndListEntry> *g = e->value.associates.first; g; g = g->next)
						n++;
					break;
				}
		}
		return n;
	}
};

// Only works in WKO and as an ENODE in an EQUATION. (I could use an Enode0V struct.)
struct ValueNumAssociators : public CValue
{
	int x; CFinder *f;
	ValueNumAssociators(int a, CFinder *b) : x(a), f(b) {}
	valuetype get(SequenceEnv *env)
	{
		int n = 0;
		GameObject *o; f->begin(env);
		while(o = f->getnext())
		{
			for(DynListEntry<GOAssociation> *e = o->association.first; e; e = e->next)
				if(e->value.category == x)
				{
					for(DynListEntry<GameObjAndListEntry> *g = e->value.associators.first; g; g = g->next)
						n++;
					break;
				}
		}
		return n;
	}
};

struct ValueIndexedItemValue : public CValue
{
	int x; CValue *y; CFinder *f;
	ValueIndexedItemValue(int a, CValue *b, CFinder *c) : x(a), y(b), f(c) {}
	valuetype get(SequenceEnv *env)
	{
		GameObject *o = f->getfirst(env);
		if(!o) return 0;
		return o->getIndexedItem(x, y->get(env));
	}
};

struct ValueAngleBetween : public CValue
{
	int mode; CFinder *x, *y;
	ValueAngleBetween(int a, CFinder *b, CFinder *c) : mode(a), x(b), y(c) {}
	valuetype get(SequenceEnv *env)
	{
		// NOTE: doesn't seem to work, it always returns 0
		return 0;
		/*PosOri a, b;
		int n = FinderToPosOri(&a, x, env) && FinderToPosOri(&b, y, env);
		if(!n) return 0;
		...*/
	}
};

struct ValueCurrentlyDoingTask : public CValue
{
	int cat; CFinder *f;
	ValueCurrentlyDoingTask(int a, CFinder *b) : cat(a), f(b) {}
	valuetype get(SequenceEnv *env)
	{
		f->begin(env);
		GameObject *o = f->getnext();
		if(!o) return 0;
		do {
			if(!o->ordercfg.order.len) return 0;
			SOrder *s = &o->ordercfg.order.first->value;
			if(s->processState >= 4) return 0;
			STask *t = &s->task.getEntry(s->currentTask)->value;
			if(t->processState >= 4) return 0;
			if(t->type->cat != cat) return 0;
		} while(o = f->getnext());
		return 1;
	}
};

struct ValueIsVisible : public CValue
{
	CFinder *f, *g;
	ValueIsVisible(CFinder *a, CFinder *b) : f(a), g(b) {}
	valuetype get(SequenceEnv *env)
	{
		// TO IMPLEMENT
		return 1;
	}
};

struct ValueIsDiscovered : public CValue
{
	CFinder *f, *g;
	ValueIsDiscovered(CFinder *a, CFinder *b) : f(a), g(b) {}
	valuetype get(SequenceEnv *env)
	{
		// TO IMPLEMENT
		return 1;
	}
};

struct ValueWithinForwardArc : public CValue
{
	CFinder *f, *g; CValue *v, *w;
	ValueWithinForwardArc(CFinder *a, CFinder *b, CValue *c, CValue *d) : f(a), g(b), v(c), w(d) {}
	valuetype get(SequenceEnv *env)
	{
		PosOri o, p; FinderToPosOri(&o, f, env); FinderToPosOri(&p, g, env);
		valuetype md = w->get(env);
		Vector3 dt = p.pos - o.pos;
		if(dt.len2xz() > md) return 0;

		valuetype ag = ((v->get(env) / 2) * M_PI) / 180;
		Vector3 fr = Vector3(sin(o.ori.y), 0, -cos(o.ori.y));
		if(acos(fr.dot2xz(dt.normal2xz())) > ag) return 0;
		return 1;
	}
};

struct ValueBuildingType : public CValue
{
	int t; CFinder *f;
	ValueBuildingType(int a, CFinder *b) : t(a), f(b) {}
	valuetype get(SequenceEnv *env)
	{
		GameObject *o; f->begin(env);
		o = f->getnext(); if(!o) return 0;
		do {
			if(o->objdef->buildingType != t)
				return 0;
		} while(o = f->getnext());
		return 1;
	}
};

struct ValueAIControlled : public CValue
{
	CFinder *f;
	ValueAIControlled(CFinder *a) : f(a) {}
	valuetype get(SequenceEnv *env)
	{
		GameObject *o; f->begin(env);
		o = f->getnext(); if(!o) return 0;
		do {
			if(!o->aicontroller) return 0;
			if(o->aicontroller->masterPlan == -1)
			  if(!o->aicontroller->workOrders.len)
			    if(!o->aicontroller->commissions.len)
			      return 0;
		} while(o = f->getnext());
		return 1;
	}
};

// DEFINED_VALUE will use ValueConstant.

CValue *ReadValue(char ***wpnt)
{
	char **word = *wpnt; CValue *cv;
	switch(stfind_cs(VALUE_str, VALUE_NUM, word[0]))
	{
		case VALUE_CONSTANT:
			cv = new ValueConstant(atof(word[1]));
			*wpnt += 2; return cv;
		case VALUE_DEFINED_VALUE:
			cv = new ValueConstant(defvalue[strDefValue.find(word[1])]);
			*wpnt += 2; return cv;
		case VALUE_ITEM_VALUE:
			{int x = strItems.find(word[1]);
			if(x == -1) x = 0; //mustbefound(x); // TODO: WARNING
			*wpnt += 2;
			return new ValueItemValue(x, ReadFinder(wpnt));}
		case VALUE_NUM_OBJECTS:
			*wpnt += 1;
			return new ValueNumObjects(ReadFinder(wpnt));
		case VALUE_EQUATION_RESULT:
			{int x = strEquation.find(word[1]); mustbefound(x);
			*wpnt += 2;
			return new ValueEquationResult(x, ReadFinder(wpnt));}
		case VALUE_OBJECT_CLASS:
			{int d = stfind_cs(CLASS_str, CLASS_NUM, word[1]); mustbefound(d);
			*wpnt += 2; return new ValueObjectClass(d, ReadFinder(wpnt));}
		case VALUE_OBJECT_TYPE:
			{int d = FindObjDef(stfind_cs(CLASS_str, CLASS_NUM, word[1]), word[2]);
			mustbefound(d); *wpnt += 3;
			return new ValueObjectType(&objdef[d], ReadFinder(wpnt));}
		case VALUE_OBJECT_ID:
			{*wpnt += 1;
			if(1) return new ValueObjectID_WKB(ReadFinder(wpnt));
			// else
			CFinder *f = ReadFinder(wpnt);
			return new ValueObjectID_WKO(f, ReadFinder(wpnt));}
		case VALUE_IS_SUBSET_OF:
			{*wpnt += 1;
			CFinder *f = ReadFinder(wpnt);
			return new ValueIsSubsetOf(f, ReadFinder(wpnt));}
		case VALUE_WATER_BENEATH:
			{*wpnt += 1;
			return new ValueWaterBeneath(ReadFinder(wpnt));}
		case VALUE_BLUEPRINT_ITEM_VALUE:
			{int i = strItems.find(word[1]); mustbefound(i);
			int d = FindObjDef(stfind_cs(CLASS_str, CLASS_NUM, word[2]), word[3]);
			mustbefound(d);
			*wpnt += 4;
			return new ValueBlueprintItemValue(i, &objdef[d]);}
		case VALUE_DISTANCE_BETWEEN:
			{int m = stfind_cs(DISTCALCMODE_str, DISTCALCMODE_NUM, word[1]);
			mustbefound(m);
			*wpnt += 2;
			CFinder *a = ReadFinder(wpnt);
			return new ValueDistanceBetween(m, a, ReadFinder(wpnt));}
		case VALUE_DIPLOMATIC_STATUS_AT_LEAST:
			{int m = strDiplomaticStatus.find(word[1]); mustbefound(m);
			*wpnt += 2;
			CFinder *a = ReadFinder(wpnt);
			return new ValueDiplomaticStatusAtLeast(m, a, ReadFinder(wpnt));}
		case VALUE_CURRENTLY_DOING_ORDER:
			{int c = strOrderCat.find(word[1]);
			*wpnt += 2;
			return new ValueCurrentlyDoingOrder(c, ReadFinder(wpnt));}
		case VALUE_TOTAL_ITEM_VALUE:
			{int x = strItems.find(word[1]);
			if(x == -1) x = 0; //mustbefound(x); // TODO: WARNING
			*wpnt += 2;
			return new ValueTotalItemValue(x, ReadFinder(wpnt));}
		case VALUE_HAS_APPEARANCE:
			{*wpnt += 1;
			CFinder *f = ReadFinder(wpnt);
			int x = strAppearTag.find(**wpnt);
			*wpnt += 1;
			return new ValueHasAppearance(f, x);}
		case VALUE_CAN_REACH:
			{*wpnt += 1;
			CFinder *f = ReadFinder(wpnt);
			CPosition *p = ReadCPosition(wpnt);
			return new ValueCanReach(f, p);}
		case VALUE_SAME_PLAYER:
			{*wpnt += 1;
			CFinder *a = ReadFinder(wpnt);
			return new ValueSamePlayer(a, ReadFinder(wpnt));}
		case VALUE_ARE_ASSOCIATED:
			{*wpnt += 1;
			CFinder *a = ReadFinder(wpnt);
			int c = strAssociateCat.find(**wpnt); mustbefound(c);
			*wpnt += 1;
			return new ValueAreAssociated(a, c, ReadFinder(wpnt));}
		case VALUE_IS_IDLE:
			{*wpnt += 1;
			return new ValueIsIdle(ReadFinder(wpnt));}
		case VALUE_IS_DISABLED:
			{*wpnt += 1;
			return new ValueIsDisabled(ReadFinder(wpnt));}
		case VALUE_IS_MUSIC_PLAYING:
			{*wpnt += 1;
			return new ValueIsMusicPlaying(ReadFinder(wpnt));}
		case VALUE_VALUE_TAG_INTERPRETATION:
			{int x = strValueTag.find(word[1]); mustbefound(x);
			*wpnt += 2;
			return new ValueValueTagInterpretation(x, ReadFinder(wpnt));}
		case VALUE_TILE_ITEM:
			{int x = strItems.find(word[1]); mustbefound(x);
			*wpnt += 2;
			return new ValueTileItem(x, ReadFinder(wpnt));}
		case VALUE_NUM_REFERENCERS:
			{int x = strTaskCat.find(word[1]); mustbefound(x);
			*wpnt += 2;
			return new ValueNumReferencers(x, ReadFinder(wpnt));}
		case VALUE_IS_ACCESSIBLE:
			{*wpnt += 1;
			CFinder *a = ReadFinder(wpnt);
			return new ValueIsAccessible(a, ReadFinder(wpnt));}
		case VALUE_HAS_DIRECT_LINE_OF_SIGHT_TO:
			{*wpnt += 1;
			CFinder *a = ReadFinder(wpnt);
			CPosition *b = ReadCPosition(wpnt);
			return new ValueHasDirectLineOfSightTo(a, b, ReadFinder(wpnt));}
		case VALUE_FINDER_RESULTS_COUNT:
			{int x = strFinderDef.find(word[1]); mustbefound(x);
			*wpnt += 2; return new ValueFinderResultsCount(x, ReadFinder(wpnt));}
		case VALUE_INDEXED_ITEM_VALUE:
			{int x = strItems.find(word[1]);
			*wpnt += 2; CValue *y = ReadValue(wpnt);
			return new ValueIndexedItemValue(x, y, ReadFinder(wpnt));}
		case VALUE_ANGLE_BETWEEN:
			{int m = stfind_cs(DISTCALCMODE_str, DISTCALCMODE_NUM, word[1]);
			mustbefound(m);
			*wpnt += 2;
			CFinder *a = ReadFinder(wpnt);
			return new ValueAngleBetween(m, a, ReadFinder(wpnt));}
		case VALUE_CURRENTLY_DOING_TASK:
			{int c = strTaskCat.find(word[1]);
			*wpnt += 2;
			return new ValueCurrentlyDoingTask(c, ReadFinder(wpnt));}
		case VALUE_IS_VISIBLE:
			{*wpnt += 1; CFinder *f = ReadFinder(wpnt);
			return new ValueIsVisible(f, ReadFinder(wpnt));}
		case VALUE_IS_DISCOVERED:
			{*wpnt += 1; CFinder *f = ReadFinder(wpnt);
			return new ValueIsDiscovered(f, ReadFinder(wpnt));}
		case VALUE_WITHIN_FORWARD_ARC:
			{*wpnt += 1; CFinder *a = ReadFinder(wpnt);
			CFinder *b = ReadFinder(wpnt);
			CValue *c = ReadValue(wpnt);
			return new ValueWithinForwardArc(a, b, c, ReadValue(wpnt));}
		case VALUE_BUILDING_TYPE:
			{int t = stfind_cs(BUILDINGTYPE_str, BUILDINGTYPE_NUM, word[1]);
			*wpnt += 2;
			return new ValueBuildingType(t, ReadFinder(wpnt));}
		case VALUE_AI_CONTROLLED:
			*wpnt += 1; return new ValueAIControlled(ReadFinder(wpnt));

		// These values are in fact ENODEs (operands) with 0 subnodes, as such
		// why not make them work as normal value determinators?
		case VALUE_NUM_ASSOCIATES:
			{*wpnt += 2;
			int x = strAssociateCat.find(word[1]); mustbefound(x);
			return new ValueNumAssociates(x, ReadFinder(wpnt));}
		case VALUE_NUM_ASSOCIATORS:
			{*wpnt += 2;
			int x = strAssociateCat.find(word[1]); mustbefound(x);
			return new ValueNumAssociators(x, ReadFinder(wpnt));}
		case VALUE_BUILDING_TYPE_OPERAND:
			{char b[128];
			strcpy(b, word[1]);
			char *p = strrchr(b, '_');
			if(p) *p = 0;
			int t = stfind_cs(BUILDINGTYPE_str, BUILDINGTYPE_NUM, b);
			*wpnt += 2;
			return new ValueBuildingType(t, ReadFinder(wpnt));}
	}
	//ferr("Unknown value type."); return 0;
	int x = strUnknownValue.find(word[0]);
	if(x == -1) {x = strUnknownValue.len; strUnknownValue.add(word[0]);}
	*wpnt += 1;
	return new ValueUnknown(x);
	//return new ValueUnknown();
};

struct Enode1V : public CValue
{
	CValue *x;
	Enode1V(CValue *a) {x = a;}
};

struct Enode2V : public CValue
{
	CValue *x, *y;
	Enode2V(CValue *a, CValue *b) {x = a; y = b;}
};

struct Enode3V : public CValue
{
	CValue *x, *y, *z;
	Enode3V(CValue *a, CValue *b, CValue *c) {x = a; y = b; z = c;}
};

struct Enode4V : public CValue
{
	CValue *x, *y, *z, *w;
	Enode4V(CValue *a, CValue *b, CValue *c, CValue *d) : x(a), y(b), z(c), w(d) {}
};

struct EnodeAbsoluteValue : public Enode1V
{
	// using Enode1V::Enode1V; (only works in C++11)
	EnodeAbsoluteValue(CValue *a) : Enode1V(a) {}
	valuetype get(SequenceEnv *env) {valuetype a = x->get(env); if(a>=0) return a; return -a;}
};

struct EnodeNot : public Enode1V
{
	EnodeNot(CValue *a) : Enode1V(a) {}
	valuetype get(SequenceEnv *env) {return !stpo(x->get(env));}
};

struct EnodeIsZero : public Enode1V
{
	EnodeIsZero(CValue *a) : Enode1V(a) {}
	valuetype get(SequenceEnv *env) {return x->get(env) == 0;}
};

// NOTE: strictly or not?
// FOUND: yes, it is strictly
struct EnodeIsPositive : public Enode1V
{
	EnodeIsPositive(CValue *a) : Enode1V(a) {}
	valuetype get(SequenceEnv *env) {return x->get(env) > 0;}
};

// NOTE: strictly or not?
// IS_POSITIVE yes, but IS_NEGATIVE?
struct EnodeIsNegative : public Enode1V
{
	EnodeIsNegative(CValue *a) : Enode1V(a) {}
	valuetype get(SequenceEnv *env) {return x->get(env) < 0;}
};

struct EnodeRandomUpTo : public Enode1V
{
	EnodeRandomUpTo(CValue *a) : Enode1V(a) {}
	valuetype get(SequenceEnv *env) {return RANDZEROTOONE * x->get(env);}
};

struct EnodeAddition : public Enode2V
{
	EnodeAddition(CValue *a, CValue *b) : Enode2V(a, b) {}
	valuetype get(SequenceEnv *env) {return x->get(env) + y->get(env);}
};

struct EnodeSubtraction : public Enode2V
{
	EnodeSubtraction(CValue *a, CValue *b) : Enode2V(a, b) {}
	valuetype get(SequenceEnv *env) {return x->get(env) - y->get(env);}
};

struct EnodeMultiplication : public Enode2V
{
	EnodeMultiplication(CValue *a, CValue *b) : Enode2V(a, b) {}
	valuetype get(SequenceEnv *env) {return x->get(env) * y->get(env);}
};

struct EnodeDivision : public Enode2V
{
	EnodeDivision(CValue *a, CValue *b) : Enode2V(a, b) {}
	valuetype get(SequenceEnv *env) {return x->get(env) / y->get(env);}
};

struct EnodeAnd : public Enode2V
{
	EnodeAnd(CValue *a, CValue *b) : Enode2V(a, b) {}
	valuetype get(SequenceEnv *env) {return stpo(x->get(env)) && stpo(y->get(env));}
};

struct EnodeOr : public Enode2V
{
	EnodeOr(CValue *a, CValue *b) : Enode2V(a, b) {}
	valuetype get(SequenceEnv *env) {return stpo(x->get(env)) || stpo(y->get(env));}
};

struct EnodeEquals : public Enode2V
{
	EnodeEquals(CValue *a, CValue *b) : Enode2V(a, b) {}
	valuetype get(SequenceEnv *env) {return x->get(env) == y->get(env);}
};

struct EnodeLessThan : public Enode2V
{
	EnodeLessThan(CValue *a, CValue *b) : Enode2V(a, b) {}
	valuetype get(SequenceEnv *env) {return x->get(env) < y->get(env);}
};

struct EnodeLessThanOrEqualTo : public Enode2V
{
	EnodeLessThanOrEqualTo(CValue *a, CValue *b) : Enode2V(a, b) {}
	valuetype get(SequenceEnv *env) {return x->get(env) <= y->get(env);}
};

struct EnodeGreaterThan : public Enode2V
{
	EnodeGreaterThan(CValue *a, CValue *b) : Enode2V(a, b) {}
	valuetype get(SequenceEnv *env) {return x->get(env) > y->get(env);}
};

struct EnodeGreaterThanOrEqualTo : public Enode2V
{
	EnodeGreaterThanOrEqualTo(CValue *a, CValue *b) : Enode2V(a, b) {}
	valuetype get(SequenceEnv *env) {return x->get(env) >= y->get(env);}
};

struct EnodeMin : public Enode2V
{
	EnodeMin(CValue *a, CValue *b) : Enode2V(a, b) {}
	valuetype get(SequenceEnv *env)
	{
		valuetype a = x->get(env), b = y->get(env);
		return (a<b)?a:b;
	}
};

struct EnodeMax : public Enode2V
{
	EnodeMax(CValue *a, CValue *b) : Enode2V(a, b) {}
	valuetype get(SequenceEnv *env)
	{
		valuetype a = x->get(env), b = y->get(env);
		return (a>b)?a:b;
	}
};

struct EnodeRandomInteger : public Enode2V
{
	EnodeRandomInteger(CValue *a, CValue *b) : Enode2V(a, b) {}
	valuetype get(SequenceEnv *env)
	{
		valuetype a = x->get(env), b = y->get(env);
		//if(a == b) return (int)a;
		return (rand()%(int)(b-a+1)) + (int)a;
	}
};

struct EnodeRandomRange : public Enode2V
{
	EnodeRandomRange(CValue *a, CValue *b) : Enode2V(a, b) {}
	valuetype get(SequenceEnv *env)
	{
		valuetype a = x->get(env);
		return RANDZEROTOONE * (y->get(env) - a) + a;
	}
};

struct EnodeIfThenElse : public Enode3V
{
	EnodeIfThenElse(CValue *a, CValue *b, CValue *c) : Enode3V(a, b, c) {}
	valuetype get(SequenceEnv *env)
	{
		if(stpo(x->get(env)))
			return y->get(env);
		else
			return z->get(env);
	}
};

struct EnodeIsBetween : public Enode3V
{
	EnodeIsBetween(CValue *a, CValue *b, CValue *c) : Enode3V(a, b, c) {}
	valuetype get(SequenceEnv *env)
	{
		// NOTE: x included in ]y;z[ (y and z not included)
		//	 y must be < z, otherwise it doesn't work
		valuetype a = x->get(env);
		return (a > y->get(env)) && (a < z->get(env));
	}
};

struct EnodeFrontBackLeftRight : public Enode4V
{
	CFinder *f, *g;
	EnodeFrontBackLeftRight(CFinder *a0, CFinder *a1, CValue *a, CValue *b,
		CValue *c, CValue *d) : f(a0), g(a1), Enode4V(a, b, c, d) {}
	valuetype get(SequenceEnv *env)
	{
		PosOri o, p; FinderToPosOri(&o, f, env); FinderToPosOri(&p, g, env);
		Vector3 d = o.pos - p.pos;
		d /= d.len2xz();
		float a = acos(d.x);
		if(d.z > 0) a = (2*M_PI) - a;
		/****/ a = -a; /****/
		a = a - p.ori.y - (M_PI/4);
		while(a >= (2*M_PI)) a -= 2*M_PI;
		while(a < 0) a += 2*M_PI;
		//printf("FBLR: %f\n", a);
		switch((int)( (a / (M_PI/2.0f) )))
		{
			case 0: return y->get(env);
			case 1: return w->get(env);
			case 2: return x->get(env);
			case 3: return z->get(env);
		}
		ferr("FBLR bug"); return 0;
	}
};

CValue *ReadEqLine(char **pntfp)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	CValue *cv, *u, *v, *w, *x;
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!strcmp(word[0], "END_EQUATION"))
			return 0;
		if(nwords < 2) ferr("You need to put something after ENODE!");
		switch(stfind_cs(ENODE_str, ENODE_NUM, word[1]))
		{
			case ENODE_ABSOLUTE_VALUE:
				return new EnodeAbsoluteValue(ReadEqLine(pntfp));
			case ENODE_RANDOM_UP_TO:
				return new EnodeRandomUpTo(ReadEqLine(pntfp));
			case ENODE_ADDITION:
				u = ReadEqLine(pntfp); v = ReadEqLine(pntfp);
				return new EnodeAddition(u, v);
			case ENODE_SUBTRACTION:
				u = ReadEqLine(pntfp); v = ReadEqLine(pntfp);
				return new EnodeSubtraction(u, v);
			case ENODE_MULTIPLICATION:
				u = ReadEqLine(pntfp); v = ReadEqLine(pntfp);
				return new EnodeMultiplication(u, v);
			case ENODE_DIVISION:
				u = ReadEqLine(pntfp); v = ReadEqLine(pntfp);
				return new EnodeDivision(u, v);

			case ENODE_NOT:
				return new EnodeNot(ReadEqLine(pntfp));
			case ENODE_IS_ZERO:
				return new EnodeIsZero(ReadEqLine(pntfp));
			case ENODE_IS_POSITIVE:
				return new EnodeIsPositive(ReadEqLine(pntfp));
			case ENODE_IS_NEGATIVE:
				return new EnodeIsNegative(ReadEqLine(pntfp));
			case ENODE_AND:
				u = ReadEqLine(pntfp); v = ReadEqLine(pntfp);
				return new EnodeAnd(u, v);
			case ENODE_OR:
				u = ReadEqLine(pntfp); v = ReadEqLine(pntfp);
				return new EnodeOr(u, v);
			case ENODE_EQUALS:
				u = ReadEqLine(pntfp); v = ReadEqLine(pntfp);
				return new EnodeEquals(u, v);
			case ENODE_MIN:
				u = ReadEqLine(pntfp); v = ReadEqLine(pntfp);
				return new EnodeMin(u, v);
			case ENODE_MAX:
				u = ReadEqLine(pntfp); v = ReadEqLine(pntfp);
				return new EnodeMax(u, v);
			case ENODE_RANDOM_INTEGER:
				u = ReadEqLine(pntfp); v = ReadEqLine(pntfp);
				return new EnodeRandomInteger(u, v);
			case ENODE_RANDOM_RANGE:
				u = ReadEqLine(pntfp); v = ReadEqLine(pntfp);
				return new EnodeRandomRange(u, v);
			case ENODE_LESS_THAN:
				u = ReadEqLine(pntfp); v = ReadEqLine(pntfp);
				return new EnodeLessThan(u, v);
			case ENODE_GREATER_THAN:
				u = ReadEqLine(pntfp); v = ReadEqLine(pntfp);
				return new EnodeGreaterThan(u, v);
			case ENODE_LESS_THAN_OR_EQUAL_TO:
				u = ReadEqLine(pntfp); v = ReadEqLine(pntfp);
				return new EnodeLessThanOrEqualTo(u, v);
			case ENODE_GREATER_THAN_OR_EQUAL_TO:
				u = ReadEqLine(pntfp); v = ReadEqLine(pntfp);
				return new EnodeGreaterThanOrEqualTo(u, v);
			case ENODE_IF_THEN_ELSE:
				u = ReadEqLine(pntfp); v = ReadEqLine(pntfp); w = ReadEqLine(pntfp);
				return new EnodeIfThenElse(u, v, w);
			case ENODE_IS_BETWEEN:
				u = ReadEqLine(pntfp); v = ReadEqLine(pntfp); w = ReadEqLine(pntfp);
				return new EnodeIsBetween(u, v, w);
			case ENODE_FRONT_BACK_LEFT_RIGHT:
				{char **t = word + 2;
				CFinder *a = ReadFinder(&t); CFinder *b = ReadFinder(&t);
				u = ReadEqLine(pntfp); v = ReadEqLine(pntfp); w = ReadEqLine(pntfp); x = ReadEqLine(pntfp);
				return new EnodeFrontBackLeftRight(a, b, u, v, w, x);}
		}
		char **pw = &word[1];
		return ReadValue(&pw);
	}
	ferr("UEOF"); return 0;
}