/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/* NIProvPerf.h
 * The main NIProvPerf application. Implements a Non-Interactive Provider.  It connects
 * to an ADH, and provides images and update bursts for a given number of items. */

#ifndef _ETAC_NI_PROV_PERF_H
#define _ETAC_NI_PROV_PERF_H

#include "providerThreads.h"
#include "channelHandler.h"
#include "rtr/rsslQueue.h"
#include "rtr/rsslTransport.h"
#if defined(_WIN32)
#include <winsock2.h>
#include <time.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Callback for newly-activated channels. */
RsslRet processActiveChannel(ChannelHandler *pChanHandler, ChannelInfo *pChannelInfo);

/* Callback for failed channels . */
void processInactiveChannel(ChannelHandler *pChanHandler, ChannelInfo *pChannelInfo, RsslError *pError);

/* Callback for received messages. */
RsslRet processMsg(ChannelHandler *pChannelHandler, ChannelInfo* pChannelInfo, RsslBuffer* pBuffer);

/* Clean up and exit application. */
void cleanUpAndExit();

#ifdef __cplusplus
};
#endif

#endif

