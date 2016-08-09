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

#include "..\global.h"

DynList<SDiplomaticStatus> diplostats, diplooffers;
uint defaultDiploStat;

void SetDiplomaticStatus(GameObject *x, GameObject *y, int s)
{
	if(x->id > y->id)
		{GameObject *c = x; x = y; y = c;}
	for(DynListEntry<SDiplomaticStatus> *e = diplostats.first; e; e = e->next)
		if(e->value.a == x)
			if(e->value.b == y)
				{e->value.status = s; return;}
	diplostats.add();
	diplostats.last->value.a = x;
	diplostats.last->value.b = y;
	diplostats.last->value.status = s;
}

int GetDiplomaticStatus(GameObject *x, GameObject *y)
{
	if(x->id > y->id)
		{GameObject *c = x; x = y; y = c;}
	for(DynListEntry<SDiplomaticStatus> *e = diplostats.first; e; e = e->next)
		if(e->value.a == x)
			if(e->value.b == y)
				return e->value.status;
	return defaultDiploStat;
}

void MakeDiplomaticOffer(GameObject *x, GameObject *y, int s)
{
	diplooffers.add();
	diplooffers.last->value.a = x;
	diplooffers.last->value.b = y;
	diplooffers.last->value.status = s;
}

void WriteDiplomaticStatuses(FILE *d)
{
	for(DynListEntry<GameObject> *e = levelobj->children.first; e; e = e->next)
	if(e->value.objdef->type == CLASS_PLAYER)
		for(DynListEntry<GameObject> *f = e->next; f; f = f->next)
		if(f->value.objdef->type == CLASS_PLAYER)
			fprintf(d, "\tDIPLOMATIC_STATUS_BETWEEN %u %u \"%s\"\n", e->value.id, f->value.id, strDiplomaticStatus.getdp(GetDiplomaticStatus(&e->value, &f->value)));
}

void WriteDiplomaticOffers(FILE *f)
{
	for(DynListEntry<SDiplomaticStatus> *e = diplooffers.first; e; e = e->next)
		fprintf(f, "\tDIPLOMATIC_OFFER %u %u \"%s\"\n", e->value.a->id, e->value.b->id, strDiplomaticStatus.getdp(e->value.status));
}
