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

#include "global.h"
#include <time.h>
#include <process.h>
#include <mbstring.h>
#include "imgui/imgui.h"

HINSTANCE hInstance;
GEContainer *actualpage = 0;
int appexit = 0;
float walkstep = 1.0f;
char *farg = 0;
uchar secret[] = {0xec,0xc9,0xdf,0xc4,0xc8,0xc3,0x8d,0xea,0xc8,0xc8,0xd9,0xde};
int bo = 0;
int playMode = 0, enableObjTooltips = 0;
ClientState *curclient = 0;
boolean objmovalign = 0;
Cursor *defcursor;
boolean mouseRot = 0; int msrotx, msroty;
boolean multiSel = 0; int mselx, msely;
GrowList<GameObject*> msellist;
bool swSelObj = 0, swLevTree = 0, swLevInfo = 0, swObjCrea = 0, swAbout = 0;

#ifdef WKBRE_RELEASE
int experimentalKeys = 0;
boolean showTimeObjInfo = 0;
#else
int experimentalKeys = 1;
boolean showTimeObjInfo = 1;
#endif

char *playercolorname[8] = {"Brown", "Blue", "Yellow", "Red", "Green", "Pink", "Orange", "Aqua"};

char *tooltip_str = "Hello!\nMy name is Mr. Tooltip!\nBye!"; int tooltip_x = 20, tooltip_y = 20;

// Variables needed for GUI:
GUIElement *getobringfront = 0; GESubmenu *actsubmenu = 0;
GUIElement *movingguielement = 0; int movge_rx, movge_ry;

// Menu
char *menubarstr[] = {"File", "Object", "Game", "View", "Window", "Help"};
MenuEntry menucmds[] = {
{"Save as...", CMD_SAVE},
{"Change WK version", CMD_CHANGE_SG_WKVER},
{"Quit", CMD_QUIT},
{0,0},
{"Create object...", CMD_CREATEOBJ},
{"Duplicate selected objects", CMD_DUPLICATESELOBJECT},
{"Convert type...", CMD_CONVERTOBJTYPE},
{"Give selected objects to...", CMD_GIVESELOBJECT},
{"Give type...", CMD_GIVEOBJTYPE},
{"Delete selected objects", CMD_DELETESELOBJECT},
{"Delete last created object", CMD_DELETELASTCREATEDOBJ},
{"Delete type...", CMD_DELETEOBJTYPE},
{"Delete class...", CMD_DELETEOBJCLASS},
{"Scale selected objects by 1.5", CMD_SELOBJSCALEBIGGER},
{"Scale selected objects by 1/1.5", CMD_SELOBJSCALESMALLER},
{"Rotate selected objects by 90°", CMD_ROTATEOBJQP},
{"Randomize subtypes", CMD_RANDOMSUBTYPE},
{"Reset objects' heights", CMD_RESETOBJPOS},
{"Rename player object...", CMD_CHANGE_PLAYER_NAME},
{"Change player color...", CMD_CHANGE_PLAYER_COLOR},
{"Select object by ID...", CMD_SELECT_OBJECT_ID},
{"Enable/disable aligned movement", CMD_CHANGE_OBJMOVALIGN},
{0,0},
{"Pause/Resume", CMD_PAUSE},
{"Increase game speed", CMD_GAME_SPEED_FASTER},
{"Decrease game speed", CMD_GAME_SPEED_SLOWER},
{"Start level", CMD_START_LEVEL},
{"Control client...", CMD_CONTROL_CLIENT},
{"Execute command...", CMD_EXECUTE_COMMAND},
{"Execute command with target...", CMD_EXECUTE_COMMAND_WITH_TARGET},
{"Execute action sequence...", CMD_RUNACTSEQ},
{"Send event...", CMD_SEND_EVENT},
{"Cancel all objects' orders", CMD_CANCEL_ALL_OBJS_ORDERS},
{"Remove battles delayed sequences", CMD_REMOVE_BATTLES_DELAYED_SEQS},
{"Quick stampdown...", CMD_STAMPDOWN_OBJECT},
{"Open game text window...", CMD_ENABLE_GTW},
{"Enable/disable gameplay shortcuts", CMD_TOGGLEEXPERIMENTALKEYS},
{0,0},
{"Move to...", CMD_CAMPOS},
{"Move to down-left corner", CMD_CAMDOWNLEFT},
{"Move to down-right corner", CMD_CAMDOWNRIGHT},
{"Move to up-left corner", CMD_CAMUPLEFT},
{"Move to up-right corner", CMD_CAMUPRIGHT},
{"Move to player's manor...", CMD_CAMMANOR},
{"Move to client's position...", CMD_CAMCLISTATE},
{"Reset orientation", CMD_CAMRESETORI},
{"Show/hide terrain", CMD_SHOWHIDELANDSCAPE},
{"Show/hide representations", CMD_TOGGLEREPRENSATIONS},
{"Show/hide object tooltips", CMD_TOGGLEOBJTOOLTIPS},
{"Show/hide time & object information", CMD_TOGGLE_TIMEOBJINFO},
{"Show/hide grid", CMD_TOGGLEGRID},
//{"BCM himap bit left", CMD_BCMHIMAPLEFT},
//{"BCM himap bit right", CMD_BCMHIMAPRIGHT},
{0,0},
{"Open all", CMD_SWOPENALL},
{"Close all", CMD_SWCLOSEALL},
{"Selected object information", CMD_SWSELOBJ},
{"Level information", CMD_SWLEVINFO},
{"Level tree", CMD_SWLEVTREE},
{"Object creation", CMD_SWOBJCREA},
{0,0},
{"About...", CMD_ABOUT},
{0,0},
};

int orderButtX[6] = {	6,	37,	69,	93, 	100,	90	};
int orderButtY[6] = {	364,	354,	361,	384,	416,	448	};

CObjectDefinition *objtypeToStampdown = 0;
bool eventAfterStampdown = 0;
goref playerToGiveStampdownObj;

void DrawTooltip()
{
	if(!tooltip_str) return;
	int x = tooltip_x, y = tooltip_y, w, h;
	if(currentCursor)
		{x += currentCursor->w; y += currentCursor->h;}
	GetTextSize(tooltip_str, &w, &h);
	if((x + w) > scrw) x = scrw-w;
	if((y + h) > scrh) y = scrh-h;
	if(x < 0) x = 0;
	if(y < 0) y = 0;
	NoTexture(0);
	DrawRect(x, y, w, h, 0xC0000080);
	DrawFont(x, y, tooltip_str);
}

void DisplaceCurCamera(Vector3 r)
{
	if(curclient)
		curclient->camerapos += r;
	else
		camerapos += r;
}

void RotateCurCamera(float x, float y)
{
	if(curclient)
		curclient->cameraori += Vector3(x, y, 0);
	else
		{campitch += x; camyaw += y;}
}

void GetCurCameraRot(float *x, float *y)
{
	if(curclient)
		{*x = curclient->cameraori.x; *y = curclient->cameraori.y;}
	else
		{*x = campitch; *y = camyaw;}
}

void SetCurCameraRot(float x, float y)
{
	if(curclient)
		{curclient->cameraori.x = x; curclient->cameraori.y = y;}
	else
		{campitch = x; camyaw = y;}
}

void SetCurCameraPos(Vector3 v)
{
	if(curclient)
		curclient->camerapos = v;
	else
		camerapos = v;
}

void SetCurCameraPosXZ(float x, float z)
{
	if(curclient)
		{curclient->camerapos.x = x; curclient->camerapos.z = z;}
	else
		{camerapos.x = x; camerapos.z = z;}
}

void SetCurCameraPosXYZ(float x, float y, float z)
{
	if(curclient)
		curclient->camerapos = Vector3(x, y, z);
	else
		camerapos = Vector3(x, y, z);
}

void PresentObject(int igo)
{
	GameObject *go = FindObjID(igo);
	printf("I'd like to present you object number %i!\n", igo);
	printf(" - Class: %s\n", CLASS_str[go->objdef->type]);
	printf(" - Colour index: %i\n", go->color);
	printf(" - Position: (%f, %f, %f)\n", go->position.x, go->position.y, go->position.z);
	printf(" - Orientation: (%f, %f, %f)\n", go->orientation.x, go->orientation.y, go->orientation.z);
	printf(" - Subtype: %i\n", go->subtype);
	printf(" - Appearance: %s\n", strAppearTag.getdp(go->appearance));
	printf(" - Renderable?: %s\n", go->renderable?"Yes":"No");
}

void RemoveObjOfType(GameObject *o, CObjectDefinition *d)
{
	DynListEntry<GameObject> *nx;
	for(DynListEntry<GameObject> *e = o->children.first; e; e = nx)
	{
		nx = e->next;
		RemoveObjOfType(&e->value, d);
	}
	if(o->objdef == d)
		RemoveObject(o);
}

void ReplaceObjsType(GameObject *o, CObjectDefinition *a, CObjectDefinition *b)
{
	for(DynListEntry<GameObject> *e = o->children.first; e; e = e->next)
		ReplaceObjsType(&e->value, a, b);
	if(o->objdef == a)
	{
		ConvertObject(o, b);
/*
		o->objdef = b;
		o->item.clear(); //memcpy(o->item, b->startItems, strItems.len * sizeof(valuetype));
		o->appearance = 0;
		o->subtype = (b->numsubtypes>1)?((rand()%(b->numsubtypes-1)) + 1):0;
		o->renderable = b->renderable;
*/
	}
}

void SetObjsCtrl(GameObject *o, CObjectDefinition *d, GameObject *p)
{
	if(o == p) //levelobj->children.getEntry(1)->value
		return;
	DynListEntry<GameObject> *nx;
	for(DynListEntry<GameObject> *e = o->children.first; e; e = nx)
	{
		nx = e->next;
		SetObjsCtrl(&e->value, d, p);
	}
	if(o->objdef == d)
		SetObjectParent(o, p);
}

