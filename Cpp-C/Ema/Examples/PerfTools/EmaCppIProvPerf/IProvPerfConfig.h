///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|          Copyright (C) 2021 Refinitiv.      All rights reserved.          --
///*|-----------------------------------------------------------------------------

#pragma once

#ifndef _IPROV_PERF_CONFIG_H
#define _IPROV_PERF_CONFIG_H

#include "Ema.h"
#include "../Common/PerfConfig.h"

const size_t MAX_PROV_THREADS = (MAX_THREADS);

const refinitiv::ema::access::UInt32 LATENCY_RANDOM_ARRAY_SET_COUNT = 20;
const refinitiv::ema::access::Int32 ALWAYS_SEND_LATENCY_UPDATE = (-1);
const refinitiv::ema::access::Int32 ALWAYS_SEND_LATENCY_GENMSG = (-1);


// Provides configuration options for the interactive Provider.
class IProvPerfConfig : public PerfConfig
{
public:
	IProvPerfConfig();
	~IProvPerfConfig();

	void clearPerfConfig();		// Use Defaults.

	refinitiv::ema::access::UInt32			runTime;					/* Time application runs before exiting(-runTime) */
	refinitiv::ema::access::UInt32			writeStatsInterval;			/* Controls how often statistics are written. */
	bool			displayStats;				/* Controls whether stats appear on the screen. */
	bool			preEncItems;				/* Whether to use pre-encoded data rather than fully encoding. */

	refinitiv::ema::access::EmaString		msgFilename;				/* File of data to use for message payloads. See -msgFile. */
	bool			logLatencyToFile;			/* Whether to log genMsg latency information to a file. See -latencyFile. */
	refinitiv::ema::access::EmaString		latencyLogFilename;			/* Name of the latency log file. See -latencyFile. */
	refinitiv::ema::access::EmaString		statsFilename;				/* Name of the statistics log file. See -statsFile. */

	refinitiv::ema::access::Int32			refreshBurstSize;			/* Total refresh rate per second */
	refinitiv::ema::access::Int32			updatesPerSec;				/* Total update rate per second(includes latency updates). */
	refinitiv::ema::access::Int32			latencyUpdatesPerSec;		/* Total latency update rate per second */
	refinitiv::ema::access::Int32			genMsgsPerSec;				/* Total generic message send rate per second(includes latency gen msgs). */
	refinitiv::ema::access::Int32			latencyGenMsgsPerSec;		/* Total latency generic messages rate per second */

	bool			nanoTime;					/* Configures timestamp format. */
	bool			measureEncode;				/* Measure time to encode messages(-measureEncode) */
	bool			measureDecode;				/* Measure time to decode latency updates (-measureDecode) */

	refinitiv::ema::access::EmaString		loginPosition;				/* Configuration data for handling the login domain. */

	refinitiv::ema::access::EmaString		providerName;				/* Name of the IProvider component in EmaConfig.xml. See -providerName */

	refinitiv::ema::access::Int32			packedMsgBufferSize;		/* Size of buffer for PackedMsg. Max size equal to max fragment size 6144 */

	refinitiv::ema::access::Int32			numberMsgInPackedMsg;		/* Amount of packed  Update Messages into PackedMsg */
private:
	// Defaults
	static refinitiv::ema::access::EmaString defSummaryFilename;
};

#endif // _IPROV_PERF_CONFIG_H
