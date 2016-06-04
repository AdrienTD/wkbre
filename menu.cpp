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
void CallCommand(int cmd);

#define MENUBUTTON_WIDTH 90
#define SUBMENUBUTTON_WIDTH 360
#define MENUBUTTON_HEIGHT 20
#define numarrayelem(x) (sizeof((x))/sizeof((x)[0]))

struct SubMenuEntry
{
	char *str; int id;
};

char *menubarstr[] = {"File", "Object", "View", "Help"};

SubMenuEntry menucmds[] = {
{"Save as...", CMD_SAVE},
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
{"Enable/disable experimental keys", CMD_TOGGLEEXPERIMENTALKEYS},
{"About...", CMD_ABOUT},
{0,0},
};

GUIPage menubar;
GUIPage menubarentry[numarrayelem(menubarstr)];

void MenuBarEntryClick(void *param)
{
	//printf("You clicked menu bar entry %i.\n", (int)param);
	actualpage = &(menubarentry[(int)param]);
}

void SubMenuButtonClick(void *param)
{
	//printf("Command id %i\n", (int)param);
	actualpage = actualpage->parent;
	CallCommand((int)param);
}

void InitMenuBar()
{
	for(int i = 0; i < numarrayelem(menubarstr); i++)
		menubar.add(new GUITextButton(i*MENUBUTTON_WIDTH, 0, MENUBUTTON_WIDTH, MENUBUTTON_HEIGHT, MenuBarEntryClick, menubarstr[i], (void*)i));

	int m = 0; GUIPage *mbe = &(menubarentry[m]);
	SubMenuEntry *e = menucmds;
	int y = 0;
	while(1)
	{
		if(!e->str)
		{
			mbe->parent = &menubar; mbe->goback = 1;
			m++; if(m >= numarrayelem(menubarstr)) break;
			mbe = &(menubarentry[m]);
			y = 0; e++; continue;
		}
		mbe->add(new GUITextButton(m*MENUBUTTON_WIDTH, MENUBUTTON_HEIGHT+y*MENUBUTTON_HEIGHT, SUBMENUBUTTON_WIDTH, MENUBUTTON_HEIGHT, SubMenuButtonClick, e->str, (void*)e->id));
		y++; e++;
	}
}