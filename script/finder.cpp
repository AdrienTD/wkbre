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

/****** TEMPLATE ******
struct FinderNew : public CFinder
{
	FinderNew() {}
	void begin(SequenceEnv *env) {}
	GameObject *getnext()
	{
	}
};
**********************/

/*struct FinderUnknown : public CFinder
{
	CFinder *clone() {return new FinderUnknown();}
	void begin(SequenceEnv *env) {ferr("Cannot initialize a finder of unknown type.");}
	GameObject *getnext() {ferr("Cannot get an object from a finder of unknown type."); return 0;}
};*/

GrowStringList strUnknownFinder;
struct FinderUnknown : public CFinder
{
	int x;
	FinderUnknown(int a) : x(a) {}
	CFinder *clone() {return new FinderUnknown(x);}
	void begin(SequenceEnv *env)
	{
		char tb[1024];
		strcpy(tb, "Object finder "); strcat(tb, strUnknownFinder.getdp(x));
		strcat(tb, " is unknown.");
		ferr(tb);
	}
	GameObject *getnext() {ferr("Cannot get an object from a finder of unknown type."); return 0;}
};

void FinderResToGB(CFinder *f, SequenceEnv *c, GrowList<goref> *l)
{
	f->begin(c);
	GameObject *o; goref a;
	while(o = f->getnext())
	{
		l->add(a);
		l->getref(l->len-1) = o;
	}
}

struct FinderUnion : public CFinder
{
	int cur;
	int num; CFinder **f; GrowList<GameObject*> l;
	FinderUnion(int a, CFinder **b) {num = a; f = b;}
	~FinderUnion()
	{
		for(int i = 0; i < num; i++) delete f[i];
		delete f;
	}
	CFinder *clone()
	{
		CFinder **b = new CFinder*[num];
		for(int i = 0; i < num; i++)
			b[i] = f[i]->clone();
		return new FinderUnion(num, b);
	}
	void begin(SequenceEnv *env)
	{
		cur = 0;
		l.clear();
		for(int i = 0; i < num; i++)
			f[i]->begin(env);
	}
	GameObject *getnext()
	{
		if(cur >= num) return 0;
		GameObject *r;
gn:		while(!( r = f[cur]->getnext() )) if(++cur >= num) return 0;

		if(l.has(r)) goto gn;
		l.add(r);
		return r;
	}
};

struct FinderSpecificID : public CFinder
{
	int id, f;
	FinderSpecificID(int a) {id = a;}
	CFinder *clone() {return new FinderSpecificID(id);}
	void begin(SequenceEnv *env) {f = 1;}
	GameObject *getnext() {if(f) {f = 0; return FindObjID(id);} return 0;}
};

struct FinderIntersection : public CFinder
{
	int num; CFinder **f; SequenceEnv *ctx;
	FinderIntersection(int a, CFinder **b) {num = a; f = b;}
	~FinderIntersection()
	{
		for(int i = 0; i < num; i++) delete f[i];
		delete f;
	}
	CFinder *clone()
	{
		CFinder **b = new CFinder*[num];
		for(int i = 0; i < num; i++)
			b[i] = f[i]->clone();
		return new FinderIntersection(num, b);
	}
	void begin(SequenceEnv *env)
	{
		f[0]->begin(ctx = env);
	}
	GameObject *getnext()
	{
		GameObject *a, *b;
		int inall;
		do
		{
			a = f[0]->getnext();
			if(!a) return 0;
			inall = 1;
			for(int i = 1; i < num; i++)
			{
				f[i]->begin(ctx);
				while(b = f[i]->getnext())
					if(b == a)
						goto nf;
				inall = 0; break;
			nf:	;
			}
		} while(!inall);
		return a;
	}
};

struct FinderSelf : public CFinder
{
	int f; SequenceEnv *c;
	FinderSelf() {}
	CFinder *clone() {return new FinderSelf();}
	void begin(SequenceEnv *a) {f = 1; c = a;}
	GameObject *getnext()
	{
		if(f)
		{
			f = 0;
			if(c->self.valid())
				return c->self.get();
		}
		return 0;
	}
};

