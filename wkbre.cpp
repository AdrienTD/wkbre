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

HINSTANCE hInstance;
GEContainer *actualpage = 0;
int appexit = 0;
float walkstep = 1.0f;
char *farg = 0;
char secret[] = {0xec,0xc9,0xdf,0xc4,0xc8,0xc3,0x8d,0xea,0xc8,0xc8,0xd9,0xde};
int bo = 0;
int playMode = 0, enableObjTooltips = 0;
ClientState *curclient = 0;
boolean objmovalign = 0;
Cursor *defcursor;

#ifdef WKBRE_RELEASE
int experimentalKeys = 0;
#else
int experimentalKeys = 1;
#endif

char *playercolorname[8] = {"Brown", "Blue", "Yellow", "Red", "Green", "Pink", "Orange", "Aqua"};

char *tooltip_str = "Hello!\nMy name is Mr. Tooltip!\nBye!"; int tooltip_x = 20, tooltip_y = 20;
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

// Variables needed for GUI:
GUIElement *getobringfront = 0; GESubmenu *actsubmenu = 0;
GUIElement *movingguielement = 0; int movge_rx, movge_ry;

// Menu
char *menubarstr[] = {"File", "Object", "View", "Help"};
MenuEntry menucmds[] = {
{"Save as...", CMD_SAVE},
{"Change WK version", CMD_CHANGE_SG_WKVER},
{"Quit", CMD_QUIT},
{0,0},
{"Delete type...", CMD_DELETEOBJTYPE},
{"Convert type...", CMD_CONVERTOBJTYPE},
{"Give type...", CMD_GIVEOBJTYPE},
{"Randomize subtypes", CMD_RANDOMSUBTYPE},
{"Reset objects' heights", CMD_RESETOBJPOS},
{"Delete class...", CMD_DELETEOBJCLASS},
{"Create object...", CMD_CREATEOBJ},
{"Delete last created object", CMD_DELETELASTCREATEDOBJ},
{"Duplicate selected objects", CMD_DUPLICATESELOBJECT},
{"Give selected objects to...", CMD_GIVESELOBJECT},
{"Delete selected objects", CMD_DELETESELOBJECT},
{"Scale selected objects by 1.5", CMD_SELOBJSCALEBIGGER},
{"Scale selected objects by 1/1.5", CMD_SELOBJSCALESMALLER},
{"Rotate selected objects by 90°", CMD_ROTATEOBJQP},
{"Select object by ID...", CMD_SELECT_OBJECT_ID},
{"Enable/disable aligned movement", CMD_CHANGE_OBJMOVALIGN},
{"Rename player object...", CMD_CHANGE_PLAYER_NAME},
{"Change player color...", CMD_CHANGE_PLAYER_COLOR},
{0,0},
{"Move to...", CMD_CAMPOS},
{"Move to down-left corner", CMD_CAMDOWNLEFT},
{"Move to down-right corner", CMD_CAMDOWNRIGHT},
{"Move to up-left corner", CMD_CAMUPLEFT},
{"Move to up-right corner", CMD_CAMUPRIGHT},
{"Move to player's manor...", CMD_CAMMANOR},
{"Move to client's position...", CMD_CAMCLISTATE},
{"Reset orientation", CMD_CAMRESETORI},
{"Show/hide landscape", CMD_SHOWHIDELANDSCAPE},
{"Show/hide representations", CMD_TOGGLEREPRENSATIONS},
{"Show/hide object tooltips", CMD_TOGGLEOBJTOOLTIPS},
{"BCM himap bit left", CMD_BCMHIMAPLEFT},
{"BCM himap bit right", CMD_BCMHIMAPRIGHT},
{0,0},
{"Enable/disable experimental features", CMD_TOGGLEEXPERIMENTALKEYS},
{"About...", CMD_ABOUT},
{0,0},
};

#ifndef WKBRE_RELEASE

void OnTestButtonClick(void *param)
{
	printf("You clicked on the button!\n");
	//appexit = 1;
}

//#define dbg(i) putchar((i) + '0')

float ytranslation = 0.0f, scaling = 1.0f;

