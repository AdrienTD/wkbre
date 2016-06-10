
#define IsPointInRect(rx, ry, rw, rh, px, py) \
 ( ((rx)<=(px)) && ((px)<(rx+rw)) && ((ry)<=(py)) && ((py)<(ry+rh)) )

typedef void (*vpcbf)(void*);

void CallCommand(int cmd);

struct GEContainer;

struct GUIElement
{
// "protected" members allow derived classes to use them, whereas "private"
// members don't.
protected:
	char *caption; // please use setCaption to change it.

public:
	int posx, posy, width, height;
	int bgColor; boolean alignment, enabled;
	GEContainer *parent;

	boolean mouseClick;
	virtual void onMouseClick(int mx, int my) {}
	virtual void onMouseRelease(int mx, int my) {}
	virtual void onResize() {}

	virtual void draw(int dx, int dy)
	{
		if(bgColor)
			{NoTexture(0);
			DrawRect(dx, dy, width, height, bgColor);}
	}

	void setCaption(char *s) {if(caption) free(s); caption = strdup(s);}
	void setRect(int x, int y, int w, int h) {posx = x; posy = y; width = w; height = h;}

	void remove();

	GUIElement() : caption(0), bgColor(0), alignment(0),
		mouseClick(0), enabled(1) {}
};

struct GEStaticText : public GUIElement
{
	void draw(int dx, int dy)
	{
		GUIElement::draw(dx, dy);
		DrawFont(dx, dy, caption);
	}
};

struct GEButton : public GUIElement
{
	vpcbf buttonClick; void *cbparam;
	GEButton() : buttonClick(0) {}
	void onMouseRelease(int mx, int my)
	{
		printf("Click! mx = %i, my = %i\n", mx, my);
		if(buttonClick) buttonClick(cbparam);
	}
};

struct GETextButton : public GEButton
{
	uint clrNormal, clrHover, clrPressed;
	GETextButton() : clrNormal(0xFFFF0000), clrHover(0xFF0000FF), clrPressed(0xFF00FF00) {}
	void draw(int dx, int dy)
	{
		int mouseHover = IsPointInRect(dx, dy, width, height, mouseX, mouseY);
		NoTexture(0);
		DrawRect(dx, dy, width, height, mouseHover?(mouseClick?clrPressed:clrHover):clrNormal);
		DrawFont(dx, dy, caption);
	}
};

struct GEPicButton : public GEButton
{
	texture texNormal, texHover, texPressed;
	void draw(int dx, int dy)
	{
		int mouseHover = IsPointInRect(dx, dy, width, height, mouseX, mouseY);
		texture t = mouseHover?(mouseClick?texPressed:texHover):texNormal;
		if(!t.valid())
		{
			if(!texPressed.valid()) t = texHover;
			if(!texHover.valid()) t = texNormal;
		}
		SetTexture(0, t);
		DrawRect(dx, dy, width, height, -1);
	}
};

struct GEContainer : public GUIElement
{
	DynList<GUIElement*> sos;
	vpcbf buttonClick; void *cbparam;

	GEContainer() : buttonClick(0) {}
	void add(GUIElement *e) {sos.add(e); e->parent = this;}
	void DrawSOS(int dx, int dy)
	{
		for(DynListEntry<GUIElement*> *e = sos.first; e; e = e->next)
			if(e->value->enabled)
				e->value->draw(dx + e->value->posx, dy + e->value->posy);
	}
	void draw(int dx, int dy)
	{
		GUIElement::draw(dx, dy);
		DrawSOS(dx, dy);
	}
	void onMouseClick(int mx, int my)
	{
		for(DynListEntry<GUIElement*> *e = sos.last; e; e = e->previous)
			if(e->value->enabled)
			if(IsPointInRect(e->value->posx, e->value->posy, e->value->width, e->value->height, mx, my))
				{e->value->mouseClick = 1;
				e->value->onMouseClick(mx - e->value->posx, my - e->value->posy); return;}
		if(buttonClick) buttonClick(cbparam);
	}
	void onMouseRelease(int mx, int my)
	{
		for(DynListEntry<GUIElement*> *e = sos.last; e; e = e->previous)
			if(e->value->enabled)
			if(IsPointInRect(e->value->posx, e->value->posy, e->value->width, e->value->height, mx, my))
				{e->value->mouseClick = 0;
				e->value->onMouseRelease(mx - e->value->posx, my - e->value->posy); break;}
	}
};

inline void GUIElement::remove()
{
	for(DynListEntry<GUIElement*> *e = parent->sos.first; e; e = e->next)
		if(e->value == this)
			{parent->sos.remove(e); break;}
	delete this;
}

extern GUIElement *movingguielement, *getobringfront; extern int movge_rx, movge_ry;

