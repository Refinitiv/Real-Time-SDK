/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
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
class Login;
class Directory;
class Dictionary;
class StreamId;
class ActiveConfig;
class ChannelConfig;

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

	static Channel* create( OmmBaseImpl&, const EmaString&, RsslReactor*, ReactorChannelType reactorChannelType = NORMAL);
	static void destroy( Channel*& );

	const EmaString& getName() const;

	RsslReactor* getRsslReactor() const;

	RsslReactorChannel* getRsslChannel() const;
	Channel& setRsslChannel( RsslReactorChannel* );

	Channel& clearRsslSocket();
	EmaVector< RsslSocket >& getRsslSocket() const;
	Channel& addRsslSocket( RsslSocket );

	ChannelState getChannelState() const;
	Channel& setChannelState( ChannelState );

	Login* getLogin() const;
	Channel& setLogin( Login* );

	Dictionary* getDictionary() const;
	Channel& setDictionary( Dictionary* );

	Channel& addDirectory( Directory* );
	Channel& removeDirectory( Directory* );

	const EmaString& toString() const;

	bool operator==( const Channel& );

	ReactorChannelType getReactorChannelType() const;

	void setParentChannel(Channel* channel);

	Channel* getParentChannel() const;

	bool hasCalledRenewal;


	void setAddedToDeleteList(bool);

	bool getAddedToDeleteList() const;

private :

	EmaString				_name;
	mutable EmaString		_toString;
	RsslReactor*			_pRsslReactor;
	RsslReactorChannel*		_pRsslChannel;
	ChannelState			_state;
	Login*					_pLogin;
	Dictionary*				_pDictionary;
	EmaVector< Directory* >	_directoryList;
	mutable bool			_toStringSet;
	EmaVector< RsslSocket >* _pRsslSocketList;
	ReactorChannelType		_reactorChannelType;
	Channel*				_pParentChannel;

	bool					_inOAuthCallback;

	bool					_addedToDeleteList;

		
	Channel( const EmaString&, RsslReactor*, ReactorChannelType reactorChannelType = NORMAL);
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

	Channel* getChannel( const RsslReactorChannel* ) const;

	UInt32 size() const;

	RsslReactorChannel* operator[]( UInt32 );

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

	void removeChannel( RsslReactorChannel* );

	void closeChannels();

	const ChannelList& getChannelList();

private :

	static const EmaString			_clientName;

	ChannelList						_channelList;

	OmmBaseImpl&					_ommBaseImpl;

	RsslReactor*					_pRsslReactor;

	bool							_bInitialChannelReadyEventReceived;

	RsslReactorChannel*				_pReconnectingReactorChannel; // This is used to close later when the login timeout occurs.

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
