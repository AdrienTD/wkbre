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

void ReadCPackage(char **pntfp, char **fstline)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	CPackage *p = &(gspackage[strPackage.find(fstline[1])]);
	p->relatedParty = 0;
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if( *((uint*)(word[0])) == '_DNE' )
			return;
		if(!stricmp(word[0], "GAME_EVENT"))
		{
			p->events.add(strGameEvent.find(word[1]));
		}
		else if(!stricmp(word[0], "ITEM_MODIFICATION"))
		{
			PackItemMod *m = p->mods.addp();
			m->action = stfind_cs(PACKAGE_ITEM_MOD_str, PACKAGE_ITEM_MOD_NUM, word[1]);
			m->item = strItems.find(word[2]);
			char **w = word + 3;
			m->value = ReadValue(&w);
		}
		else if(!stricmp(word[0], "RELATED_PARTY"))
		{
			char **w = word + 1;
			p->relatedParty = ReadFinder(&w);
		}
	}
}

void SendCPackage(GameObject *o, CPackage *p, SequenceEnv *env)
{
	for(int i = 0; i < p->mods.len; i++)
	{
		PackItemMod *m = p->mods.getpnt(i);
		valuetype v = m->value->get(env), n;
		switch(m->action)
		{
			case PACKAGE_ITEM_MOD_REPLACE:
				n = v; break;
			case PACKAGE_ITEM_MOD_INCREASE:
				n = o->getItem(m->item) + v; break;
			case PACKAGE_ITEM_MOD_REDUCE:
				n = o->getItem(m->item) - v; break;
		}
		o->setItem(m->item, n);
	}

	SequenceEnv ctx;
	env->copyAll(&ctx);
	if(p->relatedParty)
		ctx.relatedparty = p->relatedParty->getfirst(env);

	for(int i = 0; i < p->events.len; i++)
		SendGameEvent(&ctx, o, p->events[i]);
}
