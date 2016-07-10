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

#include "..\global.h"

char *ReadSGTrigger(char *fp, uint id, STrigger *t)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	t->id = id;
	t->period = 0.0f;
	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!stricmp(word[0], "END_TRIGGER"))
			return fp;
		if(!stricmp(word[0], "PERIOD"))
			t->period = atof(word[1]);
		if(!stricmp(word[0], "REFERENCE_TIME"))
		{
			if(t->period != 0.0f)
				t->referenceTime = atof(word[1]);
			else
				t->referenceTime = (float)atoi(word[1]) / 1000.0f;
		}
	}
	ferr("UEOF"); return fp;
}

char *ReadSGTask(char *fp, char *name, STask *t)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	uint y = strTask.find(name); mustbefound(y);
	t->type = &gstask[y];
	t->flags = 0;
	t->spawnBlueprint = 0;
	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!stricmp(word[0], "END_TASK"))
			return fp;
		switch(stfind_cs(STASK_str, STASK_NUM, word[0]))
		{
			case STASK_TARGET:
				{int i = (uint)_atoi64(word[1]);
				if(i == -1) t->target.deref();
				else t->target = FindObjID(i);
				break;}
			case STASK_PROXIMITY:
				t->proximity = atof(word[1]); break;
			case STASK_PROXIMITY_SATISFIED:
				if(atoi(word[1])) t->flags |= FSTASK_PROXIMITY_SATISFIED;
				else              t->flags &= ~FSTASK_PROXIMITY_SATISFIED;
				break;
			case STASK_LAST_DESTINATION_VALID:
				if(atoi(word[1])) t->flags |= FSTASK_LAST_DESTINATION_VALID;
				else              t->flags &= ~FSTASK_LAST_DESTINATION_VALID;
				break;
			case STASK_FIRST_EXECUTION:
				if(atoi(word[1])) t->flags |= FSTASK_FIRST_EXECUTION;
				else              t->flags &= ~FSTASK_FIRST_EXECUTION;
				break;
			case STASK_TRIGGERS_STARTED:
				if(atoi(word[1])) t->flags |= FSTASK_TRIGGERS_STARTED;
				else              t->flags &= ~FSTASK_TRIGGERS_STARTED;
				break;
			case STASK_TRIGGER:
				t->trigger.add();
				fp = ReadSGTrigger(fp, atoi(word[1]), &t->trigger.last->value);
				break;
			case STASK_PROCESS_STATE:
				t->processState = atoi(word[1]); break;
			case STASK_TASK_ID:
				t->taskID = atoi(word[1]); break;
			case STASK_START_SEQUENCE_EXECUTED:
				if(atoi(word[1])) t->flags |= FSTASK_START_SEQUENCE_EXECUTED;
				else              t->flags &= ~FSTASK_START_SEQUENCE_EXECUTED;
				break;
			case STASK_COST_DEDUCTED:
				if(atoi(word[1])) t->flags |= FSTASK_COST_DEDUCTED;
				else              t->flags &= ~FSTASK_COST_DEDUCTED;
				break;
			case STASK_SPAWN_BLUEPRINT:
				{int c = stfind_cs(CLASS_str, CLASS_NUM, word[1]);
				int d = FindObjDef(c, word[2]);
				if(d == -1) ferr("A sg task of type SPAWN has an invalid spawn blueprint!");
				t->spawnBlueprint = &(objdef[d]);
				break;}
			case STASK_DESTINATION:
				{t->destinations.add();
				t->destinations.last->value.x = atof(word[1]);
				t->destinations.last->value.y = atof(word[2]);
				break;}
		}
	}
	ferr("UEOF"); return fp;
}

char *ReadSGOrder(char *fp, uint t, SOrder *o)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	//uint t = strOrder.find(name); //mustbefound(t);
	//if(t == -1) return SkipClass(fp, "END_ORDER");
	o->type = &gsorder[t];
	o->cycled = 0;
	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!stricmp(word[0], "END_ORDER"))
			return fp;
		switch(stfind_cs(SORDER_str, SORDER_NUM, word[0]))
		{
			case SORDER_PROCESS_STATE:
				o->processState = atoi(word[1]); break;
			case SORDER_CYCLED:
				o->cycled = atoi(word[1]); break;
			case SORDER_ORDER_ID:
				o->orderID = atoi(word[1]); break;
			case SORDER_UNIQUE_TASK_ID:
				o->uniqueTaskID = atoi(word[1]); break;
			case SORDER_CURRENT_TASK:
				o->currentTask = atoi(word[1]); break;
			case SORDER_TASK:
				o->task.add();
				fp = ReadSGTask(fp, word[1], &o->task.last->value);
				break;
		}
	}
	ferr("UEOF"); return fp;
}

