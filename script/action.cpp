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

int scriptTraceOn = 1;

struct ActionUnknown : public CAction
{
	void run(SequenceEnv *c) {ferr("Action of unknown type executed.");}
};

struct ActionRemove : public CAction
{
	CFinder *f;
	ActionRemove(CFinder *a) : f(a) {}
	void run(SequenceEnv *c)
	{
		f->begin(c);
		GameObject *o;
		while(o = f->getnext())
			RemoveObject(o);
	}
};

struct ActionCreateObject : public CAction
{
	CObjectDefinition *def; CPosition *pos;
	ActionCreateObject(CObjectDefinition *a, CPosition *b) : def(a), pos(b) {}
	void run(SequenceEnv *c)
	{
		GameObject *o = c->self.get();
		if(!o) return;
		if(!o->player) return;
		o = CreateObject(def, o->player);
		PosOri po;
		pos->get(c, &po);
		o->position = po.pos;
		o->orientation = po.ori;
	}
};

struct ActionExecuteSequence : public CAction
{
	uint as; CFinder *f;
	ActionExecuteSequence(uint a, CFinder *b) : as(a), f(b) {}
	void run(SequenceEnv *c)
	{
		GameObject *o;
		f->begin(c);
		while(o = f->getnext())
		{
			SequenceEnv e;
			c->copyAll(&e);
			e.self = o; e.exec = c->self;
			actionseq[as]->run(&e);
		}
	}
};

// (?)
//     repeat Nx for obj1, then repeat Nx for obj2, ... (correct one)
// OR: repeat Nx creating all objects in one loop step (seems to be logical but isn't used)
// (?)
struct ActionRepeatSequence : public CAction
{
	uint as; CFinder *f; CValue *v;
	ActionRepeatSequence(uint a, CFinder *b, CValue *c) : as(a), f(b), v(c) {}
	void run(SequenceEnv *c)
	{
		GameObject *o;
		valuetype t = v->get(c);
		f->begin(c);
		while(o = f->getnext())
		{
			SequenceEnv e;
			c->copyAllButNotSelf(&e);
			e.self = o; e.exec = c->self;
			for(int i = 0; i < t; i++)
				actionseq[as]->run(&e);
		}
	}
};

struct ActionSetItem : public CAction
{
	uint x; CFinder *f; CValue *v;
	ActionSetItem(uint a, CFinder *b, CValue *c) : x(a), f(b), v(c) {}
	void run(SequenceEnv *c)
	{
		valuetype t = v->get(c); GameObject *o;
		f->begin(c);
		while(o = f->getnext())
			o->setItem(x, t);
	}
};

struct ActionIncreaseItem : public CAction
{
	uint x; CFinder *f; CValue *v;
	ActionIncreaseItem(uint a, CFinder *b, CValue *c) : x(a), f(b), v(c) {}
	void run(SequenceEnv *c)
	{
		valuetype t = v->get(c); GameObject *o;
		f->begin(c);
		while(o = f->getnext())
			o->setItem(x, o->getItem(x) + t);
	}
};

struct ActionDecreaseItem : public CAction
{
	uint x; CFinder *f; CValue *v;
	ActionDecreaseItem(uint a, CFinder *b, CValue *c) : x(a), f(b), v(c) {}
	void run(SequenceEnv *c)
	{
		valuetype t = v->get(c); GameObject *o;
		f->begin(c);
		while(o = f->getnext())
			o->setItem(x, o->getItem(x) - t);
	}
};

struct ActionUponCondition : public CAction
{
	CValue *v; ActionSeq *x, *y;
	ActionUponCondition(CValue *a, ActionSeq *b, ActionSeq *c) : v(a), x(b), y(c) {}
	void run(SequenceEnv *env)
	{
		if(stpo(v->get(env)))
			x->run(env);
		else if(y)
			y->run(env);
	}
};

struct ActionCreateObjectVia : public CAction
{
	CObjectCreation *oc; CFinder *creator;
	ActionCreateObjectVia(CObjectCreation *a, CFinder *b) : oc(a), creator(b) {}
	void run(SequenceEnv *env)
	{
		oc->run(env, creator);
	}
};

struct ActionSendEvent : public CAction
{
	int ev; CFinder *f;
	ActionSendEvent(int a, CFinder *b) : ev(a), f(b) {}
	void run(SequenceEnv *env)
	{
		// multi or solo? probably multi
		f->begin(env); GameObject *o;
		while(o = f->getnext())
			SendGameEvent(env, o, ev);
	}
};