struct FinderSequenceExecutor : public CFinder
{
	SequenceEnv *env; int f;
	FinderSequenceExecutor() {}
	CFinder *clone() {return new FinderSequenceExecutor();}
	void begin(SequenceEnv *c) {env = c; f = 1;}
	GameObject *getnext()
	{
		if(f)
		{
			f = 0;
			if(env->exec.valid())
				return env->exec.get();
		}
		return 0;
	}
};

struct FinderTarget : public CFinder
{
	SequenceEnv *env; int f;
	FinderTarget() {}
	CFinder *clone() {return new FinderTarget();}
	void begin(SequenceEnv *c) {env = c; f = 1;}
	GameObject *getnext()
	{
		if(f)
		{
			f = 0;
			if(env->target.valid())
				return env->target.get();
		}
		return 0;
	}
};

struct FinderCreator : public CFinder
{
	SequenceEnv *env; int f;
	FinderCreator() {}
	CFinder *clone() {return new FinderCreator();}
	void begin(SequenceEnv *c) {env = c; f = 1;}
	GameObject *getnext()
	{
		if(f)
		{
			f = 0;
			if(env->creator.valid())
				return env->creator.get();
		}
		return 0;
	}
};

struct FinderCandidate : public CFinder
{
	SequenceEnv *env; int f;
	FinderCandidate() {}
	CFinder *clone() {return new FinderCandidate();}
	void begin(SequenceEnv *c) {env = c; f = 1;}
	GameObject *getnext()
	{
		if(f)
		{
			f = 0;
			if(env->candidate.valid())
				return env->candidate.get();
		}
		return 0;
	}
};

struct FinderPlayer : public CFinder
{
	SequenceEnv *env; int f;
	FinderPlayer() {}
	CFinder *clone() {return new FinderPlayer();}
	void begin(SequenceEnv *c) {env = c; f = 1;}
	GameObject *getnext()
	{
		if(f)
		{
			f = 0;
			if(env->self.valid())
				return env->self->player;
			return 0;
		}
		return 0;
	}
};

struct FinderController : public CFinder
{
	SequenceEnv *env; int f;
	FinderController() {}
	CFinder *clone() {return new FinderController();}
	void begin(SequenceEnv *c) {env = c; f = 1;}
	GameObject *getnext()
	{
		if(f)
		{
			f = 0;
			if(env->self.valid())
				return env->self->parent;
			return 0;
		}
		return 0;
	}
};

struct FinderResults : public CFinder
{
	int x; CFinder *f;
	FinderResults(int a) : x(a), f(0) {}
	CFinder *clone() {return new FinderResults(x);}
	void begin(SequenceEnv *c)
	{
		if(f)	f->begin(c);
		else	(f = finderdef[x]->clone())->begin(c);
	}
	GameObject *getnext() {return f->getnext();}
};

struct FinderChain : public CFinder
{
	int num; CFinder **f;
	SequenceEnv nc;
	GrowList<goref> *rli, *rlo, arl1, arl2;
	int cotr;

	FinderChain(int a, CFinder **b) : num(a), f(b) {}
	CFinder *clone()
	{
		CFinder **b = new CFinder*[num];
		for(int i = 0; i < num; i++)
			b[i] = f[i]->clone();
		return new FinderChain(num, b);
	}

	void begin(SequenceEnv *env)
	{
		GameObject *o;
		arl1.clear(); arl2.clear();
		env->copyAll(&nc);
		goref gr = env->self;
		arl1.add(gr);
		rli = &arl1; rlo = &arl2;

		for(int i = 0; i < num; i++)
		{
			for(int j = 0; j < rli->len; j++)
			{
				nc.self = rli->get(j);
				f[i]->begin(&nc);
				while(o = f[i]->getnext())
				{
					gr = o;
					for(int k = 0; k < rlo->len; k++)
						if(rlo->get(k).get() == gr.get())
							goto fcnoadd;
					rlo->add(gr);
				fcnoadd: ;
				}
			}
			rli->clear();
			GrowList<goref> *t; t = rli; rli = rlo; rlo = t;
		}

		cotr = 0;
	}

	GameObject *getnext()
	{
		if(cotr >= rli->len) return 0;
		return rli->getpnt(cotr++)->get();
	}
};

