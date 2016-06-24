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

template <class T> struct DynListEntry
{
	T value;
	DynListEntry<T> *previous, *next;
};

template <class T> class DynList
{
public:
	DynListEntry<T> *first, *last;
	int len;

	void add()
	{
		if(len)
		{
			last->next = new DynListEntry<T>;
			last->next->previous = last;
			last->next->next = 0;
			last = last->next;
		}
		else
		{
			first = last = new DynListEntry<T>;
			last->previous = last->next = 0;
		}
		len++;
	}

	void addbegin()
	{
		if(len)
		{
			first->previous = new DynListEntry<T>;
			first->previous->previous = 0;
			first->previous->next = first;
			first = first->previous;
		}
		else
		{
			first = last = new DynListEntry<T>;
			last->previous = last->next = 0;
		}
		len++;
	}

	void addafter(DynListEntry<T> *e)
	{
		if(!e->next) {add(); return;}
		DynListEntry<T> *n = new DynListEntry<T>;
		n->previous = e;
		n->next = e->next;
		e->next->previous = n;
		e->next = n;
		len++;
	}

	void addbefore(DynListEntry<T> *e)
	{
		if(!e->previous) {addbegin(); return;}
		DynListEntry<T> *n = new DynListEntry<T>;
		n->previous = e->previous;
		n->next = e;
		e->previous->next = n;
		e->previous = n;
		len++;
	}

	void add(T t)
	{
		this->add();
		last->value = t;
	}

	void clear()
	{
		if(!len) return;
		DynListEntry<T> *e = first, *n;
		while(e)
		{
			n = e->next;
			delete e;
			e = n;
		}
		first = last = 0; len = 0;
	}

	DynListEntry<T> *getEntry(int x)
	{
		DynListEntry<T> *e = first;
		for(int i = 0; i < x; i++)
			e = e->next;
		return e;
	}

	void remove(DynListEntry<T> *e)
	{
		if(e->previous)
			e->previous->next = e->next;
		if(e->next)
			e->next->previous = e->previous;
		if(first == e) first = e->next; if(last == e) last = e->previous;
		delete e;
		len--;
	}

	void remove(int x)
	{
		remove(getEntry(x));
	}

	void move(DynListEntry<T> *e, DynList<T> *l)
	{
		if(e->previous)
			e->previous->next = e->next;
		if(e->next)
			e->next->previous = e->previous;
		if(first == e) first = e->next; if(last == e) last = e->previous;
		len--;

		e->previous = l->last;
		e->next = 0;
		if(l->last) l->last->next = e;
		l->last = e; if(!l->first) l->first = l->last;
		l->len++;
	}

	DynList() {len = 0; first = last = 0;}
	~DynList() {clear();}
};