CObjectDefinition *AskObjDef(char *head)
{
	GrowStringList l; char ts[256];
	for(int i = 0; i < strObjDef.len; i++)
	{
		if((objdef[i].type == 0) || (objdef[i].name == 0))
			{l.add("?"); continue;}
		strcpy(ts, CLASS_str[objdef[i].type]);
		strcat(ts, " ");
		strcat(ts, objdef[i].name);
		l.add(ts);
	}
	int r = ListDlgBox(&l, head?head:"Select an object definition.");
	if(r == -1) return 0;
	else return &objdef[r];
}

GameObject *AskPlayer(char *head = 0)
{
	GrowStringList l; char ts[512]; int x = 0;
	for(DynListEntry<GameObject> *e = levelobj->children.first; e; e = e->next)
	{
		if(e->value.objdef->type != CLASS_PLAYER) break; // TODO: Something better.
		sprintf(ts, "%u: %S (obj. id %u)", x++, e->value.name ? e->value.name : L"(null)", e->value.id);
		l.add(ts);
	}
	int r = ListDlgBox(&l, head?head:"Select a player.", (l.len >= 2) ? 1 : 0);
	if(r == -1) return 0;
	else return &levelobj->children.getEntry(r)->value;
}

void RandomizeObjAppear(GameObject *o)
{
	for(DynListEntry<GameObject> *e = o->children.first; e; e = e->next)
		RandomizeObjAppear(&e->value);
	SetRandomSubtypeAndAppearance(o);
}

void ResetPosition(GameObject *o)
{
	for(DynListEntry<GameObject> *e = o->children.first; e; e = e->next)
		ResetPosition(&e->value);
	if((o->objdef->type == CLASS_BUILDING) || (o->objdef->type == CLASS_CHARACTER) ||
	   (o->objdef->type == CLASS_CONTAINER) || (o->objdef->type == CLASS_PROP))
		o->position.y = GetHeight(o->position.x, o->position.z);
}

void RemoveObjOfClass(GameObject *o, int c)
{
	DynListEntry<GameObject> *nx;
	for(DynListEntry<GameObject> *e = o->children.first; e; e = nx)
	{
		nx = e->next;
		RemoveObjOfClass(&e->value, c);
	}
	if(o->objdef->type == c)
		RemoveObject(o);
}

void SendEventToObjAndSubord(GameObject *o, int v)
{
	SendGameEvent(0, o, v);
	DynListEntry<GameObject> *n;
	for(DynListEntry<GameObject> *e = o->children.first; e; e = n)
	{
		n = e->next;
		SendEventToObjAndSubord(&e->value, v);
	}
}

int AskClass(char *head = 0)
{
	GrowStringList sl;
	for(int i = 0; i < OBJTYPE_NUM; i++)
		sl.add(OBJTYPE_str[i]);
	int r = ListDlgBox(&sl, head?head:"Select an object class.");
	if(r == -1) return -1;
	return stfind_cs(CLASS_str, CLASS_NUM, OBJTYPE_str[r]);
}

CCommand *AskCommand(char *head = 0, DynList<goref> *dl = &selobjects)
{
	GrowList<CCommand *> lc;
	GrowStringList ls;
	for(DynListEntry<goref> *e = dl->first; e; e = e->next)
	{
		if(!e->value.valid()) continue;
		for(int i = 0; i < e->value->objdef->offeredCmds.len; i++)
			if(!lc.has(e->value->objdef->offeredCmds[i]))
				lc.add(e->value->objdef->offeredCmds[i]);
	}
	for(int i = 0; i < lc.len; i++)
		ls.add(lc[i]->name);
	int r = ListDlgBox(&ls, head?head:"Select a command.");
	if(r == -1) return 0;
	return lc[r];
}

DynList<goref> createdObjects;
DynList<goref> selobjects;
int menuVisible = 1;
char *statustext = 0; //"Status bar";
char statustextbuf[1024];

void UpdateSelectionInfo()
{
	DynListEntry<goref> *n;
	for(DynListEntry<goref> *e = selobjects.first; e; e = n)
	{
		n = e->next;
		if(!e->value.valid())
			selobjects.remove(e);
	}

	if(!selobjects.len)
		{statustext = 0; return;}
	if(selobjects.len == 1)
	{
		_snprintf(statustextbuf, 128,
		   "Object ID %i, %s \"%s\"", selobjects.first->value->id,
		   CLASS_str[selobjects.first->value->objdef->type],
		   selobjects.first->value->objdef->name);
		statustext = statustextbuf;
	}
	else
	{
		_snprintf(statustextbuf, 128, "%u objects selected.", selobjects.len);
		/*strcpy(statustextbuf, "Objs");
		for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
		{
			char nms[64];
			sprintf(nms, " %u", e->value.getID());
			strcat_s(statustextbuf, 128, nms);
			statustextbuf[128] = 0;
		}*/
		statustext = statustextbuf;
	}
}

void SelectObject(GameObject *o)
{
	if(!o) return;
	if(o->flags & FGO_SELECTED) return;
	o->flags |= FGO_SELECTED;
	selobjects.add();
	selobjects.last->value = o;
	UpdateSelectionInfo();
}

void DeselectObject(GameObject *o)
{
	if(!o) return;
	if(!(o->flags & FGO_SELECTED)) return;
	o->flags &= ~FGO_SELECTED;
	DynListEntry<goref> *n;
	for(DynListEntry<goref> *e = selobjects.first; e; e = n)
	{
		n = e->next;
		if(e->value.get() == o)
			selobjects.remove(e);
	}			
	UpdateSelectionInfo();
}

void DeselectAll()
{
	DynListEntry<goref> *n;
	for(DynListEntry<goref> *e = selobjects.first; e; e = n)
	{
		n = e->next;
		if(e->value.valid())
		{
			e->value->flags &= ~FGO_SELECTED;
			selobjects.remove(e);
		}
	}
	UpdateSelectionInfo();
}

void CancelAllObjsOrders(GameObject *o)
{
	if(o->ordercfg.order.len)
		//CancelAllOrders(o);
		o->ordercfg.order.clear();
	RemoveObjReference(o);
	for(DynListEntry<GameObject> *e = o->children.first; e; e = e->next)
		CancelAllObjsOrders(&e->value);
}

void AddClientsToGSL(GrowStringList *sl)
{
	for(uint i = 0; i < clistates.len; i++)
	{
		ClientState *c = clistates.getpnt(i);
		if(!c->obj.valid())
			sl->add("<PLAYER object removed>");
		else {
			char nm[256]; char *so = nm;
			wchar_t *si = c->obj->name;
			while(*si) *(so++) = *(si++);
			*so = 0;
			sl->add(nm);
		}
	}
}

