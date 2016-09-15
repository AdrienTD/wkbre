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

struct GTWPage
{
	char *textBody;
	//char *activationSound;
};

struct CGameTextWindow;

struct GTWButton
{
	CGameTextWindow *gtw;
	int px, py, pw, ph;
	char *text;
	int ocwinaction;
	ActionSeq *ocsequence;
};

struct CGameTextWindow
{
	float px, py; int pw;
	boolean forceReadAllPages, putAtBottomOfScreen;
	GrowList<GTWPage> pages;
	GrowList<GTWButton> buttons;

	GUIElement *ge, *tbe; int curpage;
};

void ReadCGameTextWindow(char **pntfp, char **fstline);
void InitGTWs();
void EnableGTW(CGameTextWindow *g);
void DisableGTW(CGameTextWindow *g);
