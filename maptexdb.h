struct MapTextureGroup;
struct MapTexture;

struct MapTextureEdge
{
	MapTexture *tex;
	uint rot; uint xflip, zflip;
	// NOTE: Using memcmp with "boolean" types is dangerous!
	bool operator==(MapTextureEdge &b) { return tex == b.tex && rot == b.rot && xflip == b.xflip && zflip == b.zflip; }
};

struct MapTexture
{
	int id;
	int x, y, w, h;
	texture t;
	int tfid;
	MapTextureGroup *grp;
	GrowList<MapTextureEdge> *atdir; // pointer to GrowList<MapTextureEdge>[4]
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