void SetModelMatrices()
{
	Matrix matWorld, rotMatrix, transMatrix, scaleMatrix;
	CreateRotationYMatrix(&rotMatrix, (timeGetTime()%2000)*(2.0f*M_PI)/2000.0f);
	CreateTranslationMatrix(&transMatrix, 0.0f, ytranslation, 0.0f);
	CreateScaleMatrix(&scaleMatrix, scaling, scaling, scaling);
	MultiplyMatrices(&matWorld, &rotMatrix, &transMatrix);
	//MultiplyMatrices(&matWorld, &matWorld, &scaleMatrix);
	matWorld *= scaleMatrix;
	Matrix finalmatrix = matWorld;

	Vector3 vEyePt( 0.0f, 18.0f,-20.0f );
	Vector3 vLookatPt( 0.0f, 0.0f, 0.0f );
	Vector3 vUpVec( 0.0f, 1.0f, 0.0f );
	Matrix matView;
	CreateLookAtLHViewMatrix( &matView, &vEyePt, &vLookatPt, &vUpVec );
	finalmatrix *= matView;

	Matrix matProj;
	CreatePerspectiveMatrix( &matProj, M_PI / 4, 1.0f, 1.0f, 100.0f );
	finalmatrix *= matProj;

	SetTransformMatrix(&finalmatrix);
}

void Test1()
{
	LoadBCP("data.bcp");
	char *t; int s;
	//LoadFile("Fonts\\Proportional\\20\\Proportional - 20.font", &t, &s);
	//LoadFile("Animations_Are_Guaranteed_To_Merge.txt", &t, &s);
	//LoadFile("Campaigns\\Kingmaker.cam", &t, &s);
	//LoadFile("test\\abc.txt", &t, &s);
	//LoadFile("Languages\\..\\Animations_Are_Guaranteed_To_Merge.txt", &t, &s);
	LoadFile("Warrior Kings Game Set\\Mechanics\\Recon.cpp", &t, &s);
	fwrite(t, s, 1, stdout);
	free(t);
}

void Test2()
{
	LoadBCP("data.bcp");
	ReadLanguageFile();
	printf("%s\n", GetLocText("AI_BIOG_BLACKLERE_THE_PEASANT"));
}

void Test3()
{
	LoadBCP("data.bcp");
	ReadLanguageFile();
	InitWindow();
	//InitRectDrawing();
	//IDirect3DTexture9 *mytex = GetTexture("Interface\\LOADING SCREEN 1.tga");
	//IDirect3DPixelShader9 *psh = LoadPixelShader("test.psh");
	InitScene(); LoadMap("testmap.bcm");
	while(!appexit)
	{
		BeginDrawing();
		//ddev->SetRenderState(D3DRS_ZENABLE, FALSE);
		//ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		//ddev->SetPixelShader(psh);
		//ddev->SetTexture(0, mytex);
		//DrawRect(0, 0, 640, 480, -1);
		DrawScene();
		EndDrawing();
		HandleWindow();

		if(keypressed[VK_UP]) camerapos += Vector3(0.0f, 0.0f, walkstep);
		if(keypressed[VK_DOWN]) camerapos -= Vector3(0.0f, 0.0f, walkstep);
		if(keypressed[VK_LEFT]) camerapos -= Vector3(walkstep, 0.0f, 0.0f);
		if(keypressed[VK_RIGHT]) camerapos += Vector3(walkstep, 0.0f, 0.0f);
		if(keypressed['E']) camerapos += Vector3(0.0f, walkstep, 0.0f);
		if(keypressed['D']) camerapos -= Vector3(0.0f, walkstep, 0.0f);
	}
}

void Test4()
{
	LoadBCP("data.bcp");
	InitWindow();
	Mesh tm("Warrior Kings Game Set\\Characters\\Abaddon\\ABADDON_MOVE.MESH3");
	Mesh tn("Warrior Kings Game Set\\Characters\\Michael\\MICHAEL1.MESH3");
	//InitMeshDrawing();
	while(!appexit)
	{
		BeginDrawing();
		BeginMeshDrawing();
		SetModelMatrices();
		tm.draw(1);
		tn.draw(2);
		EndDrawing();
		HandleWindow();
	}
}

