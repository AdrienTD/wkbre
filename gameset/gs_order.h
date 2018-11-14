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

struct CTrigger
{
	int type;
	ActionSeq *seq;
	CValue *period;
	int aptype;
};

struct CPlayAnimation;

struct CTask
{
	char *name;
	uint type;
	int cat;
	CFinder *target;
	bool usePreviousTaskTarget, rejectTargetIfItIsTerminated,
		terminateEntireOrderIfNoTarget, identifyTargetEachCycle;
	CValue *proxRequirement;
	GrowList<CTrigger> triggers;
	CValue *satf;
	CPlayAnimation *playAnim;
	bool playAnimOnce;
	ActionSeq *initSeq, *startSeq, *suspendSeq, *resumeSeq, *cancelSeq,
		*terminateSeq, *proxSatisfiedSeq, *proxDissatisfiedSeq,
		*movStartedSeq, *movCompletedSeq;
};

struct COrder
{
	char *name;
	uint type;
	int cat;
	bool cycle, cannotInterrupt;
	GrowList<CTask *> tasks;
	ActionSeq *initSeq, *startSeq, *suspendSeq, *resumeSeq, *cancelSeq,
		*terminateSeq;
	CObjectDefinition *spawnBlueprint;
};

struct COrderAssignment
{
	COrder *order; uint mode; CFinder *target;
};

void ReadCOrder(char **fp, char **fstline);
void ReadCTask(char **fp, char **fstline);
void ReadCOrderAssignment(char **fp, char **fstline);