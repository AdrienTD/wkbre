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

ITile *itiles = 0;
int numTilesX, numTilesZ, numTilesS;
DynList<goref> oobobjs;

void InitITiles()
{
	numTilesX = mapwidth - mapedge*2;
	numTilesZ = mapheight - mapedge*2;
	numTilesS = numTilesX * numTilesZ;
	itiles = new ITile[numTilesS];
}

void FreeITiles()
{
	delete [] itiles; itiles = 0;
	oobobjs.clear();
}

void ReinitITiles()
{
	FreeITiles();
	InitITiles();
}

void PutObjsInITiles(GameObject *o)
{
	if(!itiles) return;
	for(DynListEntry<GameObject> *e = o->children.first; e; e = e->next)
		PutObjsInITiles(&e->value);
	int tx = o->position.x / 5, tz = o->position.z / 5;
	if((tz < 0) || (tz >= numTilesZ) || (tx < 0) || (tx >= numTilesX))
	{
		oobobjs.add();
		oobobjs.last->value = o;
		o->itile = 0; o->itileole = oobobjs.last;
	}
	else
	{
		ITile *t = &(itiles[tz*numTilesX+tx]);
		t->objs.add();
		t->objs.last->value = o;
		o->itile = t; o->itileole = t->objs.last;
	}

	if(o->objdef->type == CLASS_TERRAIN_ZONE)
	if(o->tiles)
	{
		for(int i = 0; i < o->tiles->len; i++)
		{
			couple<uint> *p = o->tiles->getpnt(i);
			ITile *t = &(itiles[p->y * numTilesX + p->x]);
			t->zones.add();
			t->zones.last->value = o;
			//printf(".");
		}
	}	
}

void ResetITiles()
{
	if(!itiles) return;
	oobobjs.clear();
	for(int i = 0; i < numTilesS; i++)
		itiles[i].objs.clear();
	PutObjsInITiles(levelobj);
}

void GOPosChanged(GameObject *o, boolean sendEvents, boolean autoHeight)
{
	// Avoid objects being put out of bounds.
	uint lw = (mapwidth-2*mapedge)*5, lh = (mapheight-2*mapedge)*5;
	if(o->position.x < 0) o->position.x = 0;
	else if(o->position.x >= lw) o->position.x = lw-0.01;
	if(o->position.z < 0) o->position.z = 0;
	else if(o->position.z >= lh) o->position.z = lh-0.01;

	if(autoHeight) if(o->objdef->type != CLASS_MISSILE)
	{
		o->position.y = GetHeight(o->position.x, o->position.z);
		if((o->objdef->type != CLASS_CHARACTER) || o->objdef->floatsOnWater)
		{
			int tx = o->position.x / 5 + mapedge, tz = mapheight - o->position.z / 5 - mapedge;
			if((tz < 0) || (tz >= mapheight) || (tx < 0) || (tx >= mapwidth))
				return;
			float wl = maptiles[tz*mapwidth+tx].waterlev;
			if(wl >= 0)
				if(wl > o->position.y)
					o->position.y = wl;
		}
	}

	if(!itiles) return;
	int x, z;
	x = o->position.x / 5; z = o->position.z / 5;
	// If the object is outside the playable area, move it into the oobobjs list (Out Of Bounds OBJectS)
	if((z < 0) || (z >= numTilesZ) || (x < 0) || (x >= numTilesX))
	{
		if(o->itile)
		{
			o->itile->objs.move(o->itileole, &oobobjs);
			o->itile = 0;
		}
		return;
	}
	// Otherwise, if the object is inside the playable area, move it into the objects list of the tile on which the object is.
	ITile *nt = &(itiles[z*numTilesX+x]);
	if(nt != o->itile)
	{
		if(o->itile)
			o->itile->objs.move(o->itileole, &nt->objs);
		else
			oobobjs.move(o->itileole, &nt->objs);
		o->itile = nt;

		if(sendEvents)
		{
			SequenceEnv senv; senv.self = o;
			DynListEntry<goref> *n;
			for(DynListEntry<goref> *e = nt->objs.first; e; e = n)
			{
				n = e->next;
				if(e->value.valid())
				if(e->value.get() != o)
					SendGameEvent(&senv, e->value.get(), PDEVENT_ON_SHARE_TILE);
			}

			for(DynListEntry<goref> *e = nt->zones.first; e; e = n)
			{
				n = e->next;
				if(e->value.valid())
				if(e->value.get() != o)
					SendGameEvent(&senv, e->value.get(), PDEVENT_ON_OBJECT_ENTERS);
			}
		}
	}
}

void ListObjsInTiles(int tx, int tz, int l, int w, GrowList<goref> *r)
{
	for(int z = tz; z < (tz+w); z++)
	for(int x = tx; x < (tx+l); x++)
	{
		if((x < 0) || (x >= numTilesX) || (z < 0) || (z >= numTilesZ))
			continue;
		ITile *t = &(itiles[z*numTilesX+x]);
		for(DynListEntry<goref> *e = t->objs.first; e; e = e->next)
			r->add(e->value);
	}
}
