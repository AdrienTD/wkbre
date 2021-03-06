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

DynList<goref> workingObjs;

void ObjStepMove(GameObject *o, Vector3 dest)
{
	Vector3 v = dest - o->position;
	float l = v.len2xz();
	v /= l; v.y = 0;

	o->position += v * elapsed_time * 2;
	o->position.y = GetHeight(o->position.x, o->position.z);

	o->orientation.x = o->orientation.z = 0;
	o->orientation.y = M_PI - atan2(v.x, v.z);

	GOPosChanged(o);
}

void StartCurrentTaskTriggers(GameObject *o)
{
	SOrder *s = &o->ordercfg.order.first->value;
	STask *t = &s->task.getEntry(s->currentTask)->value;
	if(t->flags & FSTASK_TRIGGERS_STARTED) return;

	t->flags |= FSTASK_TRIGGERS_STARTED;
	for(int i = 0; i < t->type->triggers.len; i++)
	{
		CTrigger *c = t->type->triggers.getpnt(i);
		t->trigger.add();
		t->trigger.last->value.id = i;
		if(c->type == TASKTRIGGER_TIMER)
		{
			SequenceEnv env; env.self = o; //env.target = t->target;
			t->trigger.last->value.period = c->period->get(&env);
		}
		t->trigger.last->value.referenceTime = current_time;
		//if(c->type == TASKTRIGGER_ATTACHMENT_POINT)
		//	t->trigger.last->value.referenceTime -= 0.5f;
	}
}

void StopCurrentTaskTriggers(GameObject *o)
{
	SOrder *s = &o->ordercfg.order.first->value;
	STask *t = &s->task.getEntry(s->currentTask)->value;
	if(!(t->flags & FSTASK_TRIGGERS_STARTED)) return;

	t->flags &= ~FSTASK_TRIGGERS_STARTED;
	t->trigger.clear();
}

#define ANIMATION_LOOP_TRIGGER_SPEED 1

// For various reasons (notably because it is possible to terminate the order and
// free its memory in the trigger action sequence), only one trigger is executed
// (and only one time) in CheckCurrentTaskTriggers.

void CheckCurrentTaskTriggers(GameObject *o)
{
	SOrder *s = &o->ordercfg.order.first->value;
	STask *t = &s->task.getEntry(s->currentTask)->value;
	if(!(t->flags & FSTASK_TRIGGERS_STARTED)) return;

	for(DynListEntry<STrigger> *e = t->trigger.first; e; e = e->next)
	{
		STrigger *g = &e->value;
		CTrigger *c = t->type->triggers.getpnt(g->id);
		SequenceEnv env;
		switch(c->type)
		{
			case TASKTRIGGER_TIMER:
				if((g->referenceTime + g->period) > current_time)
					break;
				env.self = o; //env.target = t->target;
				g->referenceTime += g->period;
				g->period = c->period->get(&env);
				c->seq->run(&env);
				return;
			case TASKTRIGGER_ANIMATION_LOOP:
			case TASKTRIGGER_UNINTERRUPTIBLE_ANIMATION_LOOP:
			case TASKTRIGGER_ATTACHMENT_POINT:
				env.self = o; //env.target = t->target;
				if(t->type->satf)
				{
					if(t->type->satf->get(&env) < 1.0f)
						break;
				}
				else
				{
					if((g->referenceTime + ANIMATION_LOOP_TRIGGER_SPEED) > current_time)
						break;
					g->referenceTime += ANIMATION_LOOP_TRIGGER_SPEED;
				}
				c->seq->run(&env);
				return;
		}
		// continue loop only if trigger not executed
	}
}

