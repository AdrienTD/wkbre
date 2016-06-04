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
extern "C" {
#include "lzrw_headers.h"
}

typedef struct
{
	uint id; //, unk1, unk2;
	char *name;
} BCPFile;

struct BCPDirectory
{
	BCPDirectory *dirs; uint ndirs;
	BCPFile *files; uint nfiles;
	char *name;
	BCPDirectory *parent;
};

typedef struct
{
	uint offset, endos, size, unk, form;
} fileentry;

char gamedir[384] = ".";
FILE *bcpfile;
uint fentof;
uint nfiles = 0, ftsize;
fileentry *fent;
int bcpver;

BCPDirectory bcproot;
GrowStringList bcpstring;

char *BCPReadString()
{
	int cs = fgetc(bcpfile);
	char *r = (char*)malloc(cs+1); if(!r) ferr("BCPReadString malloc failed.");
	if(bcpver == 0)
		fread(r, cs, 1, bcpfile);
	else
	{
		char *o = r;
		for(int i = 0; i < cs; i++)
			{*(o++) = fgetc(bcpfile); fgetc(bcpfile);}
	}
	r[cs] = 0;
	char *x = (char*)bcpstring.gbData.apnt;
	bcpstring.add(r);
	free(r);
	return x;
}

void RelocateStrings(BCPDirectory *bd)
{
	for(int i = 0; i < bd->nfiles; i++)
		bd->files[i].name += (ucpuint)bcpstring.gbData.memory;
	for(int i = 0; i < bd->ndirs; i++)
	{
		bd->dirs[i].name += (ucpuint)bcpstring.gbData.memory;
		RelocateStrings(&bd->dirs[i]);
	}
}

void LookAtDir(BCPDirectory *bd, int isroot, BCPDirectory *prev)
{
	if(!isroot) bd->name = BCPReadString();
	bd->parent = prev;
	bd->nfiles = readInt(bcpfile);
	if(bd->nfiles)
	{
		bd->files = (BCPFile*)malloc(sizeof(BCPFile)*bd->nfiles);
		for(int i = 0; i < bd->nfiles; i++)
		{
			bd->files[i].id = readInt(bcpfile);
			fseek(bcpfile, 8, SEEK_CUR);
			bd->files[i].name = BCPReadString();
		}
	}
	bd->ndirs = readInt(bcpfile);
	if(bd->ndirs)
	{
		bd->dirs = (BCPDirectory*)malloc(sizeof(BCPDirectory)*bd->ndirs);
		for(int i = 0; i < bd->ndirs; i++)
			LookAtDir(&bd->dirs[i], 0, bd);
	}
}

void LoadBCP(char *fn)
{
	char abuf[384]; uchar c;
	strcpy(abuf, gamedir);
	strcat(abuf, "\\");
	strcat(abuf, fn);
	bcpfile = fopen(abuf, "rb");
	if(!bcpfile) ferr("Cannot load BCP file.");
	while(1) {c = fgetc(bcpfile); if((c == 0xFF) || (c == 0)) break;}
	fseek(bcpfile, 2, SEEK_CUR);
	fentof = readInt(bcpfile);
	ftsize = readInt(bcpfile);
	fseek(bcpfile, 9, SEEK_SET);
	bcpver = fgetc(bcpfile) - '0';

	fseek(bcpfile, fentof, SEEK_SET);
	nfiles = readInt(bcpfile);
	fent = (fileentry*)malloc(nfiles*20);
	if(!fent) ferr("Failed to allocate memory for the file position/size table.");
	fread(fent, nfiles*20, 1, bcpfile);

	fseek(bcpfile, fentof+4+nfiles*20, SEEK_SET);
	LookAtDir(&bcproot, 1, &bcproot);

	RelocateStrings(&bcproot);
}

#define MEM_REQ ( 4096*sizeof(char*) + 16 )

