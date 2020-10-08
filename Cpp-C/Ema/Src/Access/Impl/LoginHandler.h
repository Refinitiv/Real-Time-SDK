/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
*/

#ifndef __refinitiv_ema_access_LoginHandler_h
#define __refinitiv_ema_access_LoginHandler_h

#include "rtr/rsslReactor.h"
#include "EmaVector.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmServerBaseImpl;
class EmaString;
class ItemInfo;
class ClientSession;

class LoginHandler
{
public:

	static RsslReactorCallbackRet loginCallback(RsslReactor*, RsslReactorChannel*, RsslRDMLoginMsgEvent*);

	static LoginHandler* create(OmmServerBaseImpl*);

	static void destroy(LoginHandler*&);

	void initialize();

	const EmaVector< ItemInfo* >&		getLoginItemList();

	void addItemInfo(ItemInfo*);

	void removeItemInfo(ItemInfo*);

	void notifyChannelDown(ClientSession*);

private:

	static const EmaString			_clientName;

	OmmServerBaseImpl*			_pOmmServerBaseImpl;
	RsslBuffer					_rsslMsgBuffer;
	RsslEncodeIterator			_rsslEncodeIter;
	RsslDecodeIterator			_rsslDecodeIter;
	RsslRDMLoginMsg				_rsslRdmLoginMsg;

	EmaVector< ItemInfo* >		_itemInfoList;

	void sendLoginReject(RsslReactorChannel*, RsslInt32, RsslStateCodes, EmaString&);

	LoginHandler(OmmServerBaseImpl*);
	virtual ~LoginHandler();

	LoginHandler();
	LoginHandler(const LoginHandler&);
	LoginHandler& operator=(const LoginHandler&);
};

}

}

}

#endif // __refinitiv_ema_access_LoginHandler_h

