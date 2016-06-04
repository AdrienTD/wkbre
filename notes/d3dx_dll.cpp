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

HMODULE d3dxdll;
d3dxassshadfunc DLL_D3DXAssembleShader;

void InitD3DXDLL()
{
	char fntt[16];
	for(uint i = 43; i >= 24; i--)
	{
		sprintf(fntt, "d3dx9_%u.dll", i);
		d3dxdll = LoadLibrary(fntt);
		if(d3dxdll) goto xdfnd;
	}
	ferr("D3DX9 DLL not found. Please take a look at the readme file for help.");

xdfnd:	DLL_D3DXAssembleShader = (d3dxassshadfunc)GetProcAddress(d3dxdll, "D3DXAssembleShader");
}