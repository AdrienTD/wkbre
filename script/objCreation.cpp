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

void ReadCObjectCreation(char **pntfp, char **fstline)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	CObjectCreation *oc = &(cobjcr[strObjCreation.find(fstline[1])]);
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if( *((uint*)(word[0])) == '_DNE' )
			return;
		char **wpnt = word+1;
		switch(stfind_cs(OBJCREATE_str, OBJCREATE_NUM, word[0]))
		{
			case OBJCREATE_TYPE_TO_CREATE:
				{oc->type = 1;
				int t = FindObjDef(stfind_cs(CLASS_str, CLASS_NUM, word[1]), word[2]);
				mustbefound(t);
				oc->objdef = &objdef[t];
				break;}
			case OBJCREATE_MAPPED_TYPE_TO_CREATE:
				oc->type = 2;
				oc->mappedType = strTypeTag.find(word[1]);
				wpnt += 1;
				oc->mtObjRef = ReadFinder(&wpnt); break;
			case OBJCREATE_MATCH_APPEARANCE_OF:
				oc->matchAppearOf = ReadFinder(&wpnt); break;
			case OBJCREATE_CONTROLLER:
				oc->controller = ReadFinder(&wpnt); break;
			case OBJCREATE_CREATE_AT:
				oc->createAt = ReadCPosition(&wpnt); break;
			case OBJCREATE_POST_CREATION_SEQUENCE:
				oc->postCreationSeq = ReadActSeq(pntfp); break;
		}
	}
	ferr("UEOF"); return;
}

void CObjectCreation::run(SequenceEnv *env, CFinder *findcreator)
{
	SequenceEnv ne; goref z;
	env->copyAll(&ne);
	GameObject *creator;
	findcreator->begin(env);
	while(creator = findcreator->getnext())
	{
		ne.self = creator;
		ne.creator = creator;
		GameObject *ctrl = controller->getfirst(&ne);
		GameObject *o;
		if(type == 1)
			o = CreateObject(objdef, ctrl);
		else {
			CObjectDefinition *nod = mtObjRef->getfirst(&ne)->objdef->mappedType[mappedType];
			if(!nod) ferr("An OBJECT_CREATION found an object which doesn't support the specified type tag.");
			o = CreateObject(nod, ctrl);
		}
		
		if(createAt)
		{
			PosOri po;
			createAt->get(&ne, &po);
			o->position = po.pos; o->orientation = po.ori;
		}else{
			o->position = creator->position;
			o->orientation = creator->orientation;
		}
		GOPosChanged(o);

		if(matchAppearOf)
		{
			GameObject *mr = matchAppearOf->getfirst(&ne);
			if(mr->subtype >= o->objdef->numsubtypes)
				o->subtype = 0;
			else
				o->subtype = mr->subtype;
			if(o->objdef->subtypes[o->subtype].appear[mr->appearance])
				o->appearance = mr->appearance;
		}

		if(postCreationSeq)
		{
			ne.self = o;
			postCreationSeq->run(&ne);
		}
	}
}