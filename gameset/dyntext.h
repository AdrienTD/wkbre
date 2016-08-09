struct DynText
{
	char *str;
	GrowList<CValue*> values;
	void get(SequenceEnv *c, char *out);
};

void ReadDynText(DynText *dt, char **w);
