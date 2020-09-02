/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_VectorDecoder_h
#define __thomsonreuters_ema_access_VectorDecoder_h

#include "Decoder.h"
#include "VectorEntry.h"
#include "ElementListSetDef.h"
#include "FieldListSetDef.h"
#include "NoDataImpl.h"
#include "EmaBufferInt.h"

namespace rtsdk {

namespace ema {

namespace access {

class VectorDecoder : public Decoder
{
public :

	VectorDecoder();

	virtual ~VectorDecoder();

	bool setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );

	bool setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	bool setRsslData( RsslDecodeIterator* , RsslBuffer* );

	bool getNextData();

	void reset();

	const Data& getLoad() const;

	const Data* getSummaryData() const;

	bool hasSummary() const;

	bool hasTotalCountHint() const; 

	UInt32 getTotalCountHint() const;

	bool hasPermissionData() const;

	bool hasEntryPermissionData() const;

	const EmaBuffer& getEntryPermissionData();

	const EmaBuffer& getHexBuffer();

	bool getSortable() const;

	UInt32 getEntryIndex() const;

	VectorEntry::VectorAction getAction() const;

	void clone( const VectorDecoder& );

	bool decodingStarted() const;

	const RsslBuffer& getRsslBuffer() const;

	void setAtExit();

	OmmError::ErrorCode getErrorCode() const;

private :

	RsslVector					_rsslVector;

	mutable RsslBuffer			_rsslVectorBuffer;
		
	RsslVectorEntry				_rsslVectorEntry;

	RsslDecodeIterator			_decodeIter;

	EmaBufferInt				_permissionData;

	NoDataImpl					_summary;

	NoDataImpl					_load;

	EmaBufferInt				_hexBuffer;

	const RsslDataDictionary*	_pRsslDictionary;

	ElementListSetDef*			_elementListSetDef;

	FieldListSetDef*			_fieldListSetDef;

	void*						_localSetDefDb;

	UInt8						_rsslMajVer;

	UInt8						_rsslMinVer;

	OmmError::ErrorCode			_errorCode;

	bool						_decodingStarted;

	bool						_atEnd;

	bool						_atExit;
};

class VectorDecoderPool : public DecoderPool< VectorDecoder >
{
public :

	VectorDecoderPool( unsigned int size = 5 ) : DecoderPool< VectorDecoder >( size ) {};

	virtual ~VectorDecoderPool() {}

private :

	VectorDecoderPool( const VectorDecoderPool& );
	VectorDecoderPool& operator=( const VectorDecoderPool& );
};

}

}

}

#endif // __thomsonreuters_ema_access_VectorDecoder_h
