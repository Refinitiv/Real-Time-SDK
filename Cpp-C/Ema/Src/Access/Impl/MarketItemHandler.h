/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
*/

#ifndef __refinitiv_ema_access_MarketItemHandler_h
#define __refinitiv_ema_access_MarketItemHandler_h

#include "rtr/rsslReactor.h"
#include "HashTable.h"
#include "ItemInfo.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmServerBaseImpl;

class MarketItemHandler
{
public:

	static RsslReactorCallbackRet itemCallback(RsslReactor*, RsslReactorChannel*, RsslMsgEvent*);

	static MarketItemHandler* create(OmmServerBaseImpl*);

	static void destroy(MarketItemHandler*&);

	void initialize();

private:

	static const EmaString			_clientName;

	OmmServerBaseImpl* _pOmmServerBaseImpl;

	MarketItemHandler(OmmServerBaseImpl*);
	virtual ~MarketItemHandler();

	RsslBuffer					_rsslMsgBuffer;
	RsslBuffer					_rsslQosStringBuffer;
	RsslEncIterator				_rsslEncodeIter;
	RsslDecIterator				_rsslDecodeIter;
	RsslStatusMsg				_rsslStatusMsg;
	bool						_isDirectoryApiControl;

	void sendRejectMessage(RsslReactorChannel*, RsslMsg*, RsslStateCodes, EmaString&);

	void notifyOnClose(RsslMsg*, ItemInfo*);

	MarketItemHandler();
	MarketItemHandler(const MarketItemHandler&);
	MarketItemHandler& operator=(const MarketItemHandler&);
};

}

}

}

#endif // __refinitiv_ema_access_MarketItemHandler_h
