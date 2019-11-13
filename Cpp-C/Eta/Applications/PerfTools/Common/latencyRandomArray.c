/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#include "latencyRandomArray.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>

void latencyRandomArrayIterInit(LatencyRandomArrayIter *pIter)
{
	*pIter = 0;
}

void clearLatencyRandomArrayOptions(LatencyRandomArrayOptions *pOpts)
{
	memset(pOpts, 0, sizeof(LatencyRandomArrayOptions));
}

void createLatencyRandomArray(LatencyRandomArray *pRandomArray, LatencyRandomArrayOptions *pOpts)
{
	int i, setPos;

	int latencyMsgsPerTick, latencyMsgsPerTickRemainder;
	int totalMsgsPerTick, totalMsgsPerTickRemainder;

	if(pOpts->totalMsgsPerSec == 0)
	{
		printf("Random Array: Total message rate is zero.\n");
		exit(-1);
	}

	if(pOpts->latencyMsgsPerSec == 0)
	{
		printf("Random Array: Latency message rate is zero.\n");
		exit(-1);
	}

	if( pOpts->latencyMsgsPerSec > pOpts->totalMsgsPerSec)
	{
		printf("Random Array: Latency message rate is greater than total message rate.\n");
		exit(-1);
	}

	if( pOpts->arrayCount == 0)
	{
		printf("Random Array: Array count is zero.\n");
		exit(-1);
	}

	latencyMsgsPerTick = pOpts->latencyMsgsPerSec / pOpts->ticksPerSec;
	latencyMsgsPerTickRemainder = pOpts->latencyMsgsPerSec % pOpts->ticksPerSec;
	totalMsgsPerTick = pOpts->totalMsgsPerSec / pOpts->ticksPerSec;
	totalMsgsPerTickRemainder = pOpts->totalMsgsPerSec % pOpts->ticksPerSec;

	pRandomArray->_valueCount = pOpts->ticksPerSec * pOpts->arrayCount;
	pRandomArray->_array = (int *)malloc(pRandomArray->_valueCount * sizeof(int));
	if(!pRandomArray->_array)
	{
		printf("Failed to allocate latency random array.\n");
		exit(-1);
	}

	/* Build random array.
	 * The objective is to create an array that will be used for each second.
	 * It will contain one value for each tick, which indicates which message
	 * during the tick should contain latency information. */

	srand((unsigned int)time(NULL));

	for (setPos = 0; setPos < pRandomArray->_valueCount; setPos += pOpts->ticksPerSec)
	{
		/* Each array cell represents one tick.
		 * Fill the array '1'with as many latency messages as we will send per second.  */
		for(i = 0; i < pOpts->latencyMsgsPerSec; ++i)
			pRandomArray->_array[setPos + i] = 1;

		/* Fill the rest with -1 */
		for(i = pOpts->latencyMsgsPerSec; i < pOpts->ticksPerSec; ++i)
			pRandomArray->_array[setPos + i] = -1;

		/* Shuffle array to randomize which ticks send latency message */
		for ( i = 0; i < pOpts->ticksPerSec; ++i )
		{
			int pos1 = abs(rand() % pOpts->ticksPerSec);
			int pos2 = abs(rand() % pOpts->ticksPerSec);
			int tmpB = pRandomArray->_array[setPos + pos1];
			pRandomArray->_array[setPos + pos1] = pRandomArray->_array[setPos + pos2];
			pRandomArray->_array[setPos + pos2] = tmpB;
		}

		/* Now, for each tick that sends a latency message, determine which message that will be */
		for ( i = 0; i < pOpts->ticksPerSec; ++i )
			if (pRandomArray->_array[setPos + i] == 1)
				pRandomArray->_array[setPos + i] = abs(rand() % totalMsgsPerTick + ((i < totalMsgsPerTickRemainder) ? 1 : 0));
	}
	/* Self-check: We should have filled exactly the size of the array */
	assert(setPos == pRandomArray->_valueCount);

}

int latencyRandomArrayGetNext(LatencyRandomArray *pRandomArray, LatencyRandomArrayIter *pIter)
{
	if (*pIter == pRandomArray->_valueCount)
		*pIter = 0;

	return pRandomArray->_array[(*pIter)++];
}

void cleanupLatencyRandomArray(LatencyRandomArray *pRandomArray)
{
	free(pRandomArray->_array);
	pRandomArray->_array = 0;
}