void ExtractFile(int id, char **out, int *outsize, int extraBytes)
{
	uint i, s;
	char *min, *mout, *ws; uint os, sws;

	fseek(bcpfile, fent[id].offset, SEEK_SET);
	s = fent[id].endos - fent[id].offset;
	mout = (char*)malloc(fent[id].size+extraBytes); if(!mout) ferr("Cannot alloc mem for loading a file.");

	switch(fent[id].form)
	{
		case 3: // Patch file (?)
		case 1: // Uncompressed
			fread(mout, s, 1, bcpfile);
			*out = mout; *outsize = s;
			break;
		case 2: // Zero-sized file
			*out = 0; *outsize = 0;
			break;
		case 4: // LZRW3-compressed file
			{ws = (char*)malloc(MEM_REQ);
			min = (char*)malloc(s);
			fread(min, s, 1, bcpfile);
			*outsize = fent[id].size;
			ULONG us;
			compress(COMPRESS_ACTION_DECOMPRESS, (uchar*)ws, (uchar*)min, s, (uchar*)mout, &us);
			*outsize = (int)us;
			*out = mout;
			free(ws); free(min);
			break;}
		default:
			ferr("Cannot load a file with an unknown format.");
	}
}

void LoadFile(char *fn, char **out, int *outsize, int extraBytes)
{
	char *p, *c; BCPDirectory *ad = &bcproot;
	char *ftb;
	int s = strlen(fn);
	ftb = (char*)malloc(s+1);
	strcpy(ftb, fn);
	p = ftb;
	for(int i = 0; i < s; i++)
		if(p[i] == '/')
			p[i] = '\\';
	while(1)
	{
lflp:		c = strchr(p, '\\');
		if(!c)
		{
			// A file!
			for(int i = 0; i < ad->nfiles; i++)
			if(!_stricmp(p, ad->files[i].name))
			{
				// File found!
				ExtractFile(ad->files[i].id, out, outsize, extraBytes);
				free(ftb); return;
			}
			break; // File not found.

		} else {
			// A directory!
			*c = 0;
			if(!strcmp(p, "."))
				{p = c+1; goto lflp;}
			if(!strcmp(p, ".."))
			{
				// Go one directory backwards
				p = c+1;
				ad = ad->parent;
				goto lflp;
			}
			for(int i = 0; i < ad->ndirs; i++)
			if(!_stricmp(p, ad->dirs[i].name))
			{
				// Dir found!
				p = c+1;
				ad = &ad->dirs[i];
				goto lflp;
			}
			break; // Dir not found.
		}
	}
	// File not found in the BCP. Find it in the "saved" folder.
	free(ftb);
	char sn[384];
	strcpy(sn, gamedir);
	strcat(sn, "\\saved\\");
	strcat(sn, fn);
	FILE *sf = fopen(sn, "rb");
	if(!sf)
	{
		strcpy(sn, "redata\\");
		strcat(sn, fn);
		sf = fopen(sn, "rb");
		if(!sf)
			ferr("File cannot be found in data.bcp, in the \"saved\" folder and the \"redata\" directory.");
	}
	fseek(sf, 0, SEEK_END);
	int ss = ftell(sf);
	fseek(sf, 0, SEEK_SET);
	char *mout = (char*)malloc(ss+extraBytes);
	if(!mout) ferr("No mem. left for reading a file from saved dir.");
	fread(mout, ss, 1, sf);
	fclose(sf);
	*out = mout; *outsize = ss;
	return;
}

void TestBCP()
{
	printf("sizeof(BCPFile) = %i\nsizeof(BCPDirectory) = %i\n", sizeof(BCPFile), sizeof(BCPDirectory));
	printf("--- Root ---\n%i files\n", bcproot.nfiles);
	for(int i = 0; i < bcproot.nfiles; i++)
		printf(" - %i\n", bcproot.files[i].id);
	printf("%i directories\n", bcproot.ndirs);
	for(int i = 0; i < bcproot.ndirs; i++)
		printf(" - %s\n", bcproot.dirs[i].name);
}

