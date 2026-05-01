/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_PackedMsgImpl_h
#define __refinitiv_ema_access_PackedMsgImpl_h

#include "PackedMsg.h"
#include "EmaVector.h"
#include "ExceptionTranslator.h"
#include "OmmIProviderImpl.h"
#include "OmmNiProviderImpl.h"

namespace refinitiv {

namespace ema {

namespace access {

class NiProvSessionTransportBuffer
{
public:
	NiProvSessionTransportBuffer();
	virtual ~NiProvSessionTransportBuffer();


	void releaseBuffer();			// Releases the buffer and sets pBuffer to NULL
	void clear();					// Sets all elements to NULL.  This does not release the buffer.

	RsslBuffer* pBuffer;									// Buffer recieved from rsslReactorGetBuffer
	NiProviderRoutingSessionChannel* pSessionChannel;		// This is the transport buffer's assocaited session channel
	RsslChannel* pRsslChannel;								// This stores the pointer to the RsslChannel that pBuffer comes from

};

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
	EmaVector<NiProvSessionTransportBuffer*>& getRwfSessionBufferList();

	void reset();	// sets everything to NULL, clears out everything.  This is assumed to be used after the buffers have been sent and all elements in _rwfSessionBufferList have been individually cleared
private:
	UInt32 _remainingSize;
	UInt32 _packedMsgCount;
	UInt32 _maxSize;
	UInt32 _allocatedSize;				// This is used for the routing session _encodeBuffer length.
	UInt64 _clientHandle;
	UInt64 _itemHandle;
	RsslBuffer* _packedBuf;
	OmmProvider* _ommProvider;
	RsslReactorChannel* _reactorChannel;
	OmmIProviderImpl* _ommIProviderImpl;
	OmmNiProviderImpl* _ommNiProviderImpl;
	RsslEncodeIterator _eIter;
	EmaVector<NiProvSessionTransportBuffer*> _rwfSessionBufferList;	// This contains the transport buffers from the routing session rsslChannels.
	RsslBuffer			_encodeBuffer;							// This is the temporary encoding buffer for routing sessions.  This is allocated if _maxSize is larger than _allocatedSize(starts at 0), and will not be free'd in clear()

	bool _initialized;

};

}
}
}

#endif //__refinitiv_ema_access_PackedMsgImpl_h
