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

void ReadCCameraPath(char **pntfp, char **fstline)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords, strdex;
	CCameraPath *c = &(gscamerapath[strdex = strCameraPath.find(fstline[1])]);
	c->name = strCameraPath.getdp(strdex);
	c->startAtCurCamPos = c->loopCamPath = 0;
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if( *((uint*)(word[0])) == '_DNE' )
			return;
		if(!stricmp(word[0], "INTERPOLATE_TO"))
		{
			CamPathNode *n = c->nodes.addp();
			char **w = word + 1;
			n->pos = ReadCPosition(&w);
			n->time = ReadValue(&w);
		}
		else if(!stricmp(word[0], "START_AT_CURRENT_CAMERA_POSITION"))
			c->startAtCurCamPos = 1;
		else if(!stricmp(word[0], "LOOP_CAMERA_PATH"))
			c->loopCamPath = 1;
	}
}
