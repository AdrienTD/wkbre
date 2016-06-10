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

char appName[] = "wkbre", className[] = "AG_wkbreWinClass";
HWND hWindow = 0;
char fpstbuf[256];
bool keypressed[256];

IDirect3D9 *d3d9; IDirect3DDevice9 *ddev = 0;
D3DPRESENT_PARAMETERS dpp = {0, 0, D3DFMT_UNKNOWN, 0, D3DMULTISAMPLE_NONE, 0,
	D3DSWAPEFFECT_DISCARD, 0, TRUE, TRUE, D3DFMT_D24X8, D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL, 0, 0/*D3DPRESENT_INTERVAL_IMMEDIATE*/};
int scrw = 640, scrh = 480;
int drawfps = 0, drawframes = 0, objsdrawn = 0;
int mouseX = 0, mouseY = 0;
int HWVPenabled = 1, VSYNCenabled = 1;
voidfunc onClickWindow = 0;
int winMinimized = 0;

void ResetDevice()
{
	//if(dxfont) DeinitFont();
	ddev->Reset(&dpp);
	//if(dxfont) InitFont();
}

void CALLBACK OnSecond(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	drawfps = drawframes;
	drawframes = 0;
	_snprintf(fpstbuf, 255, "%s - FPS: %i - ODPF: %i", appName, drawfps, objsdrawn);
	//objsdrawn = 0;
	SetWindowText(hWindow, fpstbuf);
}

void GUIMouseMove()
{
	if(movingguielement)
	{
		movingguielement->posx = mouseX + movge_rx;
		movingguielement->posy = mouseY + movge_ry;
	}
}

void GUIMouseClick()
{
	if(actsubmenu)
		if(!IsPointInRect(actsubmenu->posx, actsubmenu->posy, actsubmenu->width, actsubmenu->height, mouseX, mouseY))
			actsubmenu->enabled = 0;
	if(actualpage)
		actualpage->onMouseClick(mouseX, mouseY);
}

void GUIMouseRelease()
{
	if(actualpage)
		actualpage->onMouseRelease(mouseX, mouseY);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_MOUSEMOVE:
			mouseX = LOWORD(lParam); mouseY = HIWORD(lParam);
			GUIMouseMove();
			break;
		case WM_LBUTTONDOWN:
			mouseX = LOWORD(lParam); mouseY = HIWORD(lParam);
			GUIMouseClick();
			break;
		case WM_LBUTTONUP:
			mouseX = LOWORD(lParam); mouseY = HIWORD(lParam);
			GUIMouseRelease();
			break;
		case WM_KEYDOWN:
			if(wParam & 0xFFFFFF00) break;
			keypressed[wParam] = 1; break;
		case WM_KEYUP:
			if(wParam & 0xFFFFFF00) break;
			keypressed[wParam] = 0; break;
		case WM_SIZE:
			if(wParam == SIZE_MINIMIZED) winMinimized = 1;
			else if(wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) winMinimized = 0;

			if((scrw == LOWORD(lParam)) && (scrh == HIWORD(lParam))) break;
			//MessageBeep(64);
			scrw = dpp.BackBufferWidth = LOWORD(lParam);
			scrh = dpp.BackBufferHeight = HIWORD(lParam);
			if(ddev && (!winMinimized)) ResetDevice(); break;
		case WM_DESTROY:
			PostQuitMessage(0); break;
		case WM_PAINT:
			ValidateRect(hwnd, 0); break;
		case WM_ERASEBKGND:
			return 1;
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

void HandleWindow()
{
	MSG msg;
	if(winMinimized) WaitMessage();
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if(msg.message == WM_QUIT) exit(msg.wParam);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void InitWindow()
{
	WNDCLASS wndclass = {0, WndProc, 0, 0, hInstance,
			LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON)), LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW+1), NULL, className};
	RECT rect = {0, 0, 640, 480};
	wndclass.hInstance = hInstance;

	// Creating Window
	if(!RegisterClass(&wndclass)) ferr("Class registration failed.");
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	hWindow = CreateWindow(className, appName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right-rect.left, rect.bottom-rect.top, NULL, NULL, hInstance, NULL);
	if(!hWindow) ferr("Window creation failed.");
	ShowWindow(hWindow, SW_SHOWNORMAL);
	SetTimer(0, 0, 1000, OnSecond);

	// Initializing Direct3D 9
	dpp.hDeviceWindow = hWindow;
	dpp.PresentationInterval = VSYNCenabled ? ((VSYNCenabled==2)?D3DPRESENT_INTERVAL_ONE:D3DPRESENT_INTERVAL_DEFAULT) : D3DPRESENT_INTERVAL_IMMEDIATE;
	d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
	if(!d3d9) ferr("Direct3D 9 init failed.");

	if(HWVPenabled)
	if(d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWindow,
	   D3DCREATE_HARDWARE_VERTEXPROCESSING /*| D3DCREATE_PUREDEVICE*/, &dpp, &ddev) == D3D_OK)
		return;
	if(d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWindow,
	   D3DCREATE_SOFTWARE_VERTEXPROCESSING /*| D3DCREATE_PUREDEVICE*/, &dpp, &ddev) != D3D_OK)
		ferr("Direct3D 9 Device creation failed.");
}