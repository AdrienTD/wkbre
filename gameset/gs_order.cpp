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

void ReadCOrder(char **pntfp, char **fstline)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords, strdex;
	COrder *co = &(gsorder[strdex = strOrder.find(fstline[1])]);
	co->name = strOrder.getdp(strdex);
	co->cycle = 0;
	co->initSeq = co->startSeq = co->suspendSeq = co->resumeSeq = co->cancelSeq = co->terminateSeq = 0;
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		//if( *((uint*)(word[0])) == '_DNE' )
		if(!stricmp(word[0], "END_ORDER"))
			return;
		switch(stfind_cs(CORDER_str, CORDER_NUM, word[0]))
		{
			case CORDER_CLASS_TYPE:
				co->type = stfind_cs(ORDTSKTYPE_str, ORDTSKTYPE_NUM, word[1]);
				break;
			case CORDER_USE_TASK:
				co->tasks.add(&(gstask[strTask.find(word[1])])); break;
			case CORDER_FLAG:
				if(!stricmp(word[1], "CYCLE_ORDER"))
					co->cycle = 1;
				break;
			case CORDER_INITIALISATION_SEQUENCE:
				co->initSeq = ReadActSeq(pntfp); break;
			case CORDER_START_SEQUENCE:
				co->startSeq = ReadActSeq(pntfp); break;
			case CORDER_SUSPENSION_SEQUENCE:
				co->suspendSeq = ReadActSeq(pntfp); break;
			case CORDER_RESUMPTION_SEQUENCE:
				co->resumeSeq = ReadActSeq(pntfp); break;
			case CORDER_CANCELLATION_SEQUENCE:
				co->cancelSeq = ReadActSeq(pntfp); break;
			case CORDER_TERMINATION_SEQUENCE:
				co->terminateSeq = ReadActSeq(pntfp); break;
		}
	}
	ferr("UEOF"); return;
}

void ReadCTrigger(char **pntfp, char **fstline, CTask *s)
{
	CTrigger *g = s->triggers.addp();
	g->seq = 0; g->period = 0;
	g->type = stfind_cs(TASKTRIGGER_str, TASKTRIGGER_NUM, fstline[1]);
	if(g->type == TASKTRIGGER_TIMER)
	{
		char **w = fstline + 2;
		g->period = ReadValue(&w);
	}
	g->seq = ReadActSeq(pntfp);
}

void ReadCTask(char **pntfp, char **fstline)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords, strdex;
	CTask *ct = &(gstask[strdex = strTask.find(fstline[1])]);
	ct->name = strTask.getdp(strdex);
	ct->target = 0; ct->proxRequirement = 0;
	ct->initSeq = ct->startSeq = ct->suspendSeq = ct->resumeSeq = ct->cancelSeq = ct->terminateSeq = ct->proxSatisfiedSeq = 0;
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!stricmp(word[0], "END_TASK"))
			return;
		switch(stfind_cs(CTASK_str, CTASK_NUM, word[0]))
		{
			case CTASK_CLASS_TYPE:
				ct->type = stfind_cs(ORDTSKTYPE_str, ORDTSKTYPE_NUM, word[1]);
				break;
			case CTASK_TASK_TARGET:
				{char **w = word + 1;
				ct->target = ReadFinder(&w);
				break;}
			case CTASK_PROXIMITY_REQUIREMENT:
				{char **w = word + 1;
				ct->proxRequirement = ReadValue(&w);
				break;}
			case CTASK_INITIALISATION_SEQUENCE:
				ct->initSeq = ReadActSeq(pntfp); break;
			case CTASK_START_SEQUENCE:
				ct->startSeq = ReadActSeq(pntfp); break;
			case CTASK_SUSPENSION_SEQUENCE:
				ct->suspendSeq = ReadActSeq(pntfp); break;
			case CTASK_RESUMPTION_SEQUENCE:
				ct->resumeSeq = ReadActSeq(pntfp); break;
			case CTASK_CANCELLATION_SEQUENCE:
				ct->cancelSeq = ReadActSeq(pntfp); break;
			case CTASK_TERMINATION_SEQUENCE:
				ct->terminateSeq = ReadActSeq(pntfp); break;
			case CTASK_PROXIMITY_SATISFIED_SEQUENCE:
				ct->proxSatisfiedSeq = ReadActSeq(pntfp); break;
			case CTASK_TRIGGER:
				ReadCTrigger(pntfp, word, ct); break;
		}
	}
	ferr("UEOF"); return;
}

void ReadCOrderAssignment(char **pntfp, char **fstline)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	COrderAssignment *co = &(orderAssign[strOrderAssign.find(fstline[1])]);
	co->mode = 0; co->order = 0; co->target = 0;
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if( *((uint*)(word[0])) == '_DNE' )
			return;
		switch(stfind_cs(CORDERASSIGN_str, CORDERASSIGN_NUM, word[0]))
		{
			case CORDERASSIGN_ORDER_ASSIGNMENT_MODE:
				co->mode = stfind_cs(ORDERASSIGNMODE_str, ORDERASSIGNMODE_NUM, word[1]);
				break;
			case CORDERASSIGN_ORDER_TO_ASSIGN:
				co->order = &(gsorder[strOrder.find(word[1])]); break;
			case CORDERASSIGN_ORDER_TARGET:
				{char **w = word + 1;
				co->target = ReadFinder(&w);
				break;}
		}
	}
	ferr("UEOF"); return;
}