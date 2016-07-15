#include "../global.h"

void ReadC3DClip(char **pntfp, char **fstline)
{
	char wwl[MAX_LINE_SIZE], *word[MAX_WORDS_IN_LINE]; int nwords, strdex;
	C3DClip *c = &(gs3dclip[str3DClip.find(fstline[1])]);
	c->postClipSeq = 0;
	while(**pntfp)
	{
		*pntfp = GetLine(*pntfp, wwl);
		nwords = GetWords(wwl, word);
		if(!nwords) continue;
		if( *((uint*)(word[0])) == '_DNE' )
			return;
		if(!stricmp(word[0], "POST_CLIP_SEQUENCE"))
			c->postClipSeq = ReadActSeq(pntfp);
	}
}

void Play3DClip(C3DClip *c)
{
	SequenceEnv env;
	env.self = FindObjID(1027);
	c->postClipSeq->run(&env);
}
