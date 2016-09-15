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

void ReadGTWPage(char **pntfp, GTWPage *g)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!stricmp(word[0], "PAGE_END"))
			return;
		else if(!stricmp(word[0], "TEXT_BODY"))
			g->textBody = GetLocText(word[1]);
	}
	ferr("UEOF");
}

void ReadGTWButton(char **pntfp, GTWButton *g)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	g->ocsequence = 0;
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!stricmp(word[0], "BUTTON_END"))
			return;
		else if(!stricmp(word[0], "BOTTOMLEFT_X"))
			g->px = atoi(word[1]);
		else if(!stricmp(word[0], "BOTTOMLEFT_Y"))
			g->py = atoi(word[1]);
		else if(!stricmp(word[0], "PIXEL_WIDTH"))
			g->pw = atoi(word[1]);
		else if(!stricmp(word[0], "PIXEL_HEIGHT"))
			g->ph = atoi(word[1]);
		else if(!stricmp(word[0], "BUTTON_TEXT"))
			g->text = GetLocText(word[1]);
		else if(!stricmp(word[0], "ON_CLICK_WINDOW_ACTION"))
			g->ocwinaction = stfind_cs(GTW_BUTTON_WINDOW_ACTION_str, GTW_BUTTON_WINDOW_ACTION_NUM, word[1]);
		else if(!stricmp(word[0], "ON_CLICK_SEQUENCE"))
			g->ocsequence = ReadActSeq(pntfp);
	}
	ferr("UEOF");
}

void ReadCGameTextWindow(char **pntfp, char **fstline)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	CGameTextWindow *c = &(gsgametextwin[strGameTextWindow.find(fstline[1])]);
	c->forceReadAllPages = c->putAtBottomOfScreen = 0;
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!stricmp(word[0], "GAME_TEXT_WINDOW_END"))
			return;
		else if(!stricmp(word[0], "BOTTOMLEFT_X"))
			c->px = atof(word[1]);
		else if(!stricmp(word[0], "BOTTOMLEFT_Y"))
			c->py = atof(word[1]);
		else if(!stricmp(word[0], "PIXEL_WIDTH"))
			c->pw = atoi(word[1]);
		else if(!stricmp(word[0], "FORCE_READ_ALL_PAGES"))
			c->forceReadAllPages = atoi(word[1]);
		else if(!stricmp(word[0], "PUT_AT_BOTTOM_OF_SCREEN"))
			c->putAtBottomOfScreen = 1;
		else if(!stricmp(word[0], "PAGE"))
			ReadGTWPage(pntfp, c->pages.addp());
		else if(!stricmp(word[0], "BUTTON"))
			{GTWButton *b = c->buttons.addp();
			b->gtw = c;
			ReadGTWButton(pntfp, b);}
	}
	ferr("UEOF");
}

void TurnGTWPage(CGameTextWindow *g, int p)
{
	g->curpage = p;
	g->tbe->setCaption(g->pages.getpnt(g->curpage)->textBody);
}

void OnGTWButtonClick(void *param)
{
	GTWButton *b = (GTWButton*)param;
	switch(b->ocwinaction)
	{
		case GTW_BUTTON_WINDOW_ACTION_CLOSE_WINDOW:
			b->gtw->ge->enabled = 0; break;
		case GTW_BUTTON_WINDOW_ACTION_MOVE_NEXT_PAGE:
			if(b->gtw->curpage < (b->gtw->pages.len-1)) TurnGTWPage(b->gtw, b->gtw->curpage+1); break;
		case GTW_BUTTON_WINDOW_ACTION_MOVE_PREVIOUS_PAGE:
			if(b->gtw->curpage > 0) TurnGTWPage(b->gtw, b->gtw->curpage-1); break;
		case GTW_BUTTON_WINDOW_ACTION_MOVE_FIRST_PAGE:
			TurnGTWPage(b->gtw, 0); break;
	}
	if(b->ocsequence)
	{
		SequenceEnv env;
		//env.self = curclient->player; // ???
		env.self = FindObjID(1027);
		b->ocsequence->run(&env);
	}
}

void CreateGTW(CGameTextWindow *g)
{
	GEWindow *w = new GEWindow;
	g->ge = w;
	actualpage->add(w);
	w->enabled = 0;
	w->bgColor = 0x80000080;

	GEStaticText *t = new GEStaticText;
	g->tbe = t;
	w->add(t);
	t->setRect(0, 20, 520, 160);
	t->setCaption(g->pages.getpnt(0)->textBody);

	for(int i = 0; i < g->buttons.len; i++)
	{
		GTWButton *x = g->buttons.getpnt(i);
		GETextButton *b = new GETextButton;
		w->add(b);
		b->setRect(i*150, 180, 145, 20);
		b->setCaption(x->text);
		b->buttonClick = OnGTWButtonClick;
		b->cbparam = (void*)x;
	}

	g->curpage = 0;
}

void InitGTWs()
{
	for(int i = 0; i < strGameTextWindow.len; i++)
		CreateGTW(&(gsgametextwin[i]));
}

void EnableGTW(CGameTextWindow *g)
{
	if(g->ge->enabled) return;
	g->ge->enabled = 1;
	TurnGTWPage(g, 0);
	g->ge->setRect(scrw/2-260, scrh-200-48, 520, 200);
	g->ge->onResize();
}

void DisableGTW(CGameTextWindow *g)
{
	if(!g->ge->enabled) return;
	g->ge->enabled = 0;
}