struct CamPathNode
{
	CPosition *pos;
	CValue *time;
};

struct CCameraPath
{
	boolean startAtCurCamPos, loopCamPath;
	GrowList<CamPathNode> nodes;
};

void ReadCCameraPath(char **pntfp, char **fstline);
