struct MapTexture
{
	int id;
	int x, y, w, h;
	texture t;
	int tfid;
};

struct MapTextureGroup
{
	char *name;
	GrowList<MapTexture> *tex;
};

extern GrowList<MapTextureGroup> maptexgroup;
extern GrowStringList maptexfilenames;

void FreeLTTT();
void FreeMapTextures();
void LoadLTTT(char *fn);
void LoadMapTextures();
