/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ChannelCallbackClient_h
#define __thomsonreuters_ema_access_ChannelCallbackClient_h

#include "rtr/rsslReactor.h"
#include "EmaList.h"
#include "EmaVector.h"
#include "Mutex.h"
#include "OmmLoggerClient.h"

namespace thomsonreuters {
	
namespace ema {

namespace access {

class OmmConsumerImpl;
class Login;
class Directory;
class Dictionary;
class StreamId;
class ChannelConfig;

class Channel : public ListLinks< Channel > 
{
public :

	enum ChannelState {
		ChannelDownEnum = 0,
		ChannelDownReconnectingEnum,
		ChannelUpEnum,
		ChannelReadyEnum
	};

	static Channel* create( OmmConsumerImpl& , const EmaString& , RsslReactor* );
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

	const EmaList< Directory* >& getDirectoryList() const;
	Channel& addDirectory( Directory* );
	Channel& removeDirectory( Directory* );

	Int32 getNextStreamId( UInt32 numberOfBatchItems = 0 );
	void returnStreamId( Int32 );

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
	EmaList< Directory* >	_directoryList;
	mutable bool			_toStringSet;

	Int32            nextStreamId;
	EmaList< StreamId* > recoveredStreamIds;
	Mutex streamIdMutex;
  
	Channel( const EmaString& , RsslReactor* );
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

	const Channel * front() const { return _list.front(); }

private :

	EmaList< Channel* >			_list;

	ChannelList( const ChannelList& );
	ChannelList& operator=( const ChannelList& );
};

class ChannelCallbackClient
{
public :

	static ChannelCallbackClient* create( OmmConsumerImpl& , RsslReactor* );

	static void destroy( ChannelCallbackClient*& );

	RsslReactorCallbackRet processCallback(  RsslReactor* , RsslReactorChannel* , RsslReactorChannelEvent* );

	void initialize( RsslRDMLoginRequest* , RsslRDMDirectoryRequest* );

	void removeChannel( RsslReactorChannel* );

	void closeChannels();

	const ChannelList & getChannelList();

private :

	static const EmaString			_clientName;

	ChannelList						_channelList;

	OmmConsumerImpl&				_ommConsImpl;

	RsslReactor*					_pRsslReactor;

	void  channelParametersToString (ChannelConfig *, EmaString& );

	ChannelCallbackClient( OmmConsumerImpl& , RsslReactor* );

	virtual ~ChannelCallbackClient();

	ChannelCallbackClient();
	ChannelCallbackClient( const ChannelCallbackClient& );
	ChannelCallbackClient& operator=( const ChannelCallbackClient& );
};

}

}

}

#endif // __thomsonreuters_ema_access_ChannelCallbackClient_h
