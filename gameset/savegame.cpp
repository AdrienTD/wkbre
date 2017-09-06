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

struct RSAssociationListEntry
{
	GameObject *o; uint cat, len;
	uint *ids;
};

struct PredecEntry
{
	uint objdef;
	uint numids, *id;
};

GameObject *gameobj[MAX_GAMEOBJECTS];
uint nextgoid = 0, nexthandle = 0;
GrowList<PredecEntry> predec;
GameObject *levelobj;
char sggameset[384] = "\0";
GrowList<RSAssociationListEntry> toassolist;

uint game_type = 0, update_id, random_seed; float next_update_time_stamp;
float current_time, previous_time, elapsed_time; uint paused, lock_count;
DynList<goref> humanplayers;
uint wkver = 0;
DynList<DelayedSequenceEntry> delayedSeq;
DynList<SequenceOverPeriodEntry> exePeriodSeq, repPeriodSeq;
wchar_t *campaignName = 0, *serverName = 0;

DynList<GameObject*> norefobjs;

valuetype GameObject::getItem(int x)
{
	for(DynListEntry<GOItem> *e = this->item.first; e; e = e->next)
		if(e->value.num == x)
			return e->value.value;
	return this->objdef->startItems[x];
}

void GameObject::setItem(int x, valuetype v)
{
	for(DynListEntry<GOItem> *e = this->item.first; e; e = e->next)
		if(e->value.num == x)
			{e->value.value = v; return;}
	this->item.add();
	this->item.last->value.num = x;
	this->item.last->value.value = v;
}

valuetype GameObject::getIndexedItem(int x, int y)
{
	for(DynListEntry<GOIndexedItem> *e = this->indexeditem.first; e; e = e->next)
		if(e->value.num == x)
			if(e->value.index == y)
				return e->value.value;
	return 0;
}

void GameObject::setIndexedItem(int x, int y, valuetype v)
{
	for(DynListEntry<GOIndexedItem> *e = this->indexeditem.first; e; e = e->next)
		if(e->value.num == x)
			if(e->value.index == y)
				{e->value.value = v; return;}
	this->indexeditem.add();
	this->indexeditem.last->value.num = x;
	this->indexeditem.last->value.index = y;
	this->indexeditem.last->value.value = v;
}

GameObject *SubFindObjID(int id, GameObject *p)
{
	if(p->id == id)
		return p;
	for(DynListEntry<GameObject> *e = p->children.first; e; e = e->next)
	{
		GameObject *r = SubFindObjID(id, &e->value);
		if(r) return r;
	}
	return 0;
}

GameObject *FindObjID(int id)
{
	return SubFindObjID(id, levelobj);
}

void DeleteInvalidRefs(DynList<goref> *l)
{
	for(DynListEntry<goref> *e = l->first; e; e = e->next)
		if(!e->value.valid())
			l->remove(e);
}

char *ProcessPredec(char *fp)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	//printf("--- PREDEC ---\n");
	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!strcmp(word[0], "END_PREDEC")) {fflush(stdout); return fp;}
		int t = stfind_cs(CLASS_str, CLASS_NUM, word[0]);
		int d = FindObjDef(t, word[1]);
		int nobj = atoi(word[2]);	// Num. obj. ids. to read.
		int noc = 0;			// Num. obj. ids. read.
		int n = nwords - 3;		// Num. obj. ids. in line.
		char **p = ((char**)word) + 3;

		PredecEntry pe;
		pe.objdef = d;
		pe.numids = nobj;
		pe.id = (uint*)malloc(nobj * sizeof(uint));
		uint *ip = pe.id;

		while(1)
		{
			for(int i = 0; i < n; i++)
			{
				int o = atoi(*(p++));
				*(ip++) = o;
			}
			noc += n; if(noc >= nobj) break;
			fp = GetLine(fp, wwl);
			nwords = GetWords(wwl, word);			
			n = nwords; p = word;
		}

		predec.add(pe);
	}
	ferr("UEOF"); return fp;
}

uint FindPredec(int id)
{
	for(int i = 0; i < predec.len; i++)
	{
		PredecEntry *p = predec.getpnt(i);
		for(int j = 0; j < p->numids; j++)
			if(p->id[j] == id)
				return p->objdef;
	}
	return 0;
}

void FreePredec()
{
	for(int i = 0; i < predec.len; i++)
		free(predec.getpnt(i)->id);
	predec.clear();
}

char *LoadGameObjectP0(char *fp, char **fline, int fwords, GameObject *parent)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	char endword[32] = "END_\0";

	int t = stfind_cs(CLASS_str, CLASS_NUM, fline[0]); mustbefound(t);
	int id = atoi(fline[1]);
	int od;
	if(!(od = FindPredec(id)))
		{od = FindObjDef(t, (fwords>=3)?fline[2]:"Standard");
		mustbefound(od);}

	GameObject *go = CreateObject(&objdef[od], parent, id);

	strcat(endword, fline[0]);
	if(!strcmp(fline[0], "LEVEL")) levelobj = go;

	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!strcmp(endword, word[0]))
			return fp;

		switch(stfind_cs(GAMEOBJ_str, GAMEOBJ_NUM, word[0]))
		{
			case GAMEOBJ_MAP:
				LoadMap(word[1]); break;
			case GAMEOBJ_NEXT_UNIQUE_ID:
				nextgoid = atoi(word[1]); wkver = WKVER_ORIGINAL; break;
			case GAMEOBJ_COLOUR_INDEX:
				go->color = atoi(word[1]); break;
			case GAMEOBJ_PLAYER:
			case GAMEOBJ_BUILDING:
			case GAMEOBJ_CHARACTER:
			case GAMEOBJ_MARKER:
			//
			case GAMEOBJ_CITY:
			case GAMEOBJ_TOWN:
			case GAMEOBJ_CONTAINER:
			case GAMEOBJ_FORMATION:
			case GAMEOBJ_MISSILE:
			case GAMEOBJ_PROP:
			case GAMEOBJ_ARMY:
			case GAMEOBJ_TERRAIN_ZONE:
				fp = LoadGameObjectP0(fp, word, nwords, go); break;
		}
	}
	ferr("UEOF"); return fp;
}

