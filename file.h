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

typedef struct
{
	uint offset, endos, size, unk, form;
} fileentry;

class BCPWriter
{
private:
	FILE * file;
	GrowList<fileentry> ftable;

	void write8(uchar n) { fwrite(&n, 1, 1, file); }
	void write16(ushort n) { fwrite(&n, 2, 1, file); }
	void write32(uint n) { fwrite(&n, 4, 1, file); }
public:

	struct WFile
	{
		uint id; u64 time; char* name;
	};

	struct WDirectory
	{
		GrowList<WDirectory*> dirs;
		GrowList<WFile> files;
		strbase<char> name;
		BCPWriter *writer;

		WDirectory* addDir(char *name);
		void insertFile(uint id, char *name);
		~WDirectory();

		void write();
	};
	WDirectory root;

	BCPWriter(char *fn);
	uint createFile(void *pnt, size_t siz);
	void finalize();
	void copyFile(char *fn);
};

extern char gamedir[384];
extern bool allowBCPPatches, allowDataDirectory, macFileNamesFallbackEnabled;

void LoadBCP(char *fn);
void LoadFile(char *fn, char **out, int *outsize, int extraBytes = 0);
//void TestBCP();
int FileExists(char *fn);
GrowStringList *ListFiles(char *dn, GrowStringList *gsl = 0);
GrowStringList *ListDirectories(char *dn);
//void SetGameDir();