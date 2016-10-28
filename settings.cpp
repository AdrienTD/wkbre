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

void ReadSettingsFile()
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE], *fp; int nwords;

	FILE *f = fopen("settings.txt", "rb");
	if(!f) return;

	fseek(f, 0, SEEK_END);
	int fs = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *fc = (char*)malloc(fs+1); if(!fc) ferr("No mem. for settings file.");
	fread(fc, fs, 1, f);
	fc[fs] = 0;
	fclose(f);
	fp = fc;

	while(*fp)
	{
		fp = GetLine(fp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		switch(stfind_cs(SETTING_str, SETTING_NUM, word[0]))
		{
			case SETTING_GAME_DIR:
				strcpy_s(gamedir, 384, word[1]); break;
			case SETTING_FAR_Z_VALUE:
				farzvalue = atof(word[1]); break;
			case SETTING_HARDWARE_VERTEX_PROCESSING:
				HWVPenabled = atoi(word[1]); break;
			case SETTING_OCCLUSION_RATE:
				occlurate = atof(word[1]); break;
			case SETTING_VSYNC:
				VSYNCenabled = atoi(word[1]); break;
			case SETTING_TEXTURE_COMPRESSION:
				TXCenabled = atoi(word[1]); break;
			case SETTING_MAP_MAX_PART_SIZE:
				mappartw = atoi(word[1]);
				mapparth = atoi(word[2]); break;
			case SETTING_FOG:
				fogenabled = atoi(word[1]); break;
			case SETTING_FONT:
				dfontfn = _strdup(word[1]); break;
			case SETTING_HARDWARE_CURSOR:
				hardwareCursor = atoi(word[1]); break;
			case SETTING_FULLSCREEN:
				fullscreen = atoi(word[1]); break;
			case SETTING_SCREEN_SIZE:
				scrw = atoi(word[1]); scrh = atoi(word[2]); break;
			case SETTING_MULTI_BCP:
				allowBCPPatches = atoi(word[1]); break;
			case SETTING_RENDERER:
				reqRender = atoi(word[1]); break;
			case SETTING_USE_MAP_TEXTURE_DATABASE:
				usemaptexdb = atoi(word[1]); break;
			case SETTING_MESH_BATCHING:
				meshbatching = atoi(word[1]); break;
			case SETTING_ANIMATED_MODELS:
				animsEnabled = atoi(word[1]); break;
		}
	}

	free(fc);
}