char *LoadGameObjectP1(char *fp, char **fline, int fwords, GameObject *parent)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	char endword[32] = "END_\0";

	//GameObject *go = FindObjID(atoi(fline[1]));
	GameObject *go = SubFindObjID(atoi(fline[1]), parent?parent:levelobj);
	int od = go->objdef->id;

	strcat(endword, fline[0]);
	//if(!strcmp(fline[0], "LEVEL")) levelobj = go;

	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!strcmp(endword, word[0]))
			return fp;
		int a, b; GameObject *x;
		switch(stfind_cs(GAMEOBJ_str, GAMEOBJ_NUM, word[0]))
		{
			case GAMEOBJ_POSITION:
				go->position.x = atof(word[1]);
				go->position.y = atof(word[2]);
				go->position.z = atof(word[3]);
				break;
			case GAMEOBJ_ORIENTATION:
				go->orientation.x = atof(word[1]);
				go->orientation.y = atof(word[2]);
				if(nwords >= 4)
					go->orientation.z = atof(word[3]);
				break;
			case GAMEOBJ_SCALE:
				go->scale.x = atof(word[1]);
				go->scale.y = atof(word[2]);
				go->scale.z = atof(word[3]);
				break;
			case GAMEOBJ_ITEM:
				a = strItems.find(word[1]); //mustbefound(a);
				if(a != -1)
					go->setItem(a, atov(word[2]));
				break;
			case GAMEOBJ_SUBTYPE:
				a = pstObjDef[od]->find(word[1]); mustbefound(a);
				go->subtype = a; break;
			case GAMEOBJ_APPEARANCE:
				a = pstObjDef[od]->find(word[1]); mustbefound(a);
				b = strAppearTag.find(word[2]); mustbefound(b);
				go->subtype = a; go->appearance = b; break;
			case GAMEOBJ_RECONNAISSANCE:
				if(atoi(word[1]))
					go->flags |= FGO_RECONNAISSANCE;
				else
					go->flags &= ~FGO_RECONNAISSANCE;
				break;
			case GAMEOBJ_FOG_OF_WAR:
				if(atoi(word[1]))
					go->flags |= FGO_FOG_OF_WAR;
				else
					go->flags &= ~FGO_FOG_OF_WAR;
				break;
			case GAMEOBJ_RECTANGLE:
				if(!go->rects) go->rects = new GrowList<int>;
				go->rects->add(atoi(word[1]));
				go->rects->add(atoi(word[2]));
				go->rects->add(atoi(word[3]));
				go->rects->add(atoi(word[4])); break;
			case GAMEOBJ_ALIAS:
				a = strAlias.find(word[1]); //mustbefound(a);
				if(a == -1) break; // TODO: WARNING
				b = atoi(word[2]);
				for(int i = 0; i < b; i++)
					if(x = FindObjID(atoi(word[3+i])))
						AliasObj(x, a);
				break;
			case GAMEOBJ_ASSOCIATE:
				/*RSAssociationListEntry r;
				r.o = go;
				r.cat = strAssociateCat.find(word[1]);
				if(r.cat == -1) break;
				r.len = atoi(word[2]);
				r.ids = (uint*)malloc(r.len * sizeof(uint));
				for(int i = 0; i < r.len; i++)
					r.ids[i] = atoi(word[3+i]);
				toassolist.add(r);*/
				a = strAssociateCat.find(word[1]);
				b = atoi(word[2]);
				for(int i = 0; i < b; i++)
				{
					x = FindObjID(atoi(word[3+i]));
					if(!x) continue; // TODO: WARNING
					AssociateObjToObj(go, a, x);
				}
				break;
			case GAMEOBJ_ORDER_CONFIGURATION:
				fp = ReadOrderConfiguration(fp, go); break;
			case GAMEOBJ_TERMINATED:
				go->flags |= FGO_TERMINATED; break;
			case GAMEOBJ_PLAYER_TERMINATED:
				go->flags |= FGO_PLAYER_TERMINATED; break;
			case GAMEOBJ_DISABLE_COUNT:
				go->disableCount = atoi(word[1]); break;
			case GAMEOBJ_INDIVIDUAL_REACTION:
				a = strReaction.find(word[1]);
				if(a == -1) break; // TODO: WARNING
				go->iReaction.add(a);
				break;
			case GAMEOBJ_PLAYER:
			case GAMEOBJ_BUILDING:
			case GAMEOBJ_CHARACTER:
			case GAMEOBJ_MARKER:
			//
			case GAMEOBJ_CITY:
			case GAMEOBJ_TOWN:
			case GAMEOBJ_CONTAINER:
			case GAMEOBJ_FORMATION:
			case GAMEOBJ_MISSILE:
			case GAMEOBJ_PROP:
			case GAMEOBJ_ARMY:
			case GAMEOBJ_TERRAIN_ZONE:
				fp = LoadGameObjectP1(fp, word, nwords, go); break;
			case GAMEOBJ_DIPLOMATIC_STATUS_BETWEEN:
				{int s; mustbefound(s = strDiplomaticStatus.find(word[3]));
				SetDiplomaticStatus(FindObjID(atoi(word[1])), FindObjID(atoi(word[2])), s);
				break;}
			case GAMEOBJ_NAME:
			{
				if(go->name)
					delete [] go->name;
				if(wkver == WKVER_BATTLES)
				if(objdef[od].type == CLASS_PLAYER)
				{
					// 16-bit string
					int nn = atoi(word[1]);
					go->name = new wchar_t[nn+1];
					for(int i = 0; i < nn; i++)
						go->name[i] = atoi(word[2+i]);
					go->name[nn] = 0;
					break;
				}
				// 8-bit string
				int nn = strlen(word[1]);
				go->name = new wchar_t[nn+1];
				for(int i = 0; i < nn; i++)
					go->name[i] = (word[1])[i];
				go->name[nn] = 0;
			} break;
			case GAMEOBJ_PARAM_BLOCK:
				fp = ReadParamBlock(fp, go); break;
			case GAMEOBJ_TILES:
				{int nt = atoi(word[1]);
				if(!nt) break;
				if(!go->tiles) go->tiles = new GrowList<couple<uint> >;
				for(int i = 0; i < nt; i++)
				{
					couple<uint> *c = go->tiles->addp();
					c->x = atoi(word[2+i*2]);
					c->y = atoi(word[2+i*2+1]);
				}
				break;}
			case GAMEOBJ_SELECTABLE:
				if(atoi(word[1]))
					go->flags |= FGO_SELECTABLE;
				else
					go->flags &= ~FGO_SELECTABLE;
				break;
			case GAMEOBJ_TARGETABLE:
				if(atoi(word[1]))
					go->flags |= FGO_TARGETABLE;
				else
					go->flags &= ~FGO_TARGETABLE;
				break;
			case GAMEOBJ_RENDERABLE:
				if(atoi(word[1]))
					go->flags |= FGO_RENDERABLE;
				else
					go->flags &= ~FGO_RENDERABLE;
				break;
			case GAMEOBJ_AI_CONTROLLER:
				fp = ReadAIController(fp, go); break;
			case GAMEOBJ_DIPLOMATIC_OFFER:
				{int s; mustbefound(s = strDiplomaticStatus.find(word[3]));
				MakeDiplomaticOffer(FindObjID(atoi(word[1])), FindObjID(atoi(word[2])), s);
				break;}
		}
	}
	ferr("UEOF"); return fp;
}

