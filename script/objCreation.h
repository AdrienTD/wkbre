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

struct CObjectCreation
{
	uint type;
	union {
		CObjectDefinition *objdef;
		uint mappedType;
	};
	CFinder *mtObjRef;
	CFinder *matchAppearOf;		// optional
	CFinder *controller;
	CPosition *createAt;		// optional, default: LOCATION_OF FINDER_SELF/FINDER_CREATOR
	ActionSeq *postCreationSeq;	// optional

	void run(SequenceEnv *env, CFinder *creator);
};

void ReadCObjectCreation(char **pntfp, char **fstline);
