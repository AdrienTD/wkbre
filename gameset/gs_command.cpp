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

void ReadCCommand(char **pntfp, char **fstline)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords, strdex;
	CCommand *c = &(gscommand[strdex = strCommand.find(fstline[1])]);
	c->name = strCommand.getdp(strdex);
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if( *((uint*)(word[0])) == '_DNE' )
			return;
		if(!strcmp(word[0], "BUTTON_ENABLED"))
			c->buttonEnabled = GetTexture(word[1], 1);
		else if(!strcmp(word[0], "BUTTON_DEPRESSED"))
			c->buttonDepressed = GetTexture(word[1], 1);
		else if(!strcmp(word[0], "BUTTON_HIGHLIGHTED"))
			c->buttonHighlighted = GetTexture(word[1], 1);
		// ...
		else if(!strcmp(word[0], "CURSOR"))
		{
			if(FileExists(word[1]))
				c->cursor = LoadCursor(word[1]);
		}
		else if(!strcmp(word[0], "CURSOR_CONDITION"))
			c->cursorConditions.add(strEquation.find(word[1]));
		else if(!strcmp(word[0], "CURSOR_AVAILABLE"))
		{
			c->cursorAvailableConds.add(&gscondition[strCondition.find(word[1])]);
			if(FileExists(word[2]))
				c->cursorAvailableCurs.add(LoadCursor(word[2]));
			else
				c->cursorAvailableCurs.add(0);
		}
		else if(!strcmp(word[0], "ICON_CONDITION"))
			c->iconConditions.add(strEquation.find(word[1]));
		else if(!strcmp(word[0], "USE_ORDER"))
			c->order = &(gsorder[strOrder.find(word[1])]);
		else if(!strcmp(word[0], "START_SEQUENCE"))
			c->startSeq = ReadActSeq(pntfp);
	}
	ferr("UEOF");
}

int CanExecuteCommand(CObjectDefinition *od, CCommand *cc)
{
	for(int i = 0; i < od->offeredCmds.len; i++)
		if(od->offeredCmds[i] == cc)
			return 1;
	return 0;
}

void ExecuteCommand(GameObject *o, CCommand *c, GameObject *tg, int assignmode)
{
	if(!CanExecuteCommand(o->objdef, c)) return;
	if(c->startSeq)
	{
		SequenceEnv env; env.self = o; env.target = tg; findertargetcommand = 1;
		c->startSeq->run(&env);
		findertargetcommand = 0;
	}
	if(c->order)
		AssignOrder(o, c->order, assignmode, tg);
}

void ReadCCondition(char **pntfp, char **fstline)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	CCondition *c = &(gscondition[strCondition.find(fstline[1])]);
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if( *((uint*)(word[0])) == '_DNE' )
			return;
		char **w = word + 1;
		if(!strcmp(word[0], "TEST"))
			c->test = ReadValue(&w);
		else if(!strcmp(word[0], "HINT"))
			ReadDynText(&c->hint, w);
	}
	ferr("UEOF");
}

void OnCmdButtonClick(void *param)
{
	DynListEntry<goref> *n;
	for(DynListEntry<goref> *e = selobjects.first; e; e = n)
	{
		n = e->next;
		if(e->value.valid())
			ExecuteCommand(e->value.get(), (CCommand*)param, 0, ORDERASSIGNMODE_FORGET_EVERYTHING_ELSE);
	}
}

void CreateCommandButtons()
{
	for(int i = 0; i < strCommand.len; i++)
	{
		CCommand *c = &(gscommand[i]);
		GEPicButton *b = new GEPicButton;
		c->gButton = b;
		actualpage->add(b);
		b->enabled = 0;
		b->buttonClick = OnCmdButtonClick;
		b->cbparam = (void*)c;
		b->texNormal = c->buttonEnabled;
		b->texHover = c->buttonHighlighted;
		b->texPressed = c->buttonDepressed;
	}
}