struct FinderAlias : public CFinder
{
	int t; DynListEntry<goref> *e;
	FinderAlias(int a) : t(a) {}
	CFinder *clone() {return new FinderAlias(t);}
	void begin(SequenceEnv *env) {e = alias[t].first;}
	GameObject *getnext()
	{
		if(!e) return 0;
		while(!e->value.valid()) {e = e->next; if(!e) return 0;}
		GameObject *o = e->value.get();
		e = e->next;
		return o;
	}
};

int IsFSOResult(SequenceEnv *s, GameObject *o, int q, CObjectDefinition *d, int cl)
{
	printf("IsFSOResult %i, q = %i\n", o->id, q);
	if(cl != -1)
		if(o->objdef->type != cl)
			return 0;
	// BY_EQUATION
	if(q != -1)
	{
		SequenceEnv c;
		s->copyAll(&c);
		c.candidate = o;
		if(stpo(equation[q]->get(&c)))
			return 1;
		return 0;
	}
	// BY_BLUEPRINT
	if(d)
		if(o->objdef != d)
			return 0;
	return 1;
}

/*void AddSORefToList(SequenceEnv *s, GrowList<goref> *l, GameObject *o, int c, int m, int q, CObjectDefinition *d)
{
	goref z;
	for(DynListEntry<GameObject> *e = o->children.first; e; e = e->next)
		if(e->value.objdef->type == c)
			if(IsFSOResult(s, &(e->value), q, d))
				{z = &(e->value); l->add(z); if(m) AddSORefToList(s, l, &(e->value), c, m, q, d);}
}*/

struct FinderSubordinates : public CFinder
{
	int eq, cl, il; CObjectDefinition *bl; CFinder *fd;
	FinderSubordinates(int a, CObjectDefinition *b, int c, int d, CFinder *e) : eq(a), bl(b), cl(c), il(d), fd(e) {}
	CFinder *clone() {return new FinderSubordinates(eq, bl, cl, il, fd->clone());}

	DynListEntry<GameObject> *w; GameObject *p; int nomore; SequenceEnv *ctx;
	void NextParent()
	{
		while(1)
		{	p = fd->getnext();
			if(!p) {nomore = 1; return;}
			if(!p->children.len) continue;
			w = p->children.first; break;
		}
	}

	void NextCandidate()
	{
		if(!il && w->value.children.len) w = w->value.children.first;
		else if(w->next) w = w->next;
		else
		{	//w = (DynListEntry<GameObject> *)(w->value.parent);	// !!!! //
			//if(&(w->value) == p)
			//	NextParent();
			//else	w = w->next;

			while(!w->next)
			{
				w = (DynListEntry<GameObject> *)(w->value.parent);	// !!!! //
				if(&(w->value) == p)
					{NextParent(); break;}
			}
			w = w->next;
		}
	}

	void begin(SequenceEnv *env)
	{
		printf("----- FINDER_SUBORDINATES BEGIN -----\n");
		nomore = 0;
		fd->begin(ctx = env);
		NextParent();
	}
	GameObject *getnext()
	{
		printf("FinderSubordinates::getnext\n");

		//if(nomore) return 0;

		while(!nomore)
		{
			GameObject *o = &(w->value);
			if(IsFSOResult(ctx, o, eq, bl, cl))
				{NextCandidate(); printf("Return %i\n", o->id); return o;}
			NextCandidate();
		}

		printf("----- FINDER_SUBORDINATES END -----\n");
		return 0;
	}
};

struct FinderAssociates : public CFinder
{
	int cat;
	DynListEntry<GameObjAndListEntry> *el;

	FinderAssociates(int a) : cat(a) {}
	CFinder *clone() {return new FinderAssociates(cat);}

	void begin(SequenceEnv *env)
	{
		if(!env->self.valid()) {el = 0; return;}
		GameObject *se = env->self.get();
		for(DynListEntry<GOAssociation> *a = se->association.first; a; a = a->next)
			if(a->value.category == cat)
				{el = a->value.associates.first; return;}
		el = 0;
	}

