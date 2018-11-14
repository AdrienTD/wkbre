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

extern Vector3 camerapos, vLAD;
extern Matrix matrix, matView, mWorld;
extern float farzvalue, occlurate, verticalfov;
extern float camyaw, campitch;

extern int enableMap;
extern goref currentSelection;
extern int drawdebug;
extern int fogenabled;
extern int showrepresentations;

extern Vector3 stdownpos; extern int stdownvalid;
extern Vector3 mapstdownpos; extern int mapstdownvalid;

extern bool meshbatching, animsEnabled;

void InitOOBMList();
void MakeVisObjList();
void InitScene();
void DrawScene();
int IsPointOnScreen(Vector3 a);