/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

/* ConsPerf.h
 * The ConsPerf application. Implements a consumer, which requests items from a
 * provider and process the images and updates it receives. It also includes support
 * for posting and sending generic messages. */

#ifndef _ETAC_CONS_PERF_H
#define _ETAC_CONS_PERF_H

#include "statistics.h"
#include "consPerfConfig.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslDataPackage.h"
#include "rtr/rsslThread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>

#ifdef _WIN32
#include <winsock2.h>
#include <time.h>
#include <process.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <unistd.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Collects test statistics from all threads. */
void collectStats(RsslBool writeStats, RsslBool displayStats, RsslUInt32 currentRuntimeSec, 
		RsslUInt32 timePassedSec);

/* Prints the end-of-test summary statistics. */
void printSummaryStatistics(FILE *file);

/* Stop and cleanup consumer threads. */
void consumerCleanupThreads();


#ifdef __cplusplus
};
#endif

#endif
