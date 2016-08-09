struct BCPack
{
	FILE *bcpfile;
	uint fentof;
	uint nfiles = 0, ftsize;
	fileentry *fent;
	int bcpver;
	BCPDirectory bcproot;
	GrowStringList bcpstring;
};

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
lflp:	return 0;
}