struct GEWindow : public GEContainer
{
	GETextButton *closebutt;
	GEWindow()
	{
		add(closebutt = new GETextButton);
		closebutt->setCaption(" X");
		closebutt->posx = 50; closebutt->posy = 0;
		closebutt->width = closebutt->height = 20;
		closebutt->bgColor = 0xFF800000;
	}
	void draw(int dx, int dy)
	{
		NoTexture(0);
		DrawGradientRect(dx, dy, width-20, 20, 0xFF0000FF, 0xFFC0C0FF, 0xFF0000FF, 0xFFC0C0FF);
		DrawRect(dx, dy+20, width, height-20, bgColor);
		DrawFont(dx, dy, caption);
		DrawSOS(dx, dy);
	}
	void onMouseClick(int mx, int my)
	{
		if((my < 20) && (mx < (width-20)))
		{
			movingguielement = this;
			movge_rx = -mx; movge_ry = -my;
		}
		getobringfront = this;
		GEContainer::onMouseClick(mx, my);
	}
	void onMouseRelease(int mx, int my)
	{
		movingguielement = 0;
		GEContainer::onMouseRelease(mx, my);
	}
	void onResize()
	{
		closebutt->posx = width - 20;
	}
};

struct GEList : public GUIElement
{
	GrowStringList *sl; int sel;
	void draw(int dx, int dy)
	{
		GUIElement::draw(dx, dy);
		for(int i = 0; i < sl->len; i++)
		{
			NoTexture(0);
			if(i == sel)
				DrawRect(dx, dy + deffont->h * i, width, deffont->h, 0xFFFF0000);
			DrawFont(dx, dy + deffont->h * i, sl->getdp(i));
		}
	}
	void onMouseClick(int mx, int my)
	{
		sel = my / deffont->h;
		printf("List selection: index %i\n", sel);
	}

};

#define MENUBUTTON_WIDTH 90
#define SUBMENUBUTTON_WIDTH 360
#define MENUBUTTON_HEIGHT 20

struct MenuEntry
{
	char *str; int cmd;
};

struct GESubmenu;

extern GESubmenu *actsubmenu;

struct GESubmenu : public GUIElement
{
	MenuEntry *en; int num;

	void draw(int dx, int dy)
	{
		GUIElement::draw(dx, dy);
		for(int i = 0; i < num; i++)
		{
			if(IsPointInRect(dx, dy+i*MENUBUTTON_HEIGHT, SUBMENUBUTTON_WIDTH, MENUBUTTON_HEIGHT, mouseX, mouseY))
			{
				NoTexture(0);
				DrawRect(dx, dy+i*MENUBUTTON_HEIGHT, SUBMENUBUTTON_WIDTH, MENUBUTTON_HEIGHT, 0xFF0000FF);
			}
			DrawFont(dx, dy+i*MENUBUTTON_HEIGHT, en[i].str);
		}
	}

	void onMouseRelease(int mx, int my)
	{
		int e = my / MENUBUTTON_HEIGHT;
		if(e >= num) return;
		CallCommand(en[e].cmd);
		//printf("Command %i\n", en[e].cmd);
		this->enabled = 0;
		actsubmenu = 0;
	}
};

struct GEMenuBar : public GUIElement
{
	char **mbestr; int nmbe;
	GESubmenu **sm;

	GEMenuBar(char **mbs, MenuEntry *en, int ne, GEContainer *parent)
	{
		//parent->add(this);
		nmbe = ne;
		mbestr = new char*[ne];
		for(int i = 0; i < ne; i++)
			mbestr[i] = mbs[i];
		sm = new GESubmenu*[ne];
		//for(int i = 0; i < ne; i++)
		//	sm[i] = new GESubmenu;
		int bme = 0, cme = 0, cb = 0;
		while(1)
		{
			if(en[cme].str == 0)
			{
				sm[cb] = new GESubmenu;
				parent->add(sm[cb]);
				sm[cb]->en = en + bme;
				sm[cb]->num = cme - bme;
				sm[cb]->setRect(cb * MENUBUTTON_WIDTH, MENUBUTTON_HEIGHT, SUBMENUBUTTON_WIDTH, sm[cb]->num * MENUBUTTON_HEIGHT);
				sm[cb]->bgColor = 0xC0000080;
				sm[cb]->enabled = 0;
				bme = cme + 1;
				cb++;
				if(cb >= ne) break;
			}
			cme++;
		}
	}				

	void draw(int dx, int dy)
	{
		GUIElement::draw(dx, dy);
		for(int i = 0; i < nmbe; i++)
		{
			if(IsPointInRect(dx+i*MENUBUTTON_WIDTH, dy, MENUBUTTON_WIDTH, MENUBUTTON_HEIGHT, mouseX, mouseY))
			{
				NoTexture(0);
				DrawRect(dx+i*MENUBUTTON_WIDTH, dy, MENUBUTTON_WIDTH, MENUBUTTON_HEIGHT, 0xFF0000FF);
			}
			DrawFont(dx+i*MENUBUTTON_WIDTH, dy, mbestr[i]);
		}
	}

	void onMouseClick(int mx, int my)
	{
		int e = mx / MENUBUTTON_WIDTH;
		if(e >= nmbe) return;
		actsubmenu = sm[e];
		sm[e]->enabled = 1;
		getobringfront = sm[e];
	}
};