struct ActionAssignAlias : public CAction
{
	uint t; CFinder *f;
	ActionAssignAlias(uint a, CFinder *b) : t(a), f(b) {}
	void run(SequenceEnv *env)
	{
		f->begin(env);
		GameObject *o;
		while(o = f->getnext())
			AliasObj(o, t);
	}
};

void ClearAlias(uint t)
{
	alias[t].clear();
}

struct ActionClearAlias : public CAction
{
	uint t;
	ActionClearAlias(uint a) : t(a) {}
	void run(SequenceEnv *env) {ClearAlias(t);}
};

void UnaliasObj(GameObject *o, int t)
{
	DynListEntry<goref> *n;
	for(DynListEntry<goref> *e = alias[t].first; e; e = n)
	{
		n = e->next;
		if(e->value.valid())
			if(e->value.get() == o)
				{alias[t].remove(e); return;}
	}
}

struct ActionUnassignAlias : public CAction
{
	uint t; CFinder *f;
	ActionUnassignAlias(uint a, CFinder *b) : t(a), f(b) {}
	void run(SequenceEnv *env)
	{
		f->begin(env);
		GameObject *o;
		while(o = f->getnext())
			UnaliasObj(o, t);
	}
};

struct ActionDisable : public CAction
{
	CFinder *f;
	ActionDisable(CFinder *a) : f(a) {}
	void run(SequenceEnv *env)
	{
		f->begin(env);
		GameObject *o;
		while(o = f->getnext())
			o->disableCount++;
	}
};

struct ActionEnable : public CAction
{
	CFinder *f;
	ActionEnable(CFinder *a) : f(a) {}
	void run(SequenceEnv *env)
	{
		f->begin(env);
		GameObject *o;
		while(o = f->getnext())
			if(o->disableCount)
				o->disableCount--;
	}
};

void ClearAssociates(GameObject *o, int t)
{
	for(DynListEntry<GOAssociation> *e = o->association.first; e; e = e->next)
		if(e->value.category == t)
		{
			DynListEntry<GameObjAndListEntry> *n;
			for(DynListEntry<GameObjAndListEntry> *f = e->value.associates.first; f; f = n)
			{
				n = f->next;
				f->value.l->remove(f->value.e);
				e->value.associates.remove(f);
			}
		}
}

struct ActionClearAssociates : public CAction
{
	int t; CFinder *f;
	ActionClearAssociates(int a, CFinder *b) : t(a), f(b) {}
	void run(SequenceEnv *env)
	{
		GameObject *o; f->begin(env);
		while(o = f->getnext())
			ClearAssociates(o, t);
	}
};

struct ActionRegisterAssociates : public CAction
{
	int t; CFinder *ors, *es;
	ActionRegisterAssociates(CFinder *a, int b, CFinder *c) : ors(a), t(b), es(c) {}
	void run(SequenceEnv *env)
	{
		GameObject *g; ors->begin(env);
		while(g = ors->getnext())
		{
			GameObject *a; es->begin(env);
			while(a = es->getnext())
				AssociateObjToObj(g, t, a);
		}
	}
};

struct ActionExecuteOneAtRandom : public CAction
{
	ActionSeq *as;
	ActionExecuteOneAtRandom(ActionSeq *a) : as(a) {}
	void run(SequenceEnv *env)
	{
		int r = rand() % as->code.len;
		as->code[r]->run(env);
	}
};

struct ActionTraceValue : public CAction
{
	char *s; CValue *v;
	ActionTraceValue(char *x, CValue *y) : s(x), v(y) {}
	void run(SequenceEnv *env)
	{
		if(scriptTraceOn)
			printf("TRACE_VALUE \"%s\": %f\n", s, v->get(env));
	}
};

struct ActionTraceFinderResults : public CAction
{
	char *s; CFinder *f;
	ActionTraceFinderResults(char *x, CFinder *y) : s(x), f(y) {}
	void run(SequenceEnv *env)
	{
		if(!scriptTraceOn) return;
		printf("TRACE_FINDER_RESULTS \"%s\": ", s);
		GameObject *o; int n = 0;
		f->begin(env);
		while(o = f->getnext())
			{printf(" %i", o->id); n++;}
		printf(" (%i objs.)\n", n);
	}
};

struct ActionTerminateThisTask : public CAction
{
	void run(SequenceEnv *env)
	{
		if(env->self.valid())
			TerminateTask(env->self.get());
	}
};

struct ActionTerminateThisOrder : public CAction
{
	void run(SequenceEnv *env)
	{
		if(env->self.valid())
			TerminateOrder(env->self.get());
	}
};