void Test5()
{
	LoadBCP("data.bcp");
	ReadLanguageFile();
	InitWindow();
	LoadGameSet("Warrior Kings Game Set\\small extensions.cpp");
	printf("Game set loaded!\n");

	printf("Declared items:\n");
	for(int i = 0; i < strItems.len; i++)
		printf(" - %s\n", strItems[i]);

	printf("Defined values:\n");
	for(int i = 0; i < defvalue.len; i++)
		printf(" - %s = %f\n", strDefValue[i], defvalue[i]);

	printf("Appearance tags:\n");
	for(int i = 0; i < strAppearTag.len; i++)
		printf(" - %s\n", strAppearTag[i]);

	printf("ObjDefs:\n");
	for(int i = 0; i < strObjDef.len; i++)
		printf(" - %s (%s)\n", strObjDef[i], CLASS_str[typeObjDef[i]]);

	printf("Looking at first object:\n");
	printf(" - Tooltip: %s\n", objdef[0].tooltip);
	printf(" - First start item: %f\n", objdef[0].startItems[0]);
	printf(" - Shadow: %i\n", objdef[0].shadowtype);

	printf("2nd's subtypes:\n");
	for(int i = 0; i < pstObjDef[1]->len; i++)
		printf(" - %s\n", (pstObjDef[1])->getdp(i));


	printf("1st's subtypes (%i):\n", pstObjDef[0]->len);
	for(int i = 0; i < pstObjDef[0]->len; i++)
		printf(" - %s\n", (pstObjDef[0])->getdp(i));

	printf("These ObjDefs have \"Default\" subtype and \"Default\" appearance:\n");
	for(int i = 0; i < strObjDef.len; i++)
		printf("- %s: %s\n", strObjDef[i], objdef[i].subtypes[0].appear[0]?"Yes":"No");

	getch();

	//InitMeshDrawing();
	int a = FindObjDef(CLASS_BUILDING, "Manor");
	while(!appexit)
	{
		BeginDrawing();
		BeginMeshDrawing();
		SetModelMatrices();
		objdef[a].subtypes[0].appear[0]->draw();
		EndDrawing();
		HandleWindow();
	}
}

char *t6fn[] = {"version.txt", "test.txt", "This Is New Data.txt",
	"Warrior Kings Game Set\\Buildings\\Church\\Church.cpp", "test\\abc.txt",
	"Languages\\..\\Animations_Are_Guaranteed_To_Merge.txt"};
void Test6()
{
	LoadBCP("data.bcp");
	for(int i = 0; i < 6; i++)
		printf("%s: %i\n", t6fn[i], FileExists(t6fn[i]));
}

#endif

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
	//__asm int 3
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

CObjectDefinition *AskObjDef(char *head = 0)
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
/*
		strcpy(ts, "?: ");
		//strcat(ts, (&e->value)->objdef->name);
		if(e->value.name)
		{
			int n = wcslen(e->value.name);
			char *w = new char[n+1];
			for(int i = 0; i < n; i++)
				w[i] = e->value.name[i];
			w[n] = 0;
			strcat(ts, w?w:"(null)");
			delete [] w;
		}
		ts[0] = '0' + (x++);
*/
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
			GameObject *a = AskPlayer("Set camera position to the manor of player:");
			if(a)
			{
				int c = strAssociateCat.find("My Initial Palace Building");
				if(c != -1)
				   for(DynListEntry<GOAssociation> *e = a->association.first; e; e = e->next)
					if(e->value.category == c)
					   if(e->value.associates.len)
						{camerapos = e->value.associates.first->value.o->position; break;}
				c = strItems.find("Palace Building");
				if(c != -1)
				{   for(int i = 0; i < MAX_GAMEOBJECTS; i++)
					if(gameobj[i])
					   if(gameobj[i]->getItem(c) != 0.0f)
						if(gameobj[i]->player == a)
						   {camerapos = gameobj[i]->position; break;}
				} else MessageBox(hWindow, "Cannot get player's manor.", appName, 16);
			}
		} break;
		case CMD_CAMPOS:
		{
			float x, z;
			if(PositionDlgBox(&x, &z, "Where would you like to go?"))
			{
				camerapos.x = x; camerapos.z = z;
			}
		} break;
		case CMD_ABOUT:
		{
			MessageBox(hWindow, "wkbre - WK (Battles) Recreated Engine\nVersion " WKBRE_VERSION "\n(C) 2015-2016 Adrien Geets\nThis program is released under the GNU GPL Version 3 license. For more information see the \"LICENSE\" file.\nRead the \"docs/help.htm\" file for help and more information.", appName, 64);
		} break;
		case CMD_RESETOBJPOS:
			ResetPosition(levelobj); break;
		case CMD_CAMDOWNLEFT:
			camerapos = Vector3(0, 0, 0); break;
		case CMD_CAMDOWNRIGHT:
			camerapos = Vector3((mapwidth-mapedge*2)*5, 0, 0); break;
		case CMD_CAMUPLEFT:
			camerapos = Vector3(0, 0, (mapheight-mapedge*2)*5); break;
		case CMD_CAMUPRIGHT:
			camerapos = Vector3((mapwidth-mapedge*2)*5, 0, (mapheight-mapedge*2)*5); break;
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
			camyaw = campitch = 0.0f; break;
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
			int r = ListDlgBox(&sl, "Copy camera position from whose client?", 0);
			if(r != -1)
			{
				ClientState *c = clistates.getpnt(r);
				camerapos = c->camerapos;
				camyaw = -c->cameraori.y;
				campitch = -c->cameraori.x;
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
			wkver = (wkver==WKVER_ORIGINAL) ? WKVER_BATTLES : WKVER_ORIGINAL;
			break;
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
					s->task.first->value.destinations.add();
					s->task.first->value.destinations.last->value.x = stdownpos.x;
					s->task.first->value.destinations.last->value.y = stdownpos.z;
				}
			}
		}
}