void ProcessCurrentTask(GameObject *o)
{
	goref g; g = o;
	SOrder *s = &o->ordercfg.order.first->value;
	STask *t = &s->task.getEntry(s->currentTask)->value;
	CTask *c =  t->type;
	if(t->processState == 0)
		InitTask(o, 0);
	if(t->processState == 1)
	{
		switch(c->type)
		{
			case ORDTSKTYPE_OBJECT_REFERENCE:
			case ORDTSKTYPE_BUILD:
			case ORDTSKTYPE_REPAIR:
			case ORDTSKTYPE_ATTACK:
				{if(!t->target.valid()) {TerminateTask(o, 0); break;}
				//if((o == t->target.get()) || (t->proximity < 0.0f)) break;
				if(c->rejectTargetIfItIsTerminated) if(t->target->flags & FGO_TERMINATED) {TerminateTask(o, 0); break;}
				Vector3 v = t->target->position - o->position;
				float d = v.len2xz();
			  	if((d < t->proximity) || (t->proximity <= -1))
				{
					if(!(t->flags & FSTASK_PROXIMITY_SATISFIED))
					{
						t->flags |= FSTASK_PROXIMITY_SATISFIED;
						if(c->proxSatisfiedSeq)
						{
							SequenceEnv env;
							env.self = o;
							//env.target = t->target;
							c->proxSatisfiedSeq->run(&env);
							if(!g.valid()) return;
						}
					}
					if(t->processState == 1)
					{
						StartCurrentTaskTriggers(o);
						CheckCurrentTaskTriggers(o);
					}
				} else {
					if(t->proximity < 1) t->proximity = 1;
					t->flags &= ~FSTASK_PROXIMITY_SATISFIED;
					StopCurrentTaskTriggers(o);
					if(o != t->target.get())
						ObjStepMove(o, t->target->position);
				}
				break;}
			case ORDTSKTYPE_MOVE:
				{if(!t->destinations.len) {TerminateTask(o, 0); break;}
				Vector3 h(t->destinations.first->value.x, 0, t->destinations.first->value.y);
				Vector3 v = h - o->position;
				float d = v.len2xz();
				if(d < 0.1f)
				{
					t->destinations.remove(t->destinations.first);
					if(!t->destinations.len) TerminateTask(o, 0);
				}
				else
					ObjStepMove(o, h);
				break;}
			case ORDTSKTYPE_UPGRADE:
				StartCurrentTaskTriggers(o);
				CheckCurrentTaskTriggers(o);
				break;
			case ORDTSKTYPE_MISSILE:
				RemoveObject(o); return;
		}

		t->flags &= ~FSTASK_FIRST_EXECUTION;
	}
	if(t->processState == 5)
	{
		s->currentTask++;
		if(s->currentTask >= s->task.len)
		{
			if(s->type->cycle)
				{s->cycled = 1; s->currentTask = 0; InitTask(o, 0);}
			else
				{s->currentTask--; TerminateOrder(o, 0);}
		}
		else
			InitTask(o, 0);
	}
}

void ProcessObjsOrder(GameObject *o)
{
	if(!o->ordercfg.order.len) return;
	DynListEntry<SOrder> *e = o->ordercfg.order.first;
	SOrder *s = &o->ordercfg.order.first->value;

	if(s->processState == 0)
		{s->currentTask = 0;
		InitOrder(o, 0);}
	else if(s->processState == 2)
		{// resumption_sequence
		ResumeOrder(o, 0);}
	else if(s->processState >= 4)
		{o->ordercfg.order.remove(o->ordercfg.order.first);
		return;}

	STask *t = &s->task.getEntry(s->currentTask)->value;
	ProcessCurrentTask(o);
}

void FindWorkingObjs(GameObject *o)
{
	if(o->ordercfg.order.len)
		{workingObjs.add(); workingObjs.last->value = o;}
	for(DynListEntry<GameObject> *e = o->children.first; e; e = e->next)
		FindWorkingObjs(&e->value);
}

// NOTE: A safe way is to delete entries in the list only in ProcessAllOrders
// during the iteration, and not elsewhere, so we can be sure that the list
// can be iterated correctly.
// Adding new objects at the end of the list is safe.

void ProcessAllOrders()
{
	workingObjs.clear();
	FindWorkingObjs(levelobj);

	DynListEntry<goref> *nextle;
	for(DynListEntry<goref> *e = workingObjs.first; e; e = nextle)
	{
		nextle = e->next;
		if(!e->value.valid())
			{workingObjs.remove(e); continue;}
		if(!e->value->ordercfg.order.len)
			{workingObjs.remove(e); continue;}

		// ...
		ProcessObjsOrder(e->value.get());

		nextle = e->next;
		// Imagine we assign an order in the last order.
		// If nextle is not updated, it can stay 0 even if
		// a new non-idle object has just been added in the list.
	}
}

