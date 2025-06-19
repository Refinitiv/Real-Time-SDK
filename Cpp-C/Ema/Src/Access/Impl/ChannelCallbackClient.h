/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2017,2019-2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_ChannelCallbackClient_h
#define __refinitiv_ema_access_ChannelCallbackClient_h

#include "rtr/rsslReactor.h"
#include "EmaList.h"
#include "EmaVector.h"
#include "Mutex.h"
#include "OmmLoggerClient.h"
#include "OmmBaseImpl.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmBaseImpl;
class LoginRdmRefreshMsgImpl;
class Directory;
class Dictionary;
class StreamId;
class ActiveConfig;
class ChannelConfig;
class ConsumerRoutingSessionChannel;

class Channel : public ListLinks< Channel >
{
public :

	enum ReactorChannelType
	{
		NORMAL,
		WARM_STANDBY,
	};

	enum ChannelState
	{
		ChannelDownEnum = 0,
		ChannelDownReconnectingEnum,
		ChannelUpEnum,
		ChannelReadyEnum
	};

	static Channel* create( OmmBaseImpl&, const EmaString&, RsslReactor*, ReactorChannelType reactorChannelType = NORMAL, RsslReactorWarmStandbyMode warmStandbyType = RSSL_RWSB_MODE_NONE);
	static void destroy( Channel*& );

	const EmaString& getName() const;

	RsslReactor* getRsslReactor() const;

	ChannelState getChannelState() const;
	Channel& setChannelState( ChannelState );

	Dictionary* getDictionary() const;
	Channel& setDictionary( Dictionary* );

	const EmaString& toString() const;

	bool operator==( const Channel& );

	ReactorChannelType getReactorChannelType() const;

	RsslReactorWarmStandbyMode getWarmStandbyMode() const;

	void setParentChannel(Channel* channel);

	Channel* getParentChannel() const;

	bool hasCalledRenewal;

	// Pointer to the ChannelConfig, which is contained in the ActiveConfig.
	ChannelConfig* pChannelConfig;

	void setAddedToDeleteList(bool);

	bool getAddedToDeleteList() const;

	Channel& setConsumerRoutingChannel(ConsumerRoutingSessionChannel*);			// Sets the Consumer Routing Session Channel associated with this Channel.
	ConsumerRoutingSessionChannel* getConsumerRoutingChannel();					// Gets the Consumer Routing Session Channel associated with this Channel.

	Channel& setChannelConfig(ChannelConfig*);
	ChannelConfig* getChannelConfig();

	OmmBaseImpl* getBaseImpl();

private :

	OmmBaseImpl&			_ommBaseImpl;
	EmaString				_name;
	mutable EmaString		_toString;
	RsslReactor*			_pRsslReactor;
	ChannelState			_state;
	Dictionary*				_pDictionary;
	mutable bool			_toStringSet;
	ReactorChannelType		_reactorChannelType;
	RsslReactorWarmStandbyMode		_warmStandbyMode;
	Channel*				_pParentChannel;
	ChannelConfig*			_pChannelConfig;
	ConsumerRoutingSessionChannel* _pRoutingChannel;

	bool					_inOAuthCallback;

	bool					_addedToDeleteList;

	Channel(OmmBaseImpl&, const EmaString&, RsslReactor*, ReactorChannelType reactorChannelType = NORMAL, RsslReactorWarmStandbyMode warmStandbyType = RSSL_RWSB_MODE_NONE);
	virtual ~Channel();

	Channel();
	Channel( const Channel& );
	Channel& operator=( const Channel& );
};

class ChannelList
{
public :

	ChannelList();
	virtual ~ChannelList();

	void addChannel( Channel* );

	void removeChannel( Channel* );

	Channel* getChannel( const EmaString& ) const;

	UInt32 size() const;

	Channel* front() const;

	void removeAllChannel();

private :

	EmaList< Channel* >			_list;
	EmaList< Channel* >			_deleteList;

	ChannelList( const ChannelList& );
	ChannelList& operator=( const ChannelList& );
};

class ChannelCallbackClient
{
public :

	static ChannelCallbackClient* create( OmmBaseImpl&, RsslReactor* );

	static void destroy( ChannelCallbackClient*& );

	RsslReactorCallbackRet processCallback( RsslReactor*, RsslReactorChannel*, RsslReactorChannelEvent* );

	void initialize( );

	Channel* channelConfigToReactorConnectInfo( ChannelConfig* , RsslReactorConnectInfo*, EmaString&);

	void removeChannel( Channel* );

	void closeChannels();

	const ChannelList& getChannelList();

private :

	static const EmaString			_clientName;

	ChannelList						_channelList;

	OmmBaseImpl&					_ommBaseImpl;

	RsslReactor*					_pRsslReactor;

	bool							_bInitialChannelReadyEventReceived;

	void channelParametersToString(ActiveConfig&, ChannelConfig*, EmaString& );

	ChannelCallbackClient( OmmBaseImpl&, RsslReactor* );

	virtual ~ChannelCallbackClient();

	ChannelCallbackClient();
	ChannelCallbackClient( const ChannelCallbackClient& );
	ChannelCallbackClient& operator=( const ChannelCallbackClient& );
};

}

}

}

#endif // __refinitiv_ema_access_ChannelCallbackClient_h