char *ReadOrderConfiguration(char *fp, GameObject *o)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!stricmp(word[0], "END_ORDER_CONFIGURATION"))
		{
			// Set the target reference (for FINDER_REFERENCERS).
			if(o->ordercfg.order.len)
			{
				SOrder *s = &o->ordercfg.order.first->value;
				if(s->task.len)
				{
					STask *t = &s->task.getEntry(s->currentTask)->value;
					if(t->target.valid())
						SetObjReference(o, t->target.get());
				}
			}
			return fp;
		}
		if(!stricmp(word[0], "UNIQUE_ORDER_ID"))
			o->ordercfg.uniqueOrderID = atoi(word[1]);
		if(!stricmp(word[0], "ORDER"))
		{
			uint t = strOrder.find(word[1]);
			if(t != -1)
			{
				o->ordercfg.order.add();
				fp = ReadSGOrder(fp, t, &o->ordercfg.order.last->value);
			} else	// TODO: WARNING
				fp = SkipClass(fp, "END_ORDER");
		}
	}
	ferr("UEOF"); return fp;
}

void WriteOrderConfiguration(FILE *f, GameObject *o, char *strtab)
{
	if(!o->ordercfg.order.len) return;
	fprintf(f, "%s\tORDER_CONFIGURATION\n%s\t\tUNIQUE_ORDER_ID %i\n", strtab, strtab, o->ordercfg.uniqueOrderID);
	for(DynListEntry<SOrder> *d = o->ordercfg.order.first; d; d = d->next)
	{
		fprintf(f, "%s\t\tORDER \"%s\"\n", strtab, d->value.type->name);
		fprintf(f, "%s\t\t\tPROCESS_STATE %u\n", strtab, d->value.processState);
		fprintf(f, "%s\t\t\tCYCLED %u\n", strtab, d->value.cycled);
		fprintf(f, "%s\t\t\tORDER_ID %u\n", strtab, d->value.orderID);
		fprintf(f, "%s\t\t\tUNIQUE_TASK_ID %u\n", strtab, d->value.uniqueTaskID);
		for(DynListEntry<STask> *a = d->value.task.first; a; a = a->next)
		{
			CTask *ct = a->value.type;
			fprintf(f, "%s\t\t\tTASK \"%s\"\n", strtab, ct->name);
			if(ct->type == ORDTSKTYPE_SPAWN)
			{
				fprintf(f, "%s\t\t\t\tSPAWN_BLUEPRINT %s \"%s\"\n", strtab, CLASS_str[a->value.spawnBlueprint->type], a->value.spawnBlueprint->name);
				fprintf(f, "%s\t\t\t\tCOST_DEDUCTED %i\n", strtab, (a->value.flags & FSTASK_COST_DEDUCTED)?1:0);
			}
			else if(ct->type == ORDTSKTYPE_UPGRADE)
				;
			else if(ct->type == ORDTSKTYPE_MOVE)
			{
				for(DynListEntry<couple<float> > *e = a->value.destinations.first; e; e = e->next)
					fprintf(f, "%s\t\t\t\tDESTINATION %f %f\n", strtab, e->value.x, e->value.y);
				fprintf(f, "%s\t\t\t\tFIRST_EXECUTION %u\n", strtab, (a->value.flags & FSTASK_FIRST_EXECUTION)?1:0);
				fprintf(f, "%s\t\t\t\tLAST_DESTINATION_VALID %u\n", strtab, (a->value.flags & FSTASK_LAST_DESTINATION_VALID)?1:0);
			}
			else if(ct->type == ORDTSKTYPE_FACE_TOWARDS)
				// TODO: FACE_TOWARDS %f %f
				fprintf(f, "%s\t\t\t\tTARGET %u\n", strtab, a->value.target.valid() ? a->value.target.getID() : (-1));
			else
			{
				fprintf(f, "%s\t\t\t\tTARGET %u\n", strtab, a->value.target.valid() ? a->value.target.getID() : (-1));
				fprintf(f, "%s\t\t\t\tPROXIMITY %.2f\n", strtab, a->value.proximity);
				fprintf(f, "%s\t\t\t\tPROXIMITY_SATISFIED %u\n", strtab, (a->value.flags & FSTASK_PROXIMITY_SATISFIED)?1:0);
				fprintf(f, "%s\t\t\t\tLAST_DESTINATION_VALID %u\n", strtab, (a->value.flags & FSTASK_LAST_DESTINATION_VALID)?1:0);
				fprintf(f, "%s\t\t\t\tFIRST_EXECUTION %u\n", strtab, (a->value.flags & FSTASK_FIRST_EXECUTION)?1:0);
			}
			fprintf(f, "%s\t\t\t\tTRIGGERS_STARTED %u\n", strtab, (a->value.flags & FSTASK_TRIGGERS_STARTED)?1:0);
			for(DynListEntry<STrigger> *g = a->value.trigger.first; g; g = g->next)
			{
				fprintf(f, "%s\t\t\t\tTRIGGER %u\n", strtab, g->value.id);
				if(g->value.period != 0.0f)
				{
					fprintf(f, "%s\t\t\t\t\tPERIOD %.2f\n", strtab, g->value.period);
					fprintf(f, "%s\t\t\t\t\tREFERENCE_TIME %.2f\n", strtab, g->value.referenceTime);
				} else {
					if(wkver >= WKVER_BATTLES)
						fprintf(f, "%s\t\t\t\t\tREFERENCE_TIME %u\n", strtab, (uint)(g->value.referenceTime * 1000.0f));
				}
				fprintf(f, "%s\t\t\t\tEND_TRIGGER\n", strtab);
			}
			fprintf(f, "%s\t\t\t\tPROCESS_STATE %u\n", strtab, a->value.processState);
			fprintf(f, "%s\t\t\t\tTASK_ID %u\n", strtab, a->value.taskID);
			fprintf(f, "%s\t\t\t\tSTART_SEQUENCE_EXECUTED %u\n", strtab, (a->value.flags & FSTASK_START_SEQUENCE_EXECUTED)?1:0);
			fprintf(f, "%s\t\t\tEND_TASK\n", strtab);
		}
		fprintf(f, "%s\t\t\tCURRENT_TASK %u\n", strtab, d->value.currentTask);
		fprintf(f, "%s\t\tEND_ORDER\n", strtab);
	}
	fprintf(f, "%s\tEND_ORDER_CONFIGURATION\n", strtab);
};

