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

char *ReadSCharacterRung(char *fp, char **fstline, SRequirement *r)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	r->charrungs.add();
	SCharacterRung *cr = &r->charrungs.last->value;
	cr->type = strObjDef.find(fstline[1]); // !!
	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!stricmp(word[0], "END_CHARACTER_RUNG"))
			return fp;
		else if(!stricmp(word[0], "BLOCKED"))
			cr->blocked = 1;
		else if(!stricmp(word[0], "DETAILED_STATE"))
			cr->detailedState = stfind_cs(SREQDETAILEDSTATE_str, SREQDETAILEDSTATE_NUM, word[1]);
	}
	ferr("UEOF"); return fp;
}

char *ReadSRequirement(char *fp, char **fstline, SCommission *c)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	c->reqs.add();
	SRequirement *r = &c->reqs.last->value;
	r->cl = stfind_cs(SREQUIREMENTCLASS_str, SREQUIREMENTCLASS_NUM, fstline[1]);
	switch(r->cl)
	{
		case SREQUIREMENTCLASS_BUILDING_REQUIREMENT:
			r->type = strObjDef.find(fstline[2]); break; // !!
		case SREQUIREMENTCLASS_CHARACTER_REQUIREMENT:
			r->type = strCharacterLadder.find(fstline[2]); break;
		case SREQUIREMENTCLASS_UPGRADE_REQUIREMENT:
			r->type = strOrder.find(fstline[2]); break;
	}
	mustbefound(r->type);
	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!stricmp(word[0], "END_REQUIREMENT"))
			return fp;
		else if(!stricmp(word[0], "STATE"))
			r->state = stfind_cs(SREQSTATE_str, SREQSTATE_NUM, word[1]);
		else if(!stricmp(word[0], "DETAILED_STATE"))
			r->detailedState = stfind_cs(SREQDETAILEDSTATE_str, SREQDETAILEDSTATE_NUM, word[1]);
		else if(!stricmp(word[0], "REQUIRED_COUNT"))
			r->requiredCount = atoi(word[1]);
		else if(!stricmp(word[0], "EXISTING_COUNT"))
			r->existingCount = atoi(word[1]);
		else if(!stricmp(word[0], "FOUNDATION"))
			{r->foundations.add();
			r->foundations.last->value = FindObjID(atoi(word[1]));}
		else if(!stricmp(word[0], "ORDER_ASSIGNED"))
			{r->ordersAssigned.add();
			r->ordersAssigned.last->value = FindObjID(atoi(word[1]));}
		else if(!stricmp(word[0], "CHARACTER_RUNG"))
			fp = ReadSCharacterRung(fp, word, r);
	}
	ferr("UEOF"); return fp;
}

char *ReadSCommission(char *fp, char **fstline, GameObject *o)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	o->aicontroller->commissions.add();
	SCommission *c = &o->aicontroller->commissions.last->value;
	c->type = strCommission.find(fstline[1]);
	c->obj = FindObjID(atoi(fstline[2]));
	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!stricmp(word[0], "END_COMMISSION"))
			return fp;
		else if(!stricmp(word[0], "COMPLETE"))
			c->complete = 1;
		else if(!stricmp(word[0], "FORCE_BALANCE"))
			c->forceBalance = 1;
		else if(!stricmp(word[0], "REQUIREMENT"))
			fp = ReadSRequirement(fp, word, c);
	}
	ferr("UEOF"); return fp;
}

char *ReadAIController(char *fp, GameObject *o)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	if(!o->aicontroller) o->aicontroller = new SAIController;
	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!stricmp(word[0], "END_AI_CONTROLLER"))
			return fp;
		else if(!stricmp(word[0], "MASTER_PLAN"))
			o->aicontroller->masterPlan = strPlan.find(word[1]);
		else if(!stricmp(word[0], "WORK_ORDER"))
		{
			o->aicontroller->workOrders.add();
			SWorkOrder *w = &o->aicontroller->workOrders.last->value;
			w->obj = FindObjID(atoi(word[1]));
			w->ofind = strFinderDef.find(word[2]);
			w->type = strWorkOrder.find(word[3]);
		}
		else if(!stricmp(word[0], "COMMISSION"))
			fp = ReadSCommission(fp, word, o);
	}
	ferr("UEOF"); return fp;
}

