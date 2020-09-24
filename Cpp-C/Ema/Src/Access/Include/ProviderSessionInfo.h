/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2020 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_access_ProviderSessionInfo_h
#define __rtsdk_ema_access_ProviderSessionInfo_h

 /**
	 @class rtsdk::ema::access::ProviderSessionInfo ProviderSessionInfo.h "Access/Include/ProviderSessionInfo.h"
	 @brief ProviderSessionInfo provides session information for OmmProvider when EMA throws OmmJsonConverterException.

	 \remark All methods in this class are \ref SingleThreaded.

	 @see OmmJsonConverterException
 */

#include "Access/Include/SessionInfo.h"
#include "Access/Include/OmmProvider.h"

namespace rtsdk {

namespace ema {

namespace access {

class EMA_ACCESS_API ProviderSessionInfo : public SessionInfo
{
public:
	/** Returns the Channel Information for this session
	@return the channel information for this session
	*/
	const ChannelInformation& getChannelInformation() const;

	/** Returns a unique login identifier for the client session of a connected client.
		@return login identifier or login handle
	*/
	UInt64 getHandle() const;

	/** Return OmmProvider instance for this event.
		@return reference to OmmProvider
	*/
	OmmProvider& getProvider() const;

	/** Returns a unique client identifier (a.k.a., client handle) associated by EMA with a connected client.
		@return client identifier or handle
	*/
	UInt64 getClientHandle() const;

private:

	friend class ErrorClientHandler;
	friend class OmmJsonConverterExceptionImpl;

	ProviderSessionInfo();
	virtual ~ProviderSessionInfo();
	ProviderSessionInfo(const ProviderSessionInfo&);
	ProviderSessionInfo& operator=(const ProviderSessionInfo&);

	UInt64			_handle;
	UInt64			_clientHandle;
	OmmProvider*	_provider;
	ChannelInformation	_channelInfo;
};

}

}

}

#endif // __rtsdk_ema_access_ProviderSessionInfo_h