int FileExists(char *fn)
{
	char *p, *c; BCPDirectory *ad = &bcproot;
	char *ftb;
	int s = strlen(fn);
	ftb = (char*)malloc(s+1);
	strcpy(ftb, fn);
	p = ftb;
	for(int i = 0; i < s; i++)
		if(p[i] == '/')
			p[i] = '\\';
	while(1)
	{
lflp:		c = strchr(p, '\\');
		if(!c)
		{
			// A file!
			for(int i = 0; i < ad->nfiles; i++)
			if(!_stricmp(p, ad->files[i].name))
			{
				// File found!
				free(ftb); return 1;
			}
			break; // File not found.
		} else {
			// A directory!
			*c = 0;
			if(!strcmp(p, "."))
				{p = c+1; goto lflp;}
			if(!strcmp(p, ".."))
			{
				// Go one directory backwards
				p = c+1;
				ad = ad->parent;
				goto lflp;
			}
			for(int i = 0; i < ad->ndirs; i++)
			if(!_stricmp(p, ad->dirs[i].name))
			{
				// Dir found!
				p = c+1;
				ad = &ad->dirs[i];
				goto lflp;
			}
			break; // Dir not found.
		}
	}
	// File not found in the BCP. Find it in the "saved" folder.
	free(ftb);
	char sn[384];
	strcpy(sn, gamedir);
	strcat(sn, "\\saved\\");
	strcat(sn, fn);
	if(_access(sn, 0) == -1)
	{
		strcpy(sn, "redata\\");
		strcat(sn, fn);
		return _access(sn, 0) != -1;
	}
	return 1;
}

GrowStringList *ListFiles(char *dn)
{
	GrowStringList *gsl = new GrowStringList;
	char *p, *c; BCPDirectory *ad = &bcproot;
	char *ftb;
	int s = strlen(dn);
	ftb = (char*)malloc(s+1);
	strcpy(ftb, dn);
	p = ftb;
	for(int i = 0; i < s; i++)
		if(p[i] == '/')
			p[i] = '\\';
	while(1)
	{
lflp:		c = strchr(p, '\\');
		if(c)
		{
			// A directory!
			*c = 0;
			if(!strcmp(p, "."))
				{p = c+1; goto lflp;}
			if(!strcmp(p, ".."))
			{
				// Go one directory backwards
				p = c+1;
				ad = ad->parent;
				goto lflp;
			}
			for(int i = 0; i < ad->ndirs; i++)
			if(!_stricmp(p, ad->dirs[i].name))
			{
				// Dir found!
				p = c+1;
				ad = &ad->dirs[i];
				goto lflp;
			}
			goto savdir; // Dir not found.
		} else {
			// A directory!
			if(!*p) goto lend;
			if(!strcmp(p, "."))
				{p = c+1; goto lflp;}
			if(!strcmp(p, ".."))
			{
				// Go one directory backwards
				ad = ad->parent;
				goto lend;
			}
			for(int i = 0; i < ad->ndirs; i++)
			if(!_stricmp(p, ad->dirs[i].name))
			{
				// Dir found!
				ad = &ad->dirs[i];
				goto lend;
			}
			goto savdir; // Dir not found.
		}
	}
lend:	for(int i = 0; i < ad->nfiles; i++)
		gsl->add(ad->files[i].name);

savdir:	free(ftb);
	char sn[384]; HANDLE hf; WIN32_FIND_DATA fnd;
	strcpy(sn, gamedir);
	strcat(sn, "\\saved\\");
	strcat(sn, dn);
	strcat(sn, "\\*.*");
	hf = FindFirstFile(sn, &fnd);
	if(hf == INVALID_HANDLE_VALUE) return gsl;
	do {
		if(!(fnd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			gsl->add(fnd.cFileName);
	} while(FindNextFile(hf, &fnd));
	return gsl;
}

void SetGameDir()
{
	FILE *f = fopen("gamedir.txt", "r"); if(!f) return;
	fgets(gamedir, 383, f);
	fclose(f);
	char *n = strrchr(gamedir, '\n');
	if(n) *n = 0;
}