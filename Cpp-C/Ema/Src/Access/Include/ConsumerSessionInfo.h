/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2020 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ConsumerSessionInfo_h
#define __thomsonreuters_ema_access_ConsumerSessionInfo_h

 /**
	 @class rtsdk::ema::access::ConsumerSessionInfo ConsumerSessionInfo.h "Access/Include/ConsumerSessionInfo.h"
	 @brief ConsumerSessionInfo provides session information for OmmConsumer when EMA throws OmmJsonConverterException.

	 \remark All methods in this class are \ref SingleThreaded.

	 @see OmmJsonConverterException
 */

#include "Access/Include/SessionInfo.h"

namespace rtsdk {

namespace ema {

namespace access {

class EMA_ACCESS_API ConsumerSessionInfo : public SessionInfo
{
public:
	/** Returns the Channel Information for this session
	@return the channel information for this session
	*/
	const ChannelInformation& getChannelInformation() const;

private:

	friend class ErrorClientHandler;
	friend class OmmJsonConverterExceptionImpl;

	ConsumerSessionInfo();
	virtual ~ConsumerSessionInfo();
	ConsumerSessionInfo(const ConsumerSessionInfo&);
	ConsumerSessionInfo& operator=(const ConsumerSessionInfo&);

	ChannelInformation	_channelInfo;
};

}

}

}

#endif // __thomsonreuters_ema_access_ConsumerSessionInfo_h