void Test7()
{
	LoadBCP("data.bcp"); ReadLanguageFile(); InitWindow(); InitScene(); InitFont(); InitCursor();
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

	//InitMenuBar();
	actualpage = new GEContainer;
	actualpage->buttonClick = T7ClickWindow;
	actualpage->buttonRightClick = T7RightClick;
	GEMenuBar *menubar = new GEMenuBar(menubarstr, menucmds, 4, actualpage);
	actualpage->add(menubar);
	menubar->setRect(0, 0, 32767, 20);
	menubar->bgColor = 0xC0000080;
	//onClickWindow = T7ClickWindow;
	InitGTWs();

	ChangeCursor(defcursor = LoadCursor("Interface\\C_DEFAULT.TGA"));

	InitOOBMList();

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
			NoTexture(0);
			if(actualpage) actualpage->draw(0, 0);
			if(statustext)
			{
				NoTexture(0);
				DrawRect(0, scrh-20 , scrw, 20, 0xC0000080);
				DrawFont(0, scrh-20, statustext);
			}
			if(enableObjTooltips) DrawTooltip();
//#ifndef WKBRE_RELEASE
		if(experimentalKeys)
		{	char st[256];
			sprintf(st, "current_time = %f", current_time);
			DrawFont(0, 80, st);
			if(selobjects.len)
			if(selobjects.first->value.valid())
			{
				int xhp = strItems.find("Hit Points"), xhc = strItems.find("Hit Point Capacity"), xf = strItems.find("Food"), xw = strItems.find("Wood"), xg = strItems.find("Gold"), xs = strItems.find("Stone");
				if((xf + xw + xg + xs) >= 0)
				{
					sprintf(st, "--- Selected object ---\nHP: %f/%f\nFood: %f\nWood: %f\nGold: %f\nStone: %f",
						selobjects.first->value->getItem(xhp),
						selobjects.first->value->getItem(xhc),
						selobjects.first->value->getItem(xf),
						selobjects.first->value->getItem(xw),
						selobjects.first->value->getItem(xg),
						selobjects.first->value->getItem(xs));
					NoTexture(0);
					DrawRect(0, 100, 280, 6*20, 0xC0000080);
					DrawFont(0, 100, st);
				}
			}
//#endif
		}
			SetTargetCursor();
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
		if(keypressed['9'])
			{keypressed['9'] = 0; CallCommand(CMD_BCMHIMAPRIGHT);}
		if(keypressed['0'])
			{keypressed['0'] = 0; CallCommand(CMD_BCMHIMAPLEFT);}
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
			menubar->enabled = menuVisible;}

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
			printf("%i material textures:\n", strMaterials.len);
			for(int i = 0; i < strMaterials.len; i++)
				printf(" - %s\n", strMaterials.getdp(i));
		}
