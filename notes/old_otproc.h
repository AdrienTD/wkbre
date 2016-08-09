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

extern DynList<goref> workingObjs;

void ObjStepMove(GameObject *o, Vector3 dest);
void ProcessCurrentTask(GameObject *o);
void ProcessObjsOrder(GameObject *o);
void ProcessAllOrders();
void InitTask(GameObject *o, int x = 0);
void SuspendTask(GameObject *o, int x = 0);
void ResumeTask(GameObject *o, int x = 0);
void TerminateTask(GameObject *o, int x = 0);
void CreateOrder(GameObject *go, SOrder *so, COrder *co, GameObject *tg = 0);
void InitOrder(GameObject *o, int x = 0);
void AssignOrderVia(GameObject *go, COrderAssignment *oa, SequenceEnv *env);
void AssignOrder(GameObject *go, COrder *ord, uint mode, GameObject *tg);
void SuspendOrder(GameObject *o, int x = 0);
void ResumeOrder(GameObject *o, int x = 0);
void CancelOrder(GameObject *o, int x = 0);
void CancelAllOrders(GameObject *o);
void TerminateOrder(GameObject *o, int x = 0);