/*void SetAssociationsFromList()
{
	for(int i = 0; i < toassolist.len; i++)
	{
		RSAssociationListEntry *r = toassolist.getpnt(i);
		GameObject *x;
		for(int j = 0; j < r->len; j++)
			if(x = FindObjID(r->ids[j]))
				AssociateObjToObj(r->o, r->cat, x);
		free(r->ids);
	}
	toassolist.clear();
}*/

void ReadSequenceOverPeriod(SequenceOverPeriodEntry *e, char **word)
{
	e->actseq = strActionSeq.find(word[1]); //mustbefound(e->actseq);
	e->executor = FindObjID(atoi(word[2]));
	e->time = atof(word[3]);
	e->period = atof(word[4]);
	e->nloops = atoi(word[5]);
	e->loopsexec = atoi(word[6]);
	e->numobj = atoi(word[7]);
	e->ola = e->obj = (goref*)malloc(sizeof(goref) * e->numobj);
	for(int i = 0; i < e->numobj; i++)
		e->obj[i] = FindObjID(atoi(word[i+8]));
}

void LoadSaveGame(char *fn)
{
	char *fcnt, *fp; int fsize;
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;

	LoadFile(fn, &fcnt, &fsize, 1);
	fcnt[fsize] = 0;
	gline = 0;
	memset(gameobj, 0, sizeof(gameobj));

	// 1st pass
	loadinginfo("Savegame pass 0");
	fp = fcnt;
	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		switch(stfind_cs(SAVEGAME_str, SAVEGAME_NUM, word[0]))
		{
			case SAVEGAME_GAME_SET:
				LoadGameSet(word[1]); strcpy(sggameset, word[1]);
				gline = 0; break;
			case SAVEGAME_NEXT_UNIQUE_ID:
				nextgoid = atoi(word[1]); wkver = WKVER_BATTLES; break;
			case SAVEGAME_PREDEC:
				fp = ProcessPredec(fp); break;

			case SAVEGAME_LEVEL:
				fp = LoadGameObjectP0(fp, word, nwords, NULL); break;
		}
	}

	// 2nd pass
	loadinginfo("Savegame pass 1");
	fp = fcnt;
	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		switch(stfind_cs(SAVEGAME_str, SAVEGAME_NUM, word[0]))
		{
			case SAVEGAME_GAME_TYPE:
				game_type = atoi(word[1]); break;
			case SAVEGAME_PART_OF_CAMPAIGN:
				// NOTE: WKO does support PART_OF_CAMPAIGN
				{if(wkver < WKVER_BATTLES) break;
				if(campaignName) delete [] campaignName;
				int nc = atoi(word[1]);
				campaignName = new wchar_t[nc+1];
				for(int i = 0; i < nc; i++)
					campaignName[i] = atoi(word[2+i]);
				campaignName[nc] = 0;
				break;}
			case SAVEGAME_SERVER_NAME:
				{if(wkver < WKVER_BATTLES) break;
				if(serverName) delete [] serverName;
				int nc = atoi(word[1]);
				serverName = new wchar_t[nc+1];
				for(int i = 0; i < nc; i++)
					serverName[i] = atoi(word[2+i]);
				serverName[nc] = 0;
				break;}
			case SAVEGAME_NUM_HUMAN_PLAYERS:
				{int num_human_players = atoi(word[1]);
				for(int i = 0; i < num_human_players; i++)
				{
					GameObject *o = FindObjID(atoi(word[2+i]));
					if(!o) continue;
					humanplayers.add();
					humanplayers.last->value = o;
				}
				break;}
			case SAVEGAME_UPDATE_ID:
				update_id = atoi(word[1]); break;
			case SAVEGAME_NEXT_UPDATE_TIME_STAMP:
				next_update_time_stamp = atof(word[1]); break;
			case SAVEGAME_CURRENT_TIME:
				current_time = atof(word[1]); break;
			case SAVEGAME_PREVIOUS_TIME:
				previous_time = atof(word[1]); break;
			case SAVEGAME_ELAPSED_TIME:
				elapsed_time = atof(word[1]); break;
			case SAVEGAME_PAUSED:
				paused = atoi(word[1]); break;
			case SAVEGAME_LOCK_COUNT:
				lock_count = atoi(word[1]); break;

			case SAVEGAME_PREDEC:
				fp = SkipClass(fp, "END_PREDEC"); break;
			case SAVEGAME_LEVEL:
				fp = LoadGameObjectP1(fp, word, nwords, NULL); break;

			case SAVEGAME_CLIENT_STATE:
				fp = ReadClientState(fp, word); break;

			case SAVEGAME_DELAYED_SEQUENCE_EXECUTION:
				{int asq = strActionSeq.find(word[2]);
				if(asq == -1) break;
				delayedSeq.add();
				DelayedSequenceEntry *e = &delayedSeq.last->value;
				e->time = atof(word[1]);
				e->actseq = asq;
				e->executor = FindObjID(atoi(word[3]));
				e->numobj = atoi(word[4]);
				e->obj = (goref*)malloc(sizeof(goref) * e->numobj);
				int w = 5;
				for(int i = 0; i < e->numobj; i++)
					e->obj[i] = FindObjID(atoi(word[w++]));
				e->unk = atoi(word[w]);
				break;}
			case SAVEGAME_EXECUTE_SEQUENCE_OVER_PERIOD:
				if(strActionSeq.find(word[1]) == -1) break;
				exePeriodSeq.add();
				ReadSequenceOverPeriod(&exePeriodSeq.last->value, word);
				break;
			case SAVEGAME_REPEAT_SEQUENCE_OVER_PERIOD:
				if(strActionSeq.find(word[1]) == -1) break;
				repPeriodSeq.add();
				ReadSequenceOverPeriod(&repPeriodSeq.last->value, word);
				break;
		}
	}

	//SetAssociationsFromList();
	FreePredec();
	free(fcnt);

	InitITiles();
	ResetITiles();
	if(lock_count > 0) lock_count--;
}