#endif
		if(keypressed['A'])
		{
			keypressed['A'] = 0;
			CCommand *c = AskCommand("Which command do you want to execute on the selected objects (with no target)?");
			for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
				if(e->value.valid())
					ExecuteCommand(e->value.get(), c, 0, ORDERASSIGNMODE_FORGET_EVERYTHING_ELSE);
		}

		if(keypressed['Y'])
		{
			keypressed['Y'] = 0;
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
		}

		if(keypressed['X']) {keypressed['X'] = 0; CallCommand(CMD_RUNACTSEQ);}

		if(keypressed['C'])
		{
			keypressed['C'] = 0;
			int t = ListDlgBox(&strTypeTag, "Select a type tag.");
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
		}

		if(keypressed['V'])
		{
			keypressed['V'] = 0;
			GameObject *p = 0;
			for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
				if(e->value.valid())
					{p = e->value.get(); break;}
			if(p)
			{
				CObjectDefinition *od;
				if(od = AskObjDef("What do you want to stampdown (create + send the 'On Stampdown' event)?"))
				{
					GameObject *o = CreateObject(od, p);
					o->position = p->position;
					GOPosChanged(o);
					SequenceEnv ctx;
					SendGameEvent(&ctx, o, PDEVENT_ON_STAMPDOWN);
				}
			}
		}

		if(keypressed['Q']) {keypressed['Q'] = 0; CallCommand(CMD_CANCEL_ALL_OBJS_ORDERS);}

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

		if(keypressed['S'])
		{
			keypressed['S'] = 0;
			SendEventToObjAndSubord(levelobj, PDEVENT_ON_LEVEL_START);
		}

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

		if(keypressed['Z'])
		{
			keypressed['Z'] = 0;
			for(DynListEntry<DelayedSequenceEntry> *e = delayedSeq.first; e; e = e->next)
				delete [] e->value.obj;
			delayedSeq.clear();
			for(DynListEntry<SequenceOverPeriodEntry> *e = exePeriodSeq.first; e; e = e->next)
				delete [] e->value.ola;
			exePeriodSeq.clear();
			for(DynListEntry<SequenceOverPeriodEntry> *e = repPeriodSeq.first; e; e = e->next)
				delete [] e->value.ola;
			repPeriodSeq.clear();
		}

		if(keypressed['H'])
		{
			keypressed['H'] = 0;
			//for(int i = 0; i < strGameTextWindow.len; i++)
			//	EnableGTW(&(gsgametextwin[i]));
			int x = ListDlgBox(&strGameTextWindow, "Display this game text window:");
			if(x != -1)
				EnableGTW(&(gsgametextwin[x]));
		}

		if(keypressed['F']) {keypressed['F'] = 0; CallCommand(CMD_CONTROL_CLIENT);}

		if(keypressed['J'])
		{
			keypressed['J'] = 0;
			int x = ListDlgBox(&strGameEvent, "Which event do you want to send to the selected objects?");
			if(x != -1)
				for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
					if(e->value.valid())
						SendGameEvent(0, e->value.get(), x);
		}
	}
	}
}

#ifndef WKBRE_RELEASE
void Test8()
{
	LoadBCP("data.bcp"); ReadLanguageFile(); InitWindow();
	LoadGameSet("Warrior Kings Game Set\\multi-player extensions.cpp");
	printf("Game set loaded!\n");
	printf("Num. objdefs.: %i\n", strObjDef.len);
	for(int i = 0; i < strObjDef.len; i++)
		{printf(" - %s, ", strObjDef[i]);
		//fflush(stdout);
		printf("%s, %i\n", CLASS_str[objdef[i].type], objdef[i].renderable);}
	printf("Num. items: %i\n", strItems.len);
	for(int i = 0; i < strItems.len; i++)
		printf(" - %s\n", strItems.get(i));
	getch();
}

void Test9()
{
	//GrowStringList t;
	//t.add("Red"); t.add("Green"); t.add("Blue"); t.add("Yellow");
	//for(int i = 0; i < t.len; i++)
	//	printf("Color %i: %s\n", i, t.get(i));

	LoadBCP("data.bcp");
	GrowStringList *t = ListFiles("Warrior Kings Game Set\\Sounds\\narration"); //""); //"Interface\\Icons\\Special Icons\\");
	for(int i = 0; i < t->len; i++)
		printf("%04i: %s\n", i, t->getdp(i));

	int x = ListDlgBox(t);
	if(x != -1)
		printf("Selected: %i (%s)\n", x, t->getdp(x));

	delete t;
}

void Test10()
{
	LoadBCP("data.bcp"); ReadLanguageFile(); InitWindow();
	LoadGameSet("Warrior Kings Game Set\\multi-player extensions.cpp");
	CObjectDefinition *d = AskObjDef("Read starting item values of object definition:");
	if(!d) return;
	for(int i = 0; i < strItems.len; i++)
		if(d->startItems[i] != 0.0f)
			printf("%s = %f\n", strItems.getdp(i), d->startItems[i]);
}

void Test11()
{
	DynList<int> a, b;

	a.add(7); a.add(13); a.add(9); a.add(1);
	b.add(12); b.add(5); b.add(2); b.add(3);

	a.move(a.getEntry(2), &b);

	a.move(a.first, &b);
	a.move(a.first, &b);
	a.move(a.last, &b);

	b.move(b.first, &a);

	printf("A:");
	for(DynListEntry<int> *e = a.first; e; e = e->next)
		printf("\t%i", e->value);
	printf("\nB:");
	for(DynListEntry<int> *e = b.first; e; e = e->next)
		printf("\t%i", e->value);
	printf("\n");
}

void Test12()
{
	LoadBCP("data.bcp"); ReadLanguageFile(); InitWindow();
	LoadGameSet("Warrior Kings Game Set\\multi-player extensions.cpp");
	for(int i = 0; i < strDiplomaticStatus.len; i++)
	{
		printf("%i: %s", i, strDiplomaticStatus.getdp(i));
		printf((i == defaultDiploStat)?" (default)\n":"\n");
	}
	getch();
}

