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

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define isdelimiter(a) (_isspace((a))||((a)==0)||((a)==255))
#define MAX_WORD_SIZE 80
#define MAX_DEFS 1024
#define MAX_TABS 15

#ifndef _MSC_VER
#define strcat_s(a, b, c) strcat(a, c)
#endif

FILE *file = 0; FILE *foc, *foh;
char word[MAX_WORD_SIZE];
char wroot[MAX_WORD_SIZE], ((wdef[MAX_WORD_SIZE])[MAX_TABS+1])[MAX_DEFS], (wtab[MAX_WORD_SIZE])[MAX_TABS];
int numtabs, numdefs;

void ferr(int n)
{
	printf("Error %i\n", n);
	if(file) fclose(file); if(foc) fclose(foc); if(foh) fclose(foh);
	exit(n);
}

int GetWord()
{
	char c, *o;
	word[0] = 0; o = word;
	do {c = fgetc(file);} while(isspace(c) && (!feof(file)));
	if(feof(file)) return 0;
	if(c == '\"')
		while((!feof(file))) {*(o++) = c; c = fgetc(file); if(c == '\"') {*(o++) = c; break;}}
	else
		while(!isspace(c) && (!feof(file))) {*(o++) = c; c = fgetc(file);}
	*o = 0;
	return 1;
}

int main(int argc, char *argv[])
{
	int i, j; char nf1[80], nf2[80]; char *p;
	if(argc < 3) ferr(-1);
	assert(file = fopen(argv[1], "r"));
	strncpy(nf1, argv[2], 79); strncpy(nf2, argv[2], 79);
	strcat_s(nf1, 79, ".c"); strcat_s(nf2, 79, ".h");
	assert(foc = fopen(nf1, "w")); assert(foh = fopen(nf2, "w"));
	while(!feof(file))
	{
		if(!GetWord()) break;
		numtabs = numdefs = 0;
		strcpy(wroot, word);
		while(1)
		{
			if(feof(file)) ferr(-2);
			if(!GetWord()) goto pend;
			if(word[0] == '|')
			{
				if(!GetWord()) goto pend; strcpy(wtab[numtabs], word);
				numtabs++; continue;
			}
			if(word[0] == '{')
				break;
		}
		while(1)
		{
			if(feof(file)) ferr(-3);
			if(!GetWord()) goto pend;
			if(word[0] == '}')
				break;
			strcpy((wdef[numdefs])[0], word);
			for(i = 1; i <= numtabs; i++)
			{
				if(!GetWord()) goto pend;
				if(p = strchr(word, '%'))
					if(p[1] == '@')
					{
						*p = 0;
						strcpy((wdef[numdefs])[i], word);
						strcat((wdef[numdefs])[i], (wdef[numdefs])[0]);
						strcat((wdef[numdefs])[i], p+2);
						continue;
					}
				strcpy((wdef[numdefs])[i], word);
			}
			numdefs++;
		}
		for(i = 0; i < numdefs; i++)
			fprintf(foh, "#define %s%s %i\n", wroot, (wdef[i])[0], i);
		fprintf(foh, "#define %sNUM %i\n", wroot, numdefs);
		fprintf(foh, "extern char *%sstr[%i];\n", wroot, numdefs);
		fprintf(foc, "char *%sstr[%i] = {\n", wroot, numdefs);
		for(i = 0; i < numdefs; i++)
			fprintf(foc, "\"%s\",\n", (wdef[i])[0]);
		fprintf(foc, "};\n");
		for(i = 0; i < numtabs; i++)
		{
			fprintf(foh, "extern %s %stab%i[%i];\n", wtab[i], wroot, i, numdefs);
			fprintf(foc, "%s %stab%i[%i] = {\n", wtab[i], wroot, i, numdefs);
			for(j = 0; j < numdefs; j++)
				fprintf(foc, "%s,\n", (wdef[j])[i+1]);
			fprintf(foc, "};\n");
		}
	}
pend:	fclose(file); if(foc) fclose(foc); if(foh) fclose(foh);
}