//**********************/
// Saving
//**********************/

void WriteGORefDynList(FILE *f, DynList<goref> *l)
{
	uint numobjs = 0;
	for(DynListEntry<goref> *e = l->first; e; e = e->next)
		if(e->value.valid())
			numobjs++;
	if(!numobjs) return;
	fprintf(f, " %u", numobjs);
	for(DynListEntry<goref> *e = l->first; e; e = e->next)
		if(e->value.valid())
			fprintf(f, " %u", e->value.getID());
}

void WriteGORefArray(FILE *f, goref *gl, int gms)
{
	//uint numobjs = 0;
	//for(uint i = 0; i < gms; i++)
	//	if(gl[i].valid())
	//		numobjs++;
	//if(!numobjs) return;
	//fprintf(f, " %u", numobjs);
	for(uint i = 0; i < gms; i++)
		if(gl[i].valid())
			fprintf(f, " %u", gl[i].getID());
}

/*int NoValidGORinArray(goref *gl, int gms)
{
	for(uint i = 0; i < gms; i++)
		if(gl[i].valid())
			return 0;
	return 1;
}*/

int NumValidGORinArray(goref *gl, int gms)
{
	int numobjs = 0;
	for(uint i = 0; i < gms; i++)
		if(gl[i].valid())
			numobjs++;
	return numobjs;
}

