#include "../global.h"

void ReadCCameraPath(char **pntfp, char **fstline)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords, strdex;
	CCameraPath *c = &(gscamerapath[strdex = strCameraPath.find(fstline[1])]);
	c->name = strCameraPath.getdp(strdex);
	c->startAtCurCamPos = c->loopCamPath = 0;
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if( *((uint*)(word[0])) == '_DNE' )
			return;
		if(!stricmp(word[0], "INTERPOLATE_TO"))
		{
			CamPathNode *n = c->nodes.addp();
			char **w = word + 1;
			n->pos = ReadCPosition(&w);
			n->time = ReadValue(&w);
		}
		else if(!stricmp(word[0], "START_AT_CURRENT_CAMERA_POSITION"))
			c->startAtCurCamPos = 1;
		else if(!stricmp(word[0], "LOOP_CAMERA_PATH"))
			c->loopCamPath = 1;
	}
}
