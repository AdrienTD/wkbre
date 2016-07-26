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

GrowStringList strUnknownAction;
struct ActionUnknown : public CAction
{
	//void run(SequenceEnv *c) {ferr("Action of unknown type executed.");}
	int x;
	ActionUnknown(int a) : x(a) {}
	void run(SequenceEnv *c)
	{
		char tb[1024];
		strcpy(tb, "Action ");
		strcat(tb, strUnknownAction.getdp(x));
		strcat(tb, " is unknown.");
		ferr(tb);
	}
};

struct ActionDoNothing : public CAction
{
	void run(SequenceEnv *c) {;}
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
		o->position = po.pos; GOPosChanged(o);
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

struct ActionClearAlias : public CAction
{
	uint t;
	ActionClearAlias(uint a) : t(a) {}
	void run(SequenceEnv *env) {ClearAlias(t);}
};

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

struct ActionDeregisterAssociates : public CAction
{
	int t; CFinder *ors, *es;
	ActionDeregisterAssociates(CFinder *a, int b, CFinder *c) : ors(a), t(b), es(c) {}
	void run(SequenceEnv *env)
	{
		GameObject *g; ors->begin(env);
		while(g = ors->getnext())
		{
			GameObject *a; es->begin(env);
			while(a = es->getnext())
				DisassociateObjToObj(g, t, a);
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
			if(o->objdef->subtypes[o->subtype].appear[apt])
				o->appearance = apt;
	}
};

struct ActionConvertTo : public CAction
{
	CObjectDefinition *d; CFinder *f;
	ActionConvertTo(CObjectDefinition *a, CFinder *b) : d(a), f(b) {}
	void run(SequenceEnv *env)
	{
		GameObject *o;
		f->begin(env);
		while(o = f->getnext())
			ConvertObject(o, d);
	}
};

struct ActionConvertAccordingToTag : public CAction
{
	int t; CFinder *f;
	ActionConvertAccordingToTag(int a, CFinder *b) : t(a), f(b) {}
	void run(SequenceEnv *env)
	{
		GameObject *o;
		f->begin(env);
		while(o = f->getnext())
			if(o->objdef->mappedType[t])
				ConvertObject(o, o->objdef->mappedType[t]);
	}
};

struct ActionExecuteSequenceAfterDelay : public CAction
{
	int x; CFinder *f; CValue *v;
	ActionExecuteSequenceAfterDelay(int a, CFinder *b, CValue *c) : x(a), f(b), v(c) {}
	void run(SequenceEnv *env)
	{
		f->begin(env);
		GameObject *o = f->getnext(); if(!o) return;
		delayedSeq.add();
		DelayedSequenceEntry *dse = &delayedSeq.last->value;
		dse->time = current_time + v->get(env);
		dse->actseq = x;
		dse->executor = env->self;
		dse->unk = 1;

		GrowList<GameObject*> gl;
		do {
			gl.add(o);
		} while(o = f->getnext());

		dse->numobj = gl.len;
		dse->obj = new goref[dse->numobj];
		for(int i = 0; i < dse->numobj; i++)
			dse->obj[i] = gl[i];
	}
};

struct ActionAddReaction : public CAction
{
	int x; CFinder *f;
	ActionAddReaction(int a, CFinder *b) : x(a), f(b) {}
	void run(SequenceEnv *env)
	{
		f->begin(env);
		GameObject *o;
		while(o = f->getnext())
			AddReaction(o, x);
	}
};

struct ActionRemoveReaction : public CAction
{
	int x; CFinder *f;
	ActionRemoveReaction(int a, CFinder *b) : x(a), f(b) {}
	void run(SequenceEnv *env)
	{
		f->begin(env);
		GameObject *o;
		while(o = f->getnext())
			RemoveReaction(o, x);
	}
};

struct ActionChangeReactionProfile : public CAction
{
	int m, x; CFinder *f;
	ActionChangeReactionProfile(int c, int a, CFinder *b) : m(c), x(a), f(b) {}
	void run(SequenceEnv *env)
	{
		f->begin(env);
		GameObject *o;
		while(o = f->getnext())
		{
			if(m)	RemoveReaction(o, x);
			else	AddReaction(o, x);
		}
	}
};

struct ActionSetScale : public CAction
{
	CFinder *f; CValue *x, *y, *z;
	ActionSetScale(CFinder *a, CValue *b, CValue *c, CValue *d) : f(a), x(b), y(c), z(d) {}
	void run(SequenceEnv *env)
	{
		float a = x->get(env), b = y->get(env), c = z->get(env);
		GameObject *o;
		f->begin(env);
		while(o = f->getnext())
		{
			o->scale.x = a;
			o->scale.y = b;
			o->scale.z = c;
		}
	}
};

struct ActionTerminate : public CAction
{
	CFinder *f;
	ActionTerminate(CFinder *a) : f(a) {}
	void run(SequenceEnv *c)
	{
		f->begin(c);
		GameObject *o;
		while(o = f->getnext())
		{
			// TODO: send events like "On Termination" and/or "On Destruction"
			RemoveObject(o);
		}
	}
};

struct ActionExecuteSequenceOverPeriod : public CAction
{
	int x; CFinder *f; CValue *v;
	ActionExecuteSequenceOverPeriod(int a, CFinder *b, CValue *c) : x(a), f(b), v(c) {}
	void run(SequenceEnv *env)
	{
		f->begin(env);
		GameObject *o = f->getnext(); if(!o) return;
		exePeriodSeq.add();
		SequenceOverPeriodEntry *se = &exePeriodSeq.last->value;
		se->time = current_time;	se->period = v->get(env);
		se->actseq = x;			se->executor = env->self;

		GrowList<GameObject*> gl;
		do gl.add(o); while(o = f->getnext());

		se->numobj = gl.len;
		se->ola = se->obj = new goref[se->numobj];
		for(int i = 0; i < se->numobj; i++)
			se->obj[i] = gl[i];

		se->nloops = se->numobj; se->loopsexec = 0;
	}
};

struct ActionRepeatSequenceOverPeriod : public CAction
{
	int x; CFinder *f; CValue *r, *v;
	ActionRepeatSequenceOverPeriod(int a, CFinder *b, CValue *c, CValue *d) : x(a), f(b), r(c), v(d) {}
	void run(SequenceEnv *env)
	{
		f->begin(env);
		GameObject *o = f->getnext(); if(!o) return;
		repPeriodSeq.add();
		SequenceOverPeriodEntry *se = &repPeriodSeq.last->value;
		se->time = current_time;	se->period = v->get(env);
		se->actseq = x;			se->executor = env->self;

		GrowList<GameObject*> gl;
		do gl.add(o); while(o = f->getnext());

		se->numobj = gl.len;
		se->ola = se->obj = new goref[se->numobj];
		for(int i = 0; i < se->numobj; i++)
			se->obj[i] = gl[i];

		se->nloops = r->get(env); se->loopsexec = 0;
	}
};

struct ActionSinkAndRemove : public CAction
{
	CFinder *f;
	ActionSinkAndRemove(CFinder *a) : f(a) {}
	void run(SequenceEnv *env)
	{
		// TODO: What about sinking?
		f->begin(env);
		GameObject *o;
		while(o = f->getnext())
			RemoveObject(o);
	}
};

struct ActionSendPackage : public CAction
{
	int x; CFinder *f;
	ActionSendPackage(int a, CFinder *b) : x(a), f(b) {}
	void run(SequenceEnv *env)
	{
		GameObject *o;
		f->begin(env);
		while(o = f->getnext())
			SendCPackage(o, &(gspackage[x]), env);
	}
};

struct ActionSetSelectable : public CAction
{
	int x; CFinder *f;
	ActionSetSelectable(int a, CFinder *b) : x(a), f(b) {}
	void run(SequenceEnv *env)
	{
		GameObject *o;
		f->begin(env);
		while(o = f->getnext()) {
			o->flags &= ~FGO_SELECTABLE;
			if(x) o->flags |= FGO_SELECTABLE;
		}
	}
};

struct ActionSetTargetable : public CAction
{
	int x; CFinder *f;
	ActionSetTargetable(int a, CFinder *b) : x(a), f(b) {}
	void run(SequenceEnv *env)
	{
		GameObject *o;
		f->begin(env);
		while(o = f->getnext()) {
			o->flags &= ~FGO_TARGETABLE;
			if(x) o->flags |= FGO_TARGETABLE;
		}
	}
};

struct ActionSetRenderable : public CAction
{
	int x; CFinder *f;
	ActionSetRenderable(int a, CFinder *b) : x(a), f(b) {}
	void run(SequenceEnv *env)
	{
		GameObject *o;
		f->begin(env);
		while(o = f->getnext()) {
			o->flags &= ~FGO_RENDERABLE;
			if(x) o->flags |= FGO_RENDERABLE;
		}
	}
};

struct ActionSwitchCondition : public CAction
{
	CValue *c; GrowList<ASwitchCase> *s; // must be allocated with malloc or new
	ActionSwitchCondition(CValue *a, GrowList<ASwitchCase> *b) : c(a), s(b) {}
	void run(SequenceEnv *env)
	{
		valuetype x = c->get(env);
		for(uint i = 0; i < s->len; i++)
			if(s->getpnt(i)->v->get(env) == x)
				s->getpnt(i)->s->run(env);
	}
};

struct ActionSwitchHighest : public CAction
{
	GrowList<ASwitchCase> *s; // must be allocated with malloc or new
	ActionSwitchHighest(GrowList<ASwitchCase> *a) : s(a) {}
	void run(SequenceEnv *env)
	{
		valuetype x, r; int c = -1;
		for(uint i = 0; i < s->len; i++)
		{
			r = s->getpnt(i)->v->get(env);
			if((c == -1) || (r > x))
				{x = r; c = i;}
		}
		if(c != -1) s->getpnt(c)->s->run(env);
	}
};

struct ActionDisplayGameTextWindow : public CAction
{
	CGameTextWindow *w; CFinder *f;
	ActionDisplayGameTextWindow(CGameTextWindow *a, CFinder *b) : w(a), f(b) {}
	void run(SequenceEnv *env)
	{
		GameObject *o = f->getfirst(env);
		if(!o) return;
		if(o->id != 1027) return;
		EnableGTW(w);
	}
};

struct ActionHideGameTextWindow : public CAction
{
	CGameTextWindow *w; CFinder *f;
	ActionHideGameTextWindow(CGameTextWindow *a, CFinder *b) : w(a), f(b) {}
	void run(SequenceEnv *env)
	{
		GameObject *o = f->getfirst(env);
		if(!o) return;
		if(o->id != 1027) return;
		DisableGTW(w);
	}
};

struct ActionHideCurrentGameTextWindow : public CAction
{
	CFinder *f;
	ActionHideCurrentGameTextWindow(CFinder *a) : f(a) {}
	void run(SequenceEnv *env)
	{
		GameObject *o = f->getfirst(env);
		if(!o) return;
		if(o->id != 1027) return;
		for(int i = 0; i < strGameTextWindow.len; i++)
			DisableGTW(&(gsgametextwin[i]));
	}
};

struct ActionPlayClip : public CAction
{
	C3DClip *c; CFinder *f;
	ActionPlayClip(C3DClip *a, CFinder *b) : c(a), f(b) {}
	void run(SequenceEnv *env)
	{
		GameObject *o = f->getfirst(env);
		if(!o) return;
		if(o->id != 1027) return;
		Play3DClip(c);
	}
};

struct ActionSnapCameraToPosition : public CAction
{
	CPosition *p; CFinder *f;
	ActionSnapCameraToPosition(CPosition *a, CFinder *b) : p(a), f(b) {}
	void run(SequenceEnv *env)
	{
		GameObject *o; f->begin(env);
		while(o = f->getnext())
		{
			if(!o->client) continue;
			PosOri x; p->get(env, &x);
			o->client->camerapos = x.pos;
			o->client->cameraori = x.ori;
		}
	}
};

struct ActionFaceTowards : public CAction
{
	CFinder *x, *y;
	ActionFaceTowards(CFinder *a, CFinder *b) : x(a), y(b) {}
	void run(SequenceEnv *env)
	{
		PosOri p;
		FinderToPosOri(&p, y, env);
		GameObject *o; x->begin(env);
		while(o = x->getnext())
		{
			Vector3 d = p.pos - o->position;
			o->orientation.y = atan2(d.x, -d.z);
		}
	}
};

struct ActionIdentifyAndMarkClusters : public CAction
{
	CObjectDefinition *de; CFinder *f; CValue *vcr, *vir, *vmr;
	ActionIdentifyAndMarkClusters(CObjectDefinition *x, CFinder *y, CValue *z, CValue *w, CValue *v) : de(x), f(y), vcr(z), vir(w), vmr(v) {}
	void run(SequenceEnv *env)
	{
		GrowList<goref> gl; GameObject *o;
		f->begin(env);
		while(o = f->getnext()) gl.addp()->set(o);
		int nf = gl.len;

		float rad = vcr->get(env), fmr = vmr->get(env);
		rad *= rad;

		boolean *us = new boolean[nf];
		for(int i = 0; i < nf; i++) us[i] = 0;

		for(int i = 0; i < nf; i++)
		{
			if(us[i]) continue;
			if(!gl.getpnt(i)->valid()) continue;
			o = gl.getpnt(i)->get();

			float clrat = 0;
			for(int j = 0; j < nf; j++)
			{
				if(us[j]) continue;
				if(!gl.getpnt(j)->valid()) continue;
				GameObject *p = gl.getpnt(j)->get();

				if((p->position - o->position).sqlen2xz() > rad) continue;

				SequenceEnv nc; nc.self = p;
				clrat += vir->get(&nc);
			}

			if(clrat >= fmr)
			{
				GameObject *mark = CreateObject(de, env->self->player);
				mark->position = o->position;
				GOPosChanged(mark);
				for(int j = 0; j < nf; j++)
				{
					if(us[j]) continue;
					if(!gl.getpnt(j)->valid()) continue;
					GameObject *p = gl.getpnt(j)->get();
					if((p->position - o->position).sqlen2xz() <= rad)
						us[j] = 1;
				}
			}
		}
	}
};

struct ActionStoreCameraPosition : public CAction
{
	CFinder *f;
	ActionStoreCameraPosition(CFinder *a) : f(a) {}
	void run(SequenceEnv *env)
	{
		GameObject *o; f->begin(env);
		while(o = f->getnext())
		{
			if(!o->client) continue;
			o->client->storedpos = o->client->camerapos;
			o->client->storedori = o->client->cameraori;
		}
	}
};

struct ActionSnapCameraToStoredPosition : public CAction
{
	CFinder *f;
	ActionSnapCameraToStoredPosition(CFinder *a) : f(a) {}
	void run(SequenceEnv *env)
	{
		GameObject *o; f->begin(env);
		while(o = f->getnext())
		{
			if(!o->client) continue;
			o->client->camerapos = o->client->storedpos;
			o->client->cameraori = o->client->storedori;
		}
	}
};

struct ActionSetIndexedItem : public CAction
{
	int x; CValue *y; CFinder *f; CValue *v;
	ActionSetIndexedItem(int a, CValue *b, CFinder *c, CValue *d) : x(a), y(b), f(c), v(d) {}
	void run(SequenceEnv *env)
	{
		valuetype a = v->get(env), i = y->get(env);
		GameObject *o; f->begin(env); 
		while(o = f->getnext())
			o->setIndexedItem(x, i, a);
	}
};

struct ActionIncreaseIndexedItem : public CAction
{
	int x; CValue *y; CFinder *f; CValue *v;
	ActionIncreaseIndexedItem(int a, CValue *b, CFinder *c, CValue *d) : x(a), y(b), f(c), v(d) {}
	void run(SequenceEnv *env)
	{
		valuetype a = v->get(env), i = y->get(env);
		GameObject *o; f->begin(env); 
		while(o = f->getnext())
			o->setIndexedItem(x, i, o->getIndexedItem(x, i) + a);
	}
};

struct ActionDecreaseIndexedItem : public CAction
{
	int x; CValue *y; CFinder *f; CValue *v;
	ActionDecreaseIndexedItem(int a, CValue *b, CFinder *c, CValue *d) : x(a), y(b), f(c), v(d) {}
	void run(SequenceEnv *env)
	{
		valuetype a = v->get(env), i = y->get(env);
		GameObject *o; f->begin(env); 
		while(o = f->getnext())
			o->setIndexedItem(x, i, o->getIndexedItem(x, i) - a);
	}
};

struct ActionSetReconnaissance : public CAction
{
	CFinder *f;
	ActionSetReconnaissance(CFinder *a) : f(a) {}
	void run(SequenceEnv *env)
	{
		GameObject *o; f->begin(env); 
		while(o = f->getnext())
			o->flags |= FGO_RECONNAISSANCE;
	}
};

struct ActionEnableDiplomaticReportWindow : public CAction
{
	char *s; CFinder *f;
	ActionEnableDiplomaticReportWindow(char *a, CFinder *b) : s(a), f(b) {}
	void run(SequenceEnv *env)
	{
		GameObject *o = f->getfirst(env);
		if(!o) return;
		if(!o->client) return;
		o->client->winReport = 1;
	}
};

struct ActionDisableDiplomaticReportWindow : public CAction
{
	char *s; CFinder *f;
	ActionDisableDiplomaticReportWindow(char *a, CFinder *b) : s(a), f(b) {}
	void run(SequenceEnv *env)
	{
		GameObject *o = f->getfirst(env);
		if(!o) return;
		if(!o->client) return;
		o->client->winReport = 0;
	}
};

struct ActionEnableTributesWindow : public CAction
{
	char *s; CFinder *f;
	ActionEnableTributesWindow(char *a, CFinder *b) : s(a), f(b) {}
	void run(SequenceEnv *env)
	{
		GameObject *o = f->getfirst(env);
		if(!o) return;
		if(!o->client) return;
		o->client->winTributes = 1;
	}
};

struct ActionDisableTributesWindow : public CAction
{
	char *s; CFinder *f;
	ActionDisableTributesWindow(char *a, CFinder *b) : s(a), f(b) {}
	void run(SequenceEnv *env)
	{
		GameObject *o = f->getfirst(env);
		if(!o) return;
		if(!o->client) return;
		o->client->winTributes = 0;
	}
};

struct ActionCopyFacingOf : public CAction
{
	CFinder *f, *g;
	ActionCopyFacingOf(CFinder *a, CFinder *b) : f(a), g(b) {}
	void run(SequenceEnv *env)
	{
		PosOri p; FinderToPosOri(&p, g, env);
		GameObject *o; f->begin(env);
		while(o = f->getnext())
			o->orientation = p.ori;
	}
};

void ReadUponCond(char **pntfp, ActionSeq **a, ActionSeq **b);
GrowList<ASwitchCase> *ReadSwitchCases(char **pntfp);

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
		case ACTION_DEREGISTER_ASSOCIATES:
			{w += 2;
			CFinder *a = ReadFinder(&w);
			int t = strAssociateCat.find(*w); mustbefound(t); w++;
			CFinder *b = ReadFinder(&w);
			return new ActionDeregisterAssociates(a, t, b);}
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
		case ACTION_CONVERT_TO:
			{int d = FindObjDef(stfind_cs(CLASS_str, CLASS_NUM, word[2]), word[3]);
			mustbefound(d);
			w += 4;
			return new ActionConvertTo(&objdef[d], ReadFinder(&w));}
		case ACTION_CONVERT_ACCORDING_TO_TAG:
			{int t = strTypeTag.find(word[2]); mustbefound(t);
			w += 3;
			return new ActionConvertAccordingToTag(t, ReadFinder(&w));}
		case ACTION_EXECUTE_SEQUENCE_AFTER_DELAY:
			{int d = strActionSeq.find(word[2]); mustbefound(d);
			w += 3;
			CFinder *f = ReadFinder(&w);
			return new ActionExecuteSequenceAfterDelay(d, f, ReadValue(&w));}
		case ACTION_ADD_REACTION:
			{int x = strReaction.find(word[2]); mustbefound(x);
			w += 3; return new ActionAddReaction(x, ReadFinder(&w));}
		case ACTION_REMOVE_REACTION:
			{int x = strReaction.find(word[2]); mustbefound(x);
			w += 3; return new ActionRemoveReaction(x, ReadFinder(&w));}
		case ACTION_CHANGE_REACTION_PROFILE:
			{int m = stricmp(word[2], "REMOVE") ? 0 : 1;
			int x = strReaction.find(word[3]); mustbefound(x);
			w += 4; return new ActionChangeReactionProfile(m, x, ReadFinder(&w));}
		case ACTION_SET_SCALE:
			{w += 2;
			CFinder *f = ReadFinder(&w);
			CValue *x = ReadValue(&w);
			CValue *y = ReadValue(&w);
			CValue *z = ReadValue(&w);
			return new ActionSetScale(f, x, y, z);}
		case ACTION_TERMINATE:
			w += 2; return new ActionTerminate(ReadFinder(&w));
		case ACTION_EXECUTE_SEQUENCE_OVER_PERIOD:
			{int d = strActionSeq.find(word[2]); mustbefound(d);
			w += 3;
			CFinder *f = ReadFinder(&w);
			return new ActionExecuteSequenceOverPeriod(d, f, ReadValue(&w));}
		case ACTION_REPEAT_SEQUENCE_OVER_PERIOD:
			{int d = strActionSeq.find(word[2]); mustbefound(d);
			w += 3;
			CFinder *f = ReadFinder(&w);
			CValue *v = ReadValue(&w);
			return new ActionRepeatSequenceOverPeriod(d, f, v, ReadValue(&w));}
		case ACTION_SINK_AND_REMOVE:
			w += 2;
			return new ActionSinkAndRemove(ReadFinder(&w));
		case ACTION_SEND_PACKAGE:
			{int x = strPackage.find(word[2]); mustbefound(x);
			w += 3; return new ActionSendPackage(x, ReadFinder(&w));}
		case ACTION_SET_SELECTABLE:
			w += 3; return new ActionSetSelectable(atoi(word[2]), ReadFinder(&w));
		case ACTION_SET_TARGETABLE:
			w += 3; return new ActionSetTargetable(atoi(word[2]), ReadFinder(&w));
		case ACTION_SET_RENDERABLE:
			w += 3; return new ActionSetRenderable(atoi(word[2]), ReadFinder(&w));
		case ACTION_SWITCH_CONDITION:
			{w += 2; CValue *v = ReadValue(&w);
			return new ActionSwitchCondition(v, ReadSwitchCases(pntfp));}
		case ACTION_SWITCH_HIGHEST:
			return new ActionSwitchHighest(ReadSwitchCases(pntfp));
		case ACTION_DISPLAY_GAME_TEXT_WINDOW:
			{int x = strGameTextWindow.find(word[2]); mustbefound(x);
			CGameTextWindow *g = &(gsgametextwin[x]);
			w += 3; CFinder *f = ReadFinder(&w);
			return new ActionDisplayGameTextWindow(g, f);}
		case ACTION_HIDE_GAME_TEXT_WINDOW:
			{int x = strGameTextWindow.find(word[2]); mustbefound(x);
			CGameTextWindow *g = &(gsgametextwin[x]);
			w += 3; CFinder *f = ReadFinder(&w);
			return new ActionHideGameTextWindow(g, f);}
		case ACTION_HIDE_CURRENT_GAME_TEXT_WINDOW:
			w += 2; return new ActionHideCurrentGameTextWindow(ReadFinder(&w));
		case ACTION_PLAY_CLIP:
			{int x = str3DClip.find(word[2]); mustbefound(x);
			w += 3; return new ActionPlayClip(&(gs3dclip[x]), ReadFinder(&w));}
		case ACTION_SNAP_CAMERA_TO_POSITION:
			{w += 2; CPosition *p = ReadCPosition(&w);
			return new ActionSnapCameraToPosition(p, ReadFinder(&w));}
		case ACTION_FACE_TOWARDS:
			{w += 2; CFinder *x = ReadFinder(&w);
			return new ActionFaceTowards(x, ReadFinder(&w));}
		case ACTION_IDENTIFY_AND_MARK_CLUSTERS:
			{w += 3;
			CObjectDefinition *x = &(objdef[FindObjDef(CLASS_MARKER, word[2])]);
			CFinder *y = ReadFinder(&w);
			CValue *z = ReadValue(&w);
			CValue *a = ReadValue(&w);
			return new ActionIdentifyAndMarkClusters(x, y, z, a, ReadValue(&w));}
		case ACTION_STORE_CAMERA_POSITION:
			w += 2; return new ActionStoreCameraPosition(ReadFinder(&w));
		case ACTION_SNAP_CAMERA_TO_STORED_POSITION:
			w += 2; return new ActionSnapCameraToStoredPosition(ReadFinder(&w));
		case ACTION_SET_INDEXED_ITEM:
			{int d = strItems.find(word[2]); //mustbefound(d);
			w += 3; CValue *y = ReadValue(&w);
			CFinder *f = ReadFinder(&w);
			return new ActionSetIndexedItem(d, y, f, ReadValue(&w));}
		case ACTION_INCREASE_INDEXED_ITEM:
			{int d = strItems.find(word[2]); //mustbefound(d);
			w += 3; CValue *y = ReadValue(&w);
			CFinder *f = ReadFinder(&w);
			return new ActionIncreaseIndexedItem(d, y, f, ReadValue(&w));}
		case ACTION_DECREASE_INDEXED_ITEM:
			{int d = strItems.find(word[2]); //mustbefound(d);
			w += 3; CValue *y = ReadValue(&w);
			CFinder *f = ReadFinder(&w);
			return new ActionDecreaseIndexedItem(d, y, f, ReadValue(&w));}
		case ACTION_SET_RECONNAISSANCE:
			w += 2; return new ActionSetReconnaissance(ReadFinder(&w));
		case ACTION_ENABLE_DIPLOMATIC_REPORT_WINDOW:
			{char *s = GetLocText(word[2]); w += 3;
			return new ActionEnableDiplomaticReportWindow(s, ReadFinder(&w));}
		case ACTION_DISABLE_DIPLOMATIC_REPORT_WINDOW:
			{char *s = GetLocText(word[2]); w += 3;
			return new ActionDisableDiplomaticReportWindow(s, ReadFinder(&w));}
		case ACTION_ENABLE_TRIBUTES_WINDOW:
			{char *s = GetLocText(word[2]); w += 3;
			return new ActionEnableTributesWindow(s, ReadFinder(&w));}
		case ACTION_DISABLE_TRIBUTES_WINDOW:
			{char *s = GetLocText(word[2]); w += 3;
			return new ActionDisableTributesWindow(s, ReadFinder(&w));}
		case ACTION_COPY_FACING_OF:
			{w += 2; CFinder *f = ReadFinder(&w);
			return new ActionCopyFacingOf(f, ReadFinder(&w));}

