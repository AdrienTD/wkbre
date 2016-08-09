#include "../global.h"

void DynText::get(SequenceEnv *c, char *out)
{
	// TODO: Find a printf variant with a variable argument list using
	//       an array instead of va_list.
	switch(values.len)
	{
		case 0: strcpy(out, str); break;
		case 1: sprintf(out, str, values[0]->get(c)); break;
		case 2: sprintf(out, str, values[0]->get(c), values[1]->get(c)); break;
		case 3: sprintf(out, str, values[0]->get(c), values[1]->get(c), values[2]->get(c)); break;
		case 4: sprintf(out, str, values[0]->get(c), values[1]->get(c), values[2]->get(c), values[3]->get(c)); break;
		case 5: sprintf(out, str, values[0]->get(c), values[1]->get(c), values[2]->get(c), values[3]->get(c), values[4]->get(c)); break;
		default: strcpy(out, "<Too many percentages!>"); break;
	}
}

void ReadDynText(DynText *dt, char **w)
{
	//printf("ReadDynText: %s\n", w[0]);
	dt->str = GetLocText(*(w++));
	while(*w)
	{
		//printf(" - %s (%p)\n", w[0], w[0]);
		dt->values.add(ReadValue(&w));
	}
}
