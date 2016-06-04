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

int *minimap;
int colortable[8] = {0x804040, 0x0000FF, 0xFFFF00, 0xFF0000, 0x00FF00, 0xFF00FF, 0xFF8000, 0x00FFFF};
int mmw, mmh;
int bmpx = 24, bmpy = 50;
int resx, resz;

BITMAPINFO mbih = {{sizeof(BITMAPINFOHEADER), 0/**/, 0/**/, 1, 32, BI_RGB, 0, 1, 1, 0, 0}};
char *mpheadstr;

void DrawObjOnMinimap(GameObject *o)
{
	for(DynListEntry<GameObject> *e = o->children.first; e; e = e->next)
		DrawObjOnMinimap(&e->value);

	int x = o->position.x / 5;
	int y = o->position.z / 5;

	if(((unsigned int)(y*mmw+x)) < (mmw*mmh))
		minimap[y*mmw+x] = colortable[o->color];
}

void CreateMinimap()
{
	if(minimap) free(minimap);
	mbih.bmiHeader.biWidth = mmw = mapwidth - mapedge*2; mbih.bmiHeader.biHeight = mmh = mapheight - mapedge*2;
	minimap = (int*)malloc(sizeof(int) * mmw * mmh);

	// Draw heightmap.
	for(int y = 0; y < mmh; y++)
	for(int x = 0; x < mmw; x++)
	{
		int c = (int)(himap[(y+mapedge)*(mapwidth+1)+x+mapedge] / maphiscale) & 255;
		minimap[(mmh-1-y)*mmw+x] = c | (c<<8) | (c<<16);
	}

	DrawObjOnMinimap(levelobj);
}

INT_PTR CALLBACK CBPositionDlgBox(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
			CreateMinimap();
			SetWindowText(GetDlgItem(hwndDlg, IDC_APHEAD), mpheadstr);
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					{char buf[16]; buf[15] = 0;
					GetWindowText(GetDlgItem(hwndDlg, IDC_EDITXPOS), buf, 15);
					resx = atoi(buf);
					GetWindowText(GetDlgItem(hwndDlg, IDC_EDITZPOS), buf, 15);
					resz = atoi(buf);
					EndDialog(hwndDlg, 1); return TRUE;}
				case IDCANCEL:
					EndDialog(hwndDlg, 0); return TRUE;
			}
			break;
		case WM_LBUTTONDOWN:
			{int x = (LOWORD(lParam) - bmpx)*5, y = -(HIWORD(lParam) - (mmh-1) - bmpy)*5;
			if(((uint)x >= mmw*5) || ((uint)y >= mmh*5)) break;
			HWND hx = GetDlgItem(hwndDlg, IDC_EDITXPOS), hz = GetDlgItem(hwndDlg, IDC_EDITZPOS);
			char buf[16];
			_itoa(x, buf, 10); SetWindowText(hx, buf);
			_itoa(y, buf, 10); SetWindowText(hz, buf);
			break;}
		case WM_PAINT:
			{PAINTSTRUCT ps; HDC hdc = BeginPaint(hwndDlg, &ps);
			//DrawIcon(hdc, 0, 0, LoadIcon(NULL, IDI_INFORMATION));
			StretchDIBits(hdc, bmpx, bmpy, mmw, mmh, 0, 0, mmw, mmh, minimap, &mbih, DIB_RGB_COLORS, SRCCOPY);
			EndPaint(hwndDlg, &ps);
			break;}
		case WM_CLOSE:
			EndDialog(hwndDlg, 0); return TRUE;
		default:
			return FALSE;
	}
	return TRUE;
}

int PositionDlgBox(float *x, float *z, char *h)
{
	mpheadstr = h ? h : "Select/type a position.";
	if(DialogBox(hInstance, MAKEINTRESOURCE(IDD_ASKPOSITION), hWindow, CBPositionDlgBox))
	{
		*x = (float)resx; *z = (float)resz;
		return 1;
	}
	return 0;
}