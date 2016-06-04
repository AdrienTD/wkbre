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

// Misc. useful functions

#include "global.h"

int gline = 0;

#ifdef WKBRE_RELEASE
void fc_ferr(char *s)
{
	MessageBox(hWindow, s, "Fatal Error", 16);
	fflush(stdout); exit(-2);
}
#else
void fc_ferr(char *s, char *f, int l)
{
	//printf("Error: %s\n", s);
	//printf("gline = %i\n", gline);
	printf("Error: %s\nLook at: %s, Line %i\n", s, f, l);
	fflush(stdout); brkpnt(); exit(-2);
}
#endif

int stfind_cs(char **t, int len, char *f)
{
	for(int i = 0; i < len; i++)
		if(!strcmp(t[i], f))
			return i;
	return -1;
}

char *GetLine(char *f, char *str)
{
	char *o, *frp;
	o = str; frp = f;
	while(1)
	{
		if((*frp == 0) || (*frp == 255)) break;
		if(*frp == '\r') {frp++; continue;}
		if(*frp == '\n') {frp++; break;}
		if(*frp == '/')
		{
			if(*(frp+1) == '*')
			{
				frp += 2;
				while(!((*frp == '*') && (*(frp+1) == '/')))
					{if((*(frp+1) == 0) || (*(frp+1) == 255)) {frp++; goto glend;} frp++;}
				frp += 2;
			}
			if(*(frp+1) == '/')
			{
				frp += 2;
				while(*frp != '\n')
					{if(*frp == 0) goto glend; frp++;}
				frp++; goto glend;
			}
		}
		*o = *frp; o++; frp++;
	}
glend:	*o = 0; gline++;
	return frp;
}

int GetWords(char *str, char **list)
{
	char *p; int n = 0;
	p = str;
	while(*p)
	{
		while(isspace(*p)) p++;
		if(!(*p)) return n;

		if(*p != '\"')
		{
			list[n] = p;
			n++; if(n >= MAX_WORDS_IN_LINE) return n;
			while(!isspace(*p)) {if(!(*p)) return n; p++;}
			if(!(*p)) return n; // Can be removed because isspace(0) = 0
			*p = 0; p++; if(!(*p)) return n;
		} else {
			p++; if(!(*p)) return n;
			list[n] = p;
			n++; if(n >= MAX_WORDS_IN_LINE) return n;
			while(*p != '\"') {if(!(*p)) return n; p++;}
			*p = 0; p++; if(!(*p)) return n;
		}
	}
	return n;
}

uint readInt(FILE *f)
{
	uint r = 0;
	fread(&r, 4, 1, f);
	return r;
}

char *TranslateEscapeSequences(char *str)
{
	char *r = (char*)malloc(strlen(str)+1);
	if(!r) ferr("TranslateEscapeSequences mem alloc fail.");
	char *o = r; char *p = str;
	while(*p)
	{
		if(*p == '\\')
		{
			switch(*(++p))
			{
				case 'c':
					*(o++) = 1; break;
				case 't':
					*(o++) = '\t'; break;
				case 'n':
					*(o++) = '\n'; break;
				case 'r':
					*(o++) = '\r'; break;
				case '\\':
					*(o++) = '\\'; break;
				default:
					*(o++) = '\\';
					*(o++) = *p; break;
			}
		} else
			*(o++) = *p;
		p++;
	}
	*o = 0;
	return r;
}