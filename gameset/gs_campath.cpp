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

void ReadCCameraPath(char **pntfp, char **fstline)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords, strdex;
	CCameraPath *c = &(gscamerapath[strdex = strCameraPath.find(fstline[1])]);
	c->name = strCameraPath.getdp(strdex);
	c->startAtCurCamPos = c->loopCamPath = 0;
	c->postPlaySeq = 0;
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if( *((uint*)(word[0])) == '_DNE' )
			return;
		if(!stricmp(word[0], "INTERPOLATE_TO"))
		{
			CamPathNode *n = c->nodes.addp();
			char **w = word + 1;
			n->pos = ReadCPosition(&w);
			n->time = ReadValue(&w);
		}
		else if(!stricmp(word[0], "START_AT_CURRENT_CAMERA_POSITION"))
			c->startAtCurCamPos = 1;
		else if(!stricmp(word[0], "LOOP_CAMERA_PATH"))
			c->loopCamPath = 1;
		else if(!stricmp(word[0], "POST_PLAY_SEQUENCE"))
			c->postPlaySeq = ReadActSeq(pntfp);
	}
}


void SetCameraPath(CCameraPath *c, GameObject *o)
{
	ClientState *s = o->client;
	//if(s->curcpnodepos) delete [] s->curcpnodepos;
	//if(s->curcpnodetime) delete [] s->curcpnodetime;
	if(s->cammode == 1) StopCameraPath(s);
	int cn = c->startAtCurCamPos;
	int nn = c->nodes.len + (cn?1:0); s->curcpnn = nn;

	if((!cn) && (nn == 1))
	{
		SequenceEnv env; env.self = o; PosOri po;
		c->nodes.getpnt(0)->pos->get(&env, &po);
		s->camerapos = po.pos;
		s->cameraori = po.ori;
		s->cammode = 0;
		if(c->postPlaySeq)
			c->postPlaySeq->run(&env);
		return;
	}

	s->curcpnodepos = new PosOri[nn];
	s->curcpnodetime = new uint[nn];
	if(cn)
	{
		s->curcpnodepos[0].pos = s->camerapos;
		s->curcpnodepos[0].ori = s->cameraori;
	}
	SequenceEnv env; env.self = o; PosOri po;
	for(int i = 0; i < c->nodes.len; i++)
	{
		CamPathNode *n = c->nodes.getpnt(i);
		n->pos->get(&env, &(s->curcpnodepos[cn+i]));
		s->curcpnodetime[cn+i] = n->time->get(&env) * 1000;
	}
	
	s->curcpnodetime[0] = 0;
	for(int i = 1; i < nn; i++)
		s->curcpnodetime[i] += s->curcpnodetime[i-1];
	s->curcpmaxtime = s->curcpnodetime[nn-1];

	s->cammode = 1;
	s->camPathInProgress = c;
	s->camPathTime = 0;
/*
	printf("### New camera path ###\n");
	for(int i = 0; i < nn; i++)
	{
		PosOri *x = &(s->curcpnodepos[i]);
		printf(" - %f %f %f %f %f / %u\n", x->pos.x, x->pos.y, x->pos.z,
			x->ori.x, x->ori.y, s->curcpnodetime[i]);
	}
*/
}

void AbsOrientation(Vector3 *v)
{
	while(v->x < 0) v->x += 2 * M_PI;
	while(v->y < 0) v->y += 2 * M_PI;
	while(v->x >= M_PI) v->x -= 2 * M_PI;
	while(v->y >= M_PI) v->y -= 2 * M_PI;
}

float signof(float a)
{
	if(a >= 0) return 1;
	else return -1;
}

void UpdateCameraPath(ClientState *s)
{
	CCameraPath *cp = s->camPathInProgress;
	s->camPathTime += petgame * 1000;
	if(s->camPathTime >= s->curcpmaxtime)
	{
		if(cp->loopCamPath)
			s->camPathTime = cp->startAtCurCamPos ? s->curcpnodetime[1] : 0;
		else
			{StopCameraPath(s); return;}
	}

	uint a = -1;
	for(int i = 0; i < s->curcpnn - 1; i++)
		if(s->camPathTime < s->curcpnodetime[i+1])
			{a = i; break;}
	if(a == -1) ferr("Shit!");

	//printf("a = %i\n", a);
	uint ta = s->curcpnodetime[a], tb = s->curcpnodetime[a+1];
	PosOri pa = s->curcpnodepos[a], pb = s->curcpnodepos[a+1];

	float l = (float)(s->camPathTime - ta) / (float)(tb - ta);
	s->camerapos = pa.pos + (pb.pos - pa.pos) * l;
	//s->cameraori = pa.ori + (pb.ori - pa.ori) * l;

	AbsOrientation(&pa.ori); AbsOrientation(&pb.ori);
	float dx, dy;
	if(abs(pb.ori.x - pa.ori.x) <= M_PI)
		dx = pb.ori.x - pa.ori.x;
	else
		dx = -signof(pb.ori.x - pa.ori.x) * (2 * M_PI - abs(pb.ori.x - pa.ori.x));
	if(abs(pb.ori.y - pa.ori.y) <= M_PI)
		dy = pb.ori.y - pa.ori.y;
	else
		dy = -signof(pb.ori.y - pa.ori.y) * (2 * M_PI - abs(pb.ori.y - pa.ori.y));
	s->cameraori = pa.ori + Vector3(dx, dy, 0) * l;
}

void StopCameraPath(ClientState *s, boolean skipActions)
{
	if(s->cammode != 1) return;
	CCameraPath *cp = s->camPathInProgress;
	s->cammode = 0;
	if(s->curcpnodepos) delete [] s->curcpnodepos;
	if(s->curcpnodetime) delete [] s->curcpnodetime;
	s->curcpnodepos = 0; s->curcpnodetime = 0;
	if(cp->postPlaySeq && !skipActions)
	{
		SequenceEnv env; env.self = s->obj;
		cp->postPlaySeq->run(&env);
	}
}