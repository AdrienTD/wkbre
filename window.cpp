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
bool keypressed[256], keyheld[256];
bool imguienabled = false;

int scrw = 800, scrh = 600;
int drawfps = 0, drawframes = 0, objsdrawn = 0;
int mouseX = 0, mouseY = 0; bool lmbPressed = 0, rmbPressed = 0, mmbPressed = 0;
int HWVPenabled = 1, VSYNCenabled = 1, numBackBuffers = 1, reqRender = 1;
voidfunc onClickWindow = 0;
int winMinimized = 0;
int fullscreen = 0;
bool lostdev = 0;
float mouseWheel = 0.0f;

IRenderer *renderer;

void ResetDevice()
{
	renderer->Reset();
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

	if (imguienabled) {
		ImGuiIO &io = ImGui::GetIO();
		io.MousePos.x = mouseX; io.MousePos.y = mouseY;
	}
}

void GUIMouseClick()
{
	if (imguienabled) {
		ImGuiIO &io = ImGui::GetIO();
		io.MouseDown[0] = true;
		if (io.WantCaptureMouse) return;
	}

	lmbPressed = 1;
	if(actsubmenu)
		if(!IsPointInRect(actsubmenu->posx, actsubmenu->posy, actsubmenu->width, actsubmenu->height, mouseX, mouseY))
			actsubmenu->enabled = 0;
	if(actualpage)
		actualpage->onMouseClick(mouseX, mouseY);
}

void GUIMouseRightClick()
{
	if (imguienabled) {
		ImGuiIO &io = ImGui::GetIO();
		io.MouseDown[1] = true;
		if (io.WantCaptureMouse) return;
	}

	rmbPressed = 1;
	if(actsubmenu)
		if(!IsPointInRect(actsubmenu->posx, actsubmenu->posy, actsubmenu->width, actsubmenu->height, mouseX, mouseY))
			actsubmenu->enabled = 0;
	if(actualpage)
		actualpage->onMouseRightClick(mouseX, mouseY);
}

void GUIMouseMiddleClick()
{
	mmbPressed = 1;
}

void GUIMouseRelease()
{
	if (imguienabled) {
		ImGuiIO &io = ImGui::GetIO();
		io.MouseDown[0] = false;
		if (io.WantCaptureMouse) return;
	}

	lmbPressed = 0;
	if(actualpage)
		actualpage->onMouseRelease(mouseX, mouseY);
}

void GUIMouseRightRelease()
{
	if (imguienabled) {
		ImGuiIO &io = ImGui::GetIO();
		io.MouseDown[1] = false;
		if (io.WantCaptureMouse) return;
	}

	rmbPressed = 0;
}

void GUIMouseMiddleRelease()
{
	mmbPressed = 0;
}

void GUIKeyDown(int key)
{
	if (imguienabled) {
		ImGuiIO &io = ImGui::GetIO();
		if (io.WantCaptureKeyboard) {
			io.KeysDown[key] = 1;
			return;
		}
	}
	keypressed[key] = 1;
}

void GUIKeyUp(int key)
{
	if (imguienabled) {
		ImGuiIO &io = ImGui::GetIO();
		io.KeysDown[key] = 0;
	}
	keypressed[key] = 0;
}

void GUICharTyped(int c)
{
	if(c & 0xFFFF0000) return;
	if (imguienabled) {
		ImGuiIO &io = ImGui::GetIO();
		if (!io.WantCaptureKeyboard) return;
		io.AddInputCharacter((ushort)c);
	}
}

void GUIMouseWheel(int w)
{
	if (imguienabled) {
		ImGuiIO &io = ImGui::GetIO();
		io.MouseWheel += w / 120.0f;
		if (io.WantCaptureMouse) return;
	}
	mouseWheel += w / 120.0f;
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
		case WM_RBUTTONDOWN:
			mouseX = LOWORD(lParam); mouseY = HIWORD(lParam);
			GUIMouseRightClick();
			break;
		case WM_RBUTTONUP:
			mouseX = LOWORD(lParam); mouseY = HIWORD(lParam);
			GUIMouseRightRelease();
			break;
		case WM_MBUTTONDOWN:
			mouseX = LOWORD(lParam); mouseY = HIWORD(lParam);
			GUIMouseMiddleClick();
			break;
		case WM_MBUTTONUP:
			mouseX = LOWORD(lParam); mouseY = HIWORD(lParam);
			GUIMouseMiddleRelease();
			break;
		case WM_KEYDOWN:
			if(wParam & 0xFFFFFF00) break;
			GUIKeyDown(wParam);
			break;
		case WM_KEYUP:
			if(wParam & 0xFFFFFF00) break;
			GUIKeyUp(wParam);
			break;
		case WM_CHAR:
			GUICharTyped(wParam); break;
		case WM_SIZE:
			if(wParam == SIZE_MINIMIZED) {winMinimized = 1; break;}
			else if(wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) winMinimized = 0;

			if((scrw == LOWORD(lParam)) && (scrh == HIWORD(lParam))) break;
			//MessageBeep(64);
			scrw = LOWORD(lParam);
			scrh = HIWORD(lParam);
			if(/*ddev && */ (!winMinimized)) ResetDevice(); break;
		case WM_DESTROY:
			PostQuitMessage(0); break;
		case WM_PAINT:
			ValidateRect(hwnd, 0); break;
		case WM_ERASEBKGND:
			return 1;
		case WM_SYSCOMMAND:
			if((wParam & 0xFFF0) == SC_KEYMENU)
				return 0;
			else	return DefWindowProc(hwnd, uMsg, wParam, lParam);
		case WM_MOUSEWHEEL:
			GUIMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam)); break;
		case WM_CLOSE:
			QuitApp(); break;
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

void HandleWindow()
{
	// Reset pressed keys to false.
	memset(keypressed, 0, sizeof(keypressed));
	// Check for held keys.
	BYTE ks[256];
	GetKeyboardState(ks);
	bool igkbcapture = false;
	if (imguienabled)
		if (ImGui::GetIO().WantCaptureKeyboard)
			igkbcapture = true;
	if ((GetActiveWindow() == hWindow) && !igkbcapture)
		for (int i = 0; i < 256; i++)
			keyheld[i] = ks[i] & 0x80;
	else
		for (int i = 0; i < 256; i++)
			keyheld[i] = false;

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
	//fullscreen = 0; // Buggy atm.

	WNDCLASS wndclass = {CS_OWNDC, WndProc, 0, 0, hInstance,
			LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON)), LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW+1), NULL, className};
	RECT rect = {0, 0, scrw, scrh};
	wndclass.hInstance = hInstance;

	// Creating Window
	int wstyle = fullscreen ? (WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX) : WS_OVERLAPPEDWINDOW;
	if(!RegisterClass(&wndclass)) ferr("Class registration failed.");
	AdjustWindowRect(&rect, wstyle, FALSE);
	hWindow = CreateWindow(className, appName, WS_CLIPCHILDREN | WS_CLIPSIBLINGS | wstyle, CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right-rect.left, rect.bottom-rect.top, NULL, NULL, hInstance, NULL);
	if(!hWindow) ferr("Window creation failed.");
	ShowWindow(hWindow, SW_SHOWNORMAL);
	SetTimer(0, 0, 1000, OnSecond);

	switch(reqRender)
	{
		case 0:
			renderer = CreateNULLRenderer(); break;
		default:
		case 1:
			renderer = CreateD3D9Renderer(); break;
		case 2:
			renderer = CreateOGL1Renderer(); break;
	}
	renderer->Init();
}