void WriteGameObject(GameObject *o, FILE *f, int tabs)
{
	char strtab[16];
	if(tabs >= 16) tabs = 15;
	memset(strtab, '\t', tabs);
	strtab[tabs] = 0;

	fprintf(f, "\n%s%s %i \"%s\"\n", strtab, CLASS_str[o->objdef->type], o->id, o->objdef->name);
	if(o->name)
	{
		if((wkver == WKVER_BATTLES) && (o->objdef->type == CLASS_PLAYER))
		{
			uint n = wcslen(o->name);
			fprintf(f, "%s\tNAME %u", strtab, n);
			for(int i = 0; i < n; i++)
				fprintf(f, " %u", o->name[i]);
			fprintf(f, "\n");
		}
		else fprintf(f, "%s\tNAME \"%S\"\n", strtab, o->name);
	}
	if(o->objdef->type == CLASS_LEVEL) if(wkver < WKVER_BATTLES)
		fprintf(f, "\tNEXT_UNIQUE_ID %u\n", nextgoid);

	if(o->param) WriteParamBlock(f, o, strtab);
	fprintf(f, "%s\tPOSITION %.2f %.2f %.2f\n", strtab, o->position.x, o->position.y, o->position.z);
	fprintf(f, "%s\tORIENTATION %.2f %.2f %.2f\n", strtab, o->orientation.x, o->orientation.y, o->orientation.z);
	if(o->scale != o->objdef->scale)
		fprintf(f, "%s\tSCALE %.2f %.2f %.2f\n", strtab, o->scale.x, o->scale.y, o->scale.z);
	if(wkver == WKVER_BATTLES)
	{
		if(o->appearance)
			fprintf(f, "%s\tAPPEARANCE \"%s\" \"%s\"\n", strtab, o->objdef->subtypes[o->subtype].name, strAppearTag.getdp(o->appearance));
		else if(o->subtype)
			fprintf(f, "%s\tSUBTYPE \"%s\"\n", strtab, o->objdef->subtypes[o->subtype].name);
	} else {
		if((o->objdef->type == CLASS_BUILDING) || (o->objdef->type == CLASS_CHARACTER)
		 || (o->objdef->type == CLASS_CONTAINER) || (o->objdef->type == CLASS_MISSILE)
		 || (o->objdef->type == CLASS_PROP))
			fprintf(f, "%s\tAPPEARANCE \"%s\" \"%s\"\n", strtab, o->objdef->subtypes[o->subtype].name, strAppearTag.getdp(o->appearance));
	}
	WriteOrderConfiguration(f, o, strtab);
	for(int i = 0; i < strItems.len; i++)
		if(o->getItem(i) != o->objdef->startItems[i])
			fprintf(f, "%s\tITEM \"%s\" %.2f\n", strtab, strItems.getdp(i), o->getItem(i));
	for(DynListEntry<GOIndexedItem> *e = o->indexeditem.first; e; e = e->next)
		fprintf(f, "%s\tITEM \"%s %i\" %.2f\n", strtab, strItems.getdp(e->value.num), e->value.index, e->value.value);
	if(o->flags & FGO_TERMINATED)
		fprintf(f, "%s\tTERMINATED\n", strtab);
	if(o->flags & FGO_PLAYER_TERMINATED)
		fprintf(f, "%s\tPLAYER_TERMINATED\n", strtab);
	if(o->disableCount)
		fprintf(f, "%s\tDISABLE_COUNT %i\n", strtab, o->disableCount);
	if(o->rects)
		for(int i = 0; i < o->rects->len / 4; i++)
			fprintf(f, "%s\tRECTANGLE %i %i %i %i\n", strtab, o->rects->get(i*4), o->rects->get(i*4+1), o->rects->get(i*4+2), o->rects->get(i*4+3));
	for(DynListEntry<GOAssociation> *e = o->association.first; e; e = e->next)
		if(e->value.associates.len)
		{
			fprintf(f, "%s\tASSOCIATE \"%s\" %i", strtab, strAssociateCat.getdp(e->value.category), e->value.associates.len);
			for(DynListEntry<GameObjAndListEntry> *x = e->value.associates.first; x; x = x->next)
				fprintf(f, " %u", x->value.o->id);
			fprintf(f, "\n");
		}

	if(o->objdef->type == CLASS_LEVEL)
		fprintf(f, "%s\tLANDSCAPE 1032\n%s\t\tMAP \"%s\"\n%s\tEND_LANDSCAPE\n", strtab, strtab, lastmap, strtab);
	if(o->objdef->type == CLASS_PLAYER)
	{
		fprintf(f, "%s\tCOLOUR_INDEX %i\n", strtab, o->color);
		fprintf(f, "%s\tRECONNAISSANCE %i\n%s\tFOG_OF_WAR %i\n", strtab, (o->flags & FGO_RECONNAISSANCE)?1:0, strtab, (o->flags & FGO_FOG_OF_WAR)?1:0);
	}

	for(DynListEntry<uint> *e = o->iReaction.first; e; e = e->next)
		fprintf(f, "%s\tINDIVIDUAL_REACTION \"%s\"\n", strtab, strReaction.getdp(e->value));

	if(!(o->flags & FGO_SELECTABLE))
		fprintf(f, "%s\tSELECTABLE 0\n", strtab);
	if(!(o->flags & FGO_TARGETABLE))
		fprintf(f, "%s\tTARGETABLE 0\n", strtab);
	if(!(o->flags & FGO_RENDERABLE))
		fprintf(f, "%s\tRENDERABLE 0\n", strtab);

	if(o->tiles)
	{
		int nt = o->tiles->len;
		int q = nt / 50, r = nt % 50;
		for(int i = 0; i < q; i++)
		{
			fprintf(f, "%s\tTILES 50", strtab);
			for(int j = 0; j < 50; j++)
			{
				couple<uint> *c = o->tiles->getpnt(i*50+j);
				fprintf(f, " %u %u", c->x, c->y);
			}
			fprintf(f, "\n");
		}
		if(r)
		{
			fprintf(f, "%s\tTILES %i", strtab, r);
			for(int j = 0; j < r; j++)
			{
				couple<uint> *c = o->tiles->getpnt(q*50+j);
				fprintf(f, " %u %u", c->x, c->y);
			}
			fprintf(f, "\n");
		}
	}

	for(DynListEntry<GameObject> *e = o->children.first; e; e = e->next)
		WriteGameObject(&e->value, f, tabs+1);

	if(o->objdef->type == CLASS_LEVEL)
	{
		WriteDiplomaticStatuses(f);
		WriteDiplomaticOffers(f);

		for(int c = 0; c < strAlias.len; c++)
		{
			DeleteInvalidRefs(&(alias[c]));
			if(alias[c].len)
			{
				fprintf(f, "%s\tALIAS \"%s\" %u", strtab, strAlias.getdp(c), alias[c].len);
				for(DynListEntry<goref> *e = alias[c].first; e; e = e->next)
					if(e->value.valid())
						fprintf(f, " %u", e->value.get()->id);
				fprintf(f, "\n");
			}
		}
	}

	if(o->objdef->type == CLASS_PLAYER)
		if(o->aicontroller)
			WriteAIController(f, o);

	fprintf(f, "%sEND_%s\n", strtab, CLASS_str[o->objdef->type]);
}