void WriteAIController(FILE *f, GameObject *o)
{
	fprintf(f, "\t\tAI_CONTROLLER\n");
	if(o->aicontroller->masterPlan != -1)
		fprintf(f, "\t\t\tMASTER_PLAN \"%s\"\n\t\t\tEND_MASTER_PLAN\n", strPlan.getdp(o->aicontroller->masterPlan));
	for(DynListEntry<SCommission> *e = o->aicontroller->commissions.first; e; e = e->next)
	if(e->value.obj.valid())
	{
		fprintf(f, "\t\t\tCOMMISSION \"%s\" %i\n", strCommission.getdp(e->value.type), e->value.obj.getID());
		if(e->value.complete) fprintf(f, "\t\t\t\tCOMPLETE\n");
		if(e->value.forceBalance) fprintf(f, "\t\t\t\tFORCE_BALANCE\n");
		for(DynListEntry<SRequirement> *r = e->value.reqs.first; r; r = r->next)
		{
			fprintf(f, "\t\t\t\tREQUIREMENT %s ", SREQUIREMENTCLASS_str[r->value.cl]);
			switch(r->value.cl)
			{
				case SREQUIREMENTCLASS_BUILDING_REQUIREMENT:
					fprintf(f, "\"%s\"\n", strObjDef.getdp(r->value.type)); break;
				case SREQUIREMENTCLASS_CHARACTER_REQUIREMENT:
					fprintf(f, "\"%s\"\n", strCharacterLadder.getdp(r->value.type)); break;
				case SREQUIREMENTCLASS_UPGRADE_REQUIREMENT:
					fprintf(f, "\"%s\"\n", strOrder.getdp(r->value.type)); break;
			}
			fprintf(f, "\t\t\t\t\tSTATE %s\n", SREQSTATE_str[r->value.state]);
			if(r->value.cl != SREQUIREMENTCLASS_UPGRADE_REQUIREMENT)
			{
				fprintf(f, "\t\t\t\t\tREQUIRED_COUNT %i\n", r->value.requiredCount);
				fprintf(f, "\t\t\t\t\tEXISTING_COUNT %i\n", r->value.existingCount);
			}
			if(r->value.cl != SREQUIREMENTCLASS_CHARACTER_REQUIREMENT)
				fprintf(f, "\t\t\t\t\tDETAILED_STATE %s\n", SREQDETAILEDSTATE_str[r->value.detailedState]);
			if(r->value.cl == SREQUIREMENTCLASS_BUILDING_REQUIREMENT)
				for(DynListEntry<goref> *g = r->value.foundations.first; g; g = g->next)
					if(g->value.valid())
						fprintf(f, "\t\t\t\t\tFOUNDATION %i\n", g->value.getID());
			if(r->value.cl == SREQUIREMENTCLASS_CHARACTER_REQUIREMENT)
			{
				for(DynListEntry<SCharacterRung> *g = r->value.charrungs.first; g; g = g->next)
				{
					fprintf(f, "\t\t\t\t\tCHARACTER_RUNG \"%s\"\n", strObjDef.getdp(g->value.type));
					if(g->value.blocked) fprintf(f, "\t\t\t\t\t\tBLOCKED\n");
					fprintf(f, "\t\t\t\t\t\tDETAILED_STATE %s\n", SREQDETAILEDSTATE_str[g->value.detailedState]);
					fprintf(f, "\t\t\t\t\tEND_CHARACTER_RUNG\n");
				}
				for(DynListEntry<goref> *g = r->value.ordersAssigned.first; g; g = g->next)
					if(g->value.valid())
						fprintf(f, "\t\t\t\t\tORDER_ASSIGNED %i 0\n", g->value.getID());
			}
			fprintf(f, "\t\t\t\tEND_REQUIREMENT\n");
		}
		fprintf(f, "\t\t\tEND_COMMISSION\n");
	}
	for(DynListEntry<SWorkOrder> *e = o->aicontroller->workOrders.first; e; e = e->next)
		if(e->value.obj.valid())
			fprintf(f, "\t\t\tWORK_ORDER %i \"%s\" \"%s\"\n\t\t\tEND_WORK_ORDER\n", e->value.obj.getID(), strFinderDef.getdp(e->value.ofind), strWorkOrder.getdp(e->value.type));
	fprintf(f, "\t\tEND_AI_CONTROLLER\n");
}
