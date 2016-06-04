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

GrowStringList *lstdlggsl;
char *lstdlgheader;
uint lstdlgresult;
int lstdlgfirstsel;

char *strdlgheader, *strdlgout;

INT_PTR CALLBACK CBListDlgBox(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hlist = GetDlgItem(hwndDlg, IDC_FILELIST); HWND c;
	switch(uMsg)
	{
		case WM_INITDIALOG:
			for(int i = 0; i < lstdlggsl->len; i++)
				SendMessage(hlist, LB_INSERTSTRING, -1, (LPARAM)lstdlggsl->getdp(i));
			c = GetDlgItem(hwndDlg, IDC_LBHEADER);
			SetWindowText(c, lstdlgheader);
			SetFocus(hlist);
			//SendMessage(hlist, LB_SETSEL, TRUE, lstdlgfirstsel);
			SendMessage(hlist, LB_SETCURSEL, lstdlgfirstsel, 0);
			return FALSE; // Otherwise another control may be set on focus.
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_FILELIST:
					if(HIWORD(wParam) != LBN_DBLCLK) break;
				case IDOK:
					lstdlgresult = SendMessage(hlist, LB_GETCURSEL, 0, 0);
					if(lstdlgresult == LB_ERR) break;
					EndDialog(hwndDlg, 1); return TRUE;
				case IDCANCEL:
					EndDialog(hwndDlg, 0); return TRUE;
			}
			break;
		case WM_CLOSE:
			EndDialog(hwndDlg, 0); return TRUE;
		default:
			return FALSE;
	}
	return TRUE;
}

INT_PTR CALLBACK CBStringDlgBox(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
			{HWND c = GetDlgItem(hwndDlg, IDC_ASKSTRHEAD);
			SetWindowText(c, strdlgheader);
			SetFocus(GetDlgItem(hwndDlg, IDC_ASKSTRBOX));
			return FALSE;}
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					{HWND c = GetDlgItem(hwndDlg, IDC_ASKSTRBOX);
					GetWindowText(c, strdlgout, 256);
					EndDialog(hwndDlg, 1); return TRUE;}
				case IDCANCEL:
					EndDialog(hwndDlg, 0); return TRUE;
			}
			break;
		case WM_CLOSE:
			EndDialog(hwndDlg, 0); return TRUE;
		default:
			return FALSE;
	}
	return TRUE;
}

int ListDlgBox(GrowStringList *gsl, char *hs, int fsel)
{
	lstdlggsl = gsl; lstdlgheader = hs ? hs : "Select something from this list.";
	lstdlgfirstsel = fsel;
	return (DialogBox(hInstance, MAKEINTRESOURCE(IDD_OPENLEVEL), hWindow, CBListDlgBox)==1)?lstdlgresult:(-1);
}

int StrDlgBox(char *out, char *hs)
{
	strdlgheader = hs ? hs : "Type a string."; strdlgout = out;
	return DialogBox(hInstance, MAKEINTRESOURCE(IDD_ASKSTRING), hWindow, CBStringDlgBox);
}