void Test13()
{
	char s[256];
	if(StrDlgBox(s))
		MessageBox(0, s, appName, 64);
}

void Test14()
{
	InitWindow();
	LoadBCP("data.bcp"); InitFont();
	ReadLanguageFile(); char *msg = GetLocText("AI_BIOG_PAUL_THE_SIMPLE");
	while(!appexit)
	{
		BeginDrawing();
		InitRectDrawing();
		//DrawRect(2, 2, 64, 64, 0xFF);
		//for(int i = 0; i < 40; i++)
		//	DrawFont(i*12, i*12, "Hello! \x21\x22\x7d\x7e\xa1\xa2\xa3");
		//DrawFont(0, 0, msg);
		deffont->DrawInRect(scrw, 0, 0, msg, 0xFF00FFFF);
		EndDrawing();
		HandleWindow();
	}
}

char *mbs[2] = {"B1", "B2"};
MenuEntry mbe[] = {
{"x1", 1},
{"x2", 2},
{"x3", 3},
{0,0},
{"y1", 11},
{"y2", 12},
{0,0},
};

void Test15()
{
	InitWindow(); LoadBCP("data.bcp"); InitFont();

/*
	GUIPage gpage;
	GUISimpleButton gbutton(2, 2, 64, 64, OnTestButtonClick);
	GUITextButton gtxtbutton(2, 80, 100, 16, OnTestButtonClick, "Hello!");
	gpage.add(&gbutton); gpage.add(&gtxtbutton);
	//actualpage = &gpage;
	InitMenuBar();
	menubar.parent = &gpage;
	actualpage = &menubar;
*/

	actualpage = new GEContainer;
	GEMenuBar *b = new GEMenuBar(mbs, mbe, 2, actualpage);
	actualpage->add(b);
	b->setRect(0, 0, 640, 20);

	GEWindow *win = new GEWindow;
	actualpage->add(win);
	win->setCaption("Test15");
	win->setRect(32, 32, 150, 150);
	win->onResize();
	win->bgColor = 0x80008000;

	GETextButton *t = new GETextButton;
	win->add(t);
	t->setRect(16, 36, 64, 64);
	t->setCaption("LOL!");

	while(!appexit)
	{
		BeginDrawing();
		InitRectDrawing();
		actualpage->draw(0, 0);
		EndDrawing();
		HandleWindow();
	}

}

void Test16()
{
	LoadBCP("data.bcp"); ReadLanguageFile(); InitWindow();
	LoadGameSet("Warrior Kings Game Set\\multi-player extensions.cpp");

	// Investigate the "invisible explosive cart" issue.

	int ecdef = FindObjDef(CLASS_CHARACTER, "Explosive Cart"); mustbefound(ecdef);
	printf("#%i\n", ecdef);
	//__asm int 3
}

void Test17()
{
	LoadBCP("data.bcp"); ReadLanguageFile(); InitWindow();
	LoadSaveGame("Save_Games\\rescript1.sav");

	SequenceEnv se; char inbuf[256];

	printf("Equations:\n");
	for(int i = 0; i < strEquation.len; i++)
		printf(" %i: %s\n", i+1, strEquation.getdp(i));

	while(1)
	{
		printf("\nEquation number: ");
		int x = atoi(gets(inbuf))-1;
		if(x == -1) break;
		if((uint)x >= strEquation.len)
			printf("Eq. number does not exist.\n");
		else if(!equation[x])
			printf("Equation is null.\n");
		else
			printf("Result: %f\n", equation[x]->get(&se));
	}
};

void Test18()
{
	LoadBCP("data.bcp"); ReadLanguageFile(); InitWindow();
	LoadSaveGame("Save_Games\\rescript1.sav");

	SequenceEnv se; char inbuf[256];
	goref gr; gr = FindObjID(3102);
	se.self = gr;

	printf("Object finder definitions:\n");
	for(int i = 0; i < strFinderDef.len; i++)
		printf(" %i: %s\n", i+1, strFinderDef.getdp(i));

	while(1)
	{
		printf("\nFinder def. number: ");
		int x = atoi(gets(inbuf))-1;
		if(x == -1) break;
		if((uint)x >= strFinderDef.len)
			printf("OFD number does not exist.\n");
		else if(!finderdef[x])
			printf("OFD is null.\n");
		else
		{
			printf("Result:");
			CFinder *f = finderdef[x]; GameObject *o;
			f->begin(&se);
			while(o = f->getnext()) printf(" %u", o->id);
			printf("\n");
		}
	}
};

