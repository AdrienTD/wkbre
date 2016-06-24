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

#include "../global.h"

uint sysabstime = 0;
float game_speed = 1.0f;

float GetElapsedTime()
{
	uint c = timeGetTime();
	//if(c < sysabstime) printf("Teleported to the past?\n");
	float r = (c - sysabstime) / 1000.0f;
	sysabstime = c;
	return r;
}

void InitTime()
{
	GetElapsedTime();
}

void AdvanceTime()
{
	elapsed_time = GetElapsedTime() * game_speed;
	previous_time = current_time;
	current_time += elapsed_time;
}

////////////////////////////////////////////////////////////

void CheckBattlesDelayedSequences()
{
	// Execute delayed sequences
	DynListEntry<DelayedSequenceEntry> *n;
	for(DynListEntry<DelayedSequenceEntry> *e = delayedSeq.first; e; e = n)
	{
		n = e->next;
		if(!e->value.numobj) {delete [] e->value.obj; delayedSeq.remove(e); continue;}
		if(current_time >= e->value.time)
		{
			// execute sequence
			SequenceEnv c;
			c.exec = e->value.executor;
			for(uint i = 0; i < e->value.numobj; i++)
				if(e->value.obj[i].valid())
				{
					c.self = e->value.obj[i];
					actionseq[e->value.actseq]->run(&c);
				}

			delete [] e->value.obj; delayedSeq.remove(e);
		}
	}

	// Execute seqs. over periods
	DynListEntry<SequenceOverPeriodEntry> *m;
	for(DynListEntry<SequenceOverPeriodEntry> *e = exePeriodSeq.first; e; e = m)
	{
		m = e->next;
		if((!e->value.numobj) || (e->value.loopsexec >= e->value.nloops))
			{delete [] e->value.ola; repPeriodSeq.remove(e); continue;}
		float lt = e->value.period / e->value.nloops;
		while(current_time >= (e->value.time + (e->value.loopsexec+1) * lt))
		{
			if(e->value.obj[0].valid())
			{
				SequenceEnv c; c.exec = e->value.executor;
				c.self = e->value.obj[0];
				actionseq[e->value.actseq]->run(&c);
			}
			e->value.obj++; e->value.numobj--; e->value.loopsexec++;
			if((!e->value.numobj) || (e->value.loopsexec >= e->value.nloops))
				{delete [] e->value.ola; repPeriodSeq.remove(e); break;}
		}
	}

	// Execute repeated seqs. over periods
	//DynListEntry<SequenceOverPeriodEntry> *m;
	for(DynListEntry<SequenceOverPeriodEntry> *e = repPeriodSeq.first; e; e = m)
	{
		m = e->next;
		if((!e->value.numobj) || (e->value.loopsexec >= e->value.nloops))
			{delete [] e->value.ola; repPeriodSeq.remove(e); continue;}
		float lt = e->value.period / e->value.nloops;
		while(current_time >= (e->value.time + (e->value.loopsexec+1) * lt))
		{
			for(uint i = 0; i < e->value.numobj; i++)
				if(e->value.obj[i].valid())
				{
					SequenceEnv c; c.exec = e->value.executor;
					c.self = e->value.obj[i];
					actionseq[e->value.actseq]->run(&c);
				}
			e->value.loopsexec++;
			if(e->value.loopsexec >= e->value.nloops)
				{delete [] e->value.ola; repPeriodSeq.remove(e); break;}
		}
	}
}