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

#define SHADOWTYPE_STATIC 1
#define SHADOWTYPE_DYNAMIC 2

int FindObjDef(int t, char *s);
char *CreateObjDef(char *fp, char **fstline, int fwords, char *cppname, int t, int e);

struct PhysicalSubtype
{
	//char *path;
	Model **appear;
	char *name;
};

struct CReaction;
struct CValue;
struct CCommand;

// NOTE: This is an "object type declaration". All ingame objects of the same type
//       (ex: CHARACTER "Peasant") share some information in this structure.

struct CObjectDefinition
{
	int life; // 0 = uninitialized, 1 = initialized except model, 2 = totally init.
	int id;
	int type;
	valuetype *startItems;
	char *tooltip;
	int shadowtype;
	PhysicalSubtype *subtypes; int numsubtypes;
	int renderable;
	Vector3 scale;
	char *name;
	CObjectDefinition **mappedType;
	GrowList<CReaction *> ireact;
	CValue **valueTagInterpret;
	Model *representation;
	GrowList<CCommand*> offeredCmds;
	int buildingType;
	int movSpeedEq;

	void *operator new(size_t s) {void *p = malloc(s); if(p) memset(p, 0, s); return p;}
	void *operator new[](size_t s) {void *p = malloc(s); if(p) memset(p, 0, s); return p;}
	void operator delete(void *p) {free(p);}
	void operator delete[](void *p) {free(p);}
};