void SaveSaveGame(char *fn)
{
	FILE *f = fopen(fn, "w"); if(!f) ferr("Cannot open file for saving.");
	fprintf(f, "FORMAT SCENARIO\nVERSION 1.0\nMODE TEXT\n\n"); // otherwise WKO won't load the savegame.
	fprintf(f, "GAME_SET \"%s\"\n", sggameset);
	if(wkver >= WKVER_BATTLES)
		fprintf(f, "NEXT_UNIQUE_ID %u\n", nextgoid);
	if(game_type)
		fprintf(f, "GAME_TYPE %u\n", game_type);
	if(campaignName) if(wkver >= WKVER_BATTLES)
	{
		int nc = wcslen(campaignName);
		fprintf(f, "PART_OF_CAMPAIGN %u", nc);
		for(int i = 0; i < nc; i++)
			fprintf(f, " %u", campaignName[i]);
		fprintf(f, "\n");
	}
	if(serverName) if(wkver >= WKVER_BATTLES)
	{
		int nc = wcslen(serverName);
		fprintf(f, "SERVER_NAME %u", nc);
		for(int i = 0; i < nc; i++)
			fprintf(f, " %u", serverName[i]);
		fprintf(f, "\n");
	}

	DynListEntry<goref> *nhp;
	for(DynListEntry<goref> *e = humanplayers.first; e; e = nhp)
		{nhp = e->next; if(!e->value.valid()) humanplayers.remove(e);}
	fprintf(f, "NUM_HUMAN_PLAYERS %u", humanplayers.len);
	for(DynListEntry<goref> *e = humanplayers.first; e; e = e->next)
		fprintf(f, " %u", e->value.getID());

	fprintf(f, "\nUPDATE_ID %u\nNEXT_UPDATE_TIME_STAMP %f\n", update_id, next_update_time_stamp);
	fprintf(f, "TIME_MANAGER_STATE\n\tCURRENT_TIME %f\n\tPREVIOUS_TIME %f\n\tELAPSED_TIME %f\n\tPAUSED %i\n\tLOCK_COUNT %i\nEND_TIME_MANAGER_STATE\n",
		current_time, previous_time, elapsed_time, paused, lock_count + 1);

	WriteGameObject(levelobj, f, 0);

	int numexobj;
	for(DynListEntry<DelayedSequenceEntry> *e = delayedSeq.first; e; e = e->next)
	{
		if(!e->value.executor.valid()) continue;
		numexobj = NumValidGORinArray(e->value.obj, e->value.numobj);
		if(!numexobj) continue;
		fprintf(f, "DELAYED_SEQUENCE_EXECUTION %.2f \"%s\" %u %u", e->value.time, strActionSeq.getdp(e->value.actseq), e->value.executor.getID(), numexobj);
		WriteGORefArray(f, e->value.obj, e->value.numobj);
		fprintf(f, " %u\n", e->value.unk);
	}
	for(DynListEntry<SequenceOverPeriodEntry> *e = exePeriodSeq.first; e; e = e->next)
	{
		if(!e->value.executor.valid()) continue;
		numexobj = NumValidGORinArray(e->value.obj, e->value.numobj);
		if(!numexobj) continue;
		fprintf(f, "EXECUTE_SEQUENCE_OVER_PERIOD \"%s\" %u %.2f %.2f %u %u %u", strActionSeq.getdp(e->value.actseq), e->value.executor.getID(), e->value.time, e->value.period, e->value.nloops, e->value.loopsexec, numexobj);
		WriteGORefArray(f, e->value.obj, e->value.numobj);
		fprintf(f, "\n");
	}
	for(DynListEntry<SequenceOverPeriodEntry> *e = repPeriodSeq.first; e; e = e->next)
	{
		if(!e->value.executor.valid()) continue;
		numexobj = NumValidGORinArray(e->value.obj, e->value.numobj);
		if(!numexobj) continue;
		fprintf(f, "REPEAT_SEQUENCE_OVER_PERIOD \"%s\" %u %.2f %.2f %u %u %u", strActionSeq.getdp(e->value.actseq), e->value.executor.getID(), e->value.time, e->value.period, e->value.nloops, e->value.loopsexec, numexobj);
		WriteGORefArray(f, e->value.obj, e->value.numobj);
		fprintf(f, "\n");
	}
	WriteClientStates(f);
	fclose(f);
}

//**********************/
// Editing tools
//**********************/

void RemoveObjReference(GameObject *o)
{
	if(!o->curtarget) return;
	o->curtarget->referencers.move(o->ctgdle, &norefobjs);
	o->curtarget = 0;
}

void SetObjReference(GameObject *o, GameObject *t)
{
	RemoveObjReference(o);
	norefobjs.move(o->ctgdle, &t->referencers);
	o->curtarget = t;
}

