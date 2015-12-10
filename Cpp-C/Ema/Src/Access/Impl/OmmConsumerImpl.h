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
#include "Thread.h"
#include "OmmConsumerActiveConfig.h"
#include "OmmClient.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmConsumerImpl : public OmmBaseImpl
{
public:

	static RsslReactorCallbackRet tunnelStreamStatusEventCallback( RsslTunnelStream* , RsslTunnelStreamStatusEvent* );

	static RsslReactorCallbackRet tunnelStreamDefaultMsgCallback( RsslTunnelStream* , RsslTunnelStreamMsgEvent* );

	static RsslReactorCallbackRet tunnelStreamQueueMsgCallback( RsslTunnelStream* , RsslTunnelStreamQueueMsgEvent* );

	OmmConsumerImpl( const OmmConsumerConfig& );

	OmmConsumerImpl( const OmmConsumerConfig& , OmmConsumerErrorClient& );

	virtual ~OmmConsumerImpl();

	UInt64 registerClient( const ReqMsg& , OmmConsumerClient& , void* closure = 0, UInt64 parentHandle = 0 );

	UInt64 registerClient( const TunnelStreamRequest& , OmmConsumerClient& , void* closure = 0 );

	Int64 dispatch( Int64 timeOut = 0 );
	void addSocket( RsslSocket );
	void removeSocket( RsslSocket );
	void downloadDictionary();
	void downloadDirectory();
	void setRsslReactorChannelRole( RsslReactorChannelRole& );

private :

	void uninitialize( bool caughtExcep = false );
	OmmConsumerImpl( const OmmConsumerImpl& );
	OmmConsumerImpl& operator=( const OmmConsumerImpl& );

	OmmClient< OmmConsumerClient >* _theClient;
	OmmConsumerActiveConfig _activeConfig;
	OmmConsumerErrorClient* _ommConsumerErrorClient;

};

}}}

#endif // __thomsonreuters_ema_access_OmmConsumerImpl_h
