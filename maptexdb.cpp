#include "global.h"

typedef struct
{
	char *srcname; int srcpos;
	char *dstname; int dstpos;
} tte;

GrowList<MapTextureGroup> maptexgroup;
GrowStringList maptexfilenames;

char **ltsrcname = 0, **ltdstname = 0;
uint ltnsrc = 0, ltndst = 0;
tte *lttt = 0; uint ltne = 0;

GrowList<GrowList<MapTextureEdge>*> maptexedges; // list of all GrowList<MapTextureEdge>[4], used to free them

void FreeLTTT()
{
	for(int i = 0; i < ltnsrc; i++)
		free(ltsrcname[i]);
	if(ltsrcname) free(ltsrcname);
	for(int i = 0; i < ltndst; i++)
		free(ltdstname[i]);
	if(ltdstname) free(ltdstname);
	if(lttt) free(lttt);
	ltnsrc = ltndst = ltne = 0;
	ltsrcname = 0; ltdstname = 0; lttt = 0;
}

extern MapTextureGroup *curtexgrp; extern MapTexture *curtex; // editor variables

void FreeMapTextures()
{
	// NOTE: Currently, textures are not freed. I will have to make a reference
	//       count for each texture.
	for(int i = 0; i < maptexedges.len; i++)
		delete [] maptexedges[i];
	for(int i = 0; i < maptexgroup.len; i++)
		delete maptexgroup.getpnt(i)->tex;
	maptexgroup.clear();
	maptexfilenames.clear();
	maptexedges.clear();
	curtexgrp = 0; curtex = 0;
}

void LoadLTTT(char *fn)
{
	char *file, *p, *o; int fs;
	LoadFile(fn, &file, &fs);
	if(*(uint*)file != 'TTTL') ferr("Loading unknown file in LoadLTTT!");
	if(*(uint*)(file+4) != 1) ferr("Unknown LTTT version.");
	p = file + 8;

	ltndst = *(ushort*)p; p += 2;
	ltdstname = (char**)malloc(sizeof(char*) * ltndst);
	for(int i = 0; i < ltndst; i++)
	{
		o = ltdstname[i] = (char*)malloc(strlen(p)+1);
		while(*(o++) = *(p++));
	}

	ltnsrc = *(ushort*)p; p += 2;
	ltsrcname = (char**)malloc(sizeof(char*) * ltnsrc);
	for(int i = 0; i < ltnsrc; i++)
	{
		o = ltsrcname[i] = (char*)malloc(strlen(p)+1);
		while(*(o++) = *(p++));
	}

	ltne = *(uint*)p; p += 4;
	lttt = (tte*)malloc(sizeof(tte) * ltne);
	for(int i = 0; i < ltne; i++)
	{
		lttt[i].srcname = ltsrcname[*(ushort*)p]; p += 2;
		lttt[i].srcpos = *(uchar*)(p++);
		lttt[i].dstname = ltdstname[*(ushort*)p]; p += 2;
		lttt[i].dstpos = *(uchar*)(p++);
	}

	free(file);
}

void TranslateMapTexture(char *name, int *x, int *y)
{
	char *sn; tte *e;
	int tp = ((*y / 64) << 2) | (*x / 64);
	for(int i = 0; i < ltnsrc; i++)
		if(!stricmp(name, ltsrcname[i]))
			{sn = ltsrcname[i]; goto snf;}
	return;
snf:	for(int i = 0; i < ltne; i++)
		if(lttt[i].srcname == sn)
			if(lttt[i].srcpos == tp)
				{e = &(lttt[i]); goto dstf;}
	return;
dstf:	strcpy(name, "Packed\\");
	strcat(name, e->dstname);
	strcat(name, ".pcx");
	*x = (e->dstpos & 3) * 64;
	*y = (e->dstpos >> 2) * 64;
}

void AddMapTexture(MapTextureGroup *g, char **word)
{
	MapTexture *t = g->tex->addp();
	char n[384]; strcpy(n, word[1]); //char *n = word[1];
	t->x = atoi(word[2]); t->y = atoi(word[3]);
	t->w = atoi(word[4]); t->h = atoi(word[5]);
	t->id = atoi(word[6]);
	TranslateMapTexture(n, &t->x, &t->y);

	char p[512];
	strcpy(p, "Maps\\Map_Textures\\");
	strcat(p, n);
	//printf("[DBG] %s\n", p);
	t->t = GetTexture(p);

	int f = maptexfilenames.find(n);
	if(f != -1) t->tfid = f;
	else {
		t->tfid = maptexfilenames.len;
		maptexfilenames.add(n);
	}
}

