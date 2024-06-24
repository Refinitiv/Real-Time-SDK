/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose. See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019-2021 LSEG. All rights reserved.
*/

#include "LatencyRandomArray.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>

using namespace refinitiv::ema::access;

void LatencyRandomArrayOptions::clearLatencyRandomArrayOptions()
{
	totalMsgsPerSec = 0;
	latencyMsgsPerSec = 0;
	ticksPerSec = 0;
	arrayCount = 0;
}
	
LatencyRandomArray::LatencyRandomArray(LatencyRandomArrayOptions const& opts) :
	latencyTickNumbers( (UInt64)(opts.ticksPerSec * opts.arrayCount) ), currentIndex(0)
{
	int i, setPos;

	int latencyMsgsPerTick, latencyMsgsPerTickRemainder;
	int totalMsgsPerTick, totalMsgsPerTickRemainder;

	if(opts.totalMsgsPerSec == 0)
	{
		printf("Random Array: Total message rate is zero.\n");
		exit(-1);
	}

	if(opts.latencyMsgsPerSec == 0)
	{
		printf("Random Array: Latency message rate is zero.\n");
		exit(-1);
	}

	if(opts.latencyMsgsPerSec > opts.totalMsgsPerSec)
	{
		printf("Random Array: Latency message rate is greater than total message rate.\n");
		exit(-1);
	}

	if(opts.arrayCount == 0)
	{
		printf("Random Array: Array count is zero.\n");
		exit(-1);
	}

	latencyMsgsPerTick = opts.latencyMsgsPerSec / opts.ticksPerSec;
	latencyMsgsPerTickRemainder = opts.latencyMsgsPerSec % opts.ticksPerSec;
	totalMsgsPerTick = opts.totalMsgsPerSec / opts.ticksPerSec;
	totalMsgsPerTickRemainder = opts.totalMsgsPerSec % opts.ticksPerSec;

	size_t valueCount = opts.ticksPerSec * opts.arrayCount;

	/* Build random array.
	 * The objective is to create an array that will be used for each second.
	 * It will contain one value for each tick, which indicates which message
	 * during the tick should contain latency information. */

	srand((unsigned int)time(NULL));

	for (setPos = 0; setPos < valueCount; setPos += opts.ticksPerSec)
	{
		/* Each array cell represents one tick.
		 * Fill the array '1'with as many latency messages as we will send per second.  */
		for(i = 0; i < opts.latencyMsgsPerSec; ++i)
			latencyTickNumbers.push_back(1);

		/* Fill the rest with -1 */
		for(i = opts.latencyMsgsPerSec; i < opts.ticksPerSec; ++i)
			latencyTickNumbers.push_back(-1);

		/* Shuffle array to randomize which ticks send latency message */
		for ( i = 0; i < opts.ticksPerSec; ++i )
		{
			int pos1 = abs(rand() % opts.ticksPerSec);
			int pos2 = abs(rand() % opts.ticksPerSec);
			int tmpB = latencyTickNumbers[setPos + pos1];
			latencyTickNumbers[setPos + pos1] = latencyTickNumbers[setPos + pos2];
			latencyTickNumbers[setPos + pos2] = tmpB;
		}

		/* Now, for each tick that sends a latency message, determine which message that will be */
		for ( i = 0; i < opts.ticksPerSec; ++i )
			if (latencyTickNumbers[setPos + i] == 1)
				latencyTickNumbers[setPos + i] = abs((totalMsgsPerTick > 0 ? (rand() % totalMsgsPerTick) : 0) + ((i < totalMsgsPerTickRemainder) ? 1 : 0));
	}

	/* Self-check: We should have filled exactly the size of the array */
	assert(setPos == latencyTickNumbers.size());
}

Int32 LatencyRandomArray::getNext()
{
	UInt32 index = currentIndex;
	if (++currentIndex >= latencyTickNumbers.size())
	{
		currentIndex = 0;
	}
	return latencyTickNumbers[index];
}
