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

#include "../global.h"

GrowList<ClientState> clistates;

char *ReadClientState(char *fp, char **fstline)
{
	ClientState *c = clistates.addp();
	c->obj = FindObjID(atoi(fstline[1]));
	if(c->obj.valid()) c->obj->client = c;
	c->winDiplomacy = c->winReport = c->winTributes = 0;
	c->storedpos.x = -1;
	c->cammode = 0;

	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords;
	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if(!stricmp(word[0], "UPDATE_ID"))
			c->updateid = atoi(word[1]);
		else if(!stricmp(word[0], "CAMERA_POSITION"))
		{
			c->camerapos.x = atof(word[1]);
			c->camerapos.y = atof(word[2]);
			c->camerapos.z = atof(word[3]);
			c->cameraori.x = atof(word[4]);
			c->cameraori.y = atof(word[5]);
		}
		else if(!stricmp(word[0], "DIPLOMACY_WINDOW_ENABLED"))
			c->winDiplomacy = 1;
		else if(!stricmp(word[0], "DIPLOMATIC_REPORT_WINDOW_ENABLED"))
			c->winReport = 1;
		else if(!stricmp(word[0], "TRIBUTES_WINDOW_ENABLED"))
			c->winTributes = 1;
		else if(!stricmp(word[0], "STORED_CAMERA_POSITION"))
		{
			c->storedpos.x = atof(word[1]);
			c->storedpos.y = atof(word[2]);
			c->storedpos.z = atof(word[3]);
			c->storedori.x = atof(word[4]);
			c->storedori.y = atof(word[5]);
		}
		else if(!stricmp(word[0], "CAMERA_PATH_IN_PROGRESS"))
		{
			int s = strCameraPath.find(word[1]); mustbefound(s);
			c->camPathInProgress = &gscamerapath[s];
			c->camPathTime = atoi(word[2]);
			c->cammode = 1;
		}
		else if(!stricmp(word[0], "CAMERA_INTERPOLATION_IN_PROGRESS"))
		{
			c->camInterpolPos.x = atof(word[1]);
			c->camInterpolPos.y = atof(word[2]);
			c->camInterpolPos.z = atof(word[3]);
			c->camInterpolOri.x = atof(word[4]);
			c->camInterpolOri.y = atof(word[5]);
			c->camInterpolDur = atof(word[6]);
			c->camInterpolTime = (float)atoi(word[7]) / 1000;
			c->cammode = 2;
		}
		else if(!stricmp(word[0], "END_CLIENT_STATE"))
			return fp;
	}
	ferr("UEOF"); return fp;
}

void WriteClientStates(FILE *f)
{
	for(uint i = 0; i < clistates.len; i++)
	{
		ClientState *c = clistates.getpnt(i);
		if(!c->obj.valid()) continue;
		fprintf(f, "CLIENT_STATE %u\n", c->obj.getID());
		fprintf(f, "UPDATE_ID %u\n", c->updateid);
		fprintf(f, "CAMERA_POSITION %f %f %f %f %f\n", c->camerapos.x,
			c->camerapos.y, c->camerapos.z, c->cameraori.x,
			c->cameraori.y);
		if(c->storedpos.x >= 0)
			fprintf(f, "STORED_CAMERA_POSITION %f %f %f %f %f\n", c->storedpos.x,
				c->storedpos.y, c->storedpos.z, c->storedori.x,
				c->storedori.y);
		if(c->cammode == 1)
			fprintf(f, "CAMERA_PATH_IN_PROGRESS \"%s\" %u GAME_TIME\n", c->camPathInProgress->name, c->camPathTime);
		else if(c->cammode == 2)
			fprintf(f, "CAMERA_INTERPOLATION_IN_PROGRESS %f %f %f %f %f %f %u GAME_TIME\n", 
				c->camInterpolPos.x, c->camInterpolPos.y, c->camInterpolPos.z,
				c->camInterpolOri.x, c->camInterpolOri.y, c->camInterpolDur, c->camInterpolTime*1000);
		if(c->winDiplomacy) fprintf(f, "DIPLOMACY_WINDOW_ENABLED\n");
		if(c->winReport) fprintf(f, "DIPLOMATIC_REPORT_WINDOW_ENABLED\n");
		if(c->winTributes) fprintf(f, "TRIBUTES_WINDOW_ENABLED\n");
		fprintf(f, "END_CLIENT_STATE\n");
	}
}