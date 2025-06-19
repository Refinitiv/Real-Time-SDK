/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/* ProviderPerfClient.h
 * Provides the logic that receives OMM messages. */

#pragma once

#ifndef _PROVIDER_PERF_CLIENT_H_
#define _PROVIDER_PERF_CLIENT_H_

#include <iostream>
#include <map>

#include "ProviderThread.h"
#include "IProvPerfConfig.h"
#include "Mutex.h"

class ProviderThread;

class ProviderPerfClient : public refinitiv::ema::access::OmmProviderClient
{
public:
	ProviderPerfClient(ProviderThread*, IProvPerfConfig&);
	~ProviderPerfClient();

	void processLoginRequest(const refinitiv::ema::access::ReqMsg&, const refinitiv::ema::access::OmmProviderEvent&);
	void processMarketPriceRequest(const refinitiv::ema::access::ReqMsg&, const refinitiv::ema::access::OmmProviderEvent&);
	void processInvalidItemRequest(const refinitiv::ema::access::ReqMsg&, const refinitiv::ema::access::OmmProviderEvent&);

	bool isActiveStream(refinitiv::ema::access::UInt64);  // is active or not stream by handle

private:
	void onReqMsg(const refinitiv::ema::access::ReqMsg&, const refinitiv::ema::access::OmmProviderEvent&);
	void onClose(const refinitiv::ema::access::ReqMsg& reqMsg,
		const refinitiv::ema::access::OmmProviderEvent& event);
	void onStatusMsg(const refinitiv::ema::access::StatusMsg& statusMsg,
		const refinitiv::ema::access::OmmProviderEvent& event);
	void onPostMsg(const PostMsg& postMsg, const OmmProviderEvent& event);
	void onGenericMsg(const GenericMsg& genericMsg, const OmmProviderEvent& event);


	ProviderThread* providerThread;
	IProvPerfConfig& provPerfConfig;

	// map of <item identifier or stream handle, isActive stream>.
	// List of handles of opened sessions with client and their state (active or not).
	std::map<refinitiv::ema::access::UInt64, bool> clientHandles;
	perftool::common::Mutex	clientHandlesMutex;

	refinitiv::ema::access::UInt64 lastActiveClientHandle;  // quick access to the last client handle for the active stream

};  // class ProviderPerfClient

#endif  // _PROVIDER_PERF_CLIENT_H_
