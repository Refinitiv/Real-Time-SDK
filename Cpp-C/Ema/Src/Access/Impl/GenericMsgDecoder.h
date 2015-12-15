/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_GenericMsgDecoder_h
#define __thomsonreuters_ema_access_GenericMsgDecoder_h

#include "EmaPool.h"
#include "MsgDecoder.h"
#include "EmaStringInt.h"
#include "EmaBufferInt.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class GenericMsgDecoder : public MsgDecoder
{
public :

	GenericMsgDecoder();

	~GenericMsgDecoder();

	void setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );

	void setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	void setRsslData( RsslDecodeIterator* , RsslBuffer* );

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

private :

	RsslMsg					_rsslMsg;

	RsslMsg*				_pRsslMsg;

	mutable EmaStringInt	_name;

	mutable EmaBufferInt	_extHeader;

	mutable EmaBufferInt	_permission;

	mutable EmaBufferInt	_hexBuffer;

	UInt8					_rsslMajVer;

	UInt8					_rsslMinVer;

	mutable OmmError::ErrorCode	_errorCode;
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

#endif // __thomsonreuters_ema_access_GenericMsgDecoder_h