void Test19()
{
	LoadBCP("data.bcp"); ReadLanguageFile(); InitWindow();
	LoadGameSet("Warrior Kings Game Set\\multi-player extensions.cpp");
	printf("--- GAME_EVENT ---\n");
	for(int i = 0; i < strGameEvent.len; i++)
		printf("%s\n", strGameEvent.getdp(i));
	printf("--- PACKAGE_RECEIPT_TRIGGER ---\n");
	for(int i = 0; i < strPRTrigger.len; i++)
		printf("%s\n", strPRTrigger.getdp(i));
	printf("--- Both ---\n");
	for(int i = 0; i < strGameEvent.len; i++)
		if(strPRTrigger.find(strGameEvent.getdp(i)) != -1)
			printf("%s\n", strGameEvent.getdp(i));
};

void Test20()
{
	playMode = 1;
	Test7();
}

void Test21()
{
	InitWindow();
	while(!appexit)
	{
		BeginDrawing();
		EndDrawing();
		HandleWindow();
		Sleep(1000/30);
	}
}

void Test22()
{
	int curgrp = 0; char tb[512];

	printf("Use LTTT? ");
	int uselt = atoi(gets(tb));

	LoadBCP("data.bcp");
	InitWindow();
	InitFont();

	if(uselt) LoadLTTT("Maps\\Map_Textures\\128_4Peaks.lttt");
	LoadMapTextures();

	//MessageBox(hWindow, "Terrain textures loaded.", appName, 64);
	printf("%u groups:\n", maptexgroup.len);
	for(int i = 0; i < maptexgroup.len; i++)
	{
		MapTextureGroup *g = maptexgroup.getpnt(i);
		printf(" * %s\n", g->name);
		printf("   %u textures:\n", g->tex->len);
		for(int j = 0; j < g->tex->len; j++)
			printf("    - %u\n", g->tex->getpnt(j)->id);
	}
	//texture t = GetTexture("Warrior Kings Game Set\\Textures\\Tavern.pcx");
	while(!appexit)
	{
		BeginDrawing();
		InitRectDrawing();

		//SetTexture(0, maptexgroup.getpnt(0)->tex->getpnt(0)->t);
		//SetTexture(0, t);
		//DrawRect(0, 0, 256, 256, -1);

		MapTextureGroup *g = maptexgroup.getpnt(curgrp);
		sprintf(tb, "%s (%u/%u)", g->name, curgrp, maptexgroup.len);
		DrawFont(0, 0, tb);
		for(int i = 0; i < g->tex->len; i++)
		{
			MapTexture *t = g->tex->getpnt(i);
			SetTexture(0, t->t);
			DrawRect(i * 65, 32, 64, 64, -1, t->x / 256.f, t->y / 256.f, t->w / 256.f, t->h / 256.f);
			sprintf(tb, "%u", t->id);
			DrawFont(i * 65, 96, tb);
		}

		if(keypressed[VK_LEFT])
		{
			keypressed[VK_LEFT] = 0;
			if(curgrp > 0) curgrp--;
		}
		if(keypressed[VK_RIGHT])
		{
			keypressed[VK_RIGHT] = 0;
			if(curgrp < maptexgroup.len-1) curgrp++;
		}

		EndDrawing();
		HandleWindow();
	}
}

int AskMap(char *o, char *hs = 0)
{
	GrowStringList *sl = ListDirectories("Maps");
	int x = ListDlgBox(sl, hs ? hs : "Select a map.", 0);
	if(x == -1) return 0;
	strcpy(o, "Maps\\");
	strcat(o, sl->getdp(x));
	strcat(o, "\\");
	strcat(o, sl->getdp(x));
	int e = strlen(o);
	strcpy(o+e, ".bcm");
	if(FileExists(o)) return 1;
	strcpy(o+e, ".snr");
	if(FileExists(o)) return 1;
	MessageBox(hWindow, "The selected map directory does not contain a BCM nor a SNR file.", appName, 48);
	return 0;
}

