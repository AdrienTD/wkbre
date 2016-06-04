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

#define BUTTONSTATE_NORMAL 0
#define BUTTONSTATE_HILIGHTED 1
#define BUTTONSTATE_PRESSED 2
#define NUM_BUTTONSTATES 3

class GUIPage;
extern GUIPage *actualpage;

/*void OnButtonClick()
{
	iwanttoquit = 1;
}*/

class GUIButton
{
public:
	unsigned int width, height, state; int x, y;
	char *tooltip; vfwa onClick; void *param;
	virtual void draw() = 0;
	//virtual void setState(int state) = 0;
	GUIButton() {x = y = width = height = state = 0; tooltip = 0; onClick = (vfwa)0;}
};

/*class GUIImageButton : public GUIButton
{
	Texture *tex[NUM_BUTTONSTATES];
	void draw()
	{
		// TODO
	}
};*/

class GUITextButton : public GUIButton
{
public:
	char *text;
	GUITextButton(int a, int b, int w, int h, vfwa e, char *t, void *p = 0) {x=a;y=b; text = t; width = w; height = h; onClick = e; param = p;}
	void draw()
	{
		int color[3] = {0xC0000080, 0xFF0000FF, 0xFF8080FF};
		//DrawRect(x, y, width, height, (0xFF << (state<<3)) | 0xFF000000);
		NoTexture(0);
		DrawRect(x, y, width, height, color[state]);
		DrawFont(x, y, text);
	}
};

class GUISimpleButton : public GUIButton
{
public:
	GUISimpleButton(int a, int b, int w, int h, vfwa e) {x=a;y=b; width = w; height = h; onClick = e; state = 0; tooltip = (char*)0;}
	void draw()
	{
		DrawRect(x, y, width, height, 0xFF << (state<<3));
	}
};

class GUIPage
{
public:
	GrowList<GUIButton*> buttons;
	int hib, ci;
	GUIPage *parent;
	int goback;

	GUIPage(GUIPage *p = 0) {hib = -1; ci = -1; parent = p; goback = 0;}

	void add(GUIButton *b) {buttons.add(b);}
	void draw()
	{
		if(parent) parent->draw();
		for(int i = 0; i < buttons.len; i++)
			buttons[i]->draw();
	}
	GUIButton *at(int x, int y)
	{
		for(int i = 0; i < buttons.len; i++)
		{
			if((unsigned int)(x - buttons[i]->x) < buttons[i]->width)
			if((unsigned int)(y - buttons[i]->y) < buttons[i]->height)
				return buttons[i];
		}
		//if(parent) parent->at(x, y);
		return 0;
	}
	void move(int x, int y)
	{
		int lasthib = hib;
		hib = -1;
		for(int i = 0; i < buttons.len; i++)
		{
			if((unsigned int)(x - buttons[i]->x) < buttons[i]->width)
			if((unsigned int)(y - buttons[i]->y) < buttons[i]->height)
				{hib = i; break;}
		}
		if(hib != lasthib)
		{
			if(lasthib != -1)
				buttons[lasthib]->state = BUTTONSTATE_NORMAL;
			if(hib != -1)
				buttons[hib]->state = (ci==hib)?BUTTONSTATE_PRESSED:BUTTONSTATE_HILIGHTED;
		}
		if(parent) parent->move(x, y);
	}
	int click(int x, int y)
	{
		for(int i = 0; i < buttons.len; i++)
		{
			if((unsigned int)(x - buttons[i]->x) < buttons[i]->width)
			if((unsigned int)(y - buttons[i]->y) < buttons[i]->height)
				{buttons[ci = i]->state = BUTTONSTATE_PRESSED; return 1;}
		}
		ci = -1;
		if(parent) {int r = parent->click(x, y); if(goback) actualpage = parent; return r;}
		return 0;
	}
	void release(int x, int y)
	{
		if(ci != -1)
		{
			if((unsigned int)(x - buttons[ci]->x) < buttons[ci]->width)
			if((unsigned int)(y - buttons[ci]->y) < buttons[ci]->height)
			{
				buttons[ci]->state = BUTTONSTATE_HILIGHTED;
				buttons[ci]->onClick(buttons[ci]->param);
				ci = -1; return;
			}
			buttons[ci]->state = BUTTONSTATE_NORMAL;
			ci = -1;
			return; // ?
		}
		if(parent) parent->release(x, y);
	}
};