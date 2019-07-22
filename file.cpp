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
#include "bzip2/bzlib.h"
}

#define MEM_REQ ( 4096*sizeof(char*) + 16 )

typedef struct
{
	uint id;
	u64 time;
	char *name;
} BCPFile;

struct BCPDirectory
{
	BCPDirectory *dirs; uint ndirs;
	BCPFile *files; uint nfiles;
	char *name;
	BCPDirectory *parent;
};

//typedef struct
//{
//	uint offset, endos, size, unk, form;
//} fileentry;

struct BCPack
{
	FILE *bcpfile;
	uint fentof;
	uint nfiles, ftsize;
	fileentry *fent;
	int bcpver;
	BCPDirectory bcproot;
	GrowStringList bcpstring;

	char *BCPReadString();
	void RelocateStrings(BCPDirectory *bd);
	void LookAtDir(BCPDirectory *bd, int isroot, BCPDirectory *prev);
	BCPack(char *fn);
	void ExtractFile(int id, char **out, int *outsize, int extraBytes);
	BCPFile *getFile(char *fn);
	BCPDirectory *getDirectory(char *dn);
	bool loadFile(char *fn, char **out, int *outsize, int extraBytes);
	bool fileExists(char *fn);
	void listFileNames(char *dn, GrowStringList *gsl);
	void listDirectories(char *dn, GrowStringList *gsl);
};

char gamedir[384] = ".";
bool allowBCPPatches = 1, allowDataDirectory = 0, macFileNamesFallbackEnabled = 1;
GrowList<BCPack*> bcpacks;

char *BCPack::BCPReadString()
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

void BCPack::RelocateStrings(BCPDirectory *bd)
{
	for(int i = 0; i < bd->nfiles; i++)
		bd->files[i].name += (ucpuint)bcpstring.gbData.memory;
	for(int i = 0; i < bd->ndirs; i++)
	{
		bd->dirs[i].name += (ucpuint)bcpstring.gbData.memory;
		RelocateStrings(&bd->dirs[i]);
	}
}

void BCPack::LookAtDir(BCPDirectory *bd, int isroot, BCPDirectory *prev)
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
			fread(&bd->files[i].time, 8, 1, bcpfile);
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

BCPack::BCPack(char *fn)
{
	nfiles = 0;

	char abuf[384]; uchar c;
	strcpy(abuf, gamedir);
	strcat(abuf, "\\");
	strcat(abuf, fn);
	bcpfile = fopen(abuf, "rb");
	if(!bcpfile) ferr("Cannot load BCP file.");
	fseek(bcpfile, 9, SEEK_SET);
	bcpver = fgetc(bcpfile) - '0';
	fseek(bcpfile, 0x30, SEEK_SET);
	fentof = readInt(bcpfile);
	ftsize = readInt(bcpfile);

	fseek(bcpfile, fentof, SEEK_SET);
	nfiles = readInt(bcpfile);
	fent = (fileentry*)malloc(nfiles*20);
	if(!fent) ferr("Failed to allocate memory for the file position/size table.");
	fread(fent, nfiles*20, 1, bcpfile);

	fseek(bcpfile, fentof+4+nfiles*20, SEEK_SET);
	LookAtDir(&bcproot, 1, &bcproot);

	RelocateStrings(&bcproot);
}

