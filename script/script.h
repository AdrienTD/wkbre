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

#define stpo(n) ((n)>0)

struct SequenceEnv
{
	goref self, exec, target, candidate, creator, ogiver, pkgsender;
	void copySelf(SequenceEnv *e)
		{e->self = self;}
	void copyAllButNotSelf(SequenceEnv *e)
		{e->exec = exec; e->target = target; e->candidate = candidate;
		e->creator = creator; e->ogiver = ogiver; e->pkgsender = pkgsender;}
	void copyAll(SequenceEnv *e)
		{e->self = self; e->exec = exec; e->target = target; e->candidate = candidate;
		e->creator = creator; e->ogiver = ogiver; e->pkgsender = pkgsender;}
};

#include "finder.h"
#include "value.h"
#include "position.h"
#include "action.h"
#include "objCreation.h"
#include "events.h"
