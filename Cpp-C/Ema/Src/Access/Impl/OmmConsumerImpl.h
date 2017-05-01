/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmConsumerImpl_h
#define __thomsonreuters_ema_access_OmmConsumerImpl_h

#include "OmmBaseImpl.h"
#include "OmmConsumerActiveConfig.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class DictionaryCallbackClient;
class DirectoryCallbackClient;
class OmmConsumerConfig;
class OmmConsumerClient;
class OmmConsumerErrorClient;
class ReqMsg;
class TunnelStreamRequest;

class OmmConsumerImpl : public OmmBaseImpl
{
public:

	static RsslReactorCallbackRet tunnelStreamStatusEventCallback( RsslTunnelStream*, RsslTunnelStreamStatusEvent* );

	static RsslReactorCallbackRet tunnelStreamDefaultMsgCallback( RsslTunnelStream*, RsslTunnelStreamMsgEvent* );

	static RsslReactorCallbackRet tunnelStreamQueueMsgCallback( RsslTunnelStream*, RsslTunnelStreamQueueMsgEvent* );

	OmmConsumerImpl( const OmmConsumerConfig& );

	OmmConsumerImpl(const OmmConsumerConfig&, OmmConsumerClient&, void*);

	OmmConsumerImpl( const OmmConsumerConfig&, OmmConsumerErrorClient& );

	OmmConsumerImpl(const OmmConsumerConfig&, OmmConsumerClient&, OmmConsumerErrorClient&, void*);

	virtual ~OmmConsumerImpl();

	UInt64 registerClient( const ReqMsg&, OmmConsumerClient&, void* closure = 0, UInt64 parentHandle = 0 );

	UInt64 registerClient( const TunnelStreamRequest&, OmmConsumerClient&, void* closure = 0 );

	Int64 dispatch( Int64 timeOut = 0 );

	void addSocket( RsslSocket );

	void removeSocket( RsslSocket );

	void loadDictionary();

	void reLoadDirectory();

	void loadDirectory();

	void setRsslReactorChannelRole( RsslReactorChannelRole& );

	void createDictionaryCallbackClient( DictionaryCallbackClient*&, OmmBaseImpl& );

	void createDirectoryCallbackClient( DirectoryCallbackClient*&, OmmBaseImpl& );

	void processChannelEvent( RsslReactorChannelEvent* );

private :

	void readCustomConfig( EmaConfigImpl* );

	OmmConsumerImpl( const OmmConsumerImpl& );
	OmmConsumerImpl& operator=( const OmmConsumerImpl& );

	bool isApiDispatching() const;

	OmmConsumerActiveConfig			_activeConfig;
	OmmConsumerErrorClient*			_ommConsumerErrorClient;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmConsumerImpl_h
