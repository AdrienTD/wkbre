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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <conio.h>
#include <io.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <ctype.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <mmsystem.h>
#include <d3d9.h>
//#include <d3dx9.h>

#ifdef WKBRE_RELEASE
#define WKBRE_VERSION "0.0.0.0"
#else
#define WKBRE_VERSION "DEV"
#endif

#define MAX_LINE_SIZE 4096
#define mustbefound(a) if((a) == -1) ferr("Something not found.");
#define loadinginfo(s) WriteAndDrawGfxConsole(s)

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
#ifdef _WIN64
typedef unsigned __int64 ucpuint;
typedef signed __int64 scpuint;
#else
typedef unsigned int ucpuint;
typedef signed int scpuint;
#endif

// This is the most used type for game values.
typedef float valuetype;
#define atov(a) atof(a)

// Test 7
enum AppCommand
{
	CMD_SAVE = 1,
	CMD_QUIT,
	CMD_DELETEOBJTYPE,
	CMD_CONVERTOBJTYPE,
	CMD_GIVEOBJTYPE,
	CMD_CREATEOBJ,
	CMD_DELETELASTCREATEDOBJ,
	CMD_RANDOMSUBTYPE,
	CMD_RESETOBJPOS,
	CMD_CAMPOS,
	CMD_CAMDOWNLEFT,
	CMD_CAMDOWNRIGHT,
	CMD_CAMUPLEFT,
	CMD_CAMUPRIGHT,
	CMD_CAMMANOR,
	CMD_SHOWHIDELANDSCAPE,
	CMD_BCMHIMAPLEFT,
	CMD_BCMHIMAPRIGHT,
	CMD_ABOUT,
	CMD_DELETESELOBJECT,
	CMD_DUPLICATESELOBJECT,
	CMD_DELETEOBJCLASS,
	CMD_CAMRESETORI,
	CMD_SELOBJSCALEBIGGER,
	CMD_SELOBJSCALESMALLER,
	CMD_GIVESELOBJECT,
	CMD_RUNACTSEQ,
	CMD_ROTATEOBJQP,
	CMD_TOGGLEEXPERIMENTALKEYS,
	CMD_TOGGLEREPRENSATIONS,
	CMD_CAMCLISTATE,
	CMD_PAUSE,
	CMD_TOGGLEOBJTOOLTIPS,
	CMD_GAME_SPEED_FASTER,
	CMD_GAME_SPEED_SLOWER,
};

extern "C" {
#include "defs.h"
}
#include "resource.h"
#include "vecmat.h"
#include "util.h"
#include "growbuffer.h"
#include "dynlist.h"
//#include "d3dx_dll.h"
#include "window.h"
#include "bitmap.h"
#include "texture.h"
#include "drawing.h"
#include "font.h"
#include "gui.h"
#include "file.h"
#include "language.h"
#include "gfxcon.h"
#include "map.h"
#include "mesh.h"
#include "gameset/gameset.h"
#include "scene.h"
#include "dialog.h"
#include "settings.h"
#include "minimap.h"
#include "menu.h"
//#include "script/script.h"
#include "play/play.h"
#include "play/otproc.h"

extern GUIPage *actualpage;
extern int appexit;
extern HINSTANCE hInstance;

extern DynList<goref> selobjects;
extern char *statustext;
extern char statustextbuf[1024];