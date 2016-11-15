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

#include "global.h"

Font *deffont;
char *dfontfn = "Fonts\\Proportional\\20\\Proportional - 20.font";

#define alpha(c) ( ((c)>>24) & 255 )

uint GetColor32(Bitmap *bm, int x, int y)
{
	return *(uint*)(bm->pix + ((y * bm->w) + x) * 4);
}

int Font::IsVertEmpty(Bitmap *b, int x)
{
	for(int y = 0; y < h; y++)
		if(alpha(GetColor32(b, x, y)) != 0)
			return 0;
	return 1;
}

void Font::CreateSetBmp()
{
	csb = setbmp.addp(); csx = 0; csy = 0;
	csb->w = csb->h = 256;
	csb->form = BMFORMAT_B8G8R8A8;
	csb->pal = 0;
	csb->pix = (uchar*)malloc(csb->w * csb->h * 4);
	memset(csb->pix, 0, csb->w * csb->h * 4);
}

void Font::LoadFntBmp(char *fn, int fstgl)
{
	Bitmap *b = LoadBitmap(fn);
	int x = 0, ch = fstgl;
	//printf("h = %i\n", h);
	while(1) {
		int w = 0;
		// Get pos & width of the actual character.
		while(IsVertEmpty(b, x)) {/*printf("x");*/ x++; if(x >= b->w) goto lfbend;}
		while(!IsVertEmpty(b, x+w)) {/*printf("w");*/ w++; if((x+w) >= b->w) goto lfbend;}

		Glyph *g = glyphs.addp();
		g->ch = ch++;
		g->w = g->a = w; if(gap) g->a++;

		if((csx + w) >= 256)
			{csx = 0; csy += h;}
		if((csy + h) >= 256)
			{cset++; CreateSetBmp();}
		g->x = csx; g->y = csy; g->set = cset;
		BitmapBlit32(csb, csx, csy, b, x, 0, w, h);

		csx += w;
		x += w;
	}
lfbend:	FreeBitmap(b);
}

Font::Font(char *fn)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE], *fp; int nwords;
	char bmname[256];

	cset = 0; CreateSetBmp();

	char *fb; int fs;
	LoadFile(fn, &fb, &fs);
	fp = fb;
	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!stricmp(word[0], "height"))
			h = atoi(word[1]);
		else if(!stricmp(word[0], "glyph_gap"))
			gap = atoi(word[1]);
		else if(!stricmp(word[0], "space_size"))
			space = atoi(word[1]);
		else if(!stricmp(word[0], "source_file"))
			strcpy(bmname, word[1]);
		else if(!stricmp(word[0], "first_glyph"))
			LoadFntBmp(bmname, atoi(word[1]));
		else if(!stricmp(word[0], "advance_x"))
		{
			int rc = atoi(word[1]);
			for(uint i = 0; i < glyphs.len; i++)
				if(glyphs.getpnt(i)->ch == rc)
					{glyphs.getpnt(i)->a += atoi(word[2]); break;}
		}
	}
	free(fb);
	// Generate textures...
	for(int i = 0; i < setbmp.len; i++)
		settex.add(CreateTexture(setbmp.getpnt(i), 1));
	//printf("num glyphs: %i\n", glyphs.len);
}

Glyph *Font::FindGlyph(uint ch)
{
	for(uint i = 0; i < glyphs.len; i++)
		if(glyphs.getpnt(i)->ch == ch)
			return glyphs.getpnt(i);
	return 0;
}

/*int Font::GetWidthChars(char *s, int l)
{
	int r = 0; char *p = s;
	while(*p)
	{
		if(*p == 32)
			{r += space; p++; continue;}
		Glyph *g;
		if(g = FindGlyph((uchar)*p))
			r += g->a;
		// WARNING: Last char can be longer if g->a < g->w.
		p++;
	}
	return r;
}*/

/*int Font::GetWidth(char *s)
{
	return GetWidthChars(s, strlen(s));
}*/

int Font::GetSize(char *s, int *ow, int *oh)
{
	int r = 0; char *p = s; *ow = *oh = h;
	while(*p)
	{
		if(*p == 32)
			{r += space; p++; continue;}
		if(*p == 10)
			{if(r >= *ow) *ow = r;
			r = 0; *oh += h; p++; continue;}
		Glyph *g;
		if(g = FindGlyph((uchar)*p))
			r += g->a;
		// WARNING: Last char can be longer if g->a < g->w.
		p++;
	}
	if(r >= *ow) *ow = r;
	return r;
}

void Font::Draw(int sx, int y, char *str, int c)
{
	char *p = str; int ds = -1; int x = sx;
	//GrowList<FontVertex> 
	while(*p)
	{
		if(*p == 32)
			{x += space; p++; continue;}
		if(*p == 10)
			{x = sx; y += h; p++; continue;}
		Glyph *g = FindGlyph((uchar)*p);
		if(!g)
			{p++; continue;}
		SetTexture(0, settex[g->set]);
		DrawRect(x, y, g->w, h, c, (float)g->x / 256.0f, (float)g->y / 256.0f, (float)g->w / 256.0f, (float)h / 256.0f);
		x += g->a;
		p++;
	}
}

void Font::DrawInRect(int mw, int sx, int sy, char *str, int c)
{
	char *td = str, *b; char *ns = str, *os; int wc, ow, oh;

	while(*ns)
	{
		os = ns;
		while(!isspace((uchar)*(ns++))) if(!*ns) break;
		wc = ns - td;
		b = new char[wc+1]; memcpy(b, td, wc); b[wc] = 0;
/*
		printf("\n -> ");
		for(int i = 0; i < wc; i++)
			putch(b[i]);
*/
		GetSize(b, &ow, &oh); delete [] b;

		// Draw line
		if(ow > mw)
		{
			wc = os - td;
			if(wc)
			{
				b = new char[wc+1]; memcpy(b, td, wc); b[wc] = 0;
				GetSize(b, &ow, &oh);
				Draw(sx, sy, b, c);
				delete [] b;
				sy += oh;
				td = os;
			}
		}

		// Draw last line
		if(!*ns)
			Draw(sx, sy, td, c);
	}
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void InitFont()
{
	//deffont = new Font("Fonts\\Proportional\\20\\Proportional - 20.font");
	//deffont = new Font("Fonts\\Morpheus\\20\\Morpheus - 20.font");
	deffont = new Font(dfontfn);
}

void DrawFont(int x, int y, char *str, int c)
{
/*	RECT r;
	r.left = x; r.top = y;
	r.right = r.bottom = 0;
	dxfont->DrawText(NULL, str, -1, &r, DT_NOCLIP, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
*/
	deffont->Draw(x, y, str, c);
}

void DrawFontInRect(int mx, int x, int y, char *str, int c)
{
	deffont->DrawInRect(mx, x, y, str, c);
}

void GetTextSize(char *str, int *x, int *y)
{
	deffont->GetSize(str, x, y);
}

void DeinitFont()
{
	//delete deffont;
}