	GameObject *getnext()
	{
		if(!el) return 0;
		GameObject *o = el->value.o;
		el = el->next;
		return o;
	}
};

struct FinderAssociators : public CFinder
{
	int cat;
	DynListEntry<GameObjAndListEntry> *el;

	FinderAssociators(int a) : cat(a) {}
	CFinder *clone() {return new FinderAssociators(cat);}

	void begin(SequenceEnv *env)
	{
		if(!env->self.valid()) {el = 0; return;}
		GameObject *se = env->self.get();
		for(DynListEntry<GOAssociation> *a = se->association.first; a; a = a->next)
			if(a->value.category == cat)
				{el = a->value.associators.first; return;}
		el = 0;
	}

	GameObject *getnext()
	{
		if(!el) return 0;
		GameObject *o = el->value.o;
		el = el->next;
		return o;
	}
};

struct FinderPlayers : public CFinder
{
	int eq;
	DynListEntry<GameObject> *f; int nomore;
	FinderPlayers(int a) : eq(a) {}
	CFinder *clone() {return new FinderPlayers(eq);}
	void NextCandidate()
	{
	fpnc:	if(f->value.objdef->type == CLASS_PLAYER)
		{
			SequenceEnv c;
			c.self = c.candidate = &(f->value);
			if(stpo(equation[eq]->get(&c)))
				return;
		}
		f = f->next;
		if(!f) {nomore = 1; return;}
		goto fpnc;
		
	}
	void begin(SequenceEnv *env) {f = levelobj->children.first; nomore = 0; NextCandidate();}
	GameObject *getnext()
	{
		if(nomore) return 0;
		GameObject *o = &(f->value);
		f = f->next;
		if(f)	NextCandidate();
		else	nomore = 1;
		return o;
	}
};

struct FinderAgSelection : public CFinder
{
	DynListEntry<goref> *e;
	FinderAgSelection() {}
	CFinder *clone() {return new FinderAgSelection();}
	void begin(SequenceEnv *c) {e = selobjects.first;}
	GameObject *getnext()
	{
		if(!e) return 0;
		GameObject *o = e->value.get();
		e = e->next;
		return o;
	}
};

struct FinderFilter : public CFinder
{
	int eq; CFinder *f; CValue *v;
	int nomore, max, robjs; SequenceEnv *env;
	FinderFilter(int a, CFinder *b, CValue *c) : eq(a), f(b), v(c) {}
	CFinder *clone() {return new FinderFilter(eq, f->clone(), v);}
	void begin(SequenceEnv *c)
	{
		nomore = 0; robjs = 0;
		f->begin(env = c);
		if(v) max = v->get(env);
		else max = -1;
	}
	GameObject *getnext()
	{
		if(nomore) return 0;
		if((max >= 0) && (robjs >= max)) {nomore = 1; return 0;}
		GameObject *o; SequenceEnv c;
		while(1)
		{
			// self should not be modified!
			o = f->getnext();
			if(!o) {nomore = 1; return 0;}
			env->copyAll(&c);
			c.candidate = o;
			if(stpo(equation[eq]->get(&c)))
				{robjs++; return o;}
		}
	}
};

struct FinderFilterCandidates : public CFinder
{
	CValue *v; CFinder *f;
	int nomore; SequenceEnv *env;
	FinderFilterCandidates(CValue *a, CFinder *b) : v(a), f(b) {}
	CFinder *clone() {return new FinderFilterCandidates(v, f->clone());}
	void begin(SequenceEnv *c) {nomore = 0; env = c; f->begin(env);}
	GameObject *getnext()
	{
		if(nomore) return 0;
		GameObject *o; SequenceEnv c;
		while(1)
		{
			// self should not be modified!
			o = f->getnext();
			if(!o) {nomore = 1; return 0;}
			env->copyAll(&c);
			c.candidate = o;
			if(stpo(v->get(&c)))
				return o;
		}
	}
};

struct FinderLevel : public CFinder
{
	int f;
	FinderLevel() {}
	CFinder *clone() {return new FinderLevel();}
	void begin(SequenceEnv *c) {f = 1;}
	GameObject *getnext()
	{
		if(f) {f = 0; return levelobj;}
		return 0;
	}
};

