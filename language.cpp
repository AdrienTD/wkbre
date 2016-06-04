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

GrowStringList locLabel, locText;
int numLocStr;

void ReadLanguageFile()
{
	char *fcnt, *p, line[3072], *word[MAX_WORDS_IN_LINE], *e; int s, fsize;
	// TODO: Load file data and store pointer in fcnt.
	LoadFile("Languages\\Language.txt", &fcnt, &fsize, 1);
	fcnt[fsize] = 0;
	locLabel.clear(); locText.clear(); numLocStr = 0;
	p = fcnt;
	while(*p)
	{
		p = GetLine(p, line);
		s = GetWords(line, word);
		if(s < 2) continue;
		locLabel.add(word[0]);
		e = TranslateEscapeSequences(word[1]);
		locText.add(e);
		free(e);
		numLocStr++;
	}
	free(fcnt);
}

char *GetLocText(char *l)
{
	int f = locLabel.find_backwards(l);
	if(f != -1)
		return locText.getdp(f);
	else
		return "<Not localized>";
}