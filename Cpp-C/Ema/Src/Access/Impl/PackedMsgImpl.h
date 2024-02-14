///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2023 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#ifndef __refinitiv_ema_access_PackedMsgImpl_h
#define __refinitiv_ema_access_PackedMsgImpl_h

#include "PackedMsg.h"
#include "ExceptionTranslator.h"
#include "OmmIProviderImpl.h"
#include "OmmNiProviderImpl.h"

namespace refinitiv {

namespace ema {

namespace access {

class PackedMsgImpl
{
public:
	PackedMsgImpl(OmmProvider* ommProvider);
	virtual ~PackedMsgImpl();
	void initBuffer();
	void initBuffer(UInt32 maxSize);
	void initBuffer(UInt64 clientHandle);
	void initBuffer(UInt64 clientHandle, UInt32 maxSize);
	void addMsg(const Msg& msg, UInt64 handle);
	UInt32 remainingSize() const;
	UInt32 packedMsgCount() const;
	UInt32 maxSize() const;
	void clear();
	UInt64 getClientHandle() const;
	RsslBuffer* getTransportBuffer() const;
	void setTransportBuffer(RsslBuffer*);
	RsslReactorChannel* getRsslReactorChannel() const;
private:
	UInt32 _remainingSize;
	UInt32 _packedMsgCount;
	UInt32 _maxSize;
	UInt64 _clientHandle;
	UInt64 _itemHandle;
	RsslBuffer* _packedBuf;
	OmmProvider* _ommProvider;
	RsslReactorChannel* _reactorChannel;
	OmmIProviderImpl* _ommIProviderImpl;
	OmmNiProviderImpl* _ommNiProviderImpl;
	RsslEncodeIterator _eIter;

	void reactorReleaseBuffer();
};

}
}
}

#endif //__refinitiv_ema_access_PackedMsgImpl_h