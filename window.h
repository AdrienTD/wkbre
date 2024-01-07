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

#include <windows.h>
#include "imgui/imgui.h"

extern char appName[];
extern HWND hWindow;
extern bool keypressed[256], keyheld[256];
extern int drawfps, drawframes, objsdrawn;
extern int HWVPenabled, VSYNCenabled, numBackBuffers, reqRender;
extern int scrw, scrh;
extern int mouseX, mouseY; extern bool lmbPressed, rmbPressed, mmbPressed;
extern voidfunc onClickWindow;
extern int winMinimized;
extern int fullscreen;
extern bool lostdev;
extern float mouseWheel;
extern bool imguienabled;

struct IRenderer;
extern IRenderer *renderer;

void ResetDevice();
void HandleWindow();
void InitWindow();