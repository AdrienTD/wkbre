struct C3DClip
{
	ActionSeq *postClipSeq;
};

void ReadC3DClip(char **pntfp, char **fstline);
void Play3DClip(C3DClip *c);
