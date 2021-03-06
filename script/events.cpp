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

void ReadCReaction(char **pntfp, char **fstline)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	CReaction *re = &(reaction[strReaction.find(fstline[1])]);
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if( *((uint*)(word[0])) == '_DNE' )
			return;
		if(!stricmp(word[0], "TRIGGERED_BY"))
		{
			int n;
			if((n = strPRTrigger.find(word[1])) != -1)
				re->prts.add(prtrigger+n);
			else if((n = strGameEvent.find(word[1])) != -1)
				re->events.add(n);
		}
		if(!stricmp(word[0], "ACTION"))
			re->seq.code.add(ReadAction(pntfp, word));
	}
	ferr("UEOF"); return;
}

void ReadCPRTrigger(char **pntfp, char **fstline)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	CPRTrigger *pr = &(prtrigger[strPRTrigger.find(fstline[1])]);
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if( *((uint*)(word[0])) == '_DNE' )
			return;
		if(!stricmp(word[0], "TRIGGERED_BY"))
		{
			int n;
			if((n = strGameEvent.find(word[1])) != -1)
				pr->events.add(n);
		}
		if(!stricmp(word[0], "ASSESSMENT"))
		{
			int n;
			if((n = strEquation.find(word[1])) != -1)
				pr->ass.add(n);
		}
	}
	ferr("UEOF"); return;
}

int CanBeTriggeredBy(SequenceEnv *ctx, CReaction *re, int ev)
{
	for(uint i = 0; i < re->events.len; i++)
		if(re->events[i] == ev)
			return 1;
	for(uint i = 0; i < re->prts.len; i++)
	{
		CPRTrigger *prt = re->prts[i];
		for(uint j = 0; j < prt->events.len; j++)
			if(prt->events[j] == ev)
			{
				// OK if assessments true
				for(uint k = 0; k < prt->ass.len; k++)
					if(!stpo(equation[prt->ass[k]]->get(ctx)))
						goto neprt;
				return 1;
			}
	neprt:	;
	}
	return 0;
}

/*
void SendGameEvent(SequenceEnv *ie, GameObject *o, int ev)
{
	SequenceEnv ctx;
	if(ie) ie->copyAll(&ctx);
	ctx.self = o;
	if(ie) ctx.pkgsender = ie->self;
	goref gr = o;

	// NOTE: It is still possible that the next element is removed in the
	//       reaction sequence!
	DynListEntry<uint> *n;
	for(DynListEntry<uint> *e = o->iReaction.first; e; e = n)
	{
		n = e->next;
		if(CanBeTriggeredBy(&ctx, &(reaction[e->value]), ev))
			{reaction[e->value].seq.run(&ctx);
			if(!gr.valid()) return;}
	}

	for(uint i = 0; i < o->objdef->ireact.len; i++)
		if(CanBeTriggeredBy(&ctx, o->objdef->ireact[i], ev))
			{o->objdef->ireact[i]->seq.run(&ctx);
			if(!gr.valid()) return;}
}
*/

bool IsPRTTrue(SequenceEnv *ctx, CPRTrigger *prt)
{
	// OK if assessments true
	for(uint k = 0; k < prt->ass.len; k++)
		if(!stpo(equation[prt->ass[k]]->get(ctx)))
			return 0;
	return 1;
}

void SendGameEvent(SequenceEnv *ie, GameObject *o, int ev)
{
	SequenceEnv ctx;
	if(ie) ie->copyAll(&ctx);
	ctx.self = o;
	if(ie) ctx.pkgsender = ie->self;
	goref gr = o;

	GrowList<CReaction*> executed;

	// 1. Run reactions with TRIGGERED_BY "ev" (directly!)
	DynListEntry<uint> *n;
	for(DynListEntry<uint> *e = o->iReaction.first; e; e = n)
	{
		n = e->next;
		CReaction *re = &(reaction[e->value]);
		if(re->events.has(ev))
			{re->seq.run(&ctx); executed.add(re); if(!gr.valid()) return;}
	}
	for(uint i = 0; i < o->objdef->ireact.len; i++)
	{
		CReaction *re = o->objdef->ireact[i];
		if(executed.has(re)) continue;
		if(re->events.has(ev))
			{re->seq.run(&ctx); executed.add(re); if(!gr.valid()) return;}
	}
/*
	// 2. Get a list of PACKAGE_RECEIPT_TRIGGERs triggerable by "ev" and whose
	//    assessments are true.
	GrowList<CPRTrigger*> trueprts;
	for(DynListEntry<uint> *e = o->iReaction.first; e; e = n)
	{
		n = e->next;
		CReaction *re = &(reaction[e->value]);
		if(executed.has(re)) continue;
		for(int j = 0; j < re->prts.len; j++)
			if(re->prts[j]->events.has(ev))
				if(IsPRTTrue(&ctx, re->prts[j]))
					trueprts.add(re->prts[j]);
	}
	for(uint i = 0; i < o->objdef->ireact.len; i++)
	{
		CReaction *re = o->objdef->ireact[i];
		if(executed.has(re)) continue;
		for(int j = 0; j < re->prts.len; j++)
			if(re->prts[j]->events.has(ev))
				if(IsPRTTrue(&ctx, re->prts[j]))
					trueprts.add(re->prts[j]);
	}

	// 3. Run reactions with TRIGGERED_BY "P_R_T" from the list.
	for(DynListEntry<uint> *e = o->iReaction.first; e; e = n)
	{
		n = e->next;
		CReaction *re = &(reaction[e->value]);
		if(executed.has(re)) continue;
		for(int j = 0; j < trueprts.len; j++)
			if(re->prts.has(trueprts[j]))
				{re->seq.run(&ctx); executed.add(re); if(!gr.valid()) return; else break;}
	}
	for(uint i = 0; i < o->objdef->ireact.len; i++)
	{
		CReaction *re = o->objdef->ireact[i];
		if(executed.has(re)) continue;
		for(int j = 0; j < trueprts.len; j++)
			if(re->prts.has(trueprts[j]))
				{re->seq.run(&ctx); executed.add(re); if(!gr.valid()) return; else break;}
	}
*/
	GrowList<CPRTrigger*> trueprts;

	auto f = [&trueprts,&executed,&ctx,ev](CReaction *re) -> bool
	{
		if(executed.has(re)) return 0;
		bool s = 0;
		for(int j = 0; j < re->prts.len; j++)
		{
			if(trueprts.has(re->prts[j]))
				{s = 1; break;}
			if(re->prts[j]->events.has(ev))
				if(IsPRTTrue(&ctx, re->prts[j]))
					{trueprts.add(re->prts[j]); s = 1; break;}
		}
		if(s) {re->seq.run(&ctx); executed.add(re);}
		return s;
	};

	for(DynListEntry<uint> *e = o->iReaction.first; e; e = n)
	{
		n = e->next;
		CReaction *re = &(reaction[e->value]);
		if(f(re)) if(!gr.valid()) return;
	}
	for(uint i = 0; i < o->objdef->ireact.len; i++)
	{
		CReaction *re = o->objdef->ireact[i];
		if(f(re)) if(!gr.valid()) return;
	}
}