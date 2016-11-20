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

struct Anim3PosCoord
{
	uint nframes;
	uint *ft;
	float **verts;
};

struct batchVertex;
struct RBatch;

struct Anim : public Model
{
	//Mesh *mesh;
	uint dur;
	Anim3PosCoord coord[3];

	Anim(char *fn);
	void CreateVertsFromTime(batchVertex *out, int tm, int grp);
	void prepare();
	void draw(int iwtcolor = 0);
	void drawInBatch(RBatch *batch, int grp, int uvl = 0, int dif = 0, int tm = 0);
};
