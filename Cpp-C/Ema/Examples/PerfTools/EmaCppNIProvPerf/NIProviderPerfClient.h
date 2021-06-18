///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|          Copyright (C) 2021 Refinitiv.      All rights reserved.          --
///*|-----------------------------------------------------------------------------

/* NIProviderPerfClient.h
 * Provides the logic that receives OMM messages. */

#pragma once

#ifndef _NIPROVIDER_PERF_CLIENT_H_
#define _NIPROVIDER_PERF_CLIENT_H_

#include <iostream>

#include "Ema.h"
#include "NIProviderThread.h"
#include "NIProvPerfConfig.h"

class NIProviderThread;

// Provides the logic that receives and process OMM messages
class NIProviderPerfClient : public refinitiv::ema::access::OmmProviderClient
{
public:
	NIProviderPerfClient(NIProviderThread*, NIProvPerfConfig&);
	virtual ~NIProviderPerfClient();

	bool isConnectionUp() const;

private:
	void onRefreshMsg(const refinitiv::ema::access::RefreshMsg&, const refinitiv::ema::access::OmmProviderEvent&);
	void onStatusMsg(const refinitiv::ema::access::StatusMsg&, const refinitiv::ema::access::OmmProviderEvent&);
	void onClose(const refinitiv::ema::access::ReqMsg&, const refinitiv::ema::access::OmmProviderEvent&);


	bool  _bConnectionUp;

	NIProviderThread* niProviderThread;
	NIProvPerfConfig& niProvPerfConfig;

};  // class NIProviderPerfClient

#endif  // _NIPROVIDER_PERF_CLIENT_H_
