/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "dictionary.h"
#include "rtr/retmacros.h"
#include "rtr/rsslSeries.h"
#include "rtr/rsslMsgDecoders.h"
#include "rtr/rsslRDM.h"
#include <stdlib.h>
#include <string.h>

void freeDictionary(Dictionary * dictionary)
{
	unsigned int i;
	if (!dictionary)
		return;

	for (i = 0; i < dictionary->count; i++)
	{
		free(dictionary->defs[i].name);
		free(dictionary->defs[i].longName);
	}
	free(dictionary->defs);
	free(dictionary);
}

FieldDef * getFieldDef(const Dictionary * dictionary, RsslInt16 id)
{
	int i, j, k;
	RsslUInt32 loopcount = 0;
	if (!dictionary)
		return 0;

	i = 0;
	j = dictionary->count - 1;
	if (dictionary->count == 0)
		return 0;
	while (1)
	{
		k = (i + j) / 2;
		if (id == dictionary->defs[k].fid)
			return &dictionary->defs[k];
		else if (i == j)
			return 0;
		else if (i + 1 == j)
			j++;
		else if (id < dictionary->defs[k].fid)
			j = k;
		else
			i = k;
		loopcount++;
		if (loopcount > dictionary->count)
			return 0;
	}
}

