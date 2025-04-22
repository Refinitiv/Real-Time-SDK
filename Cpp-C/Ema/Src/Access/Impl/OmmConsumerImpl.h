/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022, 2025 LSEG. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmConsumerImpl_h
#define __refinitiv_ema_access_OmmConsumerImpl_h

#include "OmmBaseImpl.h"
#include "OmmConsumerActiveConfig.h"
#include "ChannelInformation.h"
#include "OAuth2CredentialRenewal.h"
#include "LoginMsgCredentialRenewal.h"

namespace refinitiv {

namespace ema {

namespace access {

class DictionaryCallbackClient;
class DirectoryCallbackClient;
class OmmConsumerConfig;
class OmmConsumerClient;
class OmmOAuth2ConsumerClient;
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

	OmmConsumerImpl(const OmmConsumerConfig&, OmmOAuth2ConsumerClient&, void*);

	OmmConsumerImpl(const OmmConsumerConfig&, OmmConsumerClient&, void*);

	OmmConsumerImpl(const OmmConsumerConfig&, OmmConsumerClient&, OmmOAuth2ConsumerClient&, void*);

	OmmConsumerImpl( const OmmConsumerConfig&, OmmConsumerErrorClient& );

	OmmConsumerImpl(const OmmConsumerConfig&, OmmOAuth2ConsumerClient&, OmmConsumerErrorClient&, void*);

	OmmConsumerImpl(const OmmConsumerConfig&, OmmConsumerClient&, OmmConsumerErrorClient&, void*);

	OmmConsumerImpl(const OmmConsumerConfig&, OmmConsumerClient&, OmmOAuth2ConsumerClient&, OmmConsumerErrorClient&, void*);

	OmmConsumerImpl(const OmmConsumerConfig&, bool unittest); //for internal UnitTest only

	virtual ~OmmConsumerImpl();

	UInt64 registerClient( const ReqMsg&, OmmConsumerClient&, void* closure = 0, UInt64 parentHandle = 0 );

	UInt64 registerClient( const TunnelStreamRequest&, OmmConsumerClient&, void* closure = 0 );

	Int64 dispatch( Int64 timeOut = 0 );

	void addSocket( RsslSocket );

	void removeSocket( RsslSocket );

	void removeAllSocket();

	void loadDictionary();

	void reLoadDirectory();

	void loadDirectory();

	void setRsslReactorChannelRole( RsslReactorChannelRole&);

	void createDictionaryCallbackClient( DictionaryCallbackClient*&, OmmBaseImpl& );

	void createDirectoryCallbackClient( DirectoryCallbackClient*&, OmmBaseImpl& );

	void processChannelEvent( RsslReactorChannelEvent* );

	ImplementationType getImplType();
    void getChannelInformation(ChannelInformation&);
	void getSessionInformation(EmaVector<ChannelInformation>&);
	void getChannelStatistics(ChannelStatistics&);

	void modifyIOCtl(Int32 code, Int32 value);

	void renewOAuth2Credentials(OAuth2CredentialRenewal&);

	void renewLoginMsgCredentials(LoginMsgCredentialRenewal&);

	void fallbackPreferredHost();

private :

	void readCustomConfig( EmaConfigImpl* );

	OmmConsumerImpl( const OmmConsumerImpl& );
	OmmConsumerImpl& operator=( const OmmConsumerImpl& );

	bool isApiDispatching() const;

	OmmConsumerActiveConfig			_activeConfig;
};

}

}

}

#endif // __refinitiv_ema_access_OmmConsumerImpl_h