//-------------
// PARAM_BLOCK
//-------------

void InitParamBlock(GameObject *o)
{
	if(o->param) return;
	o->param = new GOParamBlock;
	o->param->order = 0;
	o->param->obj.deref();
	o->param->dest.x = o->param->face.x = -1;
}

char *ReadParamBlock(char *fp, GameObject *o)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	InitParamBlock(o);
	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!stricmp(word[0], "PARAM_ORDER_BLUEPRINT_NAME"))
		{
			int ox = strOrder.find(word[1]);
			if(ox != -1)
				{o->param->order = &(gsorder[ox]); /*o->param->obj = -1;*/}
		}
		else if(!stricmp(word[0], "PARAM_OBJECT_ID"))
			o->param->obj = FindObjID(atoi(word[1]));
		else if(!stricmp(word[0], "PARAM_DESTINATION"))
			{o->param->dest.x = atof(word[1]);
			o->param->dest.y = 0; // Also note himap can change.
			o->param->dest.z = atof(word[2]);}
		else if(!stricmp(word[0], "PARAM_POSITION_TO_FACE"))
			{o->param->face.x = atof(word[1]);
			o->param->face.y = 0;
			o->param->face.z = atof(word[2]);}
		else if(!stricmp(word[0], "END_OF_PARAM_BLOCK"))
			return fp;
	}
	ferr("UEOF"); return fp;
}

void WriteParamBlock(FILE *f, GameObject *o, char *strtab)
{
	if(!o->param) return;
	if(!o->param->order) return;
	fprintf(f, "%s\tPARAM_BLOCK\n"
		   "%s\t\tPARAM_ORDER_BLUEPRINT_NAME \"%s\"\n",
		strtab, strtab, o->param->order->name);
	if(o->param->obj.valid())
		fprintf(f, "%s\t\tPARAM_OBJECT_ID %i\n", strtab, o->param->obj.getID());
	if(o->param->dest.x > -1)
		fprintf(f, "%s\t\tPARAM_DESTINATION %f %f\n", strtab, o->param->dest.x, o->param->dest.z);
	if(o->param->face.x > -1)
		fprintf(f, "%s\t\tPARAM_POSITION_TO_FACE %f %f\n", strtab, o->param->face.x, o->param->face.z);
	fprintf(f, "%s\tEND_OF_PARAM_BLOCK\n", strtab);
}