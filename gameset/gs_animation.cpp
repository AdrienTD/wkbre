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

CPlayAnimation *ReadPlayAnimation(char **pntfp, char **fstline, int fwords)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords, strdex;
	CPlayAnimation *pa = new CPlayAnimation;
	pa->defaulttag = 0;
	if(fwords >= 2)
	if(!stricmp(fstline[1], "TAG"))
	{
		pa->defaulttag = strAnimationTag.find(fstline[2]);
		return pa;
	}
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if( *((uint*)(word[0])) == '_DNE' )
			return pa;
		if(!stricmp(word[0], "ON_EQUATION_SUCCESS"))
		{
			pa->equlist.add(strEquation.find(word[1]));
			pa->taglist.add(strAnimationTag.find(word[2]));
		}
		else if(!stricmp(word[0], "DEFAULT_ANIMATION"))
			pa->defaulttag = strAnimationTag.find(word[1]);
	}
	ferr("UEOF"); return 0;
}

int GetBestPlayAnimationTag(CPlayAnimation *pa, GameObject *o)
{
	SequenceEnv c; c.self = o;
	for(int i = 0; i < pa->equlist.len; i++)
		if(DoesObjectHaveUniqueAnimation(o, pa->taglist[i]))
			if(stpo(equation[pa->equlist[i]]->get(&c)))
				return pa->taglist[i];
	return pa->defaulttag;
}

////////////

CMovementBand *ReadMovementBand(char **pntfp)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords, strdex;
	CMovementBand *mb = new CMovementBand;
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if( *((uint*)(word[0])) == '_DNE' )
			return mb;
		if(!stricmp(word[0], "NATURAL_MOVEMENT_SPEED"))
			mb->naturalMovSpeed = atof(word[1]);
		else if(!stricmp(word[0], "PLAY_ANIMATION"))
			mb->playAnim = ReadPlayAnimation(pntfp, word, nwords);
	}
	ferr("UEOF"); return 0;
}