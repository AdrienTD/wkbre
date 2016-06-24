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

struct GameObject;
struct goref
{
private:
	uint handle, id;
public:
	// The following functions are defined in savegame.h
	goref();
	int valid();
	GameObject *get();
	void deref();
	void set(GameObject *a);
	goref(GameObject *a) {handle = -1; set(a);}

	//operator GameObject*() {return get();}
	GameObject *operator->() {return get();}
	goref &operator=(GameObject *a) {set(a); return *this;}
	goref &operator=(uint a); //{set(FindObjID(a)); return *this;}
	boolean operator==(GameObject *a) {return (get() == a);}
	uint getID() {return id;}
};

#include "objdef.h"
#include "sg_order.h"
#include "savegame.h"
#include "sg_diplostat.h"
#include "../script/script.h"
#include "gs_order.h"
#include "sg_clientstate.h"
#include "gs_command.h"
#include "itile.h"

// From loadgameset.cpp

extern GrowStringList strItems;
extern GrowStringList strDefValue; extern GrowList<valuetype> defvalue;
extern GrowStringList strAppearTag;
extern GrowStringList strDiplomaticStatus;
extern GrowStringList strObjDef; extern GrowList<int> typeObjDef; extern GrowList<GrowStringList*> pstObjDef;
extern GrowStringList strAssociateCat, strAlias;
extern GrowStringList strOrder, strTask; extern COrder *gsorder; extern CTask *gstask;
extern GrowStringList strActionSeq; extern ActionSeq **actionseq;
extern CObjectDefinition *objdef;
extern DynList<goref> *alias;
extern GrowStringList strFinderDef; extern CFinder **finderdef;
extern GrowStringList strEquation; extern CValue **equation;
extern GrowStringList strTypeTag;
extern GrowStringList strObjCreation; extern CObjectCreation *cobjcr;
extern GrowStringList strGameEvent;
extern GrowStringList strReaction; extern CReaction *reaction;
extern GrowStringList strPRTrigger; extern CPRTrigger *prtrigger;
extern GrowStringList strValueTag; extern CValue **defValueTag;
extern GrowStringList strOrderAssign; extern COrderAssignment *orderAssign;
extern GrowStringList strCommand; extern CCommand *gscommand;
extern GrowStringList strOrderCat, strTaskCat;

void LoadGameSet(char *filename);
char *SkipClass(char *fp, char *end);