struct ActionTerminateTask : public CAction
{
	CFinder *f;
	ActionTerminateTask(CFinder *a) : f(a) {}
	void run(SequenceEnv *env)
	{
		f->begin(env);
		GameObject *o;
		while(o = f->getnext())
			TerminateTask(o);
	}
};

struct ActionTerminateOrder : public CAction
{
	CFinder *f;
	ActionTerminateOrder(CFinder *a) : f(a) {}
	void run(SequenceEnv *env)
	{
		f->begin(env);
		GameObject *o;
		while(o = f->getnext())
			TerminateOrder(o);
	}
};

struct ActionTrace : public CAction
{
	char *s;
	ActionTrace(char *a) : s(a) {}
	void run(SequenceEnv *env)
	{
		printf("TRACE \"%s\"\n", s);
	}
};

struct ActionAssignOrderVia : public CAction
{
	COrderAssignment *oa; CFinder *f;
	ActionAssignOrderVia(COrderAssignment *a, CFinder *b) : oa(a), f(b) {}
	void run(SequenceEnv *env)
	{
		f->begin(env);
		GameObject *o;
		while(o = f->getnext())
			AssignOrderVia(o, oa, env);
	}
};

struct ActionCancelOrder : public CAction
{
	CFinder *f;
	ActionCancelOrder(CFinder *a) : f(a) {}
	void run(SequenceEnv *env)
	{
		f->begin(env);
		GameObject *o;
		while(o = f->getnext())
			CancelOrder(o);
	}
};

struct ActionTransferControl : public CAction
{
	CFinder *a, *b;
	ActionTransferControl(CFinder *x, CFinder *y) : a(x), b(y) {}
	void run(SequenceEnv *env)
	{
		GameObject *o, *p = b->getfirst(env);
		a->begin(env);
		while(o = a->getnext())
			SetObjectParent(o, p);
	}
};

struct ActionSwitchAppearance : public CAction
{
	int apt; CFinder *f;
	ActionSwitchAppearance(int a, CFinder *b) : apt(a), f(b) {}
	void run(SequenceEnv *env)
	{
		GameObject *o;
		f->begin(env);
		while(o = f->getnext())
			if(o->objdef->subtypes[o->subtype].appear[o->appearance])
				o->appearance = apt;
	}
};

void ReadUponCond(char **pntfp, ActionSeq **a, ActionSeq **b);

