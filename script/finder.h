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

struct CFinder
{
	//virtual ~CFinder() {}
	// Make an identical clone of this finder.
	virtual CFinder *clone() = 0;
	// Begin getting a list of objects. GetObject will return the first object.
	virtual void begin(SequenceEnv *env) = 0;
	// Get an object. Next time it will return the next object.
	// If 0 is returned, there are no objects (anymore).
	virtual GameObject *getnext() = 0;
	// Initialize and return first object.
	GameObject *getfirst(SequenceEnv *env) {this->begin(env); return this->getnext();}
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
