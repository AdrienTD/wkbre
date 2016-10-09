#include "global.h"

typedef struct
{
	char *srcname; int srcpos;
	char *dstname; int dstpos;
} tte;

GrowList<MapTextureGroup> maptexgroup;
GrowStringList maptexfilenames;

char **ltsrcname, **ltdstname;
uint ltnsrc = 0, ltndst = 0;
tte *lttt; uint ltne = 0;

void FreeLTTT()
{
	for(int i = 0; i < ltnsrc; i++)
		free(ltsrcname[i]);
	free(ltsrcname);
	for(int i = 0; i < ltndst; i++)
		free(ltdstname[i]);
	free(ltdstname);
	free(lttt);
	ltnsrc = ltndst = ltne = 0;
}

void FreeMapTextures()
{
	// NOTE: Currently, textures are not freed. I will have to make a reference
	//       count for each texture.
	for(int i = 0; i < maptexgroup.len; i++)
		delete maptexgroup.getpnt(i)->tex;
	maptexgroup.clear();
	maptexfilenames.clear();
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

void ReadMapTexture(char **pntfp, char **fstline, MapTextureGroup *g)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	AddMapTexture(g, fstline);
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!strcmp(word[0], "END_TEXTURE"))
			return;

		if(!strcmp(word[0], "MATCHING"))
			AddMapTexture(g, word);
	}
	ferr("UEOF");
}

void ReadMapTextureGroup(char **pntfp, char **fstline)
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
			ReadMapTexture(pntfp, word, g);
	}
	ferr("UEOF");
}

void LoadMapTextures()
{
	char *file; int fs;
	LoadFile("Maps\\Map_Textures\\All_Textures.dat", &file, &fs, 1);
	file[fs] = 0;

	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	char *fp = file;
	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;

		if(!strcmp(word[0], "TEXTURE_GROUP"))
			ReadMapTextureGroup(&fp, word);
	}

	free(file);
}
