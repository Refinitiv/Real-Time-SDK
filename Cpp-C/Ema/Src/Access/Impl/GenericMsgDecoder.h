/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.      --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_GenericMsgDecoder_h
#define __refinitiv_ema_access_GenericMsgDecoder_h

#include "EmaPool.h"
#include "MsgDecoder.h"
#include "EmaStringInt.h"
#include "EmaBufferInt.h"

namespace refinitiv {

namespace ema {

namespace access {

class GenericMsgDecoder : public MsgDecoder
{
public :

	GenericMsgDecoder();

	~GenericMsgDecoder();

	bool setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );

	bool setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	bool setRsslData( RsslDecodeIterator* , RsslBuffer* );

	bool hasMsgKey() const;

	bool hasName() const;

	bool hasNameType() const;

	bool hasServiceId() const;

	bool hasId() const;

	bool hasFilter() const;

	bool hasAttrib() const;

	bool hasPayload() const;

	bool hasExtendedHeader() const;

	bool hasSeqNum() const;

	bool hasSecondarySeqNum() const;

	bool hasPartNum() const;

	bool hasPermissionData() const;

	Int32 getStreamId() const;

	UInt16 getDomainType() const;

	const EmaString& getName() const;

	UInt8 getNameType() const;

	UInt32 getServiceId() const;

	Int32 getId() const;

	UInt32 getFilter() const;

	UInt32 getSeqNum() const;

	UInt32 getSecondarySeqNum() const;

	UInt16 getPartNum() const;

	bool getComplete() const;

	const EmaBuffer& getPermissionData() const;

	const EmaBuffer& getExtendedHeader() const;

	const EmaBuffer& getHexBuffer() const;

	const RsslBuffer& getRsslBuffer() const;

	OmmError::ErrorCode getErrorCode() const;

private :

	mutable EmaStringInt	_name;

	mutable EmaBufferInt	_extHeader;

	mutable EmaBufferInt	_permission;

	mutable EmaBufferInt	_hexBuffer;

	OmmError::ErrorCode		_errorCode;

	friend class GenericMsg;
};

class GenericMsgDecoderPool : public DecoderPool< GenericMsgDecoder >
{
public :

	GenericMsgDecoderPool( unsigned int size = 5 ) : DecoderPool< GenericMsgDecoder >( size ) {};

	~GenericMsgDecoderPool() {}

private :

	GenericMsgDecoderPool( const GenericMsgDecoderPool& );
	GenericMsgDecoderPool& operator=( const GenericMsgDecoderPool& );
};

}

}

}

#endif // __refinitiv_ema_access_GenericMsgDecoder_h
