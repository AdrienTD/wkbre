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

struct CCameraPath;

struct ClientState
{
	goref obj;
	int updateid;
	Vector3 camerapos, cameraori;
	boolean winDiplomacy, winReport, winTributes;
	Vector3 storedpos, storedori;

	int cammode; // 0=free, 1=camPath, 2=camInterpol
	CCameraPath *camPathInProgress; uint camPathTime;
	PosOri *curcpnodepos; uint *curcpnodetime, curcpmaxtime, curcpnn;
	Vector3 camInterpolPos, camInterpolOri; float camInterpolDur, camInterpolTime;
};

extern GrowList<ClientState> clistates;

char *ReadClientState(char *fp, char **fstline);
void WriteClientStates(FILE *f);