void BCPack::ExtractFile(int id, char **out, int *outsize, int extraBytes)
{
	uint i, s;
	char *min, *mout, *ws; uint os, sws;

	fseek(bcpfile, fent[id].offset, SEEK_SET);
	s = fent[id].endos - fent[id].offset;
	mout = (char*)malloc(fent[id].size+extraBytes); if(!mout) ferr("Cannot alloc mem for loading a file.");
	*out = mout;

	switch(fent[id].form)
	{
		case 1: // Uncompressed
			fread(mout, s, 1, bcpfile);
			*outsize = s;
			break;
		case 2: // Zero-sized file
			*outsize = 0;
			break;
		case 3: // bzip2-compressed file
			{int be;
			BZFILE *bz = BZ2_bzReadOpen(&be, bcpfile, 0, 0, NULL, 0);
			if(be != BZ_OK) ferr("Failed to initialize bzip2 decompression.");
			int l = BZ2_bzRead(&be, bz, mout, fent[id].size);
			if((be != BZ_OK) && (be != BZ_STREAM_END)) ferr("Failed to decompress bzip2 file.");
			BZ2_bzReadClose(&be, bz);
			*outsize = l;
			break;}
		case 4: // LZRW3-compressed file
			{ws = (char*)malloc(MEM_REQ);
			min = (char*)malloc(s);
			fread(min, s, 1, bcpfile);
			//*outsize = fent[id].size;
			ULONG us = fent[id].size;
			compress(COMPRESS_ACTION_DECOMPRESS, (uchar*)ws, (uchar*)min, s, (uchar*)mout, &us);
			*outsize = (int)us;
			free(ws); free(min);
			break;}
		default:
			ferr("Cannot load a file with an unknown format.");
	}
}

BCPFile *BCPack::getFile(char *fn)
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
				free(ftb); return &ad->files[i];
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
	free(ftb); return 0;
}

BCPDirectory *BCPack::getDirectory(char *dn)
{
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
				goto lend;
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
lend:	free(ftb); return ad;
savdir:	free(ftb); return 0;
}

bool BCPack::loadFile(char *fn, char **out, int *outsize, int extraBytes)
{
	BCPFile *bf = getFile(fn);
	if(!bf) return 0;
	ExtractFile(bf->id, out, outsize, extraBytes);
	return 1;
}

bool BCPack::fileExists(char *fn)
{
	BCPFile *bf = getFile(fn);
	return bf ? 1 : 0;
}

void BCPack::listFileNames(char *dn, GrowStringList *gsl)
{
	BCPDirectory *ad = getDirectory(dn);
	if(!ad) return;
	for(int i = 0; i < ad->nfiles; i++)
		if(!gsl->has(ad->files[i].name))
			gsl->add(ad->files[i].name);
}

void BCPack::listDirectories(char *dn, GrowStringList *gsl)
{
	BCPDirectory *ad = getDirectory(dn);
	if(!ad) return;
	for(int i = 0; i < ad->ndirs; i++)
		if(!gsl->has(ad->dirs[i].name))
			gsl->add(ad->dirs[i].name);
}

BCPack *mainbcp;

void LoadBCP(char *fn)
{
	//mainbcp = new BCPack(fn);

	if(!allowBCPPatches)
		{bcpacks.add(new BCPack(fn)); return;}
	char sn[384]; HANDLE hf; WIN32_FIND_DATA fnd;
	strcpy(sn, gamedir);
	strcat(sn, "\\*.bcp");
	hf = FindFirstFile(sn, &fnd);
	if(hf == INVALID_HANDLE_VALUE) return;
	do {
		bcpacks.add(new BCPack(fnd.cFileName));
	} while(FindNextFile(hf, &fnd));
}

BCPack *GetBCPWithMostRecentFile(char *fn)
{
	BCPack *bestpack = 0; u64 besttime = 0;
	for(int i = 0; i < bcpacks.len; i++)
	{
		BCPack *t = bcpacks[i];
		BCPFile *f = t->getFile(fn);
		if(!f) continue;
		if(f->time >= besttime)
		{
			bestpack = t;
			besttime = f->time;
		}
	}
	return bestpack;
}

char *GetMacName(char *fn)
{
	if (char *sh = strrchr(fn, '\\'))
	{
		sh += 1;
		if (strlen(sh) > 31)
		{
			char nsh[32];
			char *ext = strrchr(sh, '.');
			int mmplen = 31 - (ext ? strlen(ext) : 0) - 1;
			if (mmplen < 0) mmplen = 0;
			strncpy(nsh, sh, mmplen);
			nsh[mmplen] = 0;
			strcat(nsh, "_");
			strcat(nsh, ext);
			char *nfn = strdup(fn);
			nsh[31] = 0;
			*(strrchr(nfn, '\\') + 1) = 0;
			strcat(nfn, nsh);
			return nfn;
		}
	}
	return 0;
}