CAction *ReadAction(char **pntfp, char **word)
{
	char **w = word;
	switch(stfind_cs(ACTION_str, ACTION_NUM, word[1]))
	{
		case ACTION_REMOVE:
			w += 2;
			return new ActionRemove(ReadFinder(&w));
		case ACTION_CREATE_OBJECT:
			{int d = FindObjDef(stfind_cs(CLASS_str, CLASS_NUM, word[2]), word[3]);
			mustbefound(d);
			w += 4;
			return new ActionCreateObject(&objdef[d], ReadCPosition(&w));}
		case ACTION_EXECUTE_SEQUENCE:
			{int d = strActionSeq.find(word[2]); mustbefound(d);
			w += 3;
			return new ActionExecuteSequence(d, ReadFinder(&w));}
		case ACTION_REPEAT_SEQUENCE:
			{int d = strActionSeq.find(word[2]); mustbefound(d);
			w += 3;
			CFinder *f = ReadFinder(&w);
			return new ActionRepeatSequence(d, f, ReadValue(&w));}
		case ACTION_SET_ITEM:
			{int d = strItems.find(word[2]); //mustbefound(d);
			w += 3;
			CFinder *f = ReadFinder(&w);
			return new ActionSetItem(d, f, ReadValue(&w));}
		case ACTION_INCREASE_ITEM:
			{int d = strItems.find(word[2]); //mustbefound(d);
			w += 3;
			CFinder *f = ReadFinder(&w);
			return new ActionIncreaseItem(d, f, ReadValue(&w));}
		case ACTION_DECREASE_ITEM:
			{int d = strItems.find(word[2]); //mustbefound(d);
			w += 3;
			CFinder *f = ReadFinder(&w);
			return new ActionDecreaseItem(d, f, ReadValue(&w));}
		case ACTION_UPON_CONDITION:
			{ActionSeq *a, *b;
			w += 2; CValue *v = ReadValue(&w);
			ReadUponCond(pntfp, &a, &b);
			return new ActionUponCondition(v, a, b);}
		case ACTION_CREATE_OBJECT_VIA:
			{CObjectCreation *oc = &(cobjcr[strObjCreation.find(word[2])]);
			w += 3;
			return new ActionCreateObjectVia(oc, ReadFinder(&w));}
		case ACTION_SEND_EVENT:
			{int ev = strGameEvent.find(word[2]);
			w += 3;
			return new ActionSendEvent(ev, ReadFinder(&w));}
		case ACTION_CLEAR_ALIAS:
			{int t = strAlias.find(word[2]); mustbefound(t);
			return new ActionClearAlias(t);}
		case ACTION_ASSIGN_ALIAS:
			{int t = strAlias.find(word[2]); mustbefound(t);
			w += 3; return new ActionAssignAlias(t, ReadFinder(&w));}
		case ACTION_UNASSIGN_ALIAS:
			{int t = strAlias.find(word[2]); mustbefound(t);
			w += 3; return new ActionUnassignAlias(t, ReadFinder(&w));}
		case ACTION_DISABLE:
			{w += 2; return new ActionDisable(ReadFinder(&w));}
		case ACTION_ENABLE:
			{w += 2; return new ActionEnable(ReadFinder(&w));}
		case ACTION_CLEAR_ASSOCIATES:
			{int t = strAssociateCat.find(word[2]); mustbefound(t);
			w += 3; return new ActionClearAssociates(t, ReadFinder(&w));}
		case ACTION_REGISTER_ASSOCIATES:
			{w += 2;
			CFinder *a = ReadFinder(&w);
			//printf("assocat: %s\n", *w);
			int t = strAssociateCat.find(*w); mustbefound(t); w++;
			CFinder *b = ReadFinder(&w);
			return new ActionRegisterAssociates(a, t, b);}
		case ACTION_EXECUTE_ONE_AT_RANDOM:
			return new ActionExecuteOneAtRandom(ReadActSeq(pntfp));
		case ACTION_TRACE_VALUE:
			w += 3;
			return new ActionTraceValue(strdup(word[2]), ReadValue(&w));
		case ACTION_TRACE_FINDER_RESULTS:
			w += 3;
			return new ActionTraceFinderResults(strdup(word[2]), ReadFinder(&w));
		case ACTION_TERMINATE_THIS_TASK:
			return new ActionTerminateThisTask();
		case ACTION_TERMINATE_THIS_ORDER:
			return new ActionTerminateThisOrder();
		case ACTION_TERMINATE_TASK:
			w += 2; return new ActionTerminateTask(ReadFinder(&w));
		case ACTION_TERMINATE_ORDER:
			w += 2; return new ActionTerminateOrder(ReadFinder(&w));
		case ACTION_TRACE:
			return new ActionTrace(strdup(word[2]));
		case ACTION_ASSIGN_ORDER_VIA:
			{w += 3;
			COrderAssignment *oa = &(orderAssign[strOrderAssign.find(word[2])]);
			return new ActionAssignOrderVia(oa, ReadFinder(&w));}
		case ACTION_CANCEL_ORDER:
			w += 2; return new ActionCancelOrder(ReadFinder(&w));
		case ACTION_TRANSFER_CONTROL:
			{w += 2;
			CFinder *a = ReadFinder(&w);
			CFinder *b = ReadFinder(&w);
			return new ActionTransferControl(a, b);}
		case ACTION_SWITCH_APPEARANCE:
			{w += 3;
			int apt = strAppearTag.find(word[2]); mustbefound(apt);
			return new ActionSwitchAppearance(apt, ReadFinder(&w));}
	}
	//ferr("Unknown action command."); return 0;
	return new ActionUnknown();
}

CAction *ReadActLine(char **pntfp)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	while(*pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if( *((uint*)(word[0])) == '_DNE' )
			return 0;
		if(nwords < 2) continue; //ferr("You need to put something after ACTION!");
		return ReadAction(pntfp, word);
	}
	ferr("UEOF"); return 0;
}

ActionSeq *ReadActSeq(char **pntfp)
{
	ActionSeq *as = new ActionSeq(); CAction *c;
	while(c = ReadActLine(pntfp))
		as->code.add(c);
	return as;
}

void ReadUponCond(char **pntfp, ActionSeq **a, ActionSeq **b)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	*a = new ActionSeq; *b = 0;
	ActionSeq *c = *a;
	while(*pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if( *((uint*)(word[0])) == '_DNE' )
			return;
		if(!stricmp(word[0], "ELSE")) if(!*b)
			{c = *b = new ActionSeq; continue;}
		if(nwords < 2) continue; //ferr("You need to put something after ACTION!");
		c->code.add(ReadAction(pntfp, word));
	}
	ferr("UEOF"); return;
}