// Question: 2D or 3D? To determinate.
struct FinderNearestCandidate : public CFinder
{
	CFinder *f;
	int nomore; SequenceEnv *env;
	FinderNearestCandidate(CFinder *a) : f(a) {}
	CFinder *clone() {return new FinderNearestCandidate(f->clone());}
	void begin(SequenceEnv *c) {f->begin(env = c); nomore = 0;}
	GameObject *getnext()
	{
		if(nomore) return 0;
		if(!env->self.valid()) {nomore = 1; return 0;}
		GameObject *c = 0, *o, *s = env->self.get();
		float sd = -1;
		while(o = f->getnext())
		{
			float x = s->position.x - o->position.x;
			float z = s->position.z - o->position.z;
			float nd = x*x + z*z;
			if((sd < 0) || (nd < sd)) {c = o; sd = nd;}
		}
		nomore = 1;
		return c;
	}
};

struct FinderAlternative : public CFinder
{
	int num; CFinder **f;
	GameObject *ne; CFinder *x; int nomore;
	FinderAlternative(int a, CFinder **b) : num(a), f(b) {}
	CFinder *clone()
	{
		CFinder **b = new CFinder*[num];
		for(int i = 0; i < num; i++)
			b[i] = f[i]->clone();
		return new FinderAlternative(num, b);
	}
	void begin(SequenceEnv *c)
	{
		for(int i = 0; i < num; i++)
			if(ne = f[i]->getfirst(c))
				{x = f[i]; nomore = 0; return;}
		nomore = 1;
	}
	GameObject *getnext()
	{
		if(nomore) return 0;
		GameObject *tr = ne;
		if(!tr)	{nomore = 1; return 0;}
		else	{ne = x->getnext(); return tr;}
	}
};

struct FinderPackageSender : public CFinder
{
	SequenceEnv *env; int f;
	FinderPackageSender() {}
	CFinder *clone() {return new FinderPackageSender();}
	void begin(SequenceEnv *c) {env = c; f = 1;}
	GameObject *getnext()
	{
		if(f)
		{
			f = 0;
			if(env->pkgsender.valid())
				return env->pkgsender.get();
		}
		return 0;
	}
};

//////////////////////////////////////////////////////////////////////////

CFinder *ReadFinder(char ***wpnt)
{
	char **word = *wpnt; CFinder *cf;
	// FINDER_ 7 characters
	if(strlen(word[0]) < 8) goto rfunk; //ferr("Finders must begin with \"FINDER_\".");
	switch(stfind_cs(FINDER_str, FINDER_NUM, word[0]+7))
	{
		case FINDER_SPECIFIC_ID:
			cf = new FinderSpecificID(atoi(word[1]));
			*wpnt += 2; return cf;
		case FINDER_SELF:
			*wpnt += 1; return new FinderSelf();
		case FINDER_PLAYER:
			*wpnt += 1; return new FinderPlayer();
		case FINDER_CONTROLLER:
			*wpnt += 1; return new FinderController();
		case FINDER_RESULTS:
			cf = new FinderResults(strFinderDef.find(word[1]));
			*wpnt += 2; return cf;
		case FINDER_CREATOR:
			*wpnt += 1; return new FinderCreator();
		case FINDER_CANDIDATE:
			*wpnt += 1; return new FinderCandidate();
		case FINDER_ALIAS:
			{int c = strAlias.find(word[1]); mustbefound(c);
			*wpnt += 2; return new FinderAlias(c);}
		case FINDER_ASSOCIATES:
			{int c = strAssociateCat.find(word[1]); mustbefound(c);
			*wpnt += 2; return new FinderAssociates(c);}
		case FINDER_ASSOCIATORS:
			{int c = strAssociateCat.find(word[1]); mustbefound(c);
			*wpnt += 2; return new FinderAssociators(c);}
		case FINDER_PLAYERS:
			{int eq = strEquation.find(word[1]); mustbefound(eq);
			*wpnt += 2; return new FinderPlayers(eq);}
		case FINDER_AG_SELECTION:
			*wpnt += 1; return new FinderAgSelection();
		case FINDER_SEQUENCE_EXECUTOR:
			*wpnt += 1; return new FinderSequenceExecutor();
		case FINDER_LEVEL:
			*wpnt += 1; return new FinderLevel();
		case FINDER_TARGET:
			*wpnt += 1; return new FinderTarget();
		case FINDER_PACKAGE_SENDER:
			*wpnt += 1; return new FinderPackageSender();
	}
	//ferr("Unknown finder."); return 0;
rfunk:	int x = strUnknownFinder.find(word[0]);
	if(x == -1) {x = strUnknownFinder.len; strUnknownFinder.add(word[0]);}
	*wpnt += 1;
	return new FinderUnknown(x);
	//return new FinderUnknown();
};