void CallCommand(int cmd)
{
	switch(cmd)
	{
		case CMD_SAVE:
		{
#ifndef WKBRE_RELEASE
			if(keypressed[VK_SHIFT])
			{
				char t[1024], e[1024] = "\"\0";
				strcpy(t, gamedir);
				strcat(t, "\\saved\\Save_Games\\wkbre_quick_look.sav");
				SaveSaveGame(t);
				strcat(e, t);
				strcat(e, "\"");
				char *x = "C:\\Users\\Adrien\\Downloads\\SciTE\\SciTE\\SciTE.exe";
				_spawnlp(_P_NOWAIT, x, x, e, NULL);
				break;
			}
#endif
			char s[256];
			if(StrDlgBox(s, "Type the name of the new save game. It will be placed in \"saved\\Save_Games\" in the game directory. The name must end with \".sav\"."))
			{
				char t[1024] = "Save_Games\\\0";
				strcat(t, s);
				if(FileExists(t))
					if(MessageBox(hWindow, "The file name already exists. Do you want to replace this file?", appName, 48 | MB_YESNO) != IDYES)
						break;
				strcpy(t, gamedir);
				strcat(t, "\\saved\\Save_Games\\");
				strcat(t, s);
				SaveSaveGame(t);
			}
		} break;
		case CMD_DELETEOBJTYPE:
		{
			CObjectDefinition *d = AskObjDef("Select the type of objects you want to delete.");
			if(d)
				RemoveObjOfType(levelobj, d); //&objdef[FindObjDef(CLASS_CHARACTER, "Peasant")]);
		} break;
		case CMD_CONVERTOBJTYPE:
		{
			CObjectDefinition *a, *b;
			if(a = AskObjDef("What type of object do you want to convert?")) if(b = AskObjDef("Which object type should they be converted to?"))
				ReplaceObjsType(levelobj, a, b);
		} break;
		case CMD_GIVEOBJTYPE:
		{
			CObjectDefinition *d; GameObject *p;
			if(d = AskObjDef("What type of objects do you want to give?")) if(p = AskPlayer("Give the objects to:"))
				SetObjsCtrl(levelobj, d, p);
		} break;
		case CMD_CREATEOBJ:
		{
			CObjectDefinition *d; GameObject *p; float px, pz;
			if(d = AskObjDef("What sort of object do you want to create?"))
			{
				if(d->type == CLASS_PLAYER)
				{
					GameObject *o = CreateObject(d, levelobj);
					createdObjects.add();
					createdObjects.last->value = o;
				}
				else
				{
					if(p = AskPlayer("Who do you want to give the created object to?"))
					if(PositionDlgBox(&px, &pz, "Where do you want to put the created object?"))
					{
						GameObject *o = CreateObject(d, p);
						o->position = Vector3(px, GetHeight(px, pz), pz);
						GOPosChanged(o);
						createdObjects.add();
						createdObjects.last->value = o;
					}
				}
			}
		} break;
		case CMD_DELETELASTCREATEDOBJ:
		{
			if(createdObjects.len)
			{
				if(createdObjects.last->value.valid())
					RemoveObject(createdObjects.last->value.get());
				createdObjects.remove(createdObjects.last);
			}
		} break;
		case CMD_RANDOMSUBTYPE:
			RandomizeObjAppear(levelobj); break;
		case CMD_CAMMANOR:
		{
			boolean fnd = 0;
			GameObject *a = AskPlayer("Set camera position to the manor of player:");
			if(a)
			{
				int c = strAssociateCat.find("My Initial Palace Building");
				if(c != -1)
				   for(DynListEntry<GOAssociation> *e = a->association.first; e; e = e->next)
					if(e->value.category == c)
					   if(e->value.associates.len)
						{SetCurCameraPos(e->value.associates.first->value.o->position); fnd = 1; break;}
				if(fnd) break;
				c = strItems.find("Palace Building");
				if(c != -1)
				   for(int i = 0; i < MAX_GAMEOBJECTS; i++)
					if(gameobj[i])
					   if(gameobj[i]->getItem(c) != 0.0f)
						if(gameobj[i]->player == a)
						   {SetCurCameraPos(gameobj[i]->position); fnd = 1; break;}
				if(!fnd) MessageBox(hWindow, "The player's manor was not found.", appName, 16);
			}
		} break;
		case CMD_CAMPOS:
		{
			float x, z;
			if(PositionDlgBox(&x, &z, "Where would you like to go?"))
				SetCurCameraPosXZ(x, z);
		} break;
		case CMD_ABOUT:
		{
			swAbout = !swAbout;
			//MessageBox(hWindow, "wkbre - WK (Battles) Recreated Engine\nVersion " WKBRE_VERSION "\n(C) 2015-2017 AdrienTD\nThis program is released under the GNU GPL Version 3 license. For more information see the \"LICENSE\" file.\nRead the \"docs/help.htm\" file for help and more information.", appName, 64);
		} break;
		case CMD_RESETOBJPOS:
			ResetPosition(levelobj); break;
		case CMD_CAMDOWNLEFT:
			SetCurCameraPosXYZ(0, 0, 0); break;
		case CMD_CAMDOWNRIGHT:
			SetCurCameraPosXYZ((mapwidth-mapedge*2)*5, 0, 0); break;
		case CMD_CAMUPLEFT:
			SetCurCameraPosXYZ(0, 0, (mapheight-mapedge*2)*5); break;
		case CMD_CAMUPRIGHT:
			SetCurCameraPosXYZ((mapwidth-mapedge*2)*5, 0, (mapheight-mapedge*2)*5); break;
		case CMD_BCMHIMAPRIGHT:
			LoadMap(lastmap, ++bo); break;
		case CMD_BCMHIMAPLEFT:
			LoadMap(lastmap, --bo); break;
		case CMD_SHOWHIDELANDSCAPE:
			enableMap = !enableMap; break;
		case CMD_QUIT:
			exit(0); break;

		case CMD_DELETESELOBJECT:
			for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
			if(e->value.valid())
			{
				GameObject *o = e->value.get();
				//DeselectObject(o);
				RemoveObject(o);
			}
			break;
		case CMD_DUPLICATESELOBJECT:
			for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
			if(e->value.valid())
				DuplicateObject(e->value.get());
			break;
		case CMD_DELETEOBJCLASS:
		{
			int c = AskClass("Delete all objects of class:");
			if(c) RemoveObjOfClass(levelobj, c);
		} break;
		case CMD_CAMRESETORI:
			SetCurCameraRot(0.0f, 0.0f); break;
		case CMD_SELOBJSCALEBIGGER:
			for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
			if(e->value.valid())
				e->value->scale *= 1.5;
			break;
		case CMD_SELOBJSCALESMALLER:
			for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
			if(e->value.valid())
				e->value->scale /= 1.5;
			break;
		case CMD_GIVESELOBJECT:
		{
			GameObject *p;
			if(p = AskPlayer("Give selected object to:"))
				for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
				if(e->value.valid())
					SetObjectParent(e->value.get(), p);
		} break;
		case CMD_RUNACTSEQ:
		{
			GrowStringList sas;
			int x = ListDlgBox(&strActionSeq, "Which action sequence do you want to execute?");
			if(x != -1)
			{
				SequenceEnv s;
				for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
				if(e->value.valid())
					{s.self = e->value; break;}
				actionseq[x]->run(&s);
			}
		} break;
		case CMD_ROTATEOBJQP:
			for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
			if(e->value.valid())
			{
				e->value->orientation.y += M_PI / 2;
				if(e->value->orientation.y >= 2*M_PI)
					e->value->orientation.y -= M_PI * 2;
			}
			break;
		case CMD_TOGGLEEXPERIMENTALKEYS:
			experimentalKeys = !experimentalKeys; break;
		case CMD_TOGGLEREPRENSATIONS:
			showrepresentations = !showrepresentations; break;
		case CMD_CAMCLISTATE:
		{
			GrowStringList sl;
			AddClientsToGSL(&sl);
			int r = ListDlgBox(&sl, "Copy camera position and rotation from whose client?", 0);
			if(r != -1)
			{
				ClientState *c = clistates.getpnt(r);
				SetCurCameraPos(c->camerapos);
				SetCurCameraRot(c->cameraori.x, c->cameraori.y);
			}
		} break;
		case CMD_PAUSE:
			playMode = !playMode;
			if(playMode) GetElapsedTime();
			break;
		case CMD_TOGGLEOBJTOOLTIPS:
			enableObjTooltips = !enableObjTooltips; break;
		case CMD_GAME_SPEED_FASTER:
			game_speed *= 2.0f; break;
		case CMD_GAME_SPEED_SLOWER:
			game_speed /= 2.0f; break;
		case CMD_CANCEL_ALL_OBJS_ORDERS:
			CancelAllObjsOrders(levelobj); break;
		case CMD_SELECT_OBJECT_ID:
		{
			char s[256];
			if(StrDlgBox(s, "Enter the ID of the object you'd like to select."))
			{
				GameObject *o = FindObjID(atoi(s));
				if(!o) MessageBox(hWindow, "Object with specified ID not found.", appName, 16);
				else {DeselectAll(); SelectObject(o);}
			}
		} break;
		case CMD_CHANGE_SG_WKVER:
		{
			GrowStringList sl;
			sl.add("Warrior Kings"); sl.add("Warrior Kings - Battles");
			int r = ListDlgBox(&sl, "Make next saves compatible with:", (wkver==WKVER_BATTLES)?1:0);
			if(r != -1)
				wkver = r ? WKVER_BATTLES : WKVER_ORIGINAL;
		} break;
		case CMD_CONTROL_CLIENT:
		{
			GrowStringList sl;
			sl.add("<No client>");
			AddClientsToGSL(&sl);
			int r = ListDlgBox(&sl, "Which client to you want to take control?", 1);
			if(!r)
				curclient = 0;
			else if(r != -1)
				curclient = clistates.getpnt(r-1);
		} break;
		case CMD_CHANGE_OBJMOVALIGN:
			objmovalign = !objmovalign; break;
		case CMD_CHANGE_PLAYER_NAME:
			{GameObject *o = AskPlayer("Choose the player you would like to rename.");
			if(!o) break;
			char s[384];
			if(!StrDlgBox(s, "Enter the new name:")) break;
			int l = strlen(s);
			if(o->name) delete [] o->name;
			o->name = new wchar_t[l+1];
			for(int i = 0; i < l; i++)
				o->name[i] = s[i];
			o->name[l] = 0;
			break;}
		case CMD_CHANGE_PLAYER_COLOR:
			{GameObject *o = AskPlayer("Change color of player:");
			if(!o) break;
			GrowStringList sl;
			for(int i = 0; i < 8; i++)
				sl.add(playercolorname[i]);
			int x = ListDlgBox(&sl, "Change to this color:");
			if(x == -1) break;
			o->color = x;
			UpdateParentDependantValues(o);
			break;}
		case CMD_EXECUTE_COMMAND:
			{CCommand *c = AskCommand("Which command do you want to execute on the selected objects (with no target)?");
			for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
				if(e->value.valid())
					ExecuteCommand(e->value.get(), c, 0, ORDERASSIGNMODE_FORGET_EVERYTHING_ELSE);
			break;}
		case CMD_EXECUTE_COMMAND_WITH_TARGET:
			if(selobjects.len >= 2)
			 if(selobjects.last->value.valid())
			{
				DynList<goref> dl; dl.add(); dl.first->value = selobjects.first->value;
				CCommand *c = AskCommand("Which command do you want to execute on the first selected objects with the last selected object as the target?", &dl);
				DynListEntry<goref> *e = selobjects.first;
				for(int i = 0; i < selobjects.len-1; i++)
				{
					if(e->value.valid())
						ExecuteCommand(e->value.get(), c, selobjects.last->value.get(), ORDERASSIGNMODE_FORGET_EVERYTHING_ELSE);
					e = e->next;
				}
			}
			break;
		case CMD_CREATE_MAPPED_TYPE_OBJECT:
			{int t = ListDlgBox(&strTypeTag, "Select a type tag.");
			if(t != -1)
			{
				for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
				if(e->value.valid())
				if(e->value->objdef->mappedType[t])
				{
					GameObject *o = CreateObject(e->value->objdef->mappedType[t], e->value->parent);
					o->position = e->value->position;
					GOPosChanged(o);
				}
			}
			break;}
		case CMD_STAMPDOWN_OBJECT:
			{GameObject *p = 0;
			for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
				if(e->value.valid())
					{p = e->value.get(); break;}
			if(p)
			{
				CObjectDefinition *od;
				if(od = AskObjDef("What do you want to stampdown (create + send the 'On Stampdown' event)?"))
				{
					GameObject *o = CreateObject(od, p->player ? p->player : levelobj);
					o->position = p->position;
					GOPosChanged(o);
					SequenceEnv ctx;
					SendGameEvent(&ctx, o, PDEVENT_ON_STAMPDOWN);
				}
			}
			break;}
		case CMD_START_LEVEL:
			SendEventToObjAndSubord(levelobj, PDEVENT_ON_LEVEL_START);
			break;
		case CMD_REMOVE_BATTLES_DELAYED_SEQS:
			for(DynListEntry<DelayedSequenceEntry> *e = delayedSeq.first; e; e = e->next)
				delete [] e->value.obj;
			delayedSeq.clear();
			for(DynListEntry<SequenceOverPeriodEntry> *e = exePeriodSeq.first; e; e = e->next)
				delete [] e->value.ola;
			exePeriodSeq.clear();
			for(DynListEntry<SequenceOverPeriodEntry> *e = repPeriodSeq.first; e; e = e->next)
				delete [] e->value.ola;
			repPeriodSeq.clear();
			break;
		case CMD_ENABLE_GTW:
			{int x = ListDlgBox(&strGameTextWindow, "Display this game text window:");
			if(x != -1)
				EnableGTW(&(gsgametextwin[x]));
			break;}
		case CMD_SEND_EVENT:
			{int x = ListDlgBox(&strGameEvent, "Which event do you want to send to the selected objects?");
			if(x != -1)
				for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
					if(e->value.valid())
						SendGameEvent(0, e->value.get(), x);
			break;}
		case CMD_TOGGLE_TIMEOBJINFO:
			showTimeObjInfo = !showTimeObjInfo; break;
		case CMD_SWSELOBJ:
			swSelObj = !swSelObj; break;
		case CMD_SWLEVINFO:
			swLevInfo = !swLevInfo; break;
		case CMD_SWLEVTREE:
			swLevTree = !swLevTree; break;
		case CMD_SWOBJCREA:
			swObjCrea = !swObjCrea; break;
		case CMD_SWOPENALL:
			swSelObj = swLevInfo = swLevTree = swObjCrea = 1; break;
		case CMD_SWCLOSEALL:
			swSelObj = swLevInfo = swLevTree = swObjCrea = 0; break;
		case CMD_TOGGLEGRID:
			showMapGrid = !showMapGrid; break;
	}
}

