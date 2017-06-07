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

#define MAX_WORDS_IN_LINE 1024
#define macrostr(i) #i

#ifdef _MSC_VER

#ifdef _WIN64
#define brkpnt() DebugBreak()
#else
#define brkpnt() {__asm int 3}
#endif

#else
#define brkpnt() 
#endif

#ifndef _MSC_VER
#define strcpy_s(a, b, c) strcpy(a, c)
#endif

#ifdef WKBRE_RELEASE
#define ferr(s, ...) {fc_ferr(s, ##__VA_ARGS__);}
void fc_ferr(char *s, ...);
#else
#define ferr(s, ...) {fc_ferr(__FILE__, __LINE__, s, ##__VA_ARGS__);}
void fc_ferr(char *f, int l, char *s, ...);
#endif

typedef void (*vfwa)(void*);
typedef void (*voidfunc)();
template <class T> struct couple {T x, y;};

extern int gline;

int stfind_cs(char **t, int len, char *f);
char *GetLine(char *f, char *str);
int GetWords(char *str, char **list);
uint readInt(FILE *f);
char *TranslateEscapeSequences(char *str);