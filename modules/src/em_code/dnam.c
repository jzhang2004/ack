#include "em_private.h"

/* $Id$ */

void
CC_dnam(str, val)
	char *str;
	arith val;
{
	COMMA();
	NOFF(str, val);
}
