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

boolean IsCurOrderID(GameObject *o, int a)
{
	if(!o->ordercfg.order.len) return 0;
	return o->ordercfg.order.first->value.orderID == a;
}

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
			SequenceEnv env; env.self = o;
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
				env.self = o;
				g->referenceTime += g->period;
				g->period = c->period->get(&env);
				c->seq->run(&env);
				return;
			case TASKTRIGGER_ANIMATION_LOOP:
			case TASKTRIGGER_UNINTERRUPTIBLE_ANIMATION_LOOP:
			case TASKTRIGGER_ATTACHMENT_POINT:
				env.self = o;
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
	int sid = s->orderID;
	STask *t = &s->task.getEntry(s->currentTask)->value;
	CTask *c =  t->type;
	if(t->processState == 0)
		InitTask(o);
	if(t->processState == 1)
	{
		t->flags &= ~FSTASK_FIRST_EXECUTION;
		switch(c->type)
		{
			case ORDTSKTYPE_OBJECT_REFERENCE:
			case ORDTSKTYPE_BUILD:
			case ORDTSKTYPE_REPAIR:
			case ORDTSKTYPE_ATTACK:
				{if(!t->target.valid()) {TerminateTask(o); break;}
				//if((o == t->target.get()) || (t->proximity < 0.0f)) break;
				if(c->rejectTargetIfItIsTerminated) if(t->target->flags & FGO_TERMINATED) {TerminateTask(o); break;}
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
							c->proxSatisfiedSeq->run(&env);
							if(!g.valid()) return;
							if(!IsCurOrderID(o, sid)) return;
						}
					}
					if(t->processState == 1)
					{
						StartCurrentTaskTriggers(o);
						CheckCurrentTaskTriggers(o);
					}
				} else {
					if(t->proximity < 1) t->proximity = 1;
					boolean pps = t->flags & FSTASK_PROXIMITY_SATISFIED;
					t->flags &= ~FSTASK_PROXIMITY_SATISFIED;
					StopCurrentTaskTriggers(o);
					if(pps) if(c->proxDissatisfiedSeq)
					{
						SequenceEnv env; env.self = o;
						c->proxDissatisfiedSeq->run(&env);
						if(!g.valid()) return;
						if(!IsCurOrderID(o, sid)) return;
					}
					if(o != t->target.get())
						ObjStepMove(o, t->target->position);
				}
				break;}
			case ORDTSKTYPE_MOVE:
				{if(!t->destinations.len) {TerminateTask(o); break;}
				Vector3 h(t->destinations.first->value.x, 0, t->destinations.first->value.y);
				Vector3 v = h - o->position;
				float d = v.len2xz();
				if(d < 0.5f)
				{
					t->destinations.remove(t->destinations.first);
					if(!t->destinations.len) TerminateTask(o);
				}
				else
					ObjStepMove(o, h);
				break;}
			case ORDTSKTYPE_UPGRADE:
			case ORDTSKTYPE_SPAWN:
				StartCurrentTaskTriggers(o);
				CheckCurrentTaskTriggers(o);
				break;
			case ORDTSKTYPE_MISSILE:
				RemoveObject(o); return;
			case ORDTSKTYPE_FACE_TOWARDS:
				TerminateTask(o); return;
		}
	}
	if(t->processState == 5)
	{
		s->currentTask++;
		if(s->currentTask >= s->task.len)
		{
			if(s->type->cycle)
				{s->cycled = 1; s->currentTask = 0; InitTask(o);}
			else
				{s->currentTask--; TerminateOrder(o);}
		}
		else
			InitTask(o);
	}
}

void ProcessObjsOrder(GameObject *o)
{
	if(!o->ordercfg.order.len) return;
	DynListEntry<SOrder> *e = o->ordercfg.order.first;
	SOrder *s = &o->ordercfg.order.first->value;
	int sid = s->orderID; goref gr = o;

	if(s->processState == 0)
		{s->currentTask = 0;
		InitOrder(o);}
	else if(s->processState == 2)
		{// resumption_sequence
		ResumeOrder(o);}
	else if(s->processState >= 4)
		{o->ordercfg.order.remove(o->ordercfg.order.first);
		return;}

	if(gr.valid()) if(IsCurOrderID(o, sid))
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

void InitTask(GameObject *o)
{
	if(!o->ordercfg.order.len) return;
	SOrder *s = &o->ordercfg.order.first->value;
	int sid = s->orderID;
	STask *t = &s->task.getEntry(s->currentTask)->value;
	if((t->processState != 0) && (t->processState != 5)) return;

	t->processState = 1;
	StopCurrentTaskTriggers(o);
	t->flags &= ~(FSTASK_PROXIMITY_SATISFIED | FSTASK_START_SEQUENCE_EXECUTED);
	t->flags |= FSTASK_FIRST_EXECUTION;
	// TODO: FSTASK_LAST_DESTINATION_VALID ???
	RemoveObjReference(o);

	SequenceEnv env; env.self = o;
	if(t->type->initSeq)
		t->type->initSeq->run(&env);

	if(!env.self.valid()) return;
	if(!IsCurOrderID(o, sid)) return;

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
			{TerminateOrder(o); return;}

		if(t->target.valid()) SetObjReference(o, t->target.get());
		t->flags |= FSTASK_START_SEQUENCE_EXECUTED;
		if(t->type->startSeq)
			t->type->startSeq->run(&env);
	}
}

void SuspendTask(GameObject *o)
{
	if(!o->ordercfg.order.len) return;
	SOrder *s = &o->ordercfg.order.first->value;
	STask *t = &s->task.getEntry(s->currentTask)->value;
	if(t->processState != 1) return;

	RemoveObjReference(o);

	t->processState = 2;
	if(t->type->suspendSeq)
	{
		SequenceEnv env; env.self = o;
		t->type->suspendSeq->run(&env);
	}
}

