/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_LoginCallbackClient_h
#define __thomsonreuters_ema_access_LoginCallbackClient_h

#include "rtr/rsslReactor.h"
#include "EmaList.h"
#include "OmmLoggerClient.h"
#include "ItemCallbackClient.h"
#include "ChannelCallbackClient.h"
#include "AckMsg.h"
#include "GenericMsg.h"
#include "RefreshMsg.h"
#include "StatusMsg.h"
#include "ReqMsg.h"
#include "PostMsg.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class Channel;
class OmmBaseImpl;

class Login : public ListLinks< Login >
{
public :

	static Login* create( OmmBaseImpl& );

	static void destroy( Login*& );

	Login& set( RsslRDMLoginRefresh* );

	Login& set( RsslRDMLoginRequest* );

	const EmaString& toString();

	Channel* getChannel() const;

	Login& setChannel( Channel* );

	void sendLoginClose();

	bool populate( RsslRefreshMsg&, RsslBuffer& );

	bool populate( RsslStatusMsg&, RsslBuffer& );

private :

	EmaString		_username;
	EmaString		_password;
	EmaString		_position;
	EmaString		_applicationId;
	EmaString		_applicationName;
	EmaString		_instanceId;
	EmaString		_toString;
	Channel*		_pChannel;
	UInt64			_supportBatchRequest;
	UInt64			_supportEnhancedSymbolList;
	UInt64			_supportPost;
	UInt64			_singleOpen;
	UInt64			_allowSuspect;
	UInt64			_pauseResume;
	UInt64			_permissionExpressions;
	UInt64			_permissionProfile;
	UInt64			_supportViewRequest;
	UInt64			_role;
	UInt8			_userNameType;
	UInt8			_streamState;
	UInt8			_dataState;
	UInt8			_stateCode;
	bool			_toStringSet;
	bool			_usernameSet;
	bool			_passwordSet;
	bool			_positionSet;
	bool			_applicationIdSet;
	bool			_applicationNameSet;
	bool			_instanceIdSet;
	bool			_stateSet;

	Login();
	virtual ~Login();

	Login( const Login& );
	Login& operator=( const Login& );
};

class LoginList
{
public :

	LoginList();

	virtual ~LoginList();

	void addLogin( Login* );

	void removeLogin( Login* );

	Login* getLogin( Channel* );

	UInt32 size() const;

	Login* operator[]( UInt32 ) const;

	UInt32 sendLoginClose();

private :

	EmaList< Login* >		_list;

	LoginList( const LoginList& );
	LoginList& operator=( const LoginList& );
};

class LoginItem : public SingleItem
{
public :

	static LoginItem* create( OmmBaseImpl&, OmmConsumerClient& , void* , const LoginList& );

	bool open( RsslRDMLoginRequest*, const LoginList& );
	bool modify( const ReqMsg& );
	bool submit( const PostMsg& );
	bool submit( const GenericMsg& );
	bool close();

	ItemType getType() const { return Item::LoginItemEnum; }

private :

	bool submit( RsslRequestMsg* );
	bool submit( RsslGenericMsg* );
	bool submit( RsslPostMsg* );

	static const EmaString		_clientName;

	const LoginList*			_loginList;

	LoginItem( OmmBaseImpl&, OmmConsumerClient& , void* , const LoginList& );
	LoginItem();
	virtual ~LoginItem();
	LoginItem( const LoginItem& );
	LoginItem& operator=( const LoginItem& );
};

class NiProviderLoginItem : public NiProviderSingleItem
{
public:

	static NiProviderLoginItem* create( OmmBaseImpl&, OmmProviderClient&, void*, const LoginList& );

	bool open( RsslRDMLoginRequest*, const LoginList& );
	bool modify( const ReqMsg& );
	bool submit( const PostMsg& );
	bool submit( const GenericMsg& );
	bool close();

	ItemType getType() const { return Item::LoginItemEnum; }

private:

	bool submit( RsslRequestMsg* );
	bool submit( RsslGenericMsg* );
	bool submit( RsslPostMsg* );

	static const EmaString		_clientName;

	const LoginList*			_loginList;

	NiProviderLoginItem( OmmBaseImpl&, OmmProviderClient&, void*, const LoginList& );
	NiProviderLoginItem();
	virtual ~NiProviderLoginItem();
	NiProviderLoginItem( const NiProviderLoginItem& );
	NiProviderLoginItem& operator=( const NiProviderLoginItem& );
};

class LoginCallbackClient
{
public :

	static LoginCallbackClient* create( OmmBaseImpl& );

	static void destroy( LoginCallbackClient*& );

	void initialize();

	RsslReactorCallbackRet processCallback( RsslReactor*, RsslReactorChannel*, RsslRDMLoginMsgEvent* );

	RsslRDMLoginRequest* getLoginRequest();

	UInt32 sendLoginClose();

	LoginItem* getLoginItem( const ReqMsg&, OmmConsumerClient& , void* );

	NiProviderLoginItem* getLoginItem( const ReqMsg&, OmmProviderClient&, void* );

	void sendInternalMsg( LoginItem* );

	RsslReactorCallbackRet processAckMsg( RsslMsg*, RsslReactorChannel*, RsslRDMLoginMsgEvent* );

	static void handleLoginItemCallback( void* );

	const EmaString& getLoginFailureMessage();

	void processChannelEvent( RsslReactorChannelEvent* );

private :

	static const EmaString			_clientName;

	Mutex							_loginItemLock;

	LoginList						_loginList;

	RsslRDMLoginRequest				_loginRequestMsg;

	char*							_loginRequestBuffer;

	RefreshMsg						_refreshMsg;

	StatusMsg						_statusMsg;

	GenericMsg						_genericMsg;

	AckMsg							_ackMsg;

	OmmBaseImpl&					_ommBaseImpl;

	Login*							_requestLogin;

	EmaVector< Item* >				_loginItems;

	EmaVector< Item* >				_loginItemsOnChannelDown;

	EmaString						_loginFailureMsg;

	RsslReactorCallbackRet processGenericMsg( RsslMsg*, RsslReactorChannel*, RsslRDMLoginMsgEvent* );
	RsslReactorCallbackRet processRefreshMsg( RsslMsg*, RsslReactorChannel*, RsslRDMLoginMsgEvent* );
	RsslReactorCallbackRet processStatusMsg( RsslMsg*, RsslReactorChannel*, RsslRDMLoginMsgEvent* );
	bool convertRdmLoginToRsslBuffer( RsslReactorChannel*, RsslRDMLoginMsgEvent*, RsslBuffer* );

	LoginCallbackClient( OmmBaseImpl& );
	virtual ~LoginCallbackClient();

	LoginCallbackClient();
	LoginCallbackClient( const LoginCallbackClient& );
	LoginCallbackClient& operator=( const LoginCallbackClient& );
};

struct LoginItemCreationCallbackStruct
{
	LoginItemCreationCallbackStruct( LoginCallbackClient* lcbc, LoginItem* li ) : loginCallbackClient( lcbc ), loginItem( li ) {}
	LoginCallbackClient* loginCallbackClient;
	LoginItem* loginItem;
};

struct NiProviderLoginItemCreationCallbackStruct
{
	NiProviderLoginItemCreationCallbackStruct( LoginCallbackClient* lcbc, NiProviderLoginItem* li ) : loginCallbackClient( lcbc ), loginItem( li ) {}
	LoginCallbackClient* loginCallbackClient;
	NiProviderLoginItem* loginItem;
};

}

}

}

#endif // __thomsonreuters_ema_access_LoginCallbackClient_h