void LoadFile(char *fn, char **out, int *outsize, int extraBytes)
{
	BCPack *bp = GetBCPWithMostRecentFile(fn);
	if(bp) if(bp->loadFile(fn, out, outsize, extraBytes)) return;

	// File not found in the BCPs. Find it in the "data", "saved" and "redata" directories.
	char sn[384]; FILE *sf = 0;
	if(allowDataDirectory)
	{
		strcpy(sn, gamedir);
		strcat(sn, "\\data\\");
		strcat(sn, fn);
		sf = fopen(sn, "rb");
	}
	if(!sf)
	{
		strcpy(sn, gamedir);
		strcat(sn, "\\saved\\");
		strcat(sn, fn);
		sf = fopen(sn, "rb");
	}
	if (!sf)
		if(HRSRC h = FindResourceA(NULL, fn, "REDATA"))
			if (HGLOBAL g = LoadResource(NULL, h))
			{
				void *p = LockResource(g);
				uint siz = SizeofResource(NULL, h);
				void *mout = malloc(siz+extraBytes);
				memcpy(mout, p, siz);
				*out = (char*)mout;
				*outsize = siz;
				return;
			}
	if (!sf)
	{
		strcpy(sn, "redata\\");
		strcat(sn, fn);
		sf = fopen(sn, "rb");
	}
	if (!sf && macFileNamesFallbackEnabled)
		if (char *nfn = GetMacName(fn))
		{
			LoadFile(nfn, out, outsize, extraBytes);
			free(nfn);
			return;
		}
	if(!sf)
		ferr("The file \"%s\" could not be found in the game directories/archives (data.bcp, saved, redata, data, ...).\n\nIf it is the first time you use this program, be sure that you have configured the GAME_DIR parameter in settings.txt . See help.htm for more details.", fn);
		//ferr("File cannot be found in data.bcp, in the \"saved\" folder and the \"redata\" directory.");
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

int FileExists(char *fn)
{
	//if(mainbcp->fileExists(fn)) return 1;
	for(int i = 0; i < bcpacks.len; i++)
		if(bcpacks[i]->fileExists(fn))
			return 1;

	// File not found in the BCPs. Find it in the "saved" and "redata" folder.
	char sn[384];
	strcpy(sn, gamedir);
	strcat(sn, "\\saved\\");
	strcat(sn, fn);
	if (_access(sn, 0) != -1) return 1;

	if (FindResourceA(NULL, fn, "REDATA"))
		return 1;

	strcpy(sn, "redata\\");
	strcat(sn, fn);
	if (_access(sn, 0) != -1) return 1;

	if (allowDataDirectory) {
		strcpy(sn, gamedir);
		strcat(sn, "\\data\\");
		strcat(sn, fn);
		if (_access(sn, 0) != -1) return 1;
	}

	if (macFileNamesFallbackEnabled)
		if (char *nfn = GetMacName(fn)) {
			auto r = FileExists(nfn);
			free(nfn);
			return r;
		}
	return 0;
}

void FindFiles(char *sn, GrowStringList *gsl)
{
	HANDLE hf; WIN32_FIND_DATA fnd;
	hf = FindFirstFile(sn, &fnd);
	if(hf == INVALID_HANDLE_VALUE) return;
	do {
		if(!(fnd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			if(!gsl->has(fnd.cFileName))
				gsl->add(fnd.cFileName);
	} while(FindNextFile(hf, &fnd));
}

GrowStringList *ListFiles(char *dn, GrowStringList *gsl)
{
	if(!gsl) gsl = new GrowStringList;

	//mainbcp->listFileNames(dn, gsl);
	for(int i = 0; i < bcpacks.len; i++)
		bcpacks[i]->listFileNames(dn, gsl);

	char sn[384];

	if(allowDataDirectory)
	{
		strcpy(sn, gamedir); strcat(sn, "\\data\\");
		strcat(sn, dn); strcat(sn, "\\*.*");
		FindFiles(sn, gsl);
	}

	strcpy(sn, gamedir); strcat(sn, "\\saved\\");
	strcat(sn, dn); strcat(sn, "\\*.*");
	FindFiles(sn, gsl);

	return gsl;
}

GrowStringList *ListDirectories(char *dn)
{
	GrowStringList *gsl = new GrowStringList;
	for(int i = 0; i < bcpacks.len; i++)
		bcpacks[i]->listDirectories(dn, gsl);
	return gsl;
}

BCPWriter::WDirectory* BCPWriter::WDirectory::addDir(char *name)
{
	WDirectory* nwd = new WDirectory;
	nwd->writer = writer;
	nwd->name = name;
	dirs.add(nwd);
	return nwd;
}

void BCPWriter::WDirectory::insertFile(uint id, char *name)
{
	WFile* wf = files.addp();
	wf->id = id;
	wf->name = strdup(name);
	wf->time = 0;
}

BCPWriter::WDirectory::~WDirectory()
{
	for (uint i = 0; i < dirs.len; i++)
		delete dirs[i];
	for (uint i = 0; i < files.len; i++)
		free(files.getpnt(i)->name);
}

BCPWriter::BCPWriter(char *fn)
{
	file = fopen(fn, "wb");
	if (!file) ferr("Failed to create BCP for writing.");

	// Every mod is owned by BC
	fwrite("PAK File 2.01 (c) Black Cactus Games Limited \xAD\x91\xE4", 48, 1, file);

	// Placeholder, will be overwritten at the end
	write32(0); write32(0);

	root.writer = this;
}

uint BCPWriter::createFile(void *pnt, uint siz)
{
	uint id = ftable.len;
	fileentry *e = ftable.addp();
	e->offset = ftell(file);
	e->size = siz;
	e->unk = 0;

	uchar *lzws = (uchar*)malloc(MEM_REQ);
	uint slz = siz + COMPRESS_OVERRUN; //4 + siz + (siz / 16) * 2 + 4;
	void *cplz = malloc(slz);
	compress(COMPRESS_ACTION_COMPRESS, lzws, (uchar*)pnt, siz, (uchar*)cplz, &slz);
	free(lzws);

	if (slz < siz) {
		e->form = 4;
		fwrite(cplz, slz, 1, file);
	}
	else {
		e->form = 1;
		fwrite(pnt, siz, 1, file);
	}
	free(cplz);

	e->endos = ftell(file);
	return id;
}

void BCPWriter::finalize()
{
	uint ftoff = ftell(file);
	write32(ftable.len);
	for (uint i = 0; i < ftable.len; i++)
	{
		fileentry* e = ftable.getpnt(i);
		write32(e->offset);
		write32(e->endos);
		write32(e->size);
		write32(e->unk);
		write32(e->form);
	}
	root.write();
	uint eof = ftell(file);
	fseek(file, 48, SEEK_SET);
	write32(ftoff);
	write32(eof - ftoff);
	fclose(file);
}

void BCPWriter::WDirectory::write()
{
	writer->write32(files.len);
	for (uint i = 0; i < files.len; i++)
	{
		WFile *f = files.getpnt(i);
		writer->write32(f->id);
		writer->write32(f->time);
		writer->write32(f->time >> 32);
		uint sl = strlen(f->name);
		writer->write8(sl);
		for (uint j = 0; j < sl; j++)
			writer->write16((uchar)f->name[j]);
	}
	writer->write32(dirs.len);
	for (uint i = 0; i < dirs.len; i++)
	{
		WDirectory *d = dirs[i];
		uint sl = strlen(*d->name);
		writer->write8(sl);
		for (uint j = 0; j < sl; j++)
			writer->write16((*d->name)[j]);
		dirs[i]->write();
	}
}

void BCPWriter::copyFile(char *fn)
{
	BCPWriter::WDirectory *d = &root;
	char *fncopy = strdup(fn);
	char *p = fncopy;
	while (char *s = strchr(p, '\\')) {
		*s = 0;
		for (uint i = 0; i < d->dirs.len; i++)
			if (!stricmp(*(d->dirs[i]->name), p)) {
				d = d->dirs[i];
				goto dirfnd;
			}
		d = d->addDir(p);
	dirfnd:
		p = s + 1;
	}
	char *fdat; int fsiz;
	LoadFile(fn, &fdat, &fsiz);
	uint id = createFile(fdat, fsiz);
	d->insertFile(id, p);
}