void ResumeTask(GameObject *o)
{
	if(!o->ordercfg.order.len) return;
	SOrder *s = &o->ordercfg.order.first->value;
	STask *t = &s->task.getEntry(s->currentTask)->value;
	if(t->processState != 2) return;

	if(t->target.valid()) SetObjReference(o, t->target.get());

	t->processState = 1;
	if(t->type->resumeSeq)
	{
		SequenceEnv env; env.self = o;
		t->type->resumeSeq->run(&env);
	}
}

void TerminateTask(GameObject *o)
{
	if(!o->ordercfg.order.len) return;
	SOrder *s = &o->ordercfg.order.first->value;
	STask *t = &s->task.getEntry(s->currentTask)->value;
	if(t->processState == 5) return;

	RemoveObjReference(o);

	t->processState = 5;

	if(t->type->type == ORDTSKTYPE_SPAWN)
	{
		GameObject *n = CreateObject(t->spawnBlueprint, o->player);
		n->position = o->position;
		GOPosChanged(o);
		if(o->param)
		{
			AssignOrder(n, o->param->order, ORDERASSIGNMODE_FORGET_EVERYTHING_ELSE, o->param->obj.get());
			if(n->ordercfg.order.len)
			{
				STask *m = &n->ordercfg.order.first->value.task.first->value;
				if(m->type->type == ORDTSKTYPE_MOVE)
				{
					if(o->param->dest.x >= 0)
					{
						m->destinations.add();
						m->destinations.last->value.x = o->param->dest.x;
						m->destinations.last->value.y = o->param->dest.z;
					}
					if(o->param->face.x >= 0)
					{
						m->faceTowards.x = o->param->face.x;
						m->faceTowards.y = o->param->face.z;
					}
				}
			}
		}
	}

	if(t->type->terminateSeq)
	{
		SequenceEnv env; env.self = o;
		t->type->terminateSeq->run(&env);
	}
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
		SequenceEnv env; env.self = go;
		if(ct->proxRequirement)
			st->proximity = ct->proxRequirement->get(&env);
		else
			st->proximity = -1;
		st->spawnBlueprint = 0;
		st->faceTowards.x = -1;
	}

	so->task.first->value.target = tg;
}

void InitOrder(GameObject *o)
{
	if(!o->ordercfg.order.len) return;
	SOrder *s = &o->ordercfg.order.first->value;
	int sid = s->orderID;
	if((s->processState != 0) && (s->processState != 5)) return;

	s->processState = 1;
	SequenceEnv env; env.self = o;
	if(s->type->initSeq)
		s->type->initSeq->run(&env);

	if(!env.self.valid()) return; if(!IsCurOrderID(o, sid)) return;

	if(s->processState < 4)
	{
		InitTask(o);
		if(!env.self.valid()) return; if(!IsCurOrderID(o, sid)) return;
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
			SuspendOrder(go);
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

void SuspendOrder(GameObject *o)
{
	if(!o->ordercfg.order.len) return;
	SOrder *s = &o->ordercfg.order.first->value;
	int sid = s->orderID; goref gr = o;
	if(s->processState != 1) return;

	s->processState = 2;
	SuspendTask(o);
	if(!gr.valid()) return; if(!IsCurOrderID(o, sid)) return;
	if(s->type->suspendSeq)
	{
		SequenceEnv env; env.self = o;
		s->type->suspendSeq->run(&env);
	}
}

void ResumeOrder(GameObject *o)
{
	if(!o->ordercfg.order.len) return;
	SOrder *s = &o->ordercfg.order.first->value;
	int sid = s->orderID; goref gr = o;
	if(s->processState != 2) return;

	s->processState = 1;
	ResumeTask(o);
	if(!gr.valid()) return; if(!IsCurOrderID(o, sid)) return;
	if(s->type->resumeSeq)
	{
		SequenceEnv env; env.self = o;
		s->type->resumeSeq->run(&env);
	}
}

void CancelOrder(GameObject *o)
{
	if(!o->ordercfg.order.len) return;
	SOrder *s = &o->ordercfg.order.first->value;
	int sid = s->orderID; goref gr = o;
	if(s->processState >= 4) goto rmord; // note that if order is cancelled a second time in task term seq, order cancel seq might not be executed

	s->processState = 4;
	TerminateTask(o);
	if(!gr.valid()) return; if(!IsCurOrderID(o, sid)) return;
	if(s->type->cancelSeq)
	{
		SequenceEnv env; env.self = o;
		s->type->cancelSeq->run(&env);
	}

	if(!gr.valid()) return; if(!IsCurOrderID(o, sid)) return;
rmord:	o->ordercfg.order.remove(o->ordercfg.order.first);
}

void CancelAllOrders(GameObject *o)
{
	while(o->ordercfg.order.first)
		CancelOrder(o);
}

void TerminateOrder(GameObject *o)
{
	if(!o->ordercfg.order.len) return;
	SOrder *s = &o->ordercfg.order.first->value;
	int sid = s->orderID; goref gr = o;
	if(s->processState >= 4) goto rmord;

	s->processState = 5;
	TerminateTask(o);
	if(!gr.valid()) return; if(!IsCurOrderID(o, sid)) return;
	if(s->type->terminateSeq)
	{
		SequenceEnv env; env.self = o;
		s->type->terminateSeq->run(&env);
	}

	if(!gr.valid()) return; if(!IsCurOrderID(o, sid)) return;
rmord:	o->ordercfg.order.remove(o->ordercfg.order.first);
}
