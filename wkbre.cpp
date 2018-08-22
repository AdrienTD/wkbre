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
#include <direct.h>
#include "imgui/imgui.h"
#include <functional>
#include <shellapi.h>

HINSTANCE hInstance;
GEContainer *actualpage = 0;
int appexit = 0;
float walkstep = 1.0f, camwalkstep = 1.0f;
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
bool swSelObj = 0, swLevTree = 0, swLevInfo = 0, swObjCrea = 0, swAbout = 0, swMapEditor = 0, swCityCreator = 0, swTest = 0, swMinimap = 0;
bool mapeditmode = 0;
int mousetool = 0;
bool playAfterLoading = 0;
texture minimapTexture; Bitmap *minimapBitmap;

#ifdef WKBRE_RELEASE
int experimentalKeys = 1;
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
{"Rotate selected objects by 90 deg", CMD_ROTATEOBJQP},
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
{"Terrain editor", CMD_SWMAPEDITOR},
{"City creator", CMD_SWCITYCREATOR},
{"Minimap", CMD_SWMINIMAP},
{0,0},
{"About...", CMD_ABOUT},
{0,0},
};

int orderButtX[6] = {	6,	37,	69,	93, 	100,	90	};
int orderButtY[6] = {	364,	354,	361,	384,	416,	448	};

CObjectDefinition *objtypeToStampdown = 0;
bool eventAfterStampdown = 0;
goref playerToGiveStampdownObj;
float stampdownRot = 0;

MapTextureGroup *curtexgrp = 0; MapTexture *curtex = 0;
int men_rot = 0; bool men_xflip = 0, men_zflip = 0;
boolean mousetoolpress_l = 0, mousetoolpress_r = 0;
int brushsize = 1, brushshape = 0; bool randommaptex = 0, randommaptiletransform = 0;
bool himapautowater = 0;
goref newmanorplayer; int nmc_size = 3; int nmc_npeasants = 4; bool nmc_flags = 0;

// Minimap drawing...

Bitmap *doom_bmp; int doom_w, doom_h; bool doom_edge;
int doom_mmw, doom_mmh, doom_sx, doom_sy;
extern int colortable[8];

void DrawObjOnMinimapBmp(GameObject *o)
{
	float fx = o->position.x / 5;
	float fy = o->position.z / 5;
	if(doom_edge) {fx += mapedge; fy += mapedge;}
	int x = doom_sx + floor(fx * doom_mmw / doom_w);
	int y = doom_bmp->h - 1 - doom_sy - floor(fy * doom_mmh / doom_h);
	if(x >= 0 && x < doom_bmp->w && y >= 0 && y < doom_bmp->h)
	{
		int c = colortable[o->color];
		//c = ((c&0xFF0000)>>16) | (c&0xFF00) | ((c&0xFF)<<16);
		((uint*)doom_bmp->pix)[y*doom_bmp->w + x] = c | 0xFF000000;
	}
	for(DynListEntry<GameObject> *e = o->children.first; e; e = e->next)
		DrawObjOnMinimapBmp(&e->value);
}

void DrawObjectsOnMinimapBmp(Bitmap *bmp, bool edge)
{
	doom_bmp = bmp;
	doom_edge = edge;
	doom_w = edge ? mapwidth : (mapwidth - 2 * mapedge);
	doom_h = edge ? mapheight : (mapheight - 2 * mapedge);
	if(doom_w > doom_h) {doom_mmw = bmp->w; doom_mmh = doom_h * bmp->w / doom_w;}
	else                {doom_mmh = bmp->h; doom_mmw = doom_w * bmp->h / doom_h;}
	doom_sx = bmp->w/2 - doom_mmw/2;
	doom_sy = bmp->h/2 - doom_mmh/2;
	DrawObjOnMinimapBmp(levelobj);
}

// ImGui dialog boxes replacing Win32-based ones.

bool IGGSLItemsGetter(void *data, int idx, const char **out);

int ndlgmode = 0; // 0=closed, 1=Listbox, 2=String
char *ndlgheader = 0;
GrowStringList *nlstdlggsl = 0; //int nlstdlgfirstsel = 0;
char *nstrdlgout = 0;
std::function<void()> dlgOkPress = []() {printf("OK pressed.\n");};
bool ndlgfirsttime = false;

int nlstdlgsel = 0;

void OpenListDlgBox(GrowStringList *gsl, char *hs, int fsel, std::function<void()> f)
{
	ndlgheader = hs ? hs : "Select something from this list.";
	nlstdlggsl = gsl;
	nlstdlgsel = fsel;
	ndlgmode = 1;
	ndlgfirsttime = true;
	dlgOkPress = f;
}

void OpenStrDlgBox(char *out, char *hs, std::function<void()> f)
{
	ndlgheader = hs ? hs : "Type a string.";
	nstrdlgout = out;
	ndlgmode = 2;
	ndlgfirsttime = true;
	dlgOkPress = f;
}