void RemoveObject(GameObject *o)
{
	if(o->children.len)
	{
		DynListEntry<GameObject> *nx;
		for(DynListEntry<GameObject> *e = o->children.first; e; e = nx)
		{
			nx = e->next;
			RemoveObject(&e->value);
		}
	}
	for(DynListEntry<GOAssociation> *e = o->association.first; e; e = e->next)
	{
		for(DynListEntry<GameObjAndListEntry> *f = e->value.associates.first; f; f = f->next)
			f->value.l->remove(f->value.e);
		for(DynListEntry<GameObjAndListEntry> *f = e->value.associators.first; f; f = f->next)
			f->value.l->remove(f->value.e);
	}
	if(o->name) delete [] o->name;
	if(o->param) delete o->param;
	if(o->tiles) delete o->tiles;
	if(o->rects) delete o->rects;
	if(o->aicontroller) delete o->aicontroller;
	o->ordercfg.order.clear();

	RemoveObjReference(o);
	DynListEntry<GameObject*> *n;
	for(DynListEntry<GameObject*> *e = o->referencers.first; e; e = n)
	{
		n = e->next;
		RemoveObjReference(e->value);
	}
	norefobjs.remove(o->ctgdle);

	gameobj[o->handle] = 0;
	if(o->parent)
		o->parent->children.remove(o->palste);
	else	delete o;
}

void UpdateParentDependantValues(GameObject *o)
{
	if(o->parent)
	if(o->objdef->type != CLASS_PLAYER)
	{
		o->color = o->parent->color;
		o->player = o->parent->player;
	}
	for(DynListEntry<GameObject> *e = o->children.first; e; e = e->next)
		UpdateParentDependantValues(&e->value);
}

void SetObjectParent(GameObject *c, GameObject *p)
{
	c->parent->children.move(c->palste, &p->children);
	c->parent = p;
	c->palste = p->children.last;
	UpdateParentDependantValues(c);
}

void SetRandomSubtypeAndAppearance(GameObject *o)
{
	CObjectDefinition *d = o->objdef; uint s;
	s = o->subtype = (d->numsubtypes > 1) ? ((rand()%(d->numsubtypes-1)) + 1) : 0;
	if((uint)o->appearance < (uint)strAppearTag.len)
		if(d->subtypes[s].appear[o->appearance].def)	// :S
			return;
	for(uint i = 0; i < strAppearTag.len; i++)
		if(d->subtypes[s].appear[i].def)		// :S
			{o->appearance = i; return;}
	o->appearance = 0;
}

int AreObjsAssociated(GameObject *a, uint cat, GameObject *b)
{
	GOAssociation *x;
	for(DynListEntry<GOAssociation> *e = a->association.first; e; e = e->next)
		if(e->value.category == cat)
			{x = &e->value; goto acf;}
	return 0;

acf:	for(DynListEntry<GameObjAndListEntry> *e = x->associates.first; e; e = e->next)
		if(e->value.o == b)
			return 1;
	return 0;
}


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

void AssociateObjToObj(GameObject *a, uint cat, GameObject *b)
{
	if(AreObjsAssociated(a, cat, b)) return;

	GOAssociation *x, *y;

	for(DynListEntry<GOAssociation> *e = a->association.first; e; e = e->next)
		if(e->value.category == cat)
			{x = &e->value; goto acf;}
	a->association.add();
	a->association.last->value.category = cat;
	x = &a->association.last->value;

acf:	for(DynListEntry<GOAssociation> *e = b->association.first; e; e = e->next)
		if(e->value.category == cat)
			{y = &e->value; goto bcf;}
	b->association.add();
	b->association.last->value.category = cat;
	y = &b->association.last->value;

bcf:	x->associates.add();
	y->associators.add();
	x->associates.last->value.o = b;
	y->associators.last->value.o = a;
	x->associates.last->value.l = &y->associators;
	y->associators.last->value.l = &x->associates;
	x->associates.last->value.e = y->associators.last;
	y->associators.last->value.e = x->associates.last;
}

void DisassociateObjToObj(GameObject *a, uint cat, GameObject *b)
{
	GOAssociation *x;
	for(DynListEntry<GOAssociation> *e = a->association.first; e; e = e->next)
		if(e->value.category == cat)
			{x = &e->value; goto acf;}
	return;

acf:	for(DynListEntry<GameObjAndListEntry> *e = x->associates.first; e; e = e->next)
	{
		if(e->value.o == b)
		{
			e->value.l->remove(e->value.e);
			x->associates.remove(e);
			return;
		}
	}
}

int IsInAlias(GameObject *a, uint c)
{
	for(DynListEntry<goref> *e = alias[c].first; e; e = e->next)
		if(e->value.valid())
			if(e->value.get() == a)
				return 1;
	return 0;
}

void ClearAlias(uint t)
{
	alias[t].clear();
}

void AliasObj(GameObject *a, uint c)
{
	if(!IsInAlias(a, c))
		{alias[c].add(); alias[c].last->value = a;}
}

void UnaliasObj(GameObject *a, uint c)
{
	for(DynListEntry<goref> *e = alias[c].first; e; e = e->next)
		if(e->value.valid())
			if(e->value.get() == a)
				{alias[c].remove(e); return;}
}

