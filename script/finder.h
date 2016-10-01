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

#define FINDER_NO_DIS GameObject *getnext() {return defgetnext_nodis();}
#define FINDER_NO_TERM GameObject *getnext() {return defgetnext_noterm();}
#define FINDER_NO_DISTERM GameObject *getnext() {return defgetnext_nodisterm();}

struct CFinder
{
	//virtual ~CFinder() {}

	// Make an identical clone of this finder.
	virtual CFinder *clone() = 0;

	// Begin getting a list of objects. _getnext will then return the first object.
	virtual void begin(SequenceEnv *env) = 0;

	// Get an object. Next time it will return the next object.
	// If 0 is returned, there are no objects (anymore).
	virtual GameObject *_getnext() = 0;

	// getnext calls _getnext and filters out objects that should not be returned,
	// for example disabled and terminated objects. It can be overriden if
	// these objects need to be returned or not (for example FINDER_DISABLED_ASSOCIATES).
	virtual GameObject *getnext();

	// Initialize and return first object.
	GameObject *getfirst(SequenceEnv *env) {this->begin(env); return this->getnext();}

	// Define some common getfirst filters.
	GameObject *defgetnext_nodis();
	GameObject *defgetnext_noterm();
	GameObject *defgetnext_nodisterm();
};

struct CObjFindCond
{
	int refobj;
	// 0 = FINDER_SELF		1 = FINDER_CHAIN_ORIGINAL_SELF

	int diplo;
	// 0 = any player		1 = same player as refobj
	// 2 = allied to refobj		3 = enemy to refobj

	int otype;
	// 0 = any class		1 = buildings only
	// 2 = characters only		3 = buildings and characters only

	CObjFindCond() : refobj(0), diplo(0), otype(0) {}
};

extern GrowStringList strUnknownFinder;

CFinder *ReadFinder(char ***wpnt);
CFinder *ReadOFDLine(char **pntfp);
