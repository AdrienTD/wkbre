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
			c->buttonEnabled = GetTexture(word[1], 0);
		// ...
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

void ExecuteCommand(GameObject *o, CCommand *c, GameObject *tg)
{
	if(!CanExecuteCommand(o->objdef, c)) return;
	if(c->startSeq)
	{
		SequenceEnv env; env.self = o; env.target = tg;
		c->startSeq->run(&env);
	}
}

/*
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
		char **w = word;
		if(!strcmp(word[0], "TEST"))
		{
			w++;
			c->test = ReadValue(&w);
		}
		else if(!strcmp(word[0], "HINT"))
		{
			w += 2;
			c->hintstr = GetLocText(word[1]);
			while((w - word) < nwords)
				c->hintvalues.add(ReadValue(&w));
		}
	}
	ferr("UEOF");
}*/