CCommand *targetcmd = 0; int findertargetcommand = 0;

void SetTargetCursor()
{
	targetcmd = 0;
	if(!experimentalKeys) {ChangeCursor(defcursor); return;}
	//if(!currentSelection.valid()) {ChangeCursor(defcursor); return;}
	GameObject *p = currentSelection.get();

	GrowList<CCommand*> l;
	for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
		if(e->value.valid())
			for(int i = 0; i < e->value->objdef->offeredCmds.len; i++)
				if(!l.has(e->value->objdef->offeredCmds[i]))
					l.add(e->value->objdef->offeredCmds[i]);

	// Tell FINDER_TARGET to return the command target.
	findertargetcommand = 1;

	for(int i = 0; i < l.len; i++)
	{
		CCommand *c = l[i];
		for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
			if(e->value.valid())
				if(CanExecuteCommand(e->value->objdef, c))
				{
					SequenceEnv env;
					env.self = e->value.get();
					env.target = p;
					boolean x = 1;
					for(int j = 0; j < c->cursorAvailableConds.len; j++)
						if(stpo(c->cursorAvailableConds[j]->test->get(&env)))
						{
							// FOUND!
							if(c->cursorAvailableCurs[j])
								ChangeCursor(c->cursorAvailableCurs[j]);
							//statustext = c->name;
							targetcmd = c;
							findertargetcommand = 0; return;
						}
					if(!c->cursor) continue;
					for(int j = 0; j < c->cursorConditions.len; j++)
						x = x && stpo(equation[c->cursorConditions[j]]->get(&env));
					if(x)
					{
						// FOUND!
						ChangeCursor(c->cursor);
						//statustext = c->name;
						targetcmd = c;
						findertargetcommand = 0; return;
					}
				}
	}

	ChangeCursor(defcursor);
	findertargetcommand = 0;
}

void T7ClickWindow(void *param)
{
	if(objtypeToStampdown) objtypeToStampdown = 0;

	if(!multiSel)
	{
		multiSel = 1;
		mselx = mouseX; msely = mouseY;
	}

	if(!keypressed[VK_SHIFT])
		DeselectAll();
	if(!currentSelection.valid()) return;
	GameObject *cs = currentSelection.get();
	if(cs->flags & FGO_SELECTED)
		DeselectObject(cs);
	else
		SelectObject(cs);
}

void T7RightClick(void *param)
{
	if(objtypeToStampdown)
	if(playerToGiveStampdownObj.valid())
	{
		GameObject *o = CreateObject(objtypeToStampdown, playerToGiveStampdownObj.get());
		o->position = stdownpos;
		GOPosChanged(o, 0);
		if(eventAfterStampdown)
		{
			SequenceEnv ctx;
			SendGameEvent(&ctx, o, PDEVENT_ON_STAMPDOWN);
		}
		if(!keypressed[VK_SHIFT]) objtypeToStampdown = 0;
		return;
	}

	if(!experimentalKeys) return;
	//if(!currentSelection.valid()) return;
	if(!targetcmd) return;

	int m = ORDERASSIGNMODE_FORGET_EVERYTHING_ELSE;
	if(keypressed[VK_SHIFT]) m = ORDERASSIGNMODE_DO_LAST;
	if(keypressed[VK_CONTROL]) m = ORDERASSIGNMODE_DO_FIRST;

	GameObject *cs = currentSelection.get();
	for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
		if(e->value.valid())
		{
			GameObject *o = e->value.get();
			ExecuteCommand(o, targetcmd, cs, m);

			// Set move command position.
			if(stdownvalid)
			if(o->ordercfg.order.len)
			{
				SOrder *s = (m==ORDERASSIGNMODE_DO_FIRST) ? &o->ordercfg.order.first->value : &o->ordercfg.order.last->value;
				if(s->task.first->value.type->type == ORDTSKTYPE_MOVE)
				{
					s->task.first->value.destinations.clear();
					s->task.first->value.destinations.add();
					s->task.first->value.destinations.last->value.x = stdownpos.x;
					s->task.first->value.destinations.last->value.y = stdownpos.z;
				}
			}
		}
}

texture tex_toppanel, tex_bottomleftpanel, tex_bottomrightpanel;

void InitHUD()
{
	tex_toppanel = GetTexture("Interface\\InGame\\Top Panel\\Standard Panel.tga", 1);
	tex_bottomleftpanel = GetTexture("Interface\\InGame\\Bottom_Left_Panel.tga", 1);
	tex_bottomrightpanel = GetTexture("Interface\\InGame\\Bottom_Right_Panel.tga", 1);
}

void DrawHUD()
{
	float w, h;
	SetTexture(0, tex_toppanel);
	DrawRect(0, 0, scrw, 85.0f*scrh/768);
	SetTexture(0, tex_bottomleftpanel);
	w = 132.0f * scrw / 640; h = 129.0f * scrh / 480;
	DrawRect(0, scrh-h, w, h);
	SetTexture(0, tex_bottomrightpanel);
	w = 136.0f * scrw / 640; h = 136.0f * scrh / 480;
	DrawRect(scrw-w, scrh-h, w, h);
/*
	if(curclient)
	{
		char tb[512];
		sprintf(tb, "%i", (int)curclient->obj->getItem(?);
		// ...
	}
*/
}

void DisableAllCommandButtons()
{
	for(int i = 0; i < strCommand.len; i++)
		gscommand[i].gButton->enabled = 0;
}

void UpdateCommandButtons()
{
	DisableAllCommandButtons();

	GrowList<CCommand*> l;
	for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
		if(e->value.valid())
			for(int i = 0; i < e->value->objdef->offeredCmds.len; i++)
				if(!l.has(e->value->objdef->offeredCmds[i]))
				{
					CCommand *c = e->value->objdef->offeredCmds[i];
					boolean ok = 1;
					SequenceEnv env; env.self = e->value.get();
					for(int j = 0; j < c->iconConditions.len; j++)
						if(c->iconConditions[j] != -1)
							if(!stpo(equation[c->iconConditions[j]]->get(&env)))
								{ok = 0; break;}
					if(ok) l.add(c);
				}

	int x = 0;
	for(int i = 0; i < l.len; i++)
	if(l[i]->buttonEnabled.valid())
	{
		GEPicButton *b = l[i]->gButton;
		b->enabled = 1;
		b->setRect(orderButtX[x] * scrw / 640, orderButtY[x] * scrh / 480, 27 * scrw / 640, 27 * scrh / 480);
		x++;
		if(x >= 6) return;
	}	
}

int advancedSpatial = 0;
int itemsToDisplay = 0;
int selectedReaction = -1;
ImGuiTextFilter itemFilter, odFilter, objcreaFilter, ireaFilter;
ImVec4 ivcolortable[8] = {
 ImVec4(0.5f, 0.25f, 0.25f, 1), ImVec4(0, 0, 1, 1), ImVec4(1, 1, 0, 1), ImVec4(1, 0, 0, 1),
 ImVec4(0, 1, 0, 1), ImVec4(1, 0, 1, 1), ImVec4(1, 0.5f, 0, 1), ImVec4(0, 1, 1, 1)};
GrowStringList odcList;

void IGInit()
{
	char t[512];
	for(int i = 0; i < strObjDef.len; i++)
	{
		CObjectDefinition *od = &(objdef[i]);
		_snprintf(t, 511, "%s \"%s\"", CLASS_str[od->type], od->name);
		odcList.add(t);
	}
}

bool IGGSLItemsGetter(void *data, int idx, const char **out)
{
	GrowStringList *sl = (GrowStringList*)data;
	if(idx >= sl->len) return false;
	*out = sl->getdp(idx);
	return true;
}

void IGMainMenu()
{
	if(ImGui::BeginMainMenuBar())
	{
		MenuEntry *e = menucmds;
		for(int i = 0; i < 6; i++)
		{
			if(ImGui::BeginMenu(menubarstr[i]))
			{
				while(e->str)
				{
					if(ImGui::MenuItem(e->str))
						CallCommand(e->cmd);
					e++;
				}
				ImGui::EndMenu();
			}
			else
				while(e->str) e++;
			e++;
		}
		ImGui::EndMainMenuBar();
	}
}

void IGDisplayItem(GameObject *o, int item)
{
	if(!itemFilter.PassFilter(strItems.getdp(item)))
		return;
	ImGui::PushID(item);
	float n = o->getItem(item);
	ImGui::Text(strItems.getdp(item));
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);
	if(ImGui::InputFloat("", &n))
		o->setItem(item, n);
	ImGui::PopItemWidth();
	ImGui::NextColumn();
	ImGui::PopID();
}