void ReadSubFinders(int &n, CFinder** &sf, char** word, char **&pntfp)
{
	n = atoi(word[1]);
	sf = new CFinder*[n];
	for(int i = 0; i < n; i++)
		sf[i] = ReadOFDLine(pntfp);
}

CFinder *ReadOFDLine(char **pntfp)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	CFinder *cf, *u, *v, **sf; int n;
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!strcmp(word[0], "END_OBJECT_FINDER_DEFINITION"))
			return 0;
		if(strlen(word[0]) < 8) ferr("Finders must begin with \"FINDER_\".");
		switch(stfind_cs(FINDER_str, FINDER_NUM, word[0]+7))
		{
			case FINDER_UNION:
				//u = ReadOFDLine(pntfp); v = ReadOFDLine(pntfp);
				//return new FinderUnion(u, v);
				ReadSubFinders(n, sf, word, pntfp);
				return new FinderUnion(n, sf);
			case FINDER_CHAIN:
				ReadSubFinders(n, sf, word, pntfp);
				return new FinderChain(n, sf);
			case FINDER_ALTERNATIVE:
				ReadSubFinders(n, sf, word, pntfp);
				return new FinderAlternative(n, sf);
			case FINDER_SUBORDINATES:
				{int eq = -1, bn = 0, cl = -1, il = 0;
				for(int w = 1; w < nwords; )
				{
					if(!stricmp(word[w], "BY_EQUATION"))
						{eq = strEquation.find(word[w+1]);
						mustbefound(eq); w += 2;}
					else if(!stricmp(word[w], "BY_BLUEPRINT"))
						{bn = w + 1; w += 2;}
					else if(!stricmp(word[w], "OF_CLASS"))
						{cl = stfind_cs(CLASS_str, CLASS_NUM, word[w+1]);
						w += 2;}
					else if(!stricmp(word[w], "IMMEDIATE_LEVEL"))
						{il = 1; w++;}
				}
				CObjectDefinition *od = 0;
				if(cl != -1) if(bn) od = &objdef[FindObjDef(cl, word[bn])];

			/*	printf("----- Creating FINDER_SUBORDINATES with: -----\n");
				printf("word[bn] = %s\n", word[bn]);
				if(cl != -1) printf("OF_CLASS = %s\n", CLASS_str[cl]);
				if(od) printf("BY_BLUEPRINT = %s\n", od->name);
				printf("-----\n");
			*/
				CFinder *fd = ReadOFDLine(pntfp);
				return new FinderSubordinates(eq, od, cl, il, fd);}
			case FINDER_FILTER:
				{CFinder *f = ReadOFDLine(pntfp);
				int eq = strEquation.find(word[1]); mustbefound(eq);
				return new FinderFilter(eq, f, 0);}
			case FINDER_FILTER_FIRST:
				{CFinder *f = ReadOFDLine(pntfp);
				int eq = strEquation.find(word[1]); mustbefound(eq);
				char **w = word + 2;
				return new FinderFilter(eq, f, ReadValue(&w));}
			case FINDER_FILTER_CANDIDATES:
				{CFinder *f = ReadOFDLine(pntfp);
				char **w = word + 1;
				return new FinderFilterCandidates(ReadValue(&w), f);}
			case FINDER_NEAREST_CANDIDATE:
				return new FinderNearestCandidate(ReadOFDLine(pntfp));
		}
		char **pw = word;
		return ReadFinder(&pw);
	}
	ferr("UEOF"); return 0;
}