void Test23()
{
	int px = 0, pz = 0, ps = 64;

	LoadBCP("data.bcp");
	ReadLanguageFile();
	InitWindow();
	InitFont();
	//LoadSaveGame("Save_Games\\Advanced Economy.lvl");
	//LoadMap("Maps\\oritest.bcm");
	char mn[512];
	if(!AskMap(mn, "Select a map you want to view.")) return;
	LoadMap(mn);

	DynList<MapTile*> *ttlist = new DynList<MapTile*>[maptexfilenames.len];
	for(int z = 0; z < mapheight; z++)
	for(int x = 0; x < mapwidth; x++)
	{
		MapTile *t = &maptiles[z*mapwidth+x];
		ttlist[t->mt->tfid].add(t);
	}

	while(!appexit)
	{
		BeginDrawing();
		InitRectDrawing();

		//RBatch *batch = renderer->CreateBatch(1024, 1536);
		RBatch *batch = renderer->CreateBatch(2*8192, 3*8192);
		renderer->BeginBatchDrawing();
		Matrix mx; CreateZeroMatrix(&mx);
		mx._11 = 2.0f / scrw; mx._22 = -2.0f / scrh;
		mx._41 = -1 - (0.5f / (scrw)); mx._42 = 1 + (0.5f / (scrh));
		//mx._41 = -1; mx._42 = 1;
		mx._44 = 1;
		renderer->SetTransformMatrix(&mx);

		for(int i = 0; i < maptexfilenames.len; i++)
		{
			int tns = 1;
			for(DynListEntry<MapTile*> *e = ttlist[i].first; e; e = e->next)
			{
				MapTile *c = e->value;
				int dx = (c->x - px) * ps, dy = (c->z - pz) * ps;
				if(((uint)dx >= scrw) || ((uint)dy >= scrh)) continue;
				MapTexture *t = c->mt;
				if(tns) {SetTexture(0, t->t); tns = 0;}

				batchVertex *bv; ushort *bi; uint fi;
				batch->next(4, 6, &bv, &bi, &fi);
				int a = c->rot, f = ((c->zflip&1)?1:0) ^ ((c->xflip&1)?3:0);

				bv[0].x = dx;			bv[0].y = dy;
				bv[1].x = dx + ps;		bv[1].y = dy;
				bv[2].x = dx + ps;		bv[2].y = dy + ps;
				bv[3].x = dx;			bv[3].y = dy + ps;

				bv[((0^f)+a)&3].u = t->x / 256.f;		bv[((0^f)+a)&3].v = t->y / 256.f;
				bv[((1^f)+a)&3].u = (t->x+t->w) / 256.f;	bv[((1^f)+a)&3].v = t->y / 256.f;
				bv[((2^f)+a)&3].u = (t->x+t->w) / 256.f;	bv[((2^f)+a)&3].v = (t->y+t->h) / 256.f;
				bv[((3^f)+a)&3].u = t->x / 256.f;		bv[((3^f)+a)&3].v = (t->y+t->h) / 256.f;

				for(int j = 0; j < 4; j++)
					{bv[j].z = 0; bv[j].color = -1;
					}//bv[j].x -= 0.5f; bv[j].y -= 0.5f;}
					 // L-> this will drop FPS because it rereads the locked buffer!
				bi[0] = fi+0; bi[1] = fi+1; bi[2] = fi+3;
				bi[3] = fi+1; bi[4] = fi+3; bi[5] = fi+2;
			}
			batch->flush();
		}
		delete batch;

		if(keypressed[VK_LEFT])
			{keypressed[VK_LEFT] = 0; px--;}
		if(keypressed[VK_RIGHT])
			{keypressed[VK_RIGHT] = 0; px++;}
		if(keypressed[VK_UP])
			{keypressed[VK_UP] = 0; pz--;}
		if(keypressed[VK_DOWN])
			{keypressed[VK_DOWN] = 0; pz++;}
		if(keypressed[VK_MULTIPLY])
			{keypressed[VK_MULTIPLY] = 0; ps <<= 1;}
		if(keypressed[VK_DIVIDE])
			{keypressed[VK_DIVIDE] = 0; ps >>= 1;}
		if(ps <= 0) ps = 1;

		EndDrawing();
		HandleWindow();
	}
}

#define NUMTESTS 23
void (*tt[NUMTESTS])() = {Test1, Test2, Test3, Test4, Test5, Test6, Test7,
 Test8, Test9, Test10, Test11, Test12, Test13, Test14, Test15, Test16, Test17,
 Test18, Test19, Test20, Test21, Test22, Test23};

#endif

#ifndef WKBRE_RELEASE
int main(int argc, char *argv[])
#else
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, char *args, int showMode)
#endif
{
	srand(time(NULL));
	hInstance = GetModuleHandle(NULL); ReadSettingsFile(); //SetGameDir();
	//if(argc >= 2) farg = argv[1];
	//InitD3DXDLL();
#ifndef WKBRE_RELEASE
	printf("Enter test number (from 1 to %i): ", NUMTESTS);
	char s[64];
	uint n = 0;
	n = atoi(gets(s));
	if((n > NUMTESTS) || !n) n = 7;
	tt[n-1]();
#else
	Test7();
#endif
	return 0;
}