		// The following actions do nothing at the moment (but at least
		// the program won't crash when these actions are executed).
		case ACTION_STOP_SOUND:
		case ACTION_PLAY_SOUND:
		case ACTION_PLAY_SOUND_AT_POSITION:
		case ACTION_PLAY_SPECIAL_EFFECT:
		case ACTION_PLAY_SPECIAL_EFFECT_BETWEEN:
		case ACTION_PLAY_ANIMATION_IF_IDLE:
		case ACTION_ATTACH_SPECIAL_EFFECT:
		case ACTION_ATTACH_LOOPING_SPECIAL_EFFECT:
		case ACTION_DETACH_LOOPING_SPECIAL_EFFECT:
		case ACTION_REVEAL_FOG_OF_WAR:
		case ACTION_COLLAPSING_CIRCLE_ON_MINIMAP:
		case ACTION_SHOW_BLINKING_DOT_ON_MINIMAP:
		case ACTION_ENABLE_GAME_INTERFACE:
		case ACTION_DISABLE_GAME_INTERFACE:
		case ACTION_PLAY_CAMERA_PATH:
		case ACTION_STOP_CAMERA_PATH_PLAYBACK:
		case ACTION_INTERPOLATE_CAMERA_TO_POSITION:
		case ACTION_SET_ACTIVE_MISSION_OBJECTIVES:
		case ACTION_SHOW_MISSION_OBJECTIVES_ENTRY:
		case ACTION_SHOW_MISSION_OBJECTIVES_ENTRY_INACTIVE:
		case ACTION_TRACK_OBJECT_POSITION_FROM_MISSION_OBJECTIVES_ENTRY:
		case ACTION_STOP_INDICATING_POSITION_OF_MISSION_OBJECTIVES_ENTRY:
		case ACTION_FORCE_PLAY_MUSIC:
		case ACTION_ADOPT_APPEARANCE_FOR:
		case ACTION_ADOPT_DEFAULT_APPEARANCE_FOR:
			return new ActionDoNothing();
	}
	//ferr("Unknown action command."); return 0;
	//return new ActionUnknown();
	int x = strUnknownAction.find(word[1]);
	if(x == -1) {x = strUnknownAction.len; strUnknownAction.add(word[1]);}
	return new ActionUnknown(x);
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

GrowList<ASwitchCase> *ReadSwitchCases(char **pntfp)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	GrowList<ASwitchCase> *g = new GrowList<ASwitchCase>;
	while(*pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if( *((uint*)(word[0])) == '_DNE' )
			return g;
		if(!stricmp(word[0], "CASE"))
		{
			ASwitchCase *e = g->addp();
			char **w = word + 1;
			e->v = ReadValue(&w);
			e->s = ReadActSeq(pntfp);
		}
	}
	ferr("UEOF"); return 0;
}
