struct GTWPage
{
	char *textBody;
	//char *activationSound;
};

struct CGameTextWindow;

struct GTWButton
{
	CGameTextWindow *gtw;
	int px, py, pw, ph;
	char *text;
	int ocwinaction;
	ActionSeq *ocsequence;
};

struct CGameTextWindow
{
	float px, py; int pw;
	boolean forceReadAllPages, putAtBottomOfScreen;
	GrowList<GTWPage> pages;
	GrowList<GTWButton> buttons;

	GUIElement *ge, *tbe; int curpage;
};

void ReadCGameTextWindow(char **pntfp, char **fstline);
void InitGTWs();
void EnableGTW(CGameTextWindow *g);
void DisableGTW(CGameTextWindow *g);