bool IGObjectSelectable(GameObject *o)
{
	char t[256]; bool r;
	_snprintf(t, 255, "%u: %s \"%s\"###", o->id, CLASS_str[o->objdef->type], o->objdef->name);
	ImGui::PushID(o->id);
	r = ImGui::Selectable(t, o->flags & FGO_SELECTED);
	if(ImGui::IsItemClicked(1))
	{
		if(!keypressed[VK_SHIFT])
			DeselectAll();
		if(o->flags & FGO_SELECTED)
			DeselectObject(o);
		else	SelectObject(o);
	}
	ImGui::PopID();
	return r;
}

int igcurod = 0;
int igliCurrentAssoCat = -1;

void IGSelectedObject()
{
	char tb[512];
	if(!swSelObj) return;
	ImGui::SetNextWindowPos(ImVec2(373, 21), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(263, 456), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Selected object", &swSelObj);
	if(selobjects.first)
	{if(selobjects.first->value.valid())
	{
		GameObject *o = selobjects.first->value.get();
		ImGui::Value("Object id", o->id);
		ImGui::Text("Object type: %s \"%s\"", CLASS_str[o->objdef->type], o->objdef->name);
		ImGui::SameLine();
		if(ImGui::SmallButton("Change##ObjType"))
			ImGui::OpenPopup("ObjType");
		if(ImGui::BeginPopup("ObjType"))
		{
			odFilter.Draw();
			//ImGui::ListBox("##ObjTypeListBox", &igcurod, IGGSLItemsGetter, &odcList, odcList.len);
			ImGui::PushItemWidth(-1);
			ImGui::ListBoxHeader("##ObjTypeListBox");
			char t[512];
			for(int i = 0; i < strObjDef.len; i++)
			{
				CObjectDefinition *od = &(objdef[i]);
				_snprintf(t, 511, "%s \"%s\"", CLASS_str[od->type], od->name);
				if(odFilter.PassFilter(t))
				{
					ImGui::PushID(i);
					if(ImGui::Selectable(t))
					{
						ConvertObject(o, od);
						ImGui::CloseCurrentPopup();
					}
					ImGui::PopID();
				}
			}
			ImGui::ListBoxFooter();
			ImGui::PopItemWidth();
			ImGui::EndPopup();
		}
		ImGui::PushItemWidth(-1);
		if(o->objdef->type == CLASS_LEVEL || o->objdef->type == CLASS_PLAYER ||
			o->objdef->type == CLASS_TERRAIN_ZONE)
		{
			char mb[512]; wchar_t wb[512];
			if(o->name) wcstombs(mb, o->name, 511);
			else mb[0] = 0;
			ImGui::Text("Name:");
			ImGui::SameLine();
			ImGui::PushID((void*)o->id);
			if(ImGui::InputText("##Name", mb, 511))
			{
				mbstowcs(wb, mb, 511);
				delete [] o->name;
				o->name = new wchar_t[wcslen(wb)+1];
				wcscpy(o->name, wb);
			}
			ImGui::PopID();
		}
		if(o->objdef->type == CLASS_PLAYER)
		{
			ImGui::Text("Color:");
			ImGui::SameLine();
			if(ImGui::ColorButton(ivcolortable[o->color]))
				ImGui::OpenPopup("PlayerColor");
			if(ImGui::BeginPopup("PlayerColor"))
			{
				for(int y = 0; y < 4; y++)
					for(int x = 0; x < 2; x++)
					{
						int c = y*2 + x;
						if(x != 0) ImGui::SameLine();
						ImGui::PushID(c);
						if(ImGui::ColorButton(ivcolortable[c]))
						{
							o->color = c;
							UpdateParentDependantValues(o);
						}
						ImGui::PopID();
					}
				ImGui::EndPopup();
			}
		}
		ImGui::PopItemWidth();
		if(ImGui::CollapsingHeader("Location"))
		{
			ImGui::RadioButton("Basic##Spatial", &advancedSpatial, 0);
			ImGui::SameLine();
			ImGui::RadioButton("Advanced##Spatial", &advancedSpatial, 1);
			//ImGui::Text("Position: %f %f %f", o->position.x, o->position.y, o->position.z);
			if(!advancedSpatial)
			{
				ImVec2 v(o->position.x, o->position.z);
				if(ImGui::DragFloat2("Position##Basic", &v.x, 0.1f))
				{
					o->position.x = v.x; o->position.z = v.y;
					o->position.y = GetHeight(v.x, v.y);
					GOPosChanged(o, 0);
				}
				ImGui::SliderAngle("Orientation##Basic", &o->orientation.y, 0);
				if(ImGui::DragFloat("Scale##Basic", &o->scale.x, 0.1f))
					o->scale.y = o->scale.z = o->scale.x;
			}
			else
			{
				if(ImGui::DragFloat3("Position##Advanced", &o->position.x, 0.1f))
					GOPosChanged(o, 0);
				ImGui::DragFloat2("Orientation##Advanced", &o->orientation.x, 0.1f);
				ImGui::DragFloat3("Scale##Advanced", &o->scale.x, 0.1f);
			}
			ImGui::Text("Copy to all sel. objects:");
			if(ImGui::Button("Orientation##CopyButton"))
				for(DynListEntry<goref> *e = selobjects.first->next; e; e = e->next)
					if(e->value.valid())
						e->value->orientation = o->orientation;
			ImGui::SameLine();
			if(ImGui::Button("Scale##CopyButton"))
				for(DynListEntry<goref> *e = selobjects.first->next; e; e = e->next)
					if(e->value.valid())
						e->value->scale = o->scale;
		}
		if(ImGui::CollapsingHeader("Appearance"))
		{
			char **stlist = new char*[o->objdef->numsubtypes];
			for(int i = 0; i < o->objdef->numsubtypes; i++)
				stlist[i] = o->objdef->subtypes[i].name;
			ImGui::Combo("Subtype", &o->subtype, (const char**)stlist, o->objdef->numsubtypes);
			delete [] stlist;

			PhysicalSubtype *ps = &o->objdef->subtypes[o->subtype];
			char **aplist = new char*[strAppearTag.len];
			char **app = aplist;
			int *alnum = new int[strAppearTag.len];
			int *alp = alnum;
			int e, j = 0;
			for(int i = 0; i < strAppearTag.len; i++)
				if(ps->appear[i].def)
					{*(app++) = strAppearTag.getdp(i);
					*(alp++) = i;
					if(i == o->appearance) e = j;
					j++;}
			if(ImGui::Combo("Appearance", &e, (const char**)aplist, j))
				o->appearance = alnum[e];
			delete [] aplist; delete [] alnum;
		}
		if(ImGui::CollapsingHeader("Items"))
		{
			ImGui::RadioButton("Displayed##Items", &itemsToDisplay, 0);
			ImGui::SameLine();
			ImGui::RadioButton("Modified##Items", &itemsToDisplay, 1);
			ImGui::SameLine();
			ImGui::RadioButton("All##Items", &itemsToDisplay, 2);
			itemFilter.Draw();
			ImGui::Columns(2, "ItemColumns");
			ImGui::Separator();
			ImGui::Text("Item"); ImGui::NextColumn();
			ImGui::Text("Value"); ImGui::NextColumn();
			ImGui::Separator();
			ImGui::PushID("ItemNodes");
			switch(itemsToDisplay)
			{
				case 0:
					for(int i = 0; i < o->objdef->itemsDisplayed.len; i++)
						IGDisplayItem(o, o->objdef->itemsDisplayed[i]);
					break;
				case 1:
					for(DynListEntry<GOItem> *e = o->item.first; e; e = e->next)
						IGDisplayItem(o, e->value.num);
					break;
				case 2:
					for(int i = 0; i < strItems.len; i++)
						IGDisplayItem(o, i);
					break;
			}
			ImGui::PopID();
			ImGui::Columns(1);
			ImGui::Separator();
		}
		if(ImGui::CollapsingHeader("Flags"))
		{
			ImGui::Text("Disable count:");
			ImGui::SameLine();
			ImGui::PushItemWidth(-1);
			if(ImGui::InputInt("##Disable count", (int*)&o->disableCount))
				if((int)o->disableCount < 0)
					o->disableCount = 0;
			ImGui::PopItemWidth();
			ImGui::CheckboxFlags("Renderable", (uint*)&o->flags, FGO_RENDERABLE);
			ImGui::CheckboxFlags("Selectable", (uint*)&o->flags, FGO_SELECTABLE);
			ImGui::CheckboxFlags("Targetable", (uint*)&o->flags, FGO_TARGETABLE);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,0,1));
			ImGui::CheckboxFlags("Terminated", (uint*)&o->flags, FGO_TERMINATED);
			if(o->objdef->type == CLASS_PLAYER)
			{
				ImGui::CheckboxFlags("Player terminated", (uint*)&o->flags, FGO_PLAYER_TERMINATED);
				ImGui::CheckboxFlags("Reconnaissance", (uint*)&o->flags, FGO_RECONNAISSANCE);
				ImGui::CheckboxFlags("Fog of war", (uint*)&o->flags, FGO_FOG_OF_WAR);
			}
			ImGui::PopStyleColor();
		}
		if(ImGui::CollapsingHeader("Order configuration"))
		{
		if(!o->ordercfg.order.len)
			ImGui::Text("No orders");
		else {
			ImGui::PushID((void*)o->id);
			for(DynListEntry<SOrder> *e = o->ordercfg.order.first; e; e = e->next)
			{
				if(ImGui::TreeNode((void*)e->value.orderID, "%i: ORDER \"%s\"", e->value.orderID, e->value.type->name))
				{
					for(DynListEntry<STask> *f = e->value.task.first; f; f = f->next)
						if(ImGui::TreeNodeEx((void*)f->value.taskID, ImGuiTreeNodeFlags_Leaf | ((f->value.taskID == e->value.currentTask) ? ImGuiTreeNodeFlags_Selected : 0), "%i: TASK \"%s\"", f->value.taskID, f->value.type->name))
							ImGui::TreePop();
					ImGui::TreePop();
				}
			}
			ImGui::PopID();
		     }
		}
		if(ImGui::CollapsingHeader("Individual reactions"))
		{
			if(ImGui::Button("Add"))
				ImGui::OpenPopup("AddReactionMenu");
			if(ImGui::BeginPopup("AddReactionMenu"))
			{
				ireaFilter.Draw();
				ImGui::PushItemWidth(-1);
				ImGui::ListBoxHeader("##NewIndividualReactionListBox");
				for(int i = 0; i < strReaction.len; i++)
					if(ireaFilter.PassFilter(strReaction.getdp(i)))
						if(ImGui::Selectable(strReaction.getdp(i)))
						{
							AddReaction(o, i);
							ImGui::CloseCurrentPopup();
						}
				ImGui::ListBoxFooter();
				ImGui::PopItemWidth();
				ImGui::EndPopup();
			}
			ImGui::SameLine();
			if(ImGui::Button("Remove"))
				for(DynListEntry<uint> *e = o->iReaction.first; e; e = e->next)
					if(e->value == selectedReaction)
						{o->iReaction.remove(e); break;}
			ImGui::PushItemWidth(-1);
			ImGui::ListBoxHeader("##IndividualReactionListBox");
			for(DynListEntry<uint> *e = o->iReaction.first; e; e = e->next)
				if(ImGui::Selectable(strReaction.getdp(e->value), e->value == selectedReaction))
					selectedReaction = e->value;
			ImGui::ListBoxFooter();
			ImGui::PopItemWidth();
		}
		if(ImGui::CollapsingHeader("Association"))
		{
			ImGui::Text("Category:"); ImGui::SameLine();
			ImGui::PushItemWidth(-1);
			ImGui::Combo("##AssoCat", &igliCurrentAssoCat, IGGSLItemsGetter, &strAssociateCat, strAssociateCat.len);
			ImGui::PopItemWidth();

		if(igliCurrentAssoCat != -1)
		{
			GOAssociation *goa = 0;
			for(DynListEntry<GOAssociation> *e = o->association.first; e; e = e->next)
				if(e->value.category == igliCurrentAssoCat)
					goa = &e->value;

			ImGui::Text("Associates:");
			ImGui::PushItemWidth(-1);
			ImGui::ListBoxHeader("##AssociatesList");
			if(goa)
				for(DynListEntry<GameObjAndListEntry> *e = goa->associates.first; e; e = e->next)
					IGObjectSelectable(e->value.o);
			ImGui::ListBoxFooter();
			ImGui::PopItemWidth();

			ImGui::Text("Associators:");
			ImGui::PushItemWidth(-1);
			ImGui::ListBoxHeader("##AssociatorsList");
			if(goa)
				for(DynListEntry<GameObjAndListEntry> *e = goa->associators.first; e; e = e->next)
					IGObjectSelectable(e->value.o);
			ImGui::ListBoxFooter();
			ImGui::PopItemWidth();
		}
		else	ImGui::TextWrapped("Select an association category in the combobox above.");
		}
	}else	ImGui::Text("The first selection is invalid!");
	}else	ImGui::Text("No object selected!");
	ImGui::End();
}

