/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ChannelCallbackClient_h
#define __thomsonreuters_ema_access_ChannelCallbackClient_h

#include "rtr/rsslReactor.h"
#include "EmaList.h"
#include "EmaVector.h"
#include "Mutex.h"
#include "OmmLoggerClient.h"

namespace rtsdk {

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

	enum ChannelState
	{
		ChannelDownEnum = 0,
		ChannelDownReconnectingEnum,
		ChannelUpEnum,
		ChannelReadyEnum
	};

	static Channel* create( OmmBaseImpl&, const EmaString&, RsslReactor* );
	static void destroy( Channel*& );

	const EmaString& getName() const;

	RsslReactor* getRsslReactor() const;

	RsslReactorChannel* getRsslChannel() const;
	Channel& setRsslChannel( RsslReactorChannel* );

	RsslSocket getRsslSocket() const;
	Channel& setRsslSocket( RsslSocket );

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

private :

	EmaString				_name;
	mutable EmaString		_toString;
	RsslSocket				_rsslSocket;
	RsslReactor*			_pRsslReactor;
	RsslReactorChannel*		_pRsslChannel;
	ChannelState			_state;
	Login*					_pLogin;
	Dictionary*				_pDictionary;
	EmaVector< Directory* >	_directoryList;
	mutable bool			_toStringSet;
		
	Channel( const EmaString&, RsslReactor* );
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

	Channel* getChannel( RsslSocket ) const;

	UInt32 size() const;

	RsslReactorChannel* operator[]( UInt32 );

	Channel* front() const;

private :

	EmaList< Channel* >			_list;

	ChannelList( const ChannelList& );
	ChannelList& operator=( const ChannelList& );
};

class ChannelCallbackClient
{
public :

	static ChannelCallbackClient* create( OmmBaseImpl&, RsslReactor* );

	static void destroy( ChannelCallbackClient*& );

	RsslReactorCallbackRet processCallback( RsslReactor*, RsslReactorChannel*, RsslReactorChannelEvent* );

	void initialize( RsslRDMLoginRequest*, RsslRDMDirectoryRequest*, RsslReactorOAuthCredential* );

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

#endif // __thomsonreuters_ema_access_ChannelCallbackClient_h
