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

#define FSTASK_PROXIMITY_SATISFIED 1
#define FSTASK_LAST_DESTINATION_VALID 2
#define FSTASK_FIRST_EXECUTION 4
#define FSTASK_TRIGGERS_STARTED 8
#define FSTASK_START_SEQUENCE_EXECUTED 16
#define FSTASK_COST_DEDUCTED 32

struct CTask;
struct COrder;

struct STrigger
{
	uint id; float period, referenceTime;
};

struct STask
{
	CTask *type; goref target; int flags, processState, taskID; float proximity;
	CObjectDefinition *spawnBlueprint; DynList<couple<float> > destinations;
	DynList<STrigger> trigger;
};

struct SOrder
{
	COrder *type; int processState, cycled, orderID, uniqueTaskID, currentTask;
	DynList<STask> task;
};

struct GameObject;

struct SOrderConfiguration
{
	uint uniqueOrderID;
	DynList<SOrder> order;
};

char *ReadOrderConfiguration(char *fp, GameObject *o);
void WriteOrderConfiguration(FILE *f, GameObject *o, char *strtab);

char *ReadParamBlock(char *fp, GameObject *o);
void WriteParamBlock(FILE *f, GameObject *o, char *strtab);