//************//

void InitTask(GameObject *o, int x)
{
	if(!o->ordercfg.order.len) return;
	SOrder *s = &o->ordercfg.order.getEntry(x)->value;
	STask *t = &s->task.getEntry(s->currentTask)->value;
	if((t->processState != 0) && (t->processState != 5)) return;

	t->processState = 1;
	if(!x) StopCurrentTaskTriggers(o);
	t->flags &= ~(FSTASK_PROXIMITY_SATISFIED | FSTASK_START_SEQUENCE_EXECUTED);
	t->flags |= FSTASK_FIRST_EXECUTION;
	// TODO: FSTASK_LAST_DESTINATION_VALID ???
	RemoveObjReference(o);

	SequenceEnv env; env.self = o;
	if(t->type->initSeq)
		t->type->initSeq->run(&env);

	// Only execute the start sequence if the task wasn't terminated by
	// the initialisation sequence.
	if(t->processState < 4)
	{
		if(!t->target.valid() || t->type->identifyTargetEachCycle) if(t->type->target)
		{
			SequenceEnv env; env.self = o;
			t->target = t->type->target->getfirst(&env);
		}
		if(!t->target.valid() || t->type->identifyTargetEachCycle) if(t->type->usePreviousTaskTarget)
			if(s->currentTask > 0)
				t->target = s->task.getEntry(s->currentTask-1)->value.target;
		if(!t->target.valid()) if(t->type->terminateEntireOrderIfNoTarget)
			{TerminateOrder(o, x); return;}

		if(t->target.valid()) SetObjReference(o, t->target.get());
		//env.target = t->target;
		t->flags |= FSTASK_START_SEQUENCE_EXECUTED;
		if(t->type->startSeq)
			t->type->startSeq->run(&env);
	}
}

void SuspendTask(GameObject *o, int x)
{
	if(!o->ordercfg.order.len) return;
	SOrder *s = &o->ordercfg.order.getEntry(x)->value;
	STask *t = &s->task.getEntry(s->currentTask)->value;
	if(t->processState != 1) return;

	t->processState = 2;
	if(t->type->suspendSeq)
	{
		SequenceEnv env; env.self = o; //env.target = t->target;
		t->type->suspendSeq->run(&env);
	}

	RemoveObjReference(o);
}

void ResumeTask(GameObject *o, int x)
{
	if(!o->ordercfg.order.len) return;
	SOrder *s = &o->ordercfg.order.getEntry(x)->value;
	STask *t = &s->task.getEntry(s->currentTask)->value;
	if(t->processState != 2) return;

	t->processState = 1;
	if(t->type->resumeSeq)
	{
		SequenceEnv env; env.self = o; //env.target = t->target;
		t->type->resumeSeq->run(&env);
	}

	if(t->target.valid()) SetObjReference(o, t->target.get());
}

void TerminateTask(GameObject *o, int x)
{
	if(!o->ordercfg.order.len) return;
	SOrder *s = &o->ordercfg.order.getEntry(x)->value;
	STask *t = &s->task.getEntry(s->currentTask)->value;
	if(t->processState == 5) return;

	t->processState = 5;
	if(t->type->terminateSeq)
	{
		SequenceEnv env; env.self = o; //env.target = t->target;
		t->type->terminateSeq->run(&env);
	}

	RemoveObjReference(o);
}

void CreateOrder(GameObject *go, SOrder *so, COrder *co, GameObject *tg)
{
	so->type = co;
	so->processState = 0;
	so->cycled = 0;
	so->orderID = go->ordercfg.uniqueOrderID++;
	so->uniqueTaskID = co->tasks.len;
	so->currentTask = 0;

	for(int i = 0; i < co->tasks.len; i++)
	{
		CTask *ct = co->tasks.get(i);
		so->task.add();
		STask *st = &so->task.last->value;
		memset(st, 0, sizeof(STask));
		st->type = ct;
		st->target.deref();
		st->flags = FSTASK_FIRST_EXECUTION;
		st->processState = 0;
		st->taskID = i;
		SequenceEnv env; env.self = go; //env.target = tg;
		if(ct->proxRequirement)
			st->proximity = ct->proxRequirement->get(&env);
		else
			st->proximity = -1;
		st->spawnBlueprint = 0;
		st->faceTowards.x = -1;
	}

	so->task.first->value.target = tg;
}

