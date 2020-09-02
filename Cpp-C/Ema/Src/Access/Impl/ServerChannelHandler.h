/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
*/

#ifndef __thomsonreuters_ema_access_ServerChannelHandler_h
#define __thomsonreuters_ema_access_ServerChannelHandler_h

#include "rtr/rsslReactor.h"
#include "OmmServerBaseImpl.h"
#include "ClientSession.h"
#include "EmaList.h"

namespace rtsdk {

namespace ema {

namespace access {

class OmmServerBaseImpl;

class ServerChannelHandler
{
public:

	static RsslReactorCallbackRet channelEventCallback(RsslReactor*, RsslReactorChannel*, RsslReactorChannelEvent*);

	static ServerChannelHandler* create(OmmServerBaseImpl*);

	static void destroy(ServerChannelHandler*&);

	void initialize();

	void closeActiveSessions();

	void removeChannel(RsslReactorChannel* pRsslReactorChannel);

	ClientSessionPtr getClientSession(UInt64) const;

	void addClientSession(ClientSession*);

	void removeClientSession(ClientSession*);

	void closeChannel(RsslReactorChannel*);

	const EmaList<ClientSession*>& getClientSessionList();

	ClientSessionPtr getClientSessionForDictReq() const;

private:

	static const EmaString			_clientName;

	OmmServerBaseImpl* _pOmmServerBaseImpl;

	ServerChannelHandler(OmmServerBaseImpl*);
	virtual ~ServerChannelHandler();

	ServerChannelHandler();
	ServerChannelHandler(const ServerChannelHandler&);
	ServerChannelHandler& operator=(const ServerChannelHandler&);

	class UInt64rHasher
	{
	public:
		size_t operator()(const UInt64&) const;
	};

	class UInt64Equal_To
	{
	public:
		bool operator()(const UInt64&, const UInt64&) const;
	};

	typedef HashTable< UInt64, ClientSessionPtr, UInt64rHasher, UInt64Equal_To > ClientSessionHash;

	ClientSessionHash _clientSessionHash;
	EmaList<ClientSession*> _clientSessionList;
};

}

}

}

#endif // __thomsonreuters_ema_access_ServerChannelHandler_h