void IGLevelTreeNode(GameObject *o)
{
	boolean n = ImGui::TreeNodeEx((void*)o->id,
			// ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow |
			((o->flags & FGO_SELECTED) ? ImGuiTreeNodeFlags_Selected : 0) |
			((o->children.len > 0) ? 0 : ImGuiTreeNodeFlags_Leaf),
			"%u: %s \"%s\"", o->id, CLASS_str[o->objdef->type], o->objdef->name);
	if(ImGui::IsItemClicked(1))
	{
		if(!keypressed[VK_SHIFT])
			DeselectAll();
		if(o->flags & FGO_SELECTED)
			DeselectObject(o);
		else	SelectObject(o);
	}
	if(n)
	{
		for(DynListEntry<GameObject> *e = o->children.first; e; e = e->next)
			IGLevelTreeNode(&e->value);
		ImGui::TreePop();
	}
}

void IGLevelTree()
{
	if(!swLevTree) return;
	ImGui::SetNextWindowPos(ImVec2(130, 103), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(375, 243), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Level tree", &swLevTree);
	ImGui::PushID("LevelTreeNodes");
	IGLevelTreeNode(levelobj);
	ImGui::PopID();
	ImGui::End();
}

bool IGPlayerChooser(char *popupid, goref *p)
{
	bool r = 0;
	if(ImGui::Selectable("##PlayerSelButton"))
		ImGui::OpenPopup(popupid);
	if(ImGui::BeginPopup(popupid))
	{
		for(DynListEntry<GameObject> *e = levelobj->children.first; e; e = e->next)
		  if(e->value.objdef->type == CLASS_PLAYER)
		{
			ImGui::PushID(e->value.id);
			if(ImGui::Selectable("##PlayerSelectable"))
			{
				*p = &e->value;
				r = 1;
			}
			ImGui::SameLine();
			ImGui::ColorButton(ivcolortable[e->value.color], true);
			ImGui::SameLine();
			ImGui::Text("%S (%u)", e->value.name, e->value.id);
			ImGui::PopID();
		}
		ImGui::EndPopup();
	}
	ImGui::SameLine();
	if(!p->valid())
	{
		ImGui::ColorButton(ImVec4(1,1,1,1), true);
		ImGui::SameLine();
		ImGui::Text("Click me to select a player.");
	} else {
		GameObject *o = p->get();
		ImGui::ColorButton(ivcolortable[o->color], true);
		ImGui::SameLine();
		ImGui::Text("%S (%u)", o->name, o->id);
	}
	return r;
}

extern char sggameset[384];
extern uint game_type;
int igliCurrentAlias = -1;
goref drPlayer;
void IGLevelInfo()
{
	if(!swLevInfo) return;
	ImGui::SetNextWindowPos(ImVec2(183, 219), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(265, 185), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Level information", &swLevInfo);
	if(ImGui::CollapsingHeader("Properties"))
	{
		ImGui::TextWrapped("Changes on these values only take effect after saving and reopening the savegame!");
		ImGui::PushItemWidth(-1);
		ImGui::Text("Game set:");
		ImGui::InputText("##Game set", sggameset, 383);
		ImGui::Text("Map:");
		ImGui::InputText("##Map", lastmap, 255);
		ImGui::Text("Game type:");
		ImGui::InputInt("##Game type", (int*)&game_type);
		ImGui::PopItemWidth();
	}
	if(ImGui::CollapsingHeader("Time"))
	{
		ImGui::Value("Current time", current_time);
		ImGui::Value("Previous time", previous_time);
		ImGui::Value("Elapsed time", elapsed_time);
		ImGui::Text("Lock count:");
		ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		if(ImGui::InputInt("##Lock count", (int*)&lock_count))
			if((int)lock_count < 0)
				lock_count = 0;
		ImGui::PopItemWidth();
	}
	if(ImGui::CollapsingHeader("Alias"))
	{
		ImGui::Text("Category:"); ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		ImGui::Combo("##Alias", &igliCurrentAlias, IGGSLItemsGetter, &strAlias, strAlias.len);
		ImGui::PopItemWidth();

	if(igliCurrentAlias != -1)
	{
		DynList<goref> *d = &(alias[igliCurrentAlias]);

		if(ImGui::Button("Add sel.##Alias"))
			for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
				if(e->value.valid())
					AliasObj(e->value.get(), igliCurrentAlias);
		ImGui::SameLine();
		if(ImGui::Button("Remove sel.##Alias"))
			for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
				if(e->value.valid())
					UnaliasObj(e->value.get(), igliCurrentAlias);
		ImGui::SameLine();
		if(ImGui::Button("Clear##Alias"))
			ClearAlias(igliCurrentAlias);
		ImGui::SameLine();
		if(ImGui::Button("Select all##Alias"))
			for(DynListEntry<goref> *e = d->first; e; e = e->next)
				if(e->value.valid())
					SelectObject(e->value.get());

		ImGui::PushItemWidth(-1);
		ImGui::ListBoxHeader("##AliasList");

		for(DynListEntry<goref> *e = d->first; e; e = e->next)
			if(e->value.valid())
				IGObjectSelectable(e->value.get());
		ImGui::ListBoxFooter();
		ImGui::PopItemWidth();
	}
	else	ImGui::TextWrapped("Select an alias category in the combobox above.");
	}
	if(ImGui::CollapsingHeader("Diplomatic relationships"))
	{
		ImGui::Text("Player A:");
		ImGui::SameLine();
		IGPlayerChooser("DiploPlayer", &drPlayer);
		ImGui::Columns(2);
		ImGui::Separator();
		ImGui::Text("Player B");
		ImGui::NextColumn();
		ImGui::Text("Relation A<->B");
		ImGui::NextColumn();
		ImGui::Separator();

		GameObject *p = drPlayer.get();
		for(DynListEntry<GameObject> *e = levelobj->children.first; e; e = e->next)
		 if(e->value.objdef->type == CLASS_PLAYER)
		{
			GameObject *o = &e->value;
			ImGui::PushID(o->id);
			ImGui::ColorButton(ivcolortable[o->color]/*, true*/);
			ImGui::SameLine();
			ImGui::Text("%S (%u)", o->name, o->id);
			ImGui::NextColumn();
			if(drPlayer.valid()) if(&e->value != p)
			{
				ImGui::PushItemWidth(-1);
				int rel = GetDiplomaticStatus(p, o);
				if(ImGui::Combo("##Relation", &rel, IGGSLItemsGetter, (void*)&strDiplomaticStatus, strDiplomaticStatus.len))
					SetDiplomaticStatus(p, o, rel);
				ImGui::PopItemWidth();
			}
			ImGui::NextColumn();
			ImGui::PopID();
		}

		ImGui::Columns(1);
		ImGui::Separator();
	}
	ImGui::End();
}

void IGObjectCreation()
{
	if(!swObjCrea) return;
	ImGui::SetNextWindowPos(ImVec2(2, 22), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(265, 456), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Object creation", &swObjCrea);

	ImGui::Text("Give to:");
	ImGui::SameLine();
	IGPlayerChooser("PlayerToGiveCreatedObj", &playerToGiveStampdownObj);

	ImGui::Checkbox("Send \"On Stampdown\" event", &eventAfterStampdown);
	objcreaFilter.Draw();
	ImGui::PushItemWidth(-1);
	//ImGui::ListBoxHeader("##ObjTypeListBox");
	ImGui::BeginChild("ObjCreaObjTypeListBox", ImVec2(0,0), true);
	char t[512];
	for(int i = 0; i < strObjDef.len; i++)
	{
		CObjectDefinition *od = &(objdef[i]);
		_snprintf(t, 511, "%s \"%s\"", CLASS_str[od->type], od->name);
		if(objcreaFilter.PassFilter(t))
		{
			ImGui::PushID(i);
			if(ImGui::Selectable(t, od == objtypeToStampdown))
				objtypeToStampdown = od;
			ImGui::PopID();
		}
	}	
	//ImGui::ListBoxFooter();
	ImGui::EndChild();
	ImGui::PopItemWidth();
	ImGui::End();
}

void IGAbout()
{
	if(!swAbout) return;
	ImGui::Begin("About wkbre", &swAbout);
	ImGui::Indent();
	ImGui::SetWindowFontScale(2.0f);
	ImGui::TextColored(ImVec4(0.5,1,0.5,1), "wkbre");
	ImGui::SetWindowFontScale(1.0f);
	ImGui::Unindent();
	ImGui::Text("Version: " WKBRE_VERSION);
#ifdef _MSC_VER
	ImGui::Text("Compiler: MSVC %i", _MSC_VER);
#endif
#ifdef __GNUC__
	ImGui::Text("Compiler: GCC " __VERSION__);
#endif
	ImGui::Text("Build date: " __DATE__);
	ImGui::Separator();
	ImGui::Text("(C) 2015-2017 AdrienTD");
	ImGui::Text("Licensed under the GPL3 license.\nSee LICENSE for details.");
	ImGui::Separator();
	ImGui::Text("Libraries used:");
	ImGui::BulletText("LZRW3");
	ImGui::BulletText("bzip2");
	ImGui::BulletText("dear imgui");
	ImGui::End();
}

void Test7()
{
	LoadBCP("data.bcp"); ReadLanguageFile(); InitWindow(); InitScene(); InitFont(); InitCursor();
	ImGuiImpl_Init();
	memset(gameobj, 0, MAX_GAMEOBJECTS * sizeof(GameObject*));
	//ddev->SetDialogBoxMode(TRUE);

	InitGfxConsole();
	WriteGfxConsole("wkbre Version " WKBRE_VERSION); // " IN DEVELOPMENT"

#ifdef _MSC_VER
	WriteAndDrawGfxConsole("Compiled with MSVC C/C++ compiler");
#endif
#ifdef __GNUC__
	WriteAndDrawGfxConsole("Compiled with GNU C/C++ compiler");
#endif

	GrowStringList *gsl = ListFiles("Save_Games");
	int lx = ListDlgBox(gsl, "Select a saved game you want to load. The list begins with files from data.bcp and ends with your personal saved games in the \"saved\" directory.");
	if(lx == -1) return;
	char sgn[384] = "Save_Games\\\0";
	strcat(sgn, gsl->getdp(lx));
	delete gsl;
	LoadSaveGame(sgn);

	loadinginfo("Savegame loaded!\n");

	GEContainer *editInterface = new GEContainer;
	actualpage = editInterface;
	editInterface->buttonClick = T7ClickWindow;
	editInterface->buttonRightClick = T7RightClick;
/*
	GEMenuBar *menubar = new GEMenuBar(menubarstr, menucmds, 5, editInterface);
	editInterface->add(menubar);
	menubar->setRect(0, 0, 32767, 20);
	menubar->bgColor = 0xC0000080;
*/
	InitGTWs(); // :S
	CreateCommandButtons();

	boolean playView = 0;
	InitHUD();

	ChangeCursor(defcursor = LoadCursor("Interface\\C_DEFAULT.TGA"));
	Cursor *buildcursor = LoadCursor("Interface\\C_Repair.tga");

	InitOOBMList();

	IGInit();

	while(!appexit)
	{
//#ifndef WKBRE_RELEASE
		if(playMode)
		{
			AdvanceTime();
			ProcessAllOrders();
			CheckBattlesDelayedSequences();
			UpdateContainerPos(levelobj);
			UpdateClientsCam();
		}
//#endif
		// Copy controlled client's camera to drawing/scene camera.
		if(curclient)
		{
			camerapos = curclient->camerapos;
			campitch = curclient->cameraori.x;
			camyaw = curclient->cameraori.y;
		}

		if(!winMinimized)
		{
			objsdrawn = 0;
			BeginDrawing();
			DrawScene();
			InitRectDrawing();
			if(multiSel) {
				NoTexture(0);
				renderer->DrawFrame(mselx, msely, mouseX-mselx, mouseY-msely, -1);
			}

		if(showTimeObjInfo) if(!playView)
		{	char st[256];
			sprintf(st, "current_time = %f\nCamera pos: %.2f %.2f %.2f %.2f %.2f", current_time, camerapos.x, camerapos.y, camerapos.z, campitch, camyaw);
			DrawFont(0, 60, st);
			if(selobjects.len)
			if(selobjects.first->value.valid())
			{
				sprintf(st, "--- Selected object ---\nHP: %f/%f\nFood: %f\nWood: %f\nGold: %f\nStone: %f",
					selobjects.first->value->getItem(PDITEM_HP),
					selobjects.first->value->getItem(PDITEM_HPC),
					selobjects.first->value->getItem(PDITEM_FOOD),
					selobjects.first->value->getItem(PDITEM_WOOD),
					selobjects.first->value->getItem(PDITEM_GOLD),
					selobjects.first->value->getItem(PDITEM_STONE));
				NoTexture(0);
				DrawRect(0, 100, 280, 6*20, 0xC0000080);
				DrawFont(0, 100, st);
			}
		}

			if(playView)
			{
				UpdateCommandButtons();
				DrawHUD();
			}
			else	DisableAllCommandButtons();

			NoTexture(0);
			if(actualpage) actualpage->draw(0, 0);
			if(statustext) if(!playView)
			{
				NoTexture(0);
				DrawRect(0, scrh-20 , scrw, 20, 0xC0000080);
				DrawFont(0, scrh-20, statustext);
			}
			if(enableObjTooltips) DrawTooltip();

			ImGuiImpl_NewFrame();
/*
			ImGui::Text("Yes!");
			ImGui::DragFloat("Game speed", &game_speed);
*/
			if(menuVisible) IGMainMenu();
			IGSelectedObject();
			IGLevelTree();
			IGLevelInfo();
			IGObjectCreation();
			IGAbout();

			renderer->InitImGuiDrawing();
			ImGui::Render();

			InitRectDrawing();
			if(showTimeObjInfo)
			{
				char st[64];
				sprintf(st, "FPS: %i - ODPF: %i", drawfps, objsdrawn);
				DrawFont(scrw - 188, 0, st);
			}

			if(objtypeToStampdown && playerToGiveStampdownObj.valid()) ChangeCursor(buildcursor);
			else SetTargetCursor();
			DrawCursor();
			EndDrawing();
		}
		HandleWindow();

		if(currentSelection.valid())
		{
			tooltip_str = currentSelection->objdef->tooltip;
			tooltip_x = mouseX; tooltip_y = mouseY;
		}
		else	tooltip_str = 0;

		if(keypressed['\r'])
		{
			keypressed['\r'] = 0;
			playView = !playView;
		}

		if(keypressed[VK_SHIFT]) walkstep = objmovalign ? 5.0f : 4.0f;
		else walkstep = objmovalign ? 2.5f : 1.0f;
		if(keypressed[VK_UP])
		{
			if(objmovalign) keypressed[VK_UP] = 0;
			Vector3 m = Vector3(vLAD.x, 0, vLAD.z), n;
			NormalizeVector3(&n, &m);
			if(!keypressed[VK_CONTROL])
				DisplaceCurCamera(n * walkstep);
			else for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
				if(e->value.valid())
					{e->value->position += n * walkstep;
					e->value->position.y = GetHeight(e->value->position.x, e->value->position.z);
					GOPosChanged(e->value.get());}
		}
		if(keypressed[VK_DOWN])
		{
			if(objmovalign) keypressed[VK_DOWN] = 0;
			Vector3 m = Vector3(vLAD.x, 0, vLAD.z), n;
			NormalizeVector3(&n, &m);
			if(!keypressed[VK_CONTROL])
				DisplaceCurCamera(n * (-walkstep));
			else for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
				if(e->value.valid())
					{e->value->position -= n * walkstep;
					e->value->position.y = GetHeight(e->value->position.x, e->value->position.z);
					GOPosChanged(e->value.get());}
		}
		if(keypressed[VK_LEFT])
		{
			if(objmovalign) keypressed[VK_LEFT] = 0;
			Vector3 m = Vector3(-vLAD.z, 0, vLAD.x), n;
			NormalizeVector3(&n, &m);
			if(!keypressed[VK_CONTROL])
				DisplaceCurCamera(n * walkstep);
			else for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
				if(e->value.valid())
					{e->value->position += n * walkstep;
					e->value->position.y = GetHeight(e->value->position.x, e->value->position.z);
					GOPosChanged(e->value.get());}
		}
		if(keypressed[VK_RIGHT])
		{
			if(objmovalign) keypressed[VK_RIGHT] = 0;
			Vector3 m = Vector3(vLAD.z, 0, -vLAD.x), n;
			NormalizeVector3(&n, &m);
			if(!keypressed[VK_CONTROL])
				DisplaceCurCamera(n * walkstep);
			else for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
				if(e->value.valid())
					{e->value->position += n * walkstep;
					e->value->position.y = GetHeight(e->value->position.x, e->value->position.z);
					GOPosChanged(e->value.get());}
		}
		if(keypressed['E']) DisplaceCurCamera(Vector3(0.0f,  walkstep, 0.0f));
		if(keypressed['D']) DisplaceCurCamera(Vector3(0.0f, -walkstep, 0.0f));
		if(keypressed['T']) RotateCurCamera( 0.01f, 0);
		if(keypressed['G']) RotateCurCamera(-0.01f, 0);
		if(keypressed['U']) RotateCurCamera(0,  0.01f);
		if(keypressed['I']) RotateCurCamera(0, -0.01f);

		if(keypressed['1'])
			{keypressed['1'] = 0; CallCommand(CMD_CAMDOWNLEFT);}
		if(keypressed['2'])
			{keypressed['2'] = 0; CallCommand(CMD_CAMDOWNRIGHT);}
		if(keypressed['3'])
			{keypressed['3'] = 0; CallCommand(CMD_CAMUPLEFT);}
		if(keypressed['4'])
			{keypressed['4'] = 0; CallCommand(CMD_CAMUPRIGHT);}
/*		if(keypressed['9'])
			{keypressed['9'] = 0; CallCommand(CMD_BCMHIMAPRIGHT);}
		if(keypressed['0'])
			{keypressed['0'] = 0; CallCommand(CMD_BCMHIMAPLEFT);}
*/
		if(keypressed['M'])
			{keypressed['M'] = 0; CallCommand(CMD_SHOWHIDELANDSCAPE);}
		if(keypressed[VK_ESCAPE])
			{keypressed[VK_ESCAPE] = 0; CallCommand(CMD_QUIT);}

		if(keypressed[VK_F3]) {keypressed[VK_F3] = 0; CallCommand(CMD_SAVE);}

		if(keypressed['5']) {keypressed['5'] = 0; CallCommand(CMD_CAMMANOR);}

		if(keypressed[VK_F5])
			{keypressed[VK_F5] = 0; CallCommand(CMD_DELETEOBJTYPE);}
		if(keypressed[VK_F6])
			{keypressed[VK_F6] = 0; CallCommand(CMD_CONVERTOBJTYPE);}
		if(keypressed[VK_F7])
			{keypressed[VK_F7] = 0; CallCommand(CMD_GIVEOBJTYPE);}
		if(keypressed[VK_F8])
			{keypressed[VK_F8] = 0; CallCommand(CMD_CREATEOBJ);}
		if(keypressed[VK_F9])
			{keypressed[VK_F9] = 0; CallCommand(CMD_DELETELASTCREATEDOBJ);}
		if(keypressed[VK_F11])
			{keypressed[VK_F11] = 0; CallCommand(CMD_RANDOMSUBTYPE);}

		if(keypressed['6']) {keypressed['6'] = 0; CallCommand(CMD_CAMPOS);}

		if(keypressed[VK_F1])
			{keypressed[VK_F1] = 0; CallCommand(CMD_ABOUT);}

		if(keypressed[VK_F2])
			{keypressed[VK_F2] = 0; CallCommand(CMD_RESETOBJPOS);}
		if(keypressed[VK_F4])
			{keypressed[VK_F4] = 0; CallCommand(CMD_DELETEOBJCLASS);}

		if(keypressed[VK_DELETE])
			{keypressed[VK_DELETE] = 0; CallCommand(CMD_DELETESELOBJECT);}
		if(keypressed[VK_INSERT])
			{keypressed[VK_INSERT] = 0; CallCommand(CMD_DUPLICATESELOBJECT);}

		if(keypressed[VK_BACK])
			{keypressed[VK_BACK] = 0; menuVisible = !menuVisible;
			}//menubar->enabled = menuVisible;}

		if(keypressed['7']) {keypressed['7'] = 0; CallCommand(CMD_CAMRESETORI);}

		if(keypressed[VK_MULTIPLY])
			{keypressed[VK_MULTIPLY] = 0; CallCommand(CMD_SELOBJSCALEBIGGER);}
		if(keypressed[VK_DIVIDE])
			{keypressed[VK_DIVIDE] = 0; CallCommand(CMD_SELOBJSCALESMALLER);}
		if(keypressed[VK_HOME])
			{keypressed[VK_HOME] = 0; CallCommand(CMD_GIVESELOBJECT);}

		if(keypressed['R']) {keypressed['R'] = 0; CallCommand(CMD_ROTATEOBJQP);}
		if(keypressed['8']) {keypressed['8'] = 0; CallCommand(CMD_CAMCLISTATE);}

		if(keypressed['O']) {keypressed['O'] = 0; CallCommand(CMD_SELECT_OBJECT_ID);}
		if(keypressed[VK_TAB]) {keypressed[VK_TAB] = 0; CallCommand(CMD_CHANGE_OBJMOVALIGN);}

		if(keypressed['L']) {keypressed['L'] = 0; CallCommand(CMD_TOGGLE_TIMEOBJINFO);}

		if(keypressed['K']) {keypressed['K'] = 0; CallCommand(CMD_TOGGLEGRID);}

		if(keypressed[VK_NUMPAD0])
		{
			if(!mouseRot)
			{
				mouseRot = 1;
				msrotx = mouseX; msroty = mouseY;
			}
			else
			{
				float rx = (mouseX-msrotx)/200.0f;
				float ry = (mouseY-msroty)/200.0f;
				RotateCurCamera(-ry, -rx);
				msrotx = mouseX; msroty = mouseY;
			}
		}
		else mouseRot = 0;

		if(!lmbPressed)
		if(multiSel)
		{
			multiSel = 0;
			for(int i = 0; i < msellist.len; i++)
				SelectObject(msellist[i]);
		}
		msellist.clear();

	if(experimentalKeys)
	{
		if(keypressed['P']) {keypressed['P'] = 0; CallCommand(CMD_PAUSE);}
		if(keypressed[VK_ADD]) {keypressed[VK_ADD] = 0; CallCommand(CMD_GAME_SPEED_FASTER);}
		if(keypressed[VK_SUBTRACT]) {keypressed[VK_SUBTRACT] = 0; CallCommand(CMD_GAME_SPEED_SLOWER);}

		//if(keypressed['A'])
		//	{keypressed['A'] = 0; drawdebug = 1;}
#ifndef WKBRE_RELEASE
		if(keypressed['W'])
		{
			keypressed['W'] = 0;
			//GameObject *a = FindObjID(3102);
			//if(a) printf("Got %i\n", a->id);
/*
			int t = FindObjDef(CLASS_MARKER, "Standard");
			for(int i = 0; i < 16384; i++)
				CreateObject(&objdef[t], levelobj);
*/
/*
			printf("%i material textures:\n", strMaterials.len);
			for(int i = 0; i < strMaterials.len; i++)
				printf(" - %s\n", strMaterials.getdp(i));
*/
/*
			printf("%i unique models:\n", alModelFn.len);
			for(int i = 0; i < alModelFn.len; i++)
				printf(" - %s\n", alModelFn.getdp(i));
*/
			if(selobjects.len)
			if(selobjects.first->value.valid())
			{
				GameObject *o = selobjects.first->value.get();
				printf("animtag = %i, animlooping = %i\n", o->animtag, o->animlooping);
			}
		}
#endif
		if(keypressed['A']) {keypressed['A'] = 0; CallCommand(CMD_EXECUTE_COMMAND);}
		if(keypressed['Y']) {keypressed['Y'] = 0; CallCommand(CMD_EXECUTE_COMMAND_WITH_TARGET);}
		if(keypressed['X']) {keypressed['X'] = 0; CallCommand(CMD_RUNACTSEQ);}
		if(keypressed['C']) {keypressed['C'] = 0; CallCommand(CMD_CREATE_MAPPED_TYPE_OBJECT);}
		if(keypressed['V']) {keypressed['V'] = 0; CallCommand(CMD_STAMPDOWN_OBJECT);}
		//if(keypressed['Q']) {keypressed['Q'] = 0; CallCommand(CMD_CANCEL_ALL_OBJS_ORDERS);}

		if(keypressed['B'])
		{
			keypressed['B'] = 0;
			if(selobjects.len) if(selobjects.first->value.valid())
			{
				GameObject *o = selobjects.first->value.get();
				//printf("%i objects on same tile:\n", o->itile->objs.len);
				//for(DynListEntry<goref> *e = o->itile->objs.first; e; e = e->next)
				//	printf(" - %i\n", e->value.getID());
			/*	GrowList<goref> onm;
				ListObjsInTiles(o->position.x/5-1, o->position.z/5-1, 3, 3, &onm);
				printf("%i objects near it:\n", onm.len);
				for(int i = 0; i < onm.len; i++)
					printf(" - %i\n", onm.getpnt(i)->getID());
			*/
				printf("%i referencers:\n", o->referencers.len);
				for(DynListEntry<GameObject*> *e = o->referencers.first; e; e = e->next)
					printf(" - %i\n", e->value->id);
			}
		}

		if(keypressed['S']) {keypressed['S'] = 0; CallCommand(CMD_START_LEVEL);}

		if(keypressed['N'])
		{
			keypressed['N'] = 0;
			printf("--- Unknown actions ---\n");
			for(int i = 0; i < strUnknownAction.len; i++)
				printf("%s\n", strUnknownAction.getdp(i));
			printf("--- Unknown object finders ---\n");
			for(int i = 0; i < strUnknownFinder.len; i++)
				printf("%s\n", strUnknownFinder.getdp(i));
			printf("--- Unknown position determinators ---\n");
			for(int i = 0; i < strUnknownPosition.len; i++)
				printf("%s\n", strUnknownPosition.getdp(i));
			printf("--- Unknown value determinators ---\n");
			for(int i = 0; i < strUnknownValue.len; i++)
				printf("%s\n", strUnknownValue.getdp(i));
		}

		//if(keypressed['Z']) {keypressed['Z'] = 0; CallCommand(CMD_REMOVE_BATTLES_DELAYED_SEQS);}
		if(keypressed['H']) {keypressed['H'] = 0; CallCommand(CMD_ENABLE_GTW);}
		if(keypressed['F']) {keypressed['F'] = 0; CallCommand(CMD_CONTROL_CLIENT);}
		if(keypressed['J']) {keypressed['J'] = 0; CallCommand(CMD_SEND_EVENT);}
	}
	}
}

#ifndef WKBRE_RELEASE
int main(int argc, char *argv[])
#else
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, char *args, int showMode)
#endif
{
	srand(time(NULL));
	hInstance = GetModuleHandle(NULL); ReadSettingsFile();
	//if(argc >= 2) farg = argv[1];
	//InitD3DXDLL();
#ifndef WKBRE_RELEASE
	RunTest();
#else
	Test7();
#endif
	return 0;
}