void ReadMapTextureP1(char **pntfp, char **fstline, MapTextureGroup *g)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	GrowList<MapTextureEdge> *atdir = new GrowList<MapTextureEdge>[4];
	maptexedges.add(atdir);
	int tix = g->tex->len; int nt = 1;
	AddMapTexture(g, fstline);
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!strcmp(word[0], "END_TEXTURE"))
		{
			for(int i = tix; i < (tix + nt); i++)
				g->tex->getpnt(i)->atdir = atdir;
			return;
		}

		if(!strcmp(word[0], "MATCHING"))
			{AddMapTexture(g, word); nt++;}
	}
	ferr("UEOF");
}

void ReadMapTextureP2(char **pntfp, char **fstline, MapTextureGroup *g)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	int curdir = 0; int tid = atoi(fstline[6]); MapTexture *t = 0;
	for(int i = 0; i < g->tex->len; i++)
		if(g->tex->getpnt(i)->id == tid)
			{t = g->tex->getpnt(i); break;}
	if(!t) ferr(":''");
	GrowList<MapTextureEdge> *atdir = t->atdir;

	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!strcmp(word[0], "END_TEXTURE"))
			return;

		if(!strcmp(word[0], "EDGE"))
		{
			MapTextureGroup *eg = 0;
			for(int i = 0; i < maptexgroup.len; i++)
				if(!strcmp(word[2], maptexgroup.getpnt(i)->name))
					{eg = maptexgroup.getpnt(i); break;}
			//if(!eg) {printf("Unknown texture group %s in texture database autotiling edge.", word[2]); continue;}
			if(!eg) continue;
			MapTexture *et = 0; int it = atoi(word[3]);
			for(int i = 0; i < eg->tex->len; i++)
				if(eg->tex->getpnt(i)->id == it)
					{et = eg->tex->getpnt(i); break;}
			//if(!et) {printf("Unknown texture id %i in group %s in texture database autotiling edge.\n", it, eg->name); continue;}
			if(!et) continue;
			MapTextureEdge *edge = atdir[curdir].addp();
			edge->tex = et;
			edge->rot = atoi(word[4]);
			edge->xflip = atoi(word[5]);
			edge->zflip = atoi(word[6]);
		}

		for(int i = 0; i < 4; i++)
		if(!strcmp(word[0], MAPTEXDIR_str[i])) // NORTH, EAST, SOUTH, WEST
		{
			curdir = i;
			break;
		}
	}
	ferr("UEOF");
}

void ReadMapTextureGroupP1(char **pntfp, char **fstline)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	MapTextureGroup *g = maptexgroup.addp();
	g->name = strdup(fstline[1]);
	g->tex = new GrowList<MapTexture>;
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!strcmp(word[0], "END_GROUP"))
			return;

		if(!strcmp(word[0], "TEXTURE"))
			ReadMapTextureP1(pntfp, word, g);
	}
	ferr("UEOF");
}

void ReadMapTextureGroupP2(char **pntfp, char **fstline)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;

	MapTextureGroup *g = 0;
	for(int i = 0; i < maptexgroup.len; i++)
		if(!strcmp(fstline[1], maptexgroup.getpnt(i)->name))
			{g = maptexgroup.getpnt(i); break;}
	if(!g) ferr("Unknown texture database group in 2nd pass, should not happen.");

	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!strcmp(word[0], "END_GROUP"))
			return;

		if(!strcmp(word[0], "TEXTURE"))
			ReadMapTextureP2(pntfp, word, g);
	}
	ferr("UEOF");
}

void LoadMapTextures()
{
	char *file; int fs;
	LoadFile("Maps\\Map_Textures\\All_Textures.dat", &file, &fs, 1);
	file[fs] = 0;

	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE], *fp; int nwords;

	// 1st pass
	fp = file;
	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;

		if(!strcmp(word[0], "TEXTURE_GROUP"))
			ReadMapTextureGroupP1(&fp, word);
	}

	// Set the MapTexture::grp element.
	for(int i = 0; i < maptexgroup.len; i++)
	{
		MapTextureGroup *g = maptexgroup.getpnt(i);
		for(int j = 0; j < g->tex->len; j++)
		{
			MapTexture *t = g->tex->getpnt(j);
			t->grp = g;
		}
	}

	// 2nd pass
	fp = file;
	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;

		if(!strcmp(word[0], "TEXTURE_GROUP"))
			ReadMapTextureGroupP2(&fp, word);
	}

	free(file);
}
