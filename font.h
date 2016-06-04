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


struct Glyph
{
	uint ch, x, y, w, a, set;
};

uint GetColor32(Bitmap *bm, int x, int y);

struct Font
{
	texture tex;
	uint h, gap, space;
	GrowList<Glyph> glyphs;

	GrowList<Bitmap> setbmp;
	GrowList<texture> settex;
	uint cset; int csx, csy; Bitmap *csb;

	int IsVertEmpty(Bitmap *b, int x);
	void CreateSetBmp();
	void LoadFntBmp(char *fn, int fstgl);
	Font(char *fn);

	Glyph *FindGlyph(uint ch);
	int GetWidthChars(char *s, int l);
	int GetWidth(char *s);
	int GetSize(char *s, int *ow, int *oh);
	void Draw(int x, int y, char *str, int c = -1);
};

extern Font *deffont;
extern char *dfontfn;

void InitFont();
void DrawFont(int x, int y, char *str);
void GetTextSize(char *str, int *x, int *y);
void DeinitFont();