void IGNDlgBox()
{
	bool q = true, ok = false;
	static ImGuiTextFilter filter;
	if(!ndlgmode) return;
	if(ndlgfirsttime)
	{
		ImGui::SetNextWindowSize(ImVec2(400,(ndlgmode==2)?100:300), ImGuiSetCond_Always);
		ImGui::SetNextWindowPosCenter(ImGuiSetCond_Always);
		filter.Clear();
	}
	if(!ImGui::Begin("Prompt##NDlgBox", &q)) {ImGui::End(); return;}
	ImGui::PushItemWidth(-1);
	if(!q) ndlgmode = 0;
	ImGui::TextWrapped(ndlgheader);
	switch(ndlgmode)
	{
		case 1:
			if(ndlgfirsttime) ImGui::SetKeyboardFocusHere();
			filter.Draw();
			ImGui::ListBoxHeader("##ListBox", ImVec2(0,200));
			for(int i = 0; i < nlstdlggsl->len; i++)
				if(filter.PassFilter(nlstdlggsl->getdp(i)))
				{
					if(ImGui::Selectable(nlstdlggsl->getdp(i), nlstdlgsel == i))
						nlstdlgsel = i;
					if(ImGui::IsItemHovered())
						if(ImGui::IsMouseDoubleClicked(0))
							{nlstdlgsel = i;
							ok = 1;}
				}
			ImGui::ListBoxFooter();
			break;
		case 2:
			if(ndlgfirsttime) ImGui::SetKeyboardFocusHere();
			if(ImGui::InputText("##InputText", nstrdlgout, 80, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
				ok = 1;
			break;
	}
	if(ImGui::Button("OK"))
		ok = 1;
	ImGui::SameLine();
	if(ImGui::Button("Cancel") || ImGui::IsKeyDown(VK_ESCAPE))
		ndlgmode = 0;
	ImGui::PopItemWidth();
	ImGui::End();
	ndlgfirsttime = 0;
	if(ok)
		{ndlgmode = 0; dlgOkPress();}
}

bool askBeforeExit = 0;

void QuitApp()
{
	if(askBeforeExit)
		if(MessageBox(hWindow, "Do you really want to quit wkbre?\nUnsaved changes will be lost!", appName, 52) != IDYES)
			return;
	exit(0);
}

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

void NAskObjDef(char *head, std::function<void(CObjectDefinition*)> f)
{
	static GrowStringList l; char ts[256];
	l.clear();
	for(int i = 0; i < strObjDef.len; i++)
	{
		if((objdef[i].type == 0) || (objdef[i].name == 0))
			{l.add("?"); continue;}
		strcpy(ts, CLASS_str[objdef[i].type]);
		strcat(ts, " ");
		strcat(ts, objdef[i].name);
		l.add(ts);
	}
	//static std::function<void(CObjectDefinition*)> f = g;
	OpenListDlgBox(&l, head?head:"Select an object definition.", 0, 
		   [f]() {
			CObjectDefinition *cod;
			if(nlstdlgsel == -1) cod = 0;
			else cod = &objdef[nlstdlgsel];
			f(cod);
		   }
		);
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

void NAskPlayer(char *head, std::function<void(GameObject*)> f)
{
	static GrowStringList l; char ts[512]; int x = 0;
	l.clear();
	for(DynListEntry<GameObject> *e = levelobj->children.first; e; e = e->next)
	{
		if(e->value.objdef->type != CLASS_PLAYER) break; // TODO: Something better.
		sprintf(ts, "%u: %S (obj. id %u)", x++, e->value.name ? e->value.name : L"(null)", e->value.id);
		l.add(ts);
	}
	OpenListDlgBox(&l, head?head:"Select a player.", (l.len >= 2) ? 1 : 0,
		[f]() {
			if(nlstdlgsel != -1)
				f(&levelobj->children.getEntry(nlstdlgsel)->value);
		});

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
		GOPosChanged(o, 0, 1);
		//o->position.y = GetHeight(o->position.x, o->position.z);
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
char *notificationtext = 0; uint notiftimeref, notifdelay;

void GiveNotification(char *str, int delay = 3000)
{
	notificationtext = strdup(str);
	notiftimeref = GetTickCount();
	notifdelay = delay;
}

void GiveSpeedNotif()
{
	char t[128];
	sprintf_s(t, 127, "Game speed: %g", game_speed);
	GiveNotification(t);
}

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
			if(!strlen(lastmap))
				{MessageBox(hWindow, "The savegame is not linked to a terrain file.\nEither save the current terrain in \"Window > Terrain editor\",\nor set the path of the terrain file in \"Window > Level information > Properties > Map\", then try again.", appName, 48);
				break;}
		/*
			char s[256];
			if(StrDlgBox(s, "Type the name of the new save game. It will be placed in \"saved\\Save_Games\" in the game directory. The name must end with either \".sav\" or \".lvl\"."))
		*/
			static char s[400];
			strcpy(s, lastsavegamename);
			strcat(s, isLevelStarted ? ".sav" : ".lvl");
			auto f = []()
			{
				char t[1024] = "Save_Games\\\0";
				strcat(t, s);
				if(FileExists(t))
					if(MessageBox(hWindow, "The file name already exists. Do you want to replace/overwrite this file?", appName, 48 | MB_YESNO) != IDYES)
						return; //break;
				strcpy(t, gamedir);
				strcat(t, "\\saved\\Save_Games\\");
				strcat(t, s);
				if (SaveSaveGame(t))
					GiveNotification("Savegame saved!");
				else
					MessageBox(hWindow, "wkbre was not able to create the file for your savegame.\n\nBe sure that the filename doesn't contain special characters and that the \"saved\\Save_Games\" folder exists and is not write-protected.", appName, 48);
			};
			OpenStrDlgBox(s, "Save level (.lvl) or savegame (.sav) as:", f);
		} break;
		case CMD_DELETEOBJTYPE:
		{
		/*
			CObjectDefinition *d = AskObjDef("Select the type of objects you want to delete.");
			if(d)
				RemoveObjOfType(levelobj, d); //&objdef[FindObjDef(CLASS_CHARACTER, "Peasant")]);
		*/
			NAskObjDef("Select the type of objects you want to delete.",
				[](CObjectDefinition *d) {
					RemoveObjOfType(levelobj, d);
				});
		} break;
/*
		case CMD_CONVERTOBJTYPE:
		{
			CObjectDefinition *a, *b;
			if(a = AskObjDef("What type of object do you want to convert?")) if(b = AskObjDef("Which object type should they be converted to?"))
				ReplaceObjsType(levelobj, a, b);
		} break;
*/
		case CMD_CONVERTOBJTYPE:
		{
			NAskObjDef("What type of object do you want to convert?", 
				[](CObjectDefinition *a) {
					NAskObjDef("Which object type should they be converted to?",
						[a](CObjectDefinition *b) {
							ReplaceObjsType(levelobj, a, b);
						});
				});
		} break;
		case CMD_GIVEOBJTYPE:
		{
		/*
			CObjectDefinition *d; GameObject *p;
			if(d = AskObjDef("What type of objects do you want to give?")) if(p = AskPlayer("Give the objects to:"))
				SetObjsCtrl(levelobj, d, p);
		*/
			NAskObjDef("What type of objects do you want to give?",
				[](CObjectDefinition *d) {
					NAskPlayer("Give the objects to:",
						[d](GameObject *p) {
							SetObjsCtrl(levelobj, d, p);
						});
				});
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
				if(!fnd) GiveNotification("The player's manor was not found."); //MessageBox(hWindow, "The player's manor was not found.", appName, 16);
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
/*		case CMD_BCMHIMAPRIGHT:
			LoadMap(lastmap, ++bo); break;
		case CMD_BCMHIMAPLEFT:
			LoadMap(lastmap, --bo); break;
*/
		case CMD_SHOWHIDELANDSCAPE:
			enableMap = !enableMap; break;
		case CMD_QUIT:
			QuitApp(); break;

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
			if( (objtypeToStampdown && playerToGiveStampdownObj.valid()) ||
			    (mousetool == 7 && newmanorplayer.valid()) )
			{
				stampdownRot += M_PI / 2;
				if(stampdownRot >= 2*M_PI)
					stampdownRot -= M_PI * 2;
				break;
			}
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
			game_speed *= 2.0f; GiveSpeedNotif(); break;
		case CMD_GAME_SPEED_SLOWER:
			game_speed /= 2.0f; GiveSpeedNotif(); break;
		case CMD_CANCEL_ALL_OBJS_ORDERS:
			CancelAllObjsOrders(levelobj); break;
		case CMD_SELECT_OBJECT_ID:
		{
/*
			char s[256];
			if(StrDlgBox(s, "Enter the ID of the object you'd like to select."))
			{
				GameObject *o = FindObjID(atoi(s));
				if(!o) GiveNotification("Object with specified ID not found."); //MessageBox(hWindow, "Object with specified ID not found.", appName, 16);
				else {DeselectAll(); SelectObject(o);}
			}
*/
			static char s[256];
			auto f = []() {
				GameObject *o = FindObjID(atoi(s));
				if(!o) GiveNotification("Object with specified ID not found."); //MessageBox(hWindow, "Object with specified ID not found.", appName, 16);
				else {DeselectAll(); SelectObject(o);}
			};
			OpenStrDlgBox(s, "Enter the ID of the object you'd like to select.", f);
		} break;
		case CMD_CHANGE_SG_WKVER:
		{
			GrowStringList sl;
			sl.add("Warrior Kings"); sl.add("Warrior Kings - Battles");
			int r = ListDlgBox(&sl, "Make next saves compatible with:", (sg_ver==WKVER_BATTLES)?1:0);
			if(r != -1)
				sg_ver = r ? WKVER_BATTLES : WKVER_ORIGINAL;
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
			objmovalign = !objmovalign;
			GiveNotification(objmovalign ? "Aligned object movement ON." : "Aligned object movement OFF.");
			break;
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
			isLevelStarted = 1;
			GiveNotification("Level started.");
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
		case CMD_SWMAPEDITOR:
			swMapEditor = !swMapEditor; break;
		case CMD_SWCITYCREATOR:
			swCityCreator = !swCityCreator; break;
		case CMD_SWMINIMAP:
			swMinimap = !swMinimap; break;
		case CMD_SWOPENALL:
			swSelObj = swLevInfo = swLevTree = swObjCrea = swMapEditor = swCityCreator = swMinimap = 1; break;
		case CMD_SWCLOSEALL:
			swSelObj = swLevInfo = swLevTree = swObjCrea = swMapEditor = swCityCreator = swMinimap = 0; break;
		case CMD_TOGGLEGRID:
			showMapGrid = !showMapGrid; break;
		case CMD_HELP:
			ShellExecute(hWindow, "open", "help.htm", NULL, NULL, SW_SHOWNORMAL);
			break;
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

void CopyHeightmap()
{
	if(!OpenClipboard(0)) return;
	EmptyClipboard();

	int pitch = ((mapwidth+1)&3) ? (((mapwidth+1)&(~3))+4) : (mapwidth+1);
	int rppl = pitch - (mapwidth + 1);
	HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + 4*256 + pitch*(mapheight+1));
	BITMAPINFO *bi = (BITMAPINFO*)GlobalLock(hg);
	BITMAPINFOHEADER &head = bi->bmiHeader;
	int *pal = (int*)&bi->bmiColors;
	char *bmp = (char*)bi + sizeof(BITMAPINFOHEADER) + 4*256;

	head.biSize = sizeof(BITMAPINFOHEADER);
	head.biWidth = mapwidth+1;
	head.biHeight = mapheight+1;
	head.biPlanes = 1;
	head.biBitCount = 8;
	head.biCompression = BI_RGB;
	head.biSizeImage = 0;
	head.biXPelsPerMeter = 0;
	head.biYPelsPerMeter = 0;
	head.biClrUsed = 256; // GIMP hates 0. Also determining biClrUsed automatically can be annoying.
	head.biClrImportant = 0;
	for(int i = 0; i < 256; i++)
		pal[i] = i | (i << 8) | (i << 16);
	for(int y = 0; y <= mapheight; y++)
	{
		memcpy(bmp + y*pitch, himap_byte + (mapheight-y)*(mapwidth+1), mapwidth+1);
		memset(bmp + y*pitch + mapwidth+1, 0, rppl);
	}

	GlobalUnlock(hg);
	SetClipboardData(CF_DIB, hg);
	CloseClipboard();
}

void PasteHeightmap()
{
	OpenClipboard(0);
	HGLOBAL hg = GetClipboardData(CF_DIB);
	if(!hg)
	{
		MessageBox(hWindow, "Could not obtain clipboard data. Be sure you copied a bitmap.", appName, 48);
		CloseClipboard(); return;
	}
	BITMAPINFO *bi = (BITMAPINFO*)GlobalLock(hg);
	BITMAPINFOHEADER &head = bi->bmiHeader;
	if((head.biWidth != mapwidth+1) || (head.biHeight != mapheight+1))
		MessageBox(hWindow, "The bitmap to be pasted does not match map size.", appName, 48);
	else
	{
		if((head.biCompression != BI_RGB) ||
			( (head.biBitCount != 8) &&
			  (head.biBitCount != 24) &&
			  (head.biBitCount != 32) ))
			MessageBox(hWindow, "Clipboard bitmap's format/compression is not supported.", appName, 48);
		else
		{
			int pitch;
			int biClrUsed = head.biClrUsed;
			if(!biClrUsed) if(head.biBitCount == 8) biClrUsed = 256;
			uchar *bmp = (uchar*)bi + head.biSize + 4 * biClrUsed;
			int *pal = (int*)((char*)bi + head.biSize);
			switch(head.biBitCount)
			{
				case 8:
					pitch = ((mapwidth+1)&3) ? (((mapwidth+1)&(~3))+4) : (mapwidth+1);
					for(int y = 0; y <= mapheight; y++)
					for(int x = 0; x <= mapwidth; x++)
					{
						uchar c = bmp[y*pitch+x];
						uint p = pal[c];
						uint m = ((p&255) + ((p>>8)&255) + ((p>>16)&255)) / 3;
						himap_byte[(mapheight-y)*(mapwidth+1)+x] = m;
					}
					break;
				case 24:
					pitch = (((mapwidth+1)*3)&3) ? ((((mapwidth+1)*3)&(~3))+4) : ((mapwidth+1)*3);
					for(int y = 0; y <= mapheight; y++)
					for(int x = 0; x <= mapwidth; x++)
					{
						uchar c1 = bmp[y*pitch+x*3+0];
						uchar c2 = bmp[y*pitch+x*3+1];
						uchar c3 = bmp[y*pitch+x*3+2];
						himap_byte[(mapheight-y)*(mapwidth+1)+x] = (c1 + c2 + c3) / 3;
					}
					break;
				case 32:
					pitch = (mapwidth+1) * 4;
					for(int y = 0; y <= mapheight; y++)
					for(int x = 0; x <= mapwidth; x++)
					{
						uint c = bmp[y*pitch+x*4];
						uint m = ((c&255) + ((c>>8)&255) + ((c>>16)&255)) / 3;
						himap_byte[(mapheight-y)*(mapwidth+1)+x] = m;
					}
					break;
			}
			for(int i = 0; i < (mapwidth+1)*(mapheight+1); i++)
				himap[i] = (float)himap_byte[i] * maphiscale;
			if(himapautowater) FloodfillWater();
		}
	}
	GlobalUnlock(hg);
	CloseClipboard();
}

void ChangeTileTexture(MapTile *tile, MapTexture *newtex)
{
	MapTexture *oldtex = tile->mt;
	tile->mt = newtex;
	tile->pl[oldtex->tfid].move(tile->ple, &(tile->pl[newtex->tfid]));
}

void DoXFlip(MapTile *t)
{
	if(t->rot & 1) t->zflip ^= 1;
	else t->xflip ^= 1;
}

void DoZFlip(MapTile *t)
{
	if(t->rot & 1) t->xflip ^= 1;
	else t->zflip ^= 1;
}

void FindAutotileC(int tx, int tz, boolean we, boolean star, MapTextureGroup *ag) // we=0:NORTH-SOUTH, we=1:WEST-EAST
{
	if(we) {if((tx <= 0) || (tx >= mapwidth -1)) return;}
	else   {if((tz <= 0) || (tz >= mapheight-1)) return;}
	MapTile *mto = &(maptiles[tz*mapwidth+tx]);
	if(mto->mt->grp == ag) return;
	MapTile *mtn = &(maptiles[we ? (tz*mapwidth+tx-1) : ((tz-1)*mapwidth+tx)]);
	MapTile *mts = &(maptiles[we ? (tz*mapwidth+tx+1) : ((tz+1)*mapwidth+tx)]);
	MapTile *oriref = star ? mts : mtn;
	GrowList<MapTextureEdge> *gl0 = &mtn->mt->atdir[we ? MAPTEXDIR_EAST : MAPTEXDIR_SOUTH];
	GrowList<MapTextureEdge> *gl1 = &mts->mt->atdir[we ? MAPTEXDIR_WEST : MAPTEXDIR_NORTH];
	GrowList<MapTextureEdge*> reslist;
	for(int i = 0; i < gl0->len; i++)
	{
		MapTextureEdge *e0 = gl0->getpnt(i);
		for(int j = 0; j < gl1->len; j++)
		{
			MapTextureEdge *e1 = gl1->getpnt(j);
			if(!memcmp(e0, e1, sizeof(MapTextureEdge)))
				reslist.add(e0);
		}
	}
	if(reslist.len)
	{
		MapTextureEdge *e = reslist[rand() % reslist.len];
		ChangeTileTexture(mto, e->tex);

		MapTile oc = *oriref;
		mto->xflip = e->xflip;
		mto->zflip = e->zflip;
		mto->rot = e->rot;
		if(oc.rot >= 2) {oc.rot -= 2; oc.xflip = !oc.xflip; oc.zflip = !oc.zflip;}
		if(oc.zflip) DoZFlip(mto);
		mto->rot = (mto->rot + oc.rot) & 3;
	}
}

GrowList<MapTextureEdge> *GetMatchingTilesList(int tx, int tz, int edge)
{
	//if (tx < 0 || tx >= mapwidth || tz < 0 || tz >= mapheight)
	//	return new GrowList<MapTextureEdge>; // empty list

	if (tx < 0) tx = 0; if (tx >= mapwidth) tx = mapwidth - 1;
	if (tz < 0) tz = 0; if (tz >= mapheight) tz = mapheight - 1;

	MapTile *tile = &(maptiles[tz * mapwidth + tx]);
	int te = edge;
	if (tile->xflip) if (!(te & 1)) te ^= 2; // if north or south then swith to south or north respectively
	if (tile->zflip) if (te & 1) te ^= 2; // if east or west then swith to west or east respectively
	if (tile->xflip ^ tile->zflip) te += tile->rot; else te -= tile->rot;
	GrowList<MapTextureEdge> *texedgelist = &tile->mt->atdir[te & 3];
	GrowList<MapTextureEdge> *mtlist = new GrowList<MapTextureEdge>;
	for (int i = 0; i < texedgelist->len; i++)
	{
		MapTextureEdge *e = mtlist->addp();
		*e = *texedgelist->getpnt(i);
		e->rot += tile->rot;
		e->xflip ^= tile->xflip;
		e->zflip ^= tile->zflip;
		e->rot &= 3;
		if (e->rot & 2)
		{
			e->rot &= 1;
			e->xflip ^= 1;
			e->zflip ^= 1;
		}
	}
	return mtlist;
}

void FindAutotileWE(int tx, int tz)
{
	GrowList<MapTextureEdge> *m, *n, *r;
	m = GetMatchingTilesList(tx - 1, tz, MAPTEXDIR_EAST);
	n = GetMatchingTilesList(tx + 1, tz, MAPTEXDIR_WEST);
	r = new GrowList<MapTextureEdge>;
	for (int i = 0; i < m->len; i++)
	{
		MapTextureEdge *a = m->getpnt(i);
		for (int j = 0; j < n->len; j++)
		{
			MapTextureEdge *b = n->getpnt(j);
			if (!memcmp(a, b, sizeof(MapTextureEdge)))
			{
				MapTextureEdge *c = r->addp();
				*c = *a;
			}
		}
	}
	delete m, n;
	if (r->len > 0)
	{
		int c = rand() % r->len;
		MapTextureEdge *e = r->getpnt(c);
		MapTile *tile = &(maptiles[tz * mapwidth + tx]);
		ChangeTileTexture(tile, e->tex);
		tile->rot = e->rot;
		tile->xflip = e->xflip;
		tile->zflip = e->zflip;
	}
	delete r;
}

void FindAutotileF(int tx, int tz, bool edge_n, bool edge_e, bool edge_s, bool edge_w, MapTextureGroup *grp = 0)
{
	if (tx < 0 || tx >= mapwidth || tz < 0 || tz >= mapheight)
		return;

	int nedges = (edge_n ? 1 : 0) + (edge_e ? 1 : 0) + (edge_s ? 1 : 0) + (edge_w ? 1 : 0);
	GrowList<MapTextureEdge> *m[4], *r;
	int k = 0;
	if (edge_n) m[k++] = GetMatchingTilesList(tx, tz - 1, MAPTEXDIR_SOUTH);
	if (edge_e) m[k++] = GetMatchingTilesList(tx + 1, tz, MAPTEXDIR_WEST);
	if (edge_s) m[k++] = GetMatchingTilesList(tx, tz + 1, MAPTEXDIR_NORTH);
	if (edge_w) m[k++] = GetMatchingTilesList(tx - 1, tz, MAPTEXDIR_EAST);
	if (k == 0) return;
	r = new GrowList<MapTextureEdge>;
	// Place smallest list in first position
	int smlist = 0, smlen = m[0]->len;
	for (int i = 0; i < nedges; i++)
		if (m[i]->len < smlen)
		{
			smlist = i; smlen = m[i]->len;
		}
	auto t = m[0];
	m[0] = m[smlist];
	m[smlist] = t;
	for (int i = 0; i < m[0]->len; i++)
	{
		MapTextureEdge *a = m[0]->getpnt(i);
		bool inalledges = true;
		for (int n = 1; n < nedges; n++)
		{
			bool fnd = false;
			for (int j = 0; j < m[n]->len; j++)
			{
				MapTextureEdge *b = m[n]->getpnt(j);
				if (!memcmp(a, b, sizeof(MapTextureEdge)))
				{
					fnd = true; break;
				}
			}
			inalledges = inalledges && fnd;
			if (!inalledges) break;
		}
		if(inalledges)
		{
			MapTextureEdge *c = r->addp();
			*c = *a;
		}
	}
	for (int i = 0; i < nedges; i++)
		delete m[i];
	if (r->len > 0)
	{
		int c = rand() % r->len;
		MapTextureEdge *e = r->getpnt(c);
		MapTile *tile = &(maptiles[tz * mapwidth + tx]);
		if (tile->mt->grp != grp)
		{
			ChangeTileTexture(tile, e->tex);
			tile->rot = e->rot;
			tile->xflip = e->xflip;
			tile->zflip = e->zflip;
		}
	}
	delete r;
}

void SelectNextRandomMapTile()
{
	if (randommaptex)
		curtex = curtexgrp->tex->getpnt(rand() % curtexgrp->tex->len);
	if (randommaptiletransform)
	{
		men_rot = rand() & 3;
		men_xflip = rand() & 1;
		men_zflip = rand() & 1;
	}
}

void DrawATS(int x, int z)
{
	MapTile *mt = &(maptiles[z * mapwidth + x]);
	if (curtex) ChangeTileTexture(mt, curtex);
	//mt->rot = 0; mt->xflip = 0; mt->zflip = 0;
	mt->rot = men_rot;
	mt->xflip = men_xflip; mt->zflip = men_zflip;
	MapTextureGroup *grp = mt->mt->grp;
	/*
	FindAutotileC(x - 1, z, 1, 1, mt->mt->grp);
	FindAutotileC(x + 1, z, 1, 0, mt->mt->grp);
	FindAutotileC(x, z - 1, 0, 1, mt->mt->grp);
	FindAutotileC(x, z + 1, 0, 0, mt->mt->grp);
	FindAutotileC(x - 1, z - 1, 0, 1, mt->mt->grp);
	FindAutotileC(x + 1, z - 1, 0, 1, mt->mt->grp);
	FindAutotileC(x - 1, z + 1, 0, 0, mt->mt->grp);
	FindAutotileC(x + 1, z + 1, 0, 0, mt->mt->grp);
	*/
	//FindAutotileWE(x - 1, z);
	//FindAutotileWE(x + 1, z);
	FindAutotileF(x - 1, z, 0, 1, 0, 1, grp);
	FindAutotileF(x + 1, z, 0, 1, 0, 1, grp);
	//
	FindAutotileF(x, z - 1, 1, 0, 1, 0, grp);
	FindAutotileF(x, z + 1, 1, 0, 1, 0, grp);
	//
	FindAutotileF(x - 1, z - 1, 1, 1, 1, 1, grp);
	FindAutotileF(x + 1, z - 1, 1, 1, 1, 1, grp);
	FindAutotileF(x - 1, z + 1, 1, 1, 1, 1, grp);
	FindAutotileF(x + 1, z + 1, 1, 1, 1, 1, grp);

	SelectNextRandomMapTile();
}

int lakemove_mousey_ref; float lakemove_waterlev_ref; Vector3 *lakemove_sellake = 0;
uchar vertexflatten_ref;

void ApplyBrush(boolean rclick)
{
	int cx = mapstdownpos.x / 5 + mapedge, cz = mapheight - (mapstdownpos.z / 5 + mapedge);
	int m = brushsize/2;

	uchar *oldhm_byte;
	if(mousetool == 10)
	{
		oldhm_byte = new uchar[(mapwidth+1)*(mapheight+1)];
		memcpy(oldhm_byte, himap_byte, (mapwidth+1)*(mapheight+1));
	}

	for(int z = cz-m; z < cz-m+brushsize; z++)
	if((z >= 0) && (z < mapheight+1))
	for(int x = cx-m; x < cx-m+brushsize; x++)
	if((x >= 0) && (x < mapwidth+1))
	{
		if(brushshape == 1)
			if( ((x-cx)*(x-cx) + (z-cz)*(z-cz)) > (m*m))
				continue;
		if(!rclick) switch(mousetool)
		{
			case 1:
			{
				if(!curtex) break;
				if((x == mapwidth) || (z == mapheight)) break;
				MapTile *mt = &(maptiles[z * mapwidth + x]);
				ChangeTileTexture(mt, curtex);
				mt->rot = men_rot;
				mt->xflip = men_xflip; mt->zflip = men_zflip;
				SelectNextRandomMapTile();
				break;
			}
			case 2:
			{
				int i = z * (mapwidth+1) + x;
				if(himap_byte[i] < 255) himap_byte[i] += 1;
				himap[i] = (float)himap_byte[i] * maphiscale;
				break;
			}
			case 6:
			{
				int i = z * (mapwidth+1) + x;
				himap_byte[i] = vertexflatten_ref;
				himap[i] = (float)himap_byte[i] * maphiscale;
				break;
			}
			case 10:
			{
				int i = z * (mapwidth+1) + x;
				uchar nn = oldhm_byte[i-mapwidth-1];
				uchar ns = oldhm_byte[i+mapwidth+1];
				uchar nw = oldhm_byte[i-1];
				uchar ne = oldhm_byte[i+1];
				uint a = (nn + ns + nw + ne) / 4;
				if(himap_byte[i] > a) if(himap_byte[i] > 0) himap_byte[i]--;
				if(himap_byte[i] < a) if(himap_byte[i] < 255) himap_byte[i]++;
				himap[i] = (float)himap_byte[i] * maphiscale;
				break;
			}
		}
		else switch(mousetool)
		{
			case 2:
			{
				int i = z * (mapwidth+1) + x;
				if(himap_byte[i] > 0) himap_byte[i] -= 1;
				himap[i] = (float)himap_byte[i] * maphiscale;
				break;
			}
		}
	}

	if(mousetool == 10) delete [] oldhm_byte;

	if((mousetool == 2) || (mousetool == 6 && !rclick) || (mousetool == 10 && !rclick))
		if(himapautowater) FloodfillWater();
}

void CICPart(CObjectDefinition *objdef, GameObject *parent, Vector3 &p, float r)
{
	GameObject *o = CreateObject(objdef, parent);
	o->position = p;
	o->orientation = Vector3(0,r,0);
	GOPosChanged(o, 0, 1);
}
void CICMoveForward(Vector3 &p, float r, float l)
{
	float dx = sin(r) * l;
	float dz = -cos(r) * l;
	p += Vector3(dx, 0, dz);
}
void CICMoveRight(Vector3 &p, float r, float l)
{
	float dx = -cos(r) * l;
	float dz = -sin(r) * l;
	p += Vector3(dx, 0, dz);
}
void CICRotate(float &r, float nr)
{
	r += nr;
	if(r >= 2 * M_PI) r -= 2 * M_PI;
}
CObjectDefinition *CICFindObjDef(int c, char *n)
{
	int i = FindObjDef(c, n);
	if(i != -1) return &(objdef[i]);
	else return 0;
}
void CreateInitialCity(GameObject *player, Vector3 ipos, float irot)
{
	CObjectDefinition *citytype, *wall, *corner, *gatehouse, *manor, *flag, *peasant;
	citytype =  CICFindObjDef(CLASS_CITY, "New City");
	wall =      CICFindObjDef(CLASS_BUILDING, "Stockade");
	corner =    CICFindObjDef(CLASS_BUILDING, "Stockade Corner Out");
	gatehouse = CICFindObjDef(CLASS_BUILDING, "Stockade Gatehouse");
	manor =     CICFindObjDef(CLASS_BUILDING, "Manor");
	flag =      CICFindObjDef(CLASS_BUILDING, "Banner1");
	peasant =   CICFindObjDef(CLASS_CHARACTER, "Peasant");
	if(!(citytype && wall && corner && gatehouse && manor && flag && peasant))
	{
		MessageBox(hWindow, "Some object types are missing for city creation.", appName, 48);
		return;
	}

	GameObject *city = CreateObject(citytype, player);
	Vector3 pos = ipos; float rot = irot;
	CICPart(manor, city, pos, rot);
	CICMoveForward(pos, rot, (2+nmc_size)*5);
	CICMoveRight(pos, rot, 2.5f);
	CICPart(gatehouse, city, pos, rot);
	CICMoveRight(pos, rot, 12.5f);
	for(int i = 0; i < nmc_size-1; i++)
		{CICPart(wall, city, pos, rot); CICMoveRight(pos, rot, 5);}
	for(int j = 0; j < 3; j++)
	{
		CICRotate(rot, M_PI); CICPart(corner, city, pos, rot);
		CICRotate(rot, M_PI/2); CICMoveRight(pos, rot, 5);
		for(int i = 0; i < 3+2*nmc_size; i++)
			{CICPart(wall, city, pos, rot); CICMoveRight(pos, rot, 5);}
	}
	CICRotate(rot, M_PI); CICPart(corner, city, pos, rot);
	CICRotate(rot, M_PI/2); CICMoveRight(pos, rot, 5);
	for(int i = 0; i < nmc_size; i++)
		{CICPart(wall, city, pos, rot); CICMoveRight(pos, rot, 5);}

	if(!city->rects) city->rects = new GrowList<int>;
	city->rects->add(ipos.x / 5 - (1+nmc_size+1));
	city->rects->add(ipos.z / 5 - (1+nmc_size+1));
	city->rects->add(ipos.x / 5 + (1+nmc_size+1));
	city->rects->add(ipos.z / 5 + (1+nmc_size+1));

	if(nmc_flags)
	{
		Vector3 flp = ipos;
		CICMoveForward(flp, irot, 5*(2+nmc_size+1));
		CICMoveRight(flp, irot, 5*2);
		CICPart(flag, city, flp, irot);
		CICMoveRight(flp, irot, -5*3);
		CICPart(flag, city, flp, irot);
	}

	if(nmc_npeasants > 0)
	{
		float lsize = (nmc_npeasants-1) * 3.0f;
		Vector3 pp = ipos;
		CICMoveForward(pp, irot, 5*(2+nmc_size+1));
		CICMoveRight(pp, irot, -lsize/2.0f + 2.5f);
		for(int i = 0; i < nmc_npeasants; i++)
		{
			CICPart(peasant, city, pp, irot);
			CICMoveRight(pp, irot, 3.0f);
		}
	}
}

GameObject *GetTileCity(int tx, int tz)
{
	for(DynListEntry<GameObject> *pdle = levelobj->children.first; pdle; pdle = pdle->next)
	{
		GameObject *player = &pdle->value;
		for(DynListEntry<GameObject> *cdle = player->children.first; cdle; cdle = cdle->next)
		{
			GameObject *city = &cdle->value;
			if(city->objdef->type != CLASS_CITY) continue;
			if(!city->rects) continue;
			for(int i = 0; i < city->rects->len / 4; i++)
			{
				int x1, z1, x2, z2;
				x1 = city->rects->get(i*4+0);
				z1 = city->rects->get(i*4+1);
				x2 = city->rects->get(i*4+2);
				z2 = city->rects->get(i*4+3);
				if((tx >= x1) && (tx <= x2) && (tz >= z1) && (tz <= z2))
					return city;
			}
		}
	}
	return 0;
}

void T7ClickWindow(void *param)
{
	if(mousetool)
	{
		mousetoolpress_l = 1;
		switch(mousetool)
		{
			case 3:
			{
				int x = mapstdownpos.x / 5 + mapedge, z = mapheight - (mapstdownpos.z / 5 + mapedge);
				maplakes.add();
				Vector3 *l = &maplakes.last->value;
				l->x = 5 * x + 2.5f;
				l->z = 5 * (mapheight - z) + 2.5f;
				float c1 = himap[z * (mapwidth+1) + x];
				float c2 = himap[z * (mapwidth+1) + x+1];
				float c3 = himap[(z+1) * (mapwidth+1) + x];
				float c4 = himap[(z+1) * (mapwidth+1) + x+1];
				l->y = (c1 + c2 + c3 + c4) / 4.0f + 2.5f;
				FloodfillWater();
				lakemove_mousey_ref = mouseY;
				lakemove_waterlev_ref = l->y;
				lakemove_sellake = l;
				break;
			}
			case 4:
			{
				int x = mapstdownpos.x / 5 + mapedge, z = mapheight - (mapstdownpos.z / 5 + mapedge);
				MapTile *t = &(maptiles[z * mapwidth + x]);
				if(t->lake)
				{
					maplakes.remove(t->lake);
					FloodfillWater();
				}
				break;
			}
			case 5:
			{
				int x = mapstdownpos.x / 5 + mapedge, z = mapheight - (mapstdownpos.z / 5 + mapedge);
				MapTile *t = &(maptiles[z * mapwidth + x]);
				if(t->lake)
				{
					lakemove_mousey_ref = mouseY;
					lakemove_waterlev_ref = t->lake->value.y;
					lakemove_sellake = &t->lake->value;
				}
				else	lakemove_sellake = 0;
				break;
			}
			case 6:
			{
				int x = mapstdownpos.x / 5 + mapedge, z = mapheight - (mapstdownpos.z / 5 + mapedge);
				vertexflatten_ref = himap_byte[z * (mapwidth+1) + x];
				break;
			}
			case 7:
			{
				if(!newmanorplayer.valid()) break;
				int x = stdownpos.x / 5, z = stdownpos.z / 5;
				Vector3 p(5*x+2.5f, 0, 5*z+2.5f);
				CreateInitialCity(newmanorplayer.get(), p, stampdownRot);
				break;
			}
			case 47:
				int x = mapstdownpos.x / 5 + mapedge, z = mapheight - (mapstdownpos.z / 5 + mapedge);
				DrawATS(x, z);
				break;
		}
		return;
	}

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
	if(mousetool)
	{
		mousetoolpress_r = 1;
		switch(mousetool)
		{
			case 1:
			case 47:
			{
				int x = mapstdownpos.x / 5 + mapedge, z = mapheight - (mapstdownpos.z / 5 + mapedge);
				//printf("x=%i  z=%i\n", x, z);
				MapTile *mt = &(maptiles[z * mapwidth + x]);
				//printf("texgrp=%s  tex=%i\n", mt->mt->grp->name, mt->mt->id);
				curtexgrp = mt->mt->grp;
				curtex = mt->mt;
				men_rot = mt->rot;
				men_xflip = mt->xflip;
				men_zflip = mt->zflip;
				break;
			}
			case 7:
				int x = stdownpos.x / 5, z = stdownpos.z / 5;
				GameObject *c = GetTileCity(x, z);
				if(c) RemoveObject(c);
				break;
		/*
			case 47:
				int x = mapstdownpos.x / 5 + mapedge, z = mapheight - (mapstdownpos.z / 5 + mapedge);
				FindAutotile(x, z, 1);
				break;
		*/
		}
		return;
	}

	if(objtypeToStampdown)
	{
		GameObject *papa;
		if((objtypeToStampdown->type == CLASS_PLAYER) || (objtypeToStampdown->type == CLASS_TERRAIN_ZONE))
			papa = levelobj;
		else if(playerToGiveStampdownObj.valid())
			papa = playerToGiveStampdownObj.get();
		else goto nopapa;

		GameObject *o = CreateObject(objtypeToStampdown, papa);
		o->position = stdownpos;
		o->orientation = Vector3(0, stampdownRot, 0);
		GOPosChanged(o, 0);
		if(eventAfterStampdown)
		{
			SequenceEnv ctx;
			SendGameEvent(&ctx, o, PDEVENT_ON_STAMPDOWN);
		}
		if(!keypressed[VK_SHIFT]) objtypeToStampdown = 0;
		return;
	}
nopapa:
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

GrowStringList *gslToSort;

int SGSLCompare(const void *a, const void *b)
{
	return stricmp(gslToSort->getdp(*(uint*)a), gslToSort->getdp(*(uint*)b));
}

uint *SortGrowStringList(GrowStringList *gsl)
{
	uint *inds = new uint[gsl->len];
	for(int i = 0; i < gsl->len; i++)
		inds[i] = i;
	gslToSort = gsl;
	qsort(inds, gsl->len, sizeof(uint), SGSLCompare);
	return inds;
}

uint *SortIndices(int nele, int (*compar)(const void *, const void*))
{
	uint *inds = new uint[nele];
	for(int i = 0; i < nele; i++)
		inds[i] = i;
	qsort(inds, nele, sizeof(uint), compar);
	return inds;
}

int MapTexGrpCompareSortCompare(const void *a, const void *b)
{
	return strcmp(maptexgroup.getpnt(*(uint*)a)->name, maptexgroup.getpnt(*(uint*)b)->name);
}

int advancedSpatial = 0;
int itemsToDisplay = 0;
int selectedReaction = -1;
ImGuiTextFilter itemFilter, odFilter, objcreaFilter, ireaFilter;
ImVec4 ivcolortable[8] = {
 ImVec4(0.5f, 0.25f, 0.25f, 1), ImVec4(0, 0, 1, 1), ImVec4(1, 1, 0, 1), ImVec4(1, 0, 0, 1),
 ImVec4(0, 1, 0, 1), ImVec4(1, 0, 1, 1), ImVec4(1, 0.5f, 0, 1), ImVec4(0, 1, 1, 1)};
GrowStringList odcList;
uint *sortedmaptexnames, *sortedobjdefnames;

void IGInit()
{
	char t[512];
	for(int i = 0; i < strObjDef.len; i++)
	{
		CObjectDefinition *od = &(objdef[i]);
		_snprintf(t, 511, "%s \"%s\"", CLASS_str[od->type], od->name);
		odcList.add(t);
	}
	sortedobjdefnames = SortGrowStringList(&odcList);
	sortedmaptexnames = SortIndices(maptexgroup.len, MapTexGrpCompareSortCompare);
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

bool IGObjectSelectable(GameObject *o, boolean playerinfo = 0)
{
	char t[256]; bool r;
	if(playerinfo) _snprintf(t, 255, "%S (%u)", o->name, o->id);
	else _snprintf(t, 255, "%u: %s \"%s\"###", o->id, CLASS_str[o->objdef->type], o->objdef->name);
	ImGui::PushID(o->id);
	r = ImGui::Selectable(playerinfo ? "" : t, o->flags & FGO_SELECTED);
	if(ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1))
	{
		if(!keypressed[VK_SHIFT])
			DeselectAll();
		if(o->flags & FGO_SELECTED)
			DeselectObject(o);
		else	SelectObject(o);
	}
	if(playerinfo)
	{
		ImGui::SameLine();
		ImGui::ColorButton(ivcolortable[o->color], true);
		ImGui::SameLine();
		ImGui::Text(t);
	}
	ImGui::PopID();
	return r;
}

void IGwcharPntTextBox(char *id, wchar_t **txt)
{
	char mb[512]; wchar_t wb[512];
	if(*txt) wcstombs(mb, *txt, 511);
	else mb[0] = 0;
	if(ImGui::InputText(id, mb, 511))
	{
		mbstowcs(wb, mb, 511);
		delete [] *txt;
		*txt = new wchar_t[wcslen(wb)+1];
		wcscpy(*txt, wb);
	}
}

bool IGPlayerChooser(char *popupid, goref *p);

int igcurod = 0;
int igliCurrentAssoCat = -1;

void IGSelectedObject()
{
	char tb[512];
	if(!swSelObj) return;
	ImGui::SetNextWindowPos(ImVec2(373, 21), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(263, 456), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Object information", &swSelObj);
	if(selobjects.first)
	{if(selobjects.first->value.valid())
	{
		GameObject *o = selobjects.first->value.get();
		ImGui::Value("ID", o->id);
		ImGui::SameLine();
		if(ImGui::SmallButton("Change type##ObjType"))
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
				CObjectDefinition *od = &(objdef[sortedobjdefnames[i]]);
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
		ImGui::Text("Type: %s \"%s\"", CLASS_str[o->objdef->type], o->objdef->name);
		if((o != levelobj) && (o->parent != levelobj) && o->player)
		{
			ImGui::Text("Player:"); ImGui::SameLine();
			if (ImGui::SmallButton("->##SelectPlayer"))
			{
				if (!keypressed[VK_SHIFT]) DeselectAll();
				if (o->player->flags & FGO_SELECTED) DeselectObject(o->player);
				else SelectObject(o->player);
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Select player object");
			ImGui::SameLine();
			goref pl = o->player;
			if(IGPlayerChooser("SetObjectPlayer", &pl))
				SetObjectParent(o, pl.get());
		}
		if (o != levelobj)
		{
			ImGui::Text("Parent:"); ImGui::SameLine();
			if (ImGui::SmallButton("->##SelectParent"))
			{
				if (!keypressed[VK_SHIFT]) DeselectAll();
				if (o->parent->flags & FGO_SELECTED) DeselectObject(o->parent);
				else SelectObject(o->parent);
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Select parent object");
			ImGui::SameLine();
			ImGui::Text("ID %i: %s \"%s\"", o->parent->id, CLASS_str[o->parent->objdef->type], o->parent->objdef->name);
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
		ImGui::PopItemWidth();
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
			if(ImGui::CollapsingHeader("Start camera"))
			{
				ImGui::DragFloat3("Position##StartCamera", &o->startcampos.x, 0.1f);
				ImGui::DragFloat2("Orientation##StartCamera", &o->startcamori.x, 0.1f);
				if(ImGui::Button("Set##StartCamera"))
				{
					o->startcampos = camerapos;
					o->startcamori = Vector3(campitch, camyaw, 0);
				}
				ImGui::SameLine();
				if(ImGui::Button("Go to##StartCamera"))
				{
					SetCurCameraPos(o->startcampos);
					SetCurCameraRot(o->startcamori.x, o->startcamori.y);
				}
			}
		}
		if(ImGui::CollapsingHeader("Transform"))
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
					//o->position.y = GetHeight(v.x, v.y);
					GOPosChanged(o, 0);
				}
				ImGui::SliderAngle("Orientation##Basic", &o->orientation.y, 0);
				if(ImGui::DragFloat("Scale##Basic", &o->scale.x, 0.1f))
					o->scale.y = o->scale.z = o->scale.x;
			}
			else
			{
				if(ImGui::DragFloat3("Position##Advanced", &o->position.x, 0.1f))
					GOPosChanged(o, 0, 0);
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
	if(o->objdef->type == CLASS_PLAYER)
	{
		ImVec4 c = ivcolortable[o->color];
		c.x += 0.3f; c.y += 0.3f; c.z += 0.3f;
		ImGui::PushStyleColor(ImGuiCol_Text, c);
	}
	boolean n = ImGui::TreeNodeEx((void*)o->id,
			// ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow |
			((o->flags & FGO_SELECTED) ? ImGuiTreeNodeFlags_Selected : 0) |
			((o->children.len > 0) ? 0 : ImGuiTreeNodeFlags_Leaf),
			"%u: %s \"%s\"", o->id, CLASS_str[o->objdef->type], o->objdef->name);
	if(o->objdef->type == CLASS_PLAYER)
		ImGui::PopStyleColor();
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
	ImGui::PushID(popupid);
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
	ImGui::PopID();
	return r;
}

extern char sggameset[384];
extern uint game_type;
extern wchar_t *campaignName, *serverName;
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
		ImGui::Text("Campaign name:");
		IGwcharPntTextBox("##Campaign name", &campaignName);
		ImGui::Text("Server name:");
		IGwcharPntTextBox("##Server name", &serverName);
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
		{
			if(!keypressed[VK_SHIFT])
				DeselectAll();
			for(DynListEntry<goref> *e = d->first; e; e = e->next)
				if(e->value.valid())
					SelectObject(e->value.get());
		}

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
	if(ImGui::CollapsingHeader("Human players list"))
	{
		DynListEntry<goref> *nx;
		for(DynListEntry<goref> *e = humanplayers.first; e; e = nx)
		{
			nx = e->next;
			if(!e->value.valid())
				humanplayers.remove(e);
		}
		if(ImGui::Button("Move up"))
		{
			boolean canmove = 0;
			for(DynListEntry<goref> *e = humanplayers.first; e; e = e->next)
			 if(e->value.valid())
			{
				if(e->value->flags & FGO_SELECTED)
					{if(canmove) if(e->previous) humanplayers.movedown(e->previous);}
				else
					canmove = 1;
			}
		}
		ImGui::SameLine();
		if(ImGui::Button("Move down"))
		{
			boolean canmove = 0;
			for(DynListEntry<goref> *e = humanplayers.last; e; e = e->previous)
			 if(e->value.valid())
			{
				if(e->value->flags & FGO_SELECTED)
					{if(canmove) humanplayers.movedown(e);}
				else
					canmove = 1;
			}
		}
		ImGui::SameLine();
		if(ImGui::Button("Remove"))
		{
			DynListEntry<goref> *n;
			for(DynListEntry<goref> *e = humanplayers.first; e; e = n)
			{
				n = e->next;
				if(e->value->flags & FGO_SELECTED)
					humanplayers.remove(e);
			}
		}
		goref g;
		ImGui::Text("Add:"); ImGui::SameLine();
		IGPlayerChooser("AddNewHumanPlayerPopup", &g);
		if(g.valid())
			{humanplayers.add();
			humanplayers.last->value = g;}
		ImGui::PushItemWidth(-1);
		ImGui::ListBoxHeader("##HumanplayerList");
		int ni = 1;
		for(DynListEntry<goref> *e = humanplayers.first; e; e = e->next)
			if(e->value.valid())
			{
				ImGui::Text("%u.", ni++); ImGui::SameLine();
				IGObjectSelectable(e->value.get(), 1);
			}
		ImGui::ListBoxFooter();
		ImGui::PopItemWidth();
		ImGui::Text("Number of players: %u", ni-1);
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
		//CObjectDefinition *od = &(objdef[i]);
		CObjectDefinition *od = &(objdef[sortedobjdefnames[i]]);
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
	ImGui::Text("(C) 2015-2018 AdrienTD");
	ImGui::Text("Licensed under the GPL3 license.\nSee LICENSE for details.");
	ImGui::Separator();
	ImGui::Text("Libraries used:");
	ImGui::BulletText("LZRW3");
	ImGui::BulletText("libbzip2");
	ImGui::BulletText("dear imgui (MIT license)");
	ImGui::End();
}

char newmapname[128] = "wkbre_NewMap", openmappath[265];
int newmapwidth, newmapheight;

void IGMapEditor()
{
	if(!swMapEditor) return;
	ImGui::Begin("Terrain editor", &swMapEditor);
	ImGui::InputText("Name", newmapname, sizeof(newmapname)-1);

	int iWantToSave = 0;
	if(ImGui::Button("Save SNR")) iWantToSave = 1; ImGui::SameLine();
	if(ImGui::Button("Save BCM")) iWantToSave = 2; ImGui::SameLine();
	if(iWantToSave)
	{
		char p[768];
		strcpy(p, gamedir);
		strcat(p, "\\saved");			_mkdir(p);
		strcat(p, "\\Maps");			_mkdir(p);
		strcat(p, "\\"); strcat(p, newmapname);	_mkdir(p);

		strcat(p, "\\"); strcat(p, newmapname); strcat(p, (iWantToSave==1) ? ".snr" : ".bcm");
		if(_access(p, 0) != -1)
			if(MessageBox(hWindow, "The map name already exists. Do you want to overwrite the map?", appName, 48 | MB_YESNO) == IDNO)
				goto saveend;
		if(iWantToSave==1)
			{SaveMapSNR(p);
			sprintf(lastmap, "Maps\\%s\\%s.snr", newmapname, newmapname);
			GiveNotification("SNR map saved!");}
		else
			{SaveMapBCM(p);
			sprintf(lastmap, "Maps\\%s\\%s.bcm", newmapname, newmapname);
			GiveNotification("BCM map saved!");}
saveend:	;
	}
	//ImGui::SameLine();
	//if(ImGui::Button("Set"))
	//	sprintf(lastmap, "Maps\\%s\\%s.snr", newmapname, newmapname);
	ImGui::SameLine();
	if(ImGui::Button("New"))
	{
		ImGui::OpenPopup("CreateMapPopup");
		newmapwidth = mapwidth; newmapheight = mapheight;
	}
	if(ImGui::BeginPopup("CreateMapPopup"))
	{
		ImGui::InputInt("Width", &newmapwidth);
		ImGui::InputInt("Height", &newmapheight);
		if(ImGui::Button("OK"))
		{
			CreateEmptyMap(newmapwidth, newmapheight);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::SameLine();
	if(ImGui::Button("Open"))
	{
		ImGui::OpenPopup("OpenMapPopup");
		sprintf_s(openmappath, 255, "Maps\\%s\\%s.", newmapname, newmapname);
		openmappath[255] = 0;
		char *mpext = openmappath + strlen(openmappath);
		strcpy(mpext, "snr");
		if(!FileExists(openmappath))
			strcpy(mpext, "bcm");
	}
	if(ImGui::BeginPopup("OpenMapPopup"))
	{
		ImGui::InputText("##OpenMapFilePath", openmappath, 255);
		if(ImGui::Button("OK"))
		{
			if(FileExists(openmappath))
			{
				LoadMap(openmappath);
				ImGui::CloseCurrentPopup();
			}
			else	GiveNotification("Map file to open does not exist!");
		}
		ImGui::EndPopup();
	}
		
	ImGui::Text("Heightmap:"); ImGui::SameLine();
	if(ImGui::Button("Copy"))
		CopyHeightmap();
	ImGui::SameLine();
	if(ImGui::Button("Paste"))
		PasteHeightmap();
	//ImGui::Separator();
	if(ImGui::CollapsingHeader("Tools"))
	{
		//ImGui::Text("Misc:"); ImGui::SameLine();
		ImGui::RadioButton("Default##Mode", &mousetool, 0);
		ImGui::Text("Texture:"); ImGui::SameLine();
		ImGui::RadioButton("Texture##Mode", &mousetool, 1); ImGui::SameLine();
		ImGui::RadioButton("Autotiles##Mode", &mousetool, 47);
		ImGui::Text("Vertex:"); ImGui::SameLine();
		//ImGui::BeginGroup();
		ImGui::RadioButton("Elevate##Mode", &mousetool, 2); ImGui::SameLine();
		ImGui::RadioButton("Flatten##Mode", &mousetool, 6); ImGui::SameLine();
		ImGui::RadioButton("Equalize##Mode", &mousetool, 10);
		//ImGui::EndGroup();
		ImGui::Text("Lake:"); ImGui::SameLine();
		ImGui::RadioButton("Add##Mode", &mousetool, 3); ImGui::SameLine();
		ImGui::RadioButton("Move##Mode", &mousetool, 5); ImGui::SameLine();
		ImGui::RadioButton("Remove##Mode", &mousetool, 4);
		ImGui::Separator();
		ImGui::InputInt("Brush size", &brushsize);
		ImGui::Text("Brush shape:"); ImGui::SameLine();
		ImGui::RadioButton("Square", &brushshape, 0); ImGui::SameLine();
		ImGui::RadioButton("Circle", &brushshape, 1);
	}
	if(ImGui::CollapsingHeader("Properties"))
	{
		ImGui::LabelText("Map size", "w=%u\th=%u", mapwidth, mapheight);
		ImGui::LabelText("Level size", "w=%u\th=%u", mapwidth-2*mapedge, mapheight-2*mapedge);
		int newedge = mapedge;
		if(ImGui::InputInt("Edge", &newedge))
		 if((newedge != mapedge) && (newedge >= 0))
		  if((newedge < mapwidth/2) && (newedge < mapheight/2))
		{
			mapedge = newedge;
			ReinitITiles();
			ResetITiles();
		}
		if(ImGui::DragFloat("Height scale", &maphiscale, 0.01f))
		{
			for(int i = 0; i < (mapwidth+1)*(mapheight+1); i++)
				himap[i] = (float)himap_byte[i] * maphiscale;
			if(himapautowater) FloodfillWater();
		}
		float c[3];
		for(int i = 0; i < 3; i++)
			c[2-i] = ((mapsuncolor >> (8 * i)) & 255) / 255.0f;
		if(ImGui::ColorEdit3("Sun color", c))
		{
			mapsuncolor = 0;
			for(int i = 0; i < 3; i++)
				mapsuncolor |= ((int)(c[2-i]*255) & 255) << (8 * i);
		}
		ImGui::DragFloat3("Sun vector", &mapsunvector.x, 0.01f);
		for(int i = 0; i < 3; i++)
			c[2-i] = ((mapfogcolor >> (8 * i)) & 255) / 255.0f;
		if(ImGui::ColorEdit3("Fog color", c))
		{
			mapfogcolor = 0;
			for(int i = 0; i < 3; i++)
				mapfogcolor |= ((int)(c[2-i]*255) & 255) << (8 * i);
		}
		ImGui::InputText("Skybox", mapskytexdir, 255);
	}
	if(ImGui::CollapsingHeader("Texture"))
	{
		ImGui::PushItemWidth(-1);
		ImGui::Text("Group:");
		ImGui::ListBoxHeader("##Group");
		for(int i = 0; i < maptexgroup.len; i++)
		{
			//MapTextureGroup *g = maptexgroup.getpnt(i);
			MapTextureGroup *g = maptexgroup.getpnt(sortedmaptexnames[i]);
			if(ImGui::Selectable(g->name, curtexgrp == g))
				{curtexgrp = g; curtex = g->tex->getpnt(0);}
		}
		ImGui::ListBoxFooter();

		ImGui::BeginChild("Texture", ImVec2(0,96), true, ImGuiWindowFlags_HorizontalScrollbar);
		MapTextureGroup *g = curtexgrp;
		if(g)
		for(int i = 0; i < g->tex->len; i++)
		{
			MapTexture *t = g->tex->getpnt(i);
			boolean ss = curtex == t;
			ImGui::PushID(i);
			if(ss)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1,0,0,1));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1,0.3f,0.3f,1));
			}
			if(ImGui::ImageButton((ImTextureID)t->t, ImVec2(64, 64), ImVec2(t->x/256.f, t->y/256.f), ImVec2((t->x+t->w)/256.f, (t->y+t->h)/256.f)))
				curtex = t;
			if(ImGui::IsItemHovered())
				ImGui::SetTooltip("ID: %i", t->id);
			if(ss) ImGui::PopStyleColor(2);
			ImGui::PopID();
			ImGui::SameLine();
		}
		ImGui::NewLine();
		ImGui::EndChild();
		ImGui::Text("Rotation:"); ImGui::SameLine();
		ImGui::RadioButton("0°##rot", &men_rot, 0); ImGui::SameLine();
		ImGui::RadioButton("90°##rot", &men_rot, 1); ImGui::SameLine();
		ImGui::RadioButton("180°##rot", &men_rot, 2); ImGui::SameLine();
		ImGui::RadioButton("270°##rot", &men_rot, 3);
		ImGui::Text("Flip:"); ImGui::SameLine();
		ImGui::Checkbox("X", &men_xflip); ImGui::SameLine();
		ImGui::Checkbox("Z", &men_zflip);
		ImGui::Checkbox("Randomize textures", &randommaptex);
		ImGui::Checkbox("Randomize transformation", &randommaptiletransform);
		ImGui::PopItemWidth();
	}
	if(ImGui::CollapsingHeader("Lakes"))
	{
		ImGui::PushItemWidth(-1);
		if(ImGui::Button("Clear##Lakes"))
		{
			maplakes.clear();
			FloodfillWater();
		}
		ImGui::SameLine();
		if(ImGui::Button("Floodfill##Lakes"))
			FloodfillWater();
		ImGui::Checkbox("Floodfill when heightmap changes", &himapautowater);
		//ImGui::ListBoxHeader("##LakesListBox");
		ImGui::PushItemWidth(-ImGui::GetItemsLineHeightWithSpacing());
		DynListEntry<Vector3> *n;
		for(DynListEntry<Vector3> *e = maplakes.first; e; e = n)
		{
			n = e->next;
			ImGui::PushID(e);
			Vector3 *l = &e->value;
			//ImGui::BulletText("%f %f %f", l->x, l->y, l->z);
			if(ImGui::DragFloat3("##LakePos", &l->x, 0.1f))
				FloodfillWater();
			ImGui::SameLine(0, 0);
			if(ImGui::Button("X"))
			{
				maplakes.remove(e);
				FloodfillWater();
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Remove");
			ImGui::PopID();
		}
		ImGui::PopItemWidth();
		//ImGui::ListBoxFooter();
		ImGui::PopItemWidth();
	}
	ImGui::End();
}

void IGCityCreator()
{
	if(!swCityCreator) return;
	ImGui::Begin("City creator", &swCityCreator);
	ImGui::Text("Tool:"); ImGui::SameLine();
	ImGui::RadioButton("Default", &mousetool, 0); ImGui::SameLine();
	ImGui::RadioButton("Add/remove city", &mousetool, 7);
	ImGui::Text("For:"); ImGui::SameLine();
	IGPlayerChooser("NewManorPlayer", &newmanorplayer);
	ImGui::InputInt("Size", &nmc_size);
	if(nmc_size < 2) nmc_size = 2;
	ImGui::InputInt("Num. peasants", &nmc_npeasants);
	if(nmc_npeasants < 0) nmc_npeasants = 0;
	ImGui::Checkbox("Flags in front of gatehouse", &nmc_flags);
	ImGui::End();
}

void IGTest()
{
	if(!swTest) return;
	ImGui::Begin("Test", &swTest);
	if(ImGui::CollapsingHeader("Model attachment points"))
	{
		for(int i = 0; i < alModelFn.len; i++)
			if(ImGui::TreeNode(alModelFn.getdp(i)))
			{
				Model *mdl = alModel[i];
				Mesh *msh = mdl->mesh;
				for(int j = 0; j < msh->nAttachPnts; j++)
				{
					AttachmentPoint *ap = &msh->attachPnts[j];
					ImGui::Text("Tag: %s", strAttachPntTags.getdp(ap->tag));
					ImGui::Text("Path: %s", ap->path);
				}
				ImGui::TreePop();
			}
	}
	if(ImGui::CollapsingHeader("Attachment point tags list"))
	{
		ImGui::Text("Number: %i", strAttachPntTags.len);
		for(int i = 0; i < strAttachPntTags.len; i++)
			ImGui::Text("%s", strAttachPntTags.getdp(i));
	}
	if(ImGui::CollapsingHeader("Rendering settings"))
	{
		ImGui::DragFloat("farzvalue", &farzvalue);
		ImGui::DragFloat("occlurate", &occlurate);
		ImGui::DragFloat("verticalfov", &verticalfov);
	}
	ImGui::End();
}

bool mmshowedge = true, mmshowobjs = true;
int mmbmplen = 128;

void IGMinimap()
{
	static char* cbstr[4] = {"64", "128", "256", "512"};
	static int cbint[4] = {64, 128, 256, 512};
	static int cbindex = 1;
	static int viewsize = 128;
	if(!swMinimap) return;
	ImGui::SetNextWindowSize(ImVec2(206,232), ImGuiSetCond_FirstUseEver);
	bool b = ImGui::Begin("Minimap", &swMinimap);
	if (!b) {ImGui::End(); return;}
	//if(ImGui::InputInt("Resolution", &mmbmplen))
	if(ImGui::Combo("Size", &cbindex, (const char**)cbstr, 4))
	{
		mmbmplen = viewsize = cbint[cbindex];
		if(mmbmplen <= 0) mmbmplen = 1;
		FreeTexture(minimapTexture);
		minimapBitmap = new Bitmap;
		minimapBitmap->w = minimapBitmap->h = mmbmplen;
		minimapBitmap->form = BMFORMAT_B8G8R8A8;
		minimapBitmap->pix = new uchar[mmbmplen*mmbmplen*4];
		memset(minimapBitmap->pix, '0', mmbmplen*mmbmplen*4);
		minimapBitmap->pal = 0;
		minimapTexture = CreateTexture(minimapBitmap, 1);
		FreeBitmap(minimapBitmap);
	}
	//ImGui::InputInt("Zoom", &viewsize);
	ImGui::Checkbox("Edge", &mmshowedge); ImGui::SameLine();
	ImGui::Checkbox("Objects", &mmshowobjs);
	ImGui::BeginChild("MinimapRegion", ImVec2(0,0), true, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus);
	ImVec2 imgpos = ImGui::GetCursorScreenPos();
	ImGui::Image(minimapTexture, ImVec2(viewsize,viewsize));
	if(ImGui::IsMouseDown(0) && ImGui::IsItemHoveredRect() && ImGui::IsWindowHovered())
	{
		ImVec2 v;
		v.x = ImGui::GetMousePos().x - imgpos.x;
		v.y = ImGui::GetMousePos().y - imgpos.y;

		int w = mmshowedge ? mapwidth : (mapwidth - 2*mapedge);
		int h = mmshowedge ? mapheight : (mapheight - 2*mapedge);
		int mmw, mmh;
		if(w > h) {mmw = mmbmplen; mmh = h * mmbmplen / w;}
		else      {mmh = mmbmplen; mmw = w * mmbmplen / h;}
		int sx = mmbmplen/2 - mmw/2;
		int sy = mmbmplen/2 - mmh/2;
		int re = mmshowedge ? (mapedge*5) : 0;
		v.x = (v.x - sx) * w * 5 / mmw - re;
		v.y = 5*h - (v.y - sy) * h * 5 / mmh - re;

		//printf("Minimap click: x=%f, y=%f\n", v.x, v.y);
		SetCurCameraPosXZ(v.x, v.y);
	}
	ImGui::EndChild();
	ImGui::End();
}

void MoveKeyPress(Vector3 &m)
{
	Vector3 n = m.normal();
	if(!keypressed[VK_CONTROL])
		DisplaceCurCamera(n * camwalkstep);
	else for(DynListEntry<goref> *e = selobjects.first; e; e = e->next)
		if(e->value.valid())
		{
			e->value->position += n * walkstep;
			if(objmovalign)
			{
				e->value->position.x = (int)((e->value->position.x+1.25f)/2.5f) * 2.5f;
				e->value->position.z = (int)((e->value->position.z+1.25f)/2.5f) * 2.5f;
			}
			e->value->position.y = GetHeight(e->value->position.x, e->value->position.z);
			GOPosChanged(e->value.get());
		}
}

void IGMainMenuBar()
{
	ImGui::BeginMainMenuBar();

	if(ImGui::BeginMenu("File"))
	{
		if(ImGui::MenuItem("Save as...", "F3")) CallCommand(CMD_SAVE);
		if(ImGui::MenuItem("Change WK version")) CallCommand(CMD_CHANGE_SG_WKVER);
		if(ImGui::MenuItem("Quit", "Esc")) CallCommand(CMD_QUIT);
		ImGui::EndMenu();
	}
	if(ImGui::BeginMenu("Object"))
	{
		if(ImGui::MenuItem("Create object...", "F7")) CallCommand(CMD_SWOBJCREA); //CallCommand(CMD_CREATEOBJ);
		if(ImGui::MenuItem("Duplicate selection", "N")) CallCommand(CMD_DUPLICATESELOBJECT);
		if(ImGui::MenuItem("Give selection to...", "Home")) CallCommand(CMD_GIVESELOBJECT);
		if(ImGui::MenuItem("Delete selection", "Del")) CallCommand(CMD_DELETESELOBJECT);
		//if(ImGui::MenuItem("Delete last created object")) CallCommand(CMD_DELETELASTCREATEDOBJ);
		if(ImGui::MenuItem("Scale selection by 1.5", "*")) CallCommand(CMD_SELOBJSCALEBIGGER);
		if(ImGui::MenuItem("Scale selection by 1/1.5", "/")) CallCommand(CMD_SELOBJSCALESMALLER);
		if(ImGui::MenuItem("Rotate selection by 90 deg", "R")) CallCommand(CMD_ROTATEOBJQP);
		ImGui::Separator();
		if(ImGui::MenuItem("Convert type...")) CallCommand(CMD_CONVERTOBJTYPE);
		if(ImGui::MenuItem("Give type...")) CallCommand(CMD_GIVEOBJTYPE);
		if(ImGui::MenuItem("Delete type...")) CallCommand(CMD_DELETEOBJTYPE);
		if(ImGui::MenuItem("Delete class...")) CallCommand(CMD_DELETEOBJCLASS);
		ImGui::Separator();
		if(ImGui::MenuItem("Randomize subtypes", "F11")) CallCommand(CMD_RANDOMSUBTYPE);
		if(ImGui::MenuItem("Reset objects' heights", "F2")) CallCommand(CMD_RESETOBJPOS);
		//if(ImGui::MenuItem("Rename player object...")) CallCommand(CMD_CHANGE_PLAYER_NAME);
		//if(ImGui::MenuItem("Change player color...")) CallCommand(CMD_CHANGE_PLAYER_COLOR);
		if(ImGui::MenuItem("Select object by ID...", "O")) CallCommand(CMD_SELECT_OBJECT_ID);
		if(ImGui::MenuItem("Enable aligned movement", "Tab", objmovalign)) CallCommand(CMD_CHANGE_OBJMOVALIGN);
		ImGui::EndMenu();
	}
	if(ImGui::BeginMenu("Game"))
	{
		if(ImGui::MenuItem("Pause", "P", !playMode)) CallCommand(CMD_PAUSE);
		if(ImGui::MenuItem("Increase game speed", "+")) CallCommand(CMD_GAME_SPEED_FASTER);
		if(ImGui::MenuItem("Decrease game speed", "-")) CallCommand(CMD_GAME_SPEED_SLOWER);
		if(ImGui::MenuItem("Start level", "S")) CallCommand(CMD_START_LEVEL);
		if(ImGui::MenuItem("Control client...", "F")) CallCommand(CMD_CONTROL_CLIENT);
		ImGui::Separator();
		if(ImGui::MenuItem("Execute command...", "A")) CallCommand(CMD_EXECUTE_COMMAND);
		if(ImGui::MenuItem("Execute command with target...", "Y")) CallCommand(CMD_EXECUTE_COMMAND_WITH_TARGET);
		if(ImGui::MenuItem("Execute action sequence...", "X")) CallCommand(CMD_RUNACTSEQ);
		if(ImGui::MenuItem("Send event...", "J")) CallCommand(CMD_SEND_EVENT);
		if(ImGui::MenuItem("Cancel all objects' orders")) CallCommand(CMD_CANCEL_ALL_OBJS_ORDERS);
		if(ImGui::MenuItem("Remove battles delayed sequences")) CallCommand(CMD_REMOVE_BATTLES_DELAYED_SEQS);
		//if(ImGui::MenuItem("Quick stampdown...")) CallCommand(CMD_STAMPDOWN_OBJECT);
		//if(ImGui::MenuItem("Open game text window...")) CallCommand(CMD_ENABLE_GTW);
		//if(ImGui::MenuItem("Enable gameplay shortcuts")) CallCommand(CMD_TOGGLEEXPERIMENTALKEYS);
		ImGui::EndMenu();
	}
	if(ImGui::BeginMenu("View"))
	{
		if(ImGui::MenuItem("Move to...")) CallCommand(CMD_CAMPOS);
		if(ImGui::MenuItem("Move to down-left corner", "1")) CallCommand(CMD_CAMDOWNLEFT);
		if(ImGui::MenuItem("Move to down-right corner", "2")) CallCommand(CMD_CAMDOWNRIGHT);
		if(ImGui::MenuItem("Move to up-left corner", "3")) CallCommand(CMD_CAMUPLEFT);
		if(ImGui::MenuItem("Move to up-right corner", "4")) CallCommand(CMD_CAMUPRIGHT);
		if(ImGui::MenuItem("Move to player's manor...", "5")) CallCommand(CMD_CAMMANOR);
		if(ImGui::MenuItem("Move to client's position...", "8")) CallCommand(CMD_CAMCLISTATE);
		if(ImGui::MenuItem("Reset orientation", "7")) CallCommand(CMD_CAMRESETORI);
		ImGui::Separator();
		if(ImGui::MenuItem("Show terrain", "M", enableMap)) CallCommand(CMD_SHOWHIDELANDSCAPE);
		if(ImGui::MenuItem("Show representations", NULL, showrepresentations)) CallCommand(CMD_TOGGLEREPRENSATIONS);
		if(ImGui::MenuItem("Show object tooltips", NULL, enableObjTooltips)) CallCommand(CMD_TOGGLEOBJTOOLTIPS);
		if(ImGui::MenuItem("Show misc information", "L", showTimeObjInfo)) CallCommand(CMD_TOGGLE_TIMEOBJINFO);
		if(ImGui::MenuItem("Show grid", "K", showMapGrid)) CallCommand(CMD_TOGGLEGRID);
		ImGui::EndMenu();
	}
	if(ImGui::BeginMenu("Window"))
	{
		if(ImGui::MenuItem("Open all")) CallCommand(CMD_SWOPENALL);
		if(ImGui::MenuItem("Close all")) CallCommand(CMD_SWCLOSEALL);
		ImGui::Separator();
		if(ImGui::MenuItem("Object information", "F4", swSelObj)) CallCommand(CMD_SWSELOBJ);
		if(ImGui::MenuItem("Level information", "F5", swLevInfo)) CallCommand(CMD_SWLEVINFO);
		if(ImGui::MenuItem("Level tree", "F6", swLevTree)) CallCommand(CMD_SWLEVTREE);
		if(ImGui::MenuItem("Object creation", "F7", swObjCrea)) CallCommand(CMD_SWOBJCREA);
		if(ImGui::MenuItem("Terrain editor", "F8", swMapEditor)) CallCommand(CMD_SWMAPEDITOR);
		if(ImGui::MenuItem("City creator", "F9", swCityCreator)) CallCommand(CMD_SWCITYCREATOR);
		if(ImGui::MenuItem("Minimap", "6", swMinimap)) CallCommand(CMD_SWMINIMAP);
		ImGui::EndMenu();
	}
	if(ImGui::BeginMenu("Help"))
	{
		if(ImGui::MenuItem("Open help.htm", "F1")) CallCommand(CMD_HELP);
		if(ImGui::MenuItem("About...", 0, swAbout)) CallCommand(CMD_ABOUT);
		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();
}

void Test7()
{
	LoadBCP("data.bcp"); ReadLanguageFile(); InitWindow(); InitScene(); InitFont(); InitCursor();
	ImGuiImpl_Init();
	memset(gameobj, 0, MAX_GAMEOBJECTS * sizeof(GameObject*));
	//ddev->SetDialogBoxMode(TRUE);

	AutodetectGameSetVersion();

	InitGfxConsole();
	WriteGfxConsole("wkbre Version " WKBRE_VERSION); // " IN DEVELOPMENT"

#ifdef _MSC_VER
	WriteAndDrawGfxConsole("Compiled with MSVC C/C++ compiler");
#endif
#ifdef __GNUC__
	WriteAndDrawGfxConsole("Compiled with GNU C/C++ compiler");
#endif

	WriteAndDrawGfxConsole((gs_ver == WKVER_BATTLES) ? "WK Battles detected" : ((gs_ver == WKVER_ORIGINAL) ? "WK One/Original detected" : "?"));

	GrowStringList *gsl = new GrowStringList;
	gsl->add("<New empty level>");
	ListFiles("Save_Games", gsl);

/*
	int lx = ListDlgBox(gsl, "Select a saved game you want to load. The list begins with files from data.bcp and ends with your personal saved games in the \"saved\" directory.");
	if(lx == -1) return;
*/
	int lx = 0;
	bool levselected = 0;
	char gaben[64] = {0};
	bool lwfirsttime = 1;
	static ImGuiTextFilter tf;
	while(!levselected)
	{
		ImGuiImpl_NewFrame();
		ImGui::SetNextWindowSize(ImVec2(450,320), ImGuiSetCond_Once);
		ImGui::SetNextWindowPosCenter(ImGuiSetCond_Once);
		ImGui::Begin("Load level/savegame", 0, ImGuiWindowFlags_NoCollapse);
		ImGui::PushItemWidth(-1);
		ImGui::TextWrapped("Select a saved game you want to load. The list begins with files from data.bcp and ends with your personal saved games in the \"saved\" directory.");
		if(lwfirsttime) {ImGui::SetKeyboardFocusHere(); lwfirsttime = 0;}
		tf.Draw();
		ImGui::ListBoxHeader("###FileList", ImVec2(0,200));
		for(int i = 0; i < gsl->len; i++)
			if(tf.PassFilter(gsl->getdp(i)))
			{
				if(ImGui::Selectable(gsl->getdp(i), lx == i))
					lx = i;
				if(ImGui::IsItemHovered())
					if(ImGui::IsMouseDoubleClicked(0))
						{lx = i; levselected = 1;}
			}
		ImGui::ListBoxFooter();
		if(ImGui::Button("Load"))
			levselected = 1;
		ImGui::SameLine();
		if(ImGui::Button("Play"))
		{
			levselected = 1;
			playAfterLoading = 1;
		}
		if(ImGui::IsItemHovered()) ImGui::SetTooltip("Load and start/unpause the game automatically.");
		ImGui::SameLine();
		if(ImGui::Button("Exit"))
			return;
		ImGui::PopItemWidth();
		ImGui::End();

		BeginDrawing();
		DrawGfxConsoleInCurrentFrame();
		renderer->InitImGuiDrawing();
		ImGui::Render();
		EndDrawing();
		HandleWindow();
	}

	char sgn[384] = "Save_Games\\\0";
	strcat(sgn, gsl->getdp(lx));
	delete gsl;
	if(lx == 0) CreateEmptyMap(128, 128);
	LoadSaveGame((lx != 0) ? sgn : "newlevel.lvl");

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

	Bitmap minimapTempBmp;
	CreateMinimap(minimapTempBmp, mmbmplen);
	minimapBitmap = ConvertBitmapToB8G8R8A8(&minimapTempBmp);
	//DrawObjectsOnMinimapBmp(minimapBitmap, false);
	minimapTexture = CreateTexture(minimapBitmap, 1);
	FreeBitmap(minimapBitmap);
	free(minimapTempBmp.pix);

	if(playAfterLoading)
	{
		char *e; bool islvl = 0;
		if(e = strrchr(sgn, '.'))
			if(!stricmp(e+1, "lvl"))
				islvl = 1;
		if(clistates.len > 0)
		{
			curclient = clistates.getpnt(0);
			if(islvl)
			 if(curclient->obj.valid())
			{
				curclient->camerapos = curclient->obj->startcampos;
				curclient->cameraori = curclient->obj->startcamori;
			}
		}
		if (islvl)
		{
			SendEventToObjAndSubord(levelobj, PDEVENT_ON_LEVEL_START);
			isLevelStarted = 1;
		}
		playMode = 1;
	}

	askBeforeExit = 1;

	while(!appexit)
	{
		if(playMode || keypressed['0'])
		{
			keypressed['0'] = 0;
			AdvanceTime();
			ProcessAllOrders();
			CheckBattlesDelayedSequences();
			UpdateContainerPos(levelobj);
			UpdateClientsCam();
		}

		// Copy controlled client's camera to drawing/scene camera.
		if(curclient)
		{
			camerapos = curclient->camerapos;
			campitch = curclient->cameraori.x;
			camyaw = curclient->cameraori.y;
		}

		if(!winMinimized)
		{
			if(swMinimap)
			{
				CreateMinimap(minimapTempBmp, mmbmplen, mmshowedge);
				minimapBitmap = ConvertBitmapToB8G8R8A8(&minimapTempBmp);
				if(mmshowobjs) DrawObjectsOnMinimapBmp(minimapBitmap, mmshowedge);
				renderer->UpdateTexture(minimapTexture, minimapBitmap);
				FreeBitmap(minimapBitmap);
				free(minimapTempBmp.pix);
			}
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
			if(enableObjTooltips) DrawTooltip();

			ImGuiImpl_NewFrame();
			if(menuVisible) IGMainMenuBar();
			IGSelectedObject();
			IGLevelTree();
			IGLevelInfo();
			IGObjectCreation();
			IGAbout();
			IGMapEditor();
			IGCityCreator();
			IGTest();
			IGNDlgBox();
			IGMinimap();

			renderer->InitImGuiDrawing();
			ImGui::Render();

			InitRectDrawing();

			if(statustext || notificationtext) if(!playView)
			{
				NoTexture(0);
				DrawRect(0, scrh-20 , scrw, 20, notificationtext ? 0xC0800000 : 0xC0000080);
				DrawFont(0, scrh-20, notificationtext ? notificationtext : statustext);
			}
			if(notificationtext)
				if((GetTickCount() - notiftimeref) >= notifdelay)
					{free(notificationtext);
					notificationtext = 0;}

			if(showTimeObjInfo)
			{
				char st[64];
				sprintf(st, "FPS: %i - ODPF: %i", drawfps, objsdrawn);
				DrawFont(scrw - 188, 0, st);
			}

			if(mousetool) ChangeCursor(defcursor);
			else if(!objtypeToStampdown) SetTargetCursor();
			else if((objtypeToStampdown->type == CLASS_PLAYER) || (objtypeToStampdown->type == CLASS_TERRAIN_ZONE) || playerToGiveStampdownObj.valid())
				ChangeCursor(buildcursor);
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

		if(keypressed[VK_SHIFT]) {walkstep = objmovalign ? 5.0f : 4.0f; camwalkstep = 4.0f;}
		else {walkstep = objmovalign ? 2.5f : 1.0f; camwalkstep = 1.0f;}
		if(keypressed[VK_UP])
		{
			if(objmovalign) if(keypressed[VK_CONTROL]) keypressed[VK_UP] = 0;
			Vector3 m = Vector3(vLAD.x, 0, vLAD.z);
			MoveKeyPress(m);
		}
		if(keypressed[VK_DOWN])
		{
			if(objmovalign) if(keypressed[VK_CONTROL]) keypressed[VK_DOWN] = 0;
			Vector3 m = Vector3(-vLAD.x, 0, -vLAD.z);
			MoveKeyPress(m);
		}
		if(keypressed[VK_LEFT])
		{
			if(objmovalign) if(keypressed[VK_CONTROL]) keypressed[VK_LEFT] = 0;
			Vector3 m = Vector3(-vLAD.z, 0, vLAD.x);
			MoveKeyPress(m);
		}
		if(keypressed[VK_RIGHT])
		{
			if(objmovalign) if(keypressed[VK_CONTROL]) keypressed[VK_RIGHT] = 0;
			Vector3 m = Vector3(vLAD.z, 0, -vLAD.x);
			MoveKeyPress(m);
		}
		if(keypressed['E']) DisplaceCurCamera(Vector3(0.0f,  camwalkstep, 0.0f));
		if(keypressed['D']) DisplaceCurCamera(Vector3(0.0f, -camwalkstep, 0.0f));
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

		if(keypressed['M'])
			{keypressed['M'] = 0; CallCommand(CMD_SHOWHIDELANDSCAPE);}
		if(keypressed[VK_ESCAPE])
			{keypressed[VK_ESCAPE] = 0; CallCommand(CMD_QUIT);}

		if(keypressed[VK_F3]) {keypressed[VK_F3] = 0; CallCommand(CMD_SAVE);}

		if(keypressed['5']) {keypressed['5'] = 0; CallCommand(CMD_CAMMANOR);}
/*
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
*/
		if(keypressed[VK_F4])
			{keypressed[VK_F4] = 0; CallCommand(CMD_SWSELOBJ);}
		if(keypressed[VK_F5])
			{keypressed[VK_F5] = 0; CallCommand(CMD_SWLEVINFO);}
		if(keypressed[VK_F6])
			{keypressed[VK_F6] = 0; CallCommand(CMD_SWLEVTREE);}
		if(keypressed[VK_F7])
			{keypressed[VK_F7] = 0; CallCommand(CMD_SWOBJCREA);}
		if(keypressed[VK_F8])
			{keypressed[VK_F8] = 0; CallCommand(CMD_SWMAPEDITOR);}
		if(keypressed[VK_F9])
			{keypressed[VK_F9] = 0; CallCommand(CMD_SWCITYCREATOR);}
		if(keypressed[VK_F10])
			{keypressed[VK_F10] = 0; CallCommand(CMD_SWCITYCREATOR);}
		if(keypressed[VK_F11])
			{keypressed[VK_F11] = 0; CallCommand(CMD_RANDOMSUBTYPE);}

		if(keypressed['6']) {keypressed['6'] = 0; CallCommand(CMD_SWMINIMAP);}

		if(keypressed[VK_F1])
			{keypressed[VK_F1] = 0; CallCommand(CMD_HELP);}

		if(keypressed[VK_F2])
			{keypressed[VK_F2] = 0; CallCommand(CMD_RESETOBJPOS);}
		//if(keypressed[VK_F4])
		//	{keypressed[VK_F4] = 0; CallCommand(CMD_DELETEOBJCLASS);}

		if(keypressed[VK_DELETE])
			{keypressed[VK_DELETE] = 0; CallCommand(CMD_DELETESELOBJECT);}
		//if(keypressed[VK_INSERT])
		//	{keypressed[VK_INSERT] = 0; CallCommand(CMD_DUPLICATESELOBJECT);}
		if(keypressed['N'])
			{keypressed['N'] = 0; CallCommand(CMD_DUPLICATESELOBJECT);}

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

		if(keypressed[VK_NUMPAD0] || mmbPressed)
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

		if(mouseWheel != 0.0f)
		{
			DisplaceCurCamera(Vector3(0.0f,  mouseWheel*3.0f, 0.0f));
			mouseWheel = 0.0f;
		}

		if(!lmbPressed)
		if(multiSel)
		{
			multiSel = 0;
			for(int i = 0; i < msellist.len; i++)
				SelectObject(msellist[i]);
		}
		msellist.clear();

		if(!lmbPressed) mousetoolpress_l = 0;
		if(!rmbPressed) mousetoolpress_r = 0;

		if(mousetoolpress_l) switch(mousetool)
		{
			case 1:
			case 2:
			case 6:
			case 10:
				ApplyBrush(0);
				break;
			case 3:
			case 5:
				if(!lakemove_sellake) break;
				lakemove_sellake->y = (lakemove_mousey_ref - mouseY) * 0.1f + lakemove_waterlev_ref;
				FloodfillWater();
				break;
			case 47:
				int x = mapstdownpos.x / 5 + mapedge, z = mapheight - (mapstdownpos.z / 5 + mapedge);
				DrawATS(x, z);
				break;
		}

		if(mousetoolpress_r) switch(mousetool)
		{
			case 1:
			case 47:
			{
				int x = mapstdownpos.x / 5 + mapedge, z = mapheight - (mapstdownpos.z / 5 + mapedge);
				//printf("x=%i  z=%i\n", x, z);
				MapTile *mt = &(maptiles[z * mapwidth + x]);
				//printf("texgrp=%s  tex=%i\n", mt->mt->grp->name, mt->mt->id);
				curtexgrp = mt->mt->grp;
				curtex = mt->mt;
				men_rot = mt->rot;
				men_xflip = mt->xflip;
				men_zflip = mt->zflip;
				break;
			}
			case 2:
				ApplyBrush(1);
				break;
		}

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
/*
			if(selobjects.len)
			if(selobjects.first->value.valid())
			{
				GameObject *o = selobjects.first->value.get();
				printf("animtag = %i, animlooping = %i\n", o->animtag, o->animlooping);
			}
*/
			swTest = !swTest;
		}
#endif
		if(keypressed['A']) {keypressed['A'] = 0; CallCommand(CMD_EXECUTE_COMMAND);}
		if(keypressed['Y']) {keypressed['Y'] = 0; CallCommand(CMD_EXECUTE_COMMAND_WITH_TARGET);}
		if(keypressed['X']) {keypressed['X'] = 0; CallCommand(CMD_RUNACTSEQ);}
		if(keypressed['C']) {keypressed['C'] = 0; CallCommand(CMD_CREATE_MAPPED_TYPE_OBJECT);}
		if(keypressed['V']) {keypressed['V'] = 0; CallCommand(CMD_STAMPDOWN_OBJECT);}
		//if(keypressed['Q']) {keypressed['Q'] = 0; CallCommand(CMD_CANCEL_ALL_OBJS_ORDERS);}
#if 0
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
#endif
		if(keypressed['S']) {keypressed['S'] = 0; CallCommand(CMD_START_LEVEL);}
#if 0
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
#endif
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