/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019-2021, 2025 LSEG. All rights reserved.
*/

/* LatencyRandomArray.h
 * Generates a randomized array that can be used to determine which message in a burst
 * should contain latency information. 
 * Create the array using createLatencyRandomArray(). Iterate over the array using
 * latencyRandomArrayGetNext().
 */

#pragma once

#ifndef _Latency_Random_Array_H_
#define _Latency_Random_Array_H_

#include "AppVector.h"

class LatencyRandomArrayOptions
{
public:

	int	totalMsgsPerSec;	/* Total messages sent per second */
	int	latencyMsgsPerSec;	/* Total number of latency messages sent per second */
	int	ticksPerSec;		/* Bursts of messages sent per second by the user. */
	int	arrayCount;			/* Increasing this adds more random values to use
							 * (each individual array contains one second's worth of
							 * values).	*/

	LatencyRandomArrayOptions() : totalMsgsPerSec(0), latencyMsgsPerSec(0), ticksPerSec(0), arrayCount(0) {};

	/* Clears a LatencyRandomArrayOptions struct. */
	void clearLatencyRandomArrayOptions();

};


/* It contains one value for each tick, which indicates which message
 * during the tick should contain latency information. */
class LatencyRandomArray {
public:
	LatencyRandomArray(LatencyRandomArrayOptions const& opts);

	/* Iterate over the LatencyRandomArray. The value returned indicates which message in the tick
	 * should contain latency information.  If the value is -1, no latency message should be
	 * sent in that tick. The iteration starts over when the end of the array is reached. */
	refinitiv::ema::access::Int64 getNext();

private:
	refinitiv::ema::access::EmaVector<refinitiv::ema::access::Int64> latencyTickNumbers;

	refinitiv::ema::access::UInt32 currentIndex;
};  // class LatencyRandomArray

#endif  // _Latency_Random_Array_H_