GameObject *CreateObject(CObjectDefinition *def, GameObject *parent, int id)
{
	GameObject *go;

	if(parent)
	{
		parent->children.add();
		go = &parent->children.last->value;
		go->palste = parent->children.last;
		go->parent = parent;
	}
	else
	{
		go = new GameObject;
		go->palste = 0;
		go->parent = 0;
	}

	while(gameobj[nexthandle]) nexthandle = (nexthandle+1) & (MAX_GAMEOBJECTS-1);
	go->handle = nexthandle;
	nexthandle = (nexthandle+1) & (MAX_GAMEOBJECTS-1);
	gameobj[go->handle] = go;

	if(id != -1)	go->id = id;
	else		{while(FindObjID(nextgoid)) nextgoid++; go->id = nextgoid++;}
	go->objdef = def;
	go->position = go->orientation = Vector3(0,0,0); go->scale = go->objdef->scale;
	go->appearance = 0;
	SetRandomSubtypeAndAppearance(go);
	if(parent) go->color = parent->color; else go->color = 0;
	go->renderable = def->renderable;
	go->flags = FGO_SELECTABLE | FGO_TARGETABLE | FGO_RENDERABLE; go->rects = 0;
	go->disableCount = 0;
	if(go->objdef->type == CLASS_PLAYER)	go->player = go;
	else if(go->parent)			go->player = go->parent->player;
	else					go->player = 0;
	//go->name = (new wchar_t[] = L"(No name)");
	if((def->type == CLASS_LEVEL) || (def->type == CLASS_PLAYER))
		{go->name = new wchar_t[10]; wcscpy(go->name, L"(No name)");}
	else	go->name = 0;
	//go->paramOrder = 0; go->paramObj.deref(); go->paramDest.x = go->paramFace.x = -1;
	go->param = 0;
	go->tiles = 0;
	go->ordercfg.uniqueOrderID = 0;

	go->itile = 0;
	if(itiles)
		PutObjsInITiles(go);

	norefobjs.add(go);
	go->curtarget = 0; go->ctgdle = norefobjs.last;

	go->client = 0;
	go->aicontroller = 0;

	go->animtag = 0; go->animvar = 0;
	go->animtimeref = current_time * 1000;
	go->animlooping = 1; go->animlooped = 0;

	return go;
}

GameObject *DuplicateObject(GameObject *a)
{
	GameObject *o = CreateObject(a->objdef, a->parent);
	o->position = a->position; o->orientation = a->orientation; o->scale = a->scale;
	GOPosChanged(o, 0, 0);
	o->subtype = a->subtype; o->appearance = a->appearance;
	for(DynListEntry<GOItem> *e = a->item.first; e; e = e->next)
		{o->item.add(); o->item.last->value = e->value;}
	o->color = a->color;
	return o;
}

void ConvertObject(GameObject *o, CObjectDefinition *d)
{
	int os = o->objdef->subtypes[o->subtype].id;
	o->objdef = d;
	//o->appearance = 0;
	SetRandomSubtypeAndAppearance(o);

	// If the new objtype has a similar subtype, then keep it.
	for(int i = 0; i < d->numsubtypes; i++)
		if(d->subtypes[i].id == os)
			{o->subtype = i; break;}

	SetObjectAnimationIfNotPlayed(o, 0, 1);
}

void AddReaction(GameObject *o, int r)
{
	for(DynListEntry<uint> *e = o->iReaction.first; e; e = e->next)
		if(e->value == r)
			return;
	o->iReaction.add(r);
}

void RemoveReaction(GameObject *o, int r)
{
	for(DynListEntry<uint> *e = o->iReaction.first; e; e = e->next)
		if(e->value == r)
			{o->iReaction.remove(e); return;}
}

Model *GetObjectModel(GameObject *o)
{
	//return o->objdef->subtypes[o->subtype].appear[o->appearance];
	ODAppearance *ap = &(o->objdef->subtypes[o->subtype].appear[o->appearance]);
	if(ap->anim) if(ap->anim[o->animtag])
	{
		ODAnimation *at = ap->anim[o->animtag];
		if(at->numvar == 0)
		{
			if(at->mesh)
				return at->mesh;
		}
		else
		{
			if(at->var[o->animvar])
				return at->var[o->animvar];
		}
	}
	return ap->def;
}

void SetObjectAnimation(GameObject *o, int animtag, boolean loop)
{
	o->animtag = animtag;
	o->animvar = 0;
	o->animtimeref = current_time * 1000;
	o->animlooping = loop; o->animlooped = 0;
}

void SetObjectAnimationIfNotPlayed(GameObject *o, int animtag, boolean loop)
{
	o->animlooping = loop;
	if(o->animtag != animtag)
		SetObjectAnimation(o, animtag, loop);
}

void PlayMovementAnimation(GameObject *o)
{
/*
	if(o->objdef->movBands.len)
	{
		SetObjectAnimationIfNotPlayed(o,
			GetBestPlayAnimationTag(o->objdef->movBands[0]->playAnim, o),
			1);
	}
*/
	if(!o->objdef->movBands.len) return;
	if(o->objdef->movBands.len == 1)
	{
		SetObjectAnimationIfNotPlayed(o,
			GetBestPlayAnimationTag(o->objdef->movBands[0]->playAnim, o),
			1);
		return;
	}

	// Get movement speed
	float ms = 1;
	if(o->objdef->movSpeedEq != -1)
	{
		SequenceEnv env; env.self = o;
		ms = equation[o->objdef->movSpeedEq]->get(&env);
	}

	// Find best matching movement band
	int i;
	for(i = o->objdef->movBands.len-1; i >= 0; i--)
	  if(ms >= o->objdef->movBands[i]->naturalMovSpeed)
	{
		int at = GetBestPlayAnimationTag(o->objdef->movBands[i]->playAnim, o);
		if(DoesObjectHaveUniqueAnimation(o, at))
		{
			SetObjectAnimationIfNotPlayed(o, at, 1);
			return;
		}
	}
	SetObjectAnimationIfNotPlayed(o, 0, 1);
}

boolean DoesObjectHaveUniqueAnimation(GameObject *o, int rat)
{
	ODAppearance *ap = &(o->objdef->subtypes[o->subtype].appear[o->appearance]);
	if(ap->anim) if(ap->anim[rat])
	{
		ODAnimation *at = ap->anim[rat];
		if(at->numvar || at->mesh)
			return 1;
	}
	return 0;
}
