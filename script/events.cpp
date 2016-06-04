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
			if(prt->events[i] == ev)
			{
				// OK if assessment true
				for(uint k = 0; k < prt->ass.len; k++)
					if(!stpo(equation[prt->ass[k]]->get(ctx)))
						return 0;
				return 1; // If all assessments are false, there's no
					  // need to check other events because
					  // assessments are constant during this fct
					  // (except if they use random values).
			}
	}
	return 0;
}

void SendGameEvent(SequenceEnv *ie, GameObject *o, int ev)
{
	SequenceEnv ctx;
	ie->copyAll(&ctx);
	ctx.self = o;
	ctx.pkgsender = ie->self;

	goref gr = o;
	for(uint i = 0; i < o->objdef->ireact.len; i++)
		if(CanBeTriggeredBy(&ctx, o->objdef->ireact[i], ev))
			{o->objdef->ireact[i]->seq.run(&ctx);
			if(!gr.valid()) return;}
	for(DynListEntry<uint> *e = o->iReaction.first; e; e = e->next)
		if(CanBeTriggeredBy(&ctx, &(reaction[e->value]), ev))
			{reaction[e->value].seq.run(&ctx);
			if(!gr.valid()) return;}
}