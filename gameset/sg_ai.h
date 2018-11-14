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

struct SCharacterRung
{
	int type;
	bool blocked;
	int detailedState;
	SCharacterRung() : blocked(0) {}
};

struct SRequirement
{
	int cl, type, state, detailedState, requiredCount, existingCount;
	DynList<goref> foundations, ordersAssigned;
	DynList<SCharacterRung> charrungs;
};

struct SCommission
{
	int type; goref obj;
	bool complete, forceBalance;
	DynList<SRequirement> reqs;
	SCommission() : complete(0), forceBalance(0) {}
};

struct SWorkOrder
{
	goref obj;
	int ofind, type;
};

struct SAIController
{
	int masterPlan; //SPlan *masterPlan;
	DynList<SCommission> commissions;
	DynList<SWorkOrder> workOrders;
	SAIController() : masterPlan(-1) {}
};

char *ReadAIController(char *fp, GameObject *o);
void WriteAIController(FILE *f, GameObject *o);
