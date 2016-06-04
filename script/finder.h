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

CFinder *ReadFinder(char ***wpnt);
CFinder *ReadOFDLine(char **pntfp);