void InitOrder(GameObject *o, int x)
{
	if(!o->ordercfg.order.len) return;
	SOrder *s; s = &o->ordercfg.order.getEntry(x)->value;
	if((s->processState != 0) && (s->processState != 5)) return;

	s->processState = 1;
	SequenceEnv env; env.self = o;
	if(s->type->initSeq)
		s->type->initSeq->run(&env);
	if(s->processState < 4)
	{
		InitTask(o, x);
		//env.target = s->task.first->value.target;
		if(s->type->startSeq)
			s->type->startSeq->run(&env);
	}
}

void AssignOrderVia(GameObject *go, COrderAssignment *oa, SequenceEnv *env)
{
	SequenceEnv nc;
	env->copyAll(&nc);
	nc.ogiver = nc.self;
	nc.self = go;

	GameObject *tg = 0;
	if(oa->target) tg = oa->target->getfirst(&nc);

	if(!tg)
		if(oa->mode != ORDERASSIGNMODE_DO_LAST)
			if(oa->order->tasks[0]->target)
				tg = oa->order->tasks[0]->target->getfirst(&nc);

	AssignOrder(go, oa->order, oa->mode, tg);
}

void AssignOrder(GameObject *go, COrder *ord, uint mode, GameObject *tg)
{
	SOrder *so;
	switch(mode)
	{
		case ORDERASSIGNMODE_DO_FIRST:
			SuspendOrder(go, 0);
			go->ordercfg.order.addbegin();
			so = &go->ordercfg.order.first->value;
			break;
		case ORDERASSIGNMODE_FORGET_EVERYTHING_ELSE:
			CancelAllOrders(go);
		case ORDERASSIGNMODE_DO_LAST:
			go->ordercfg.order.add();
			so = &go->ordercfg.order.last->value;
			break;
	}
	CreateOrder(go, so, ord, tg);
}

void SuspendOrder(GameObject *o, int x)
{
	if(!o->ordercfg.order.len) return;
	SOrder *s; s = &o->ordercfg.order.getEntry(x)->value;
	if(s->processState != 1) return;

	s->processState = 2;
	SuspendTask(o, x);
	if(s->type->suspendSeq)
	{
		SequenceEnv env; env.self = o; //env.target = s->task.first->value.target;
		s->type->suspendSeq->run(&env);
	}
}

void ResumeOrder(GameObject *o, int x)
{
	if(!o->ordercfg.order.len) return;
	SOrder *s; s = &o->ordercfg.order.getEntry(x)->value;
	if(s->processState != 2) return;

	s->processState = 1;
	ResumeTask(o, x);
	if(s->type->resumeSeq)
	{
		SequenceEnv env; env.self = o; //env.target = s->task.first->value.target;
		s->type->resumeSeq->run(&env);
	}
}

void CancelOrder(GameObject *o, int x)
{
	if(!o->ordercfg.order.len) return;
	SOrder *s; s = &o->ordercfg.order.getEntry(x)->value;
	if(s->processState >= 4) return;

	s->processState = 4;
	TerminateTask(o, x);
	if(s->type->cancelSeq)
	{
		SequenceEnv env; env.self = o; //env.target = s->task.first->value.target;
		s->type->cancelSeq->run(&env);
	}
}

void CancelAllOrders(GameObject *o)
{
	for(int i = 0; i < o->ordercfg.order.len; i++)
		CancelOrder(o, i);
}

void TerminateOrder(GameObject *o, int x)
{
	if(!o->ordercfg.order.len) return;
	SOrder *s = &o->ordercfg.order.getEntry(x)->value;
	if(s->processState >= 4) return;

	s->processState = 5;
	TerminateTask(o, x);
	if(s->type->terminateSeq)
	{
		SequenceEnv env; env.self = o; //env.target = s->task.first->value.target;
		s->type->terminateSeq->run(&env);
	}
}
