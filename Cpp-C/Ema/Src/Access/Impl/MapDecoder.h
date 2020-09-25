/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_MapDecoder_h
#define __refinitiv_ema_access_MapDecoder_h

#include "Decoder.h"
#include "MapEntry.h"
#include "ElementListSetDef.h"
#include "FieldListSetDef.h"
#include "NoDataImpl.h"
#include "EmaBufferInt.h"

namespace rtsdk {

namespace ema {

namespace access {

class MapDecoder : public Decoder
{
public :

	MapDecoder();

	virtual ~MapDecoder();

	bool setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );

	bool setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	bool setRsslData( RsslDecodeIterator* , RsslBuffer* );

	bool getNextData();

	void reset();

	const Data& getLoad() const;

	const Data& getKeyData() const;

	const Data* getSummaryData() const;

	bool hasSummary() const;

	bool hasKeyFieldId() const; 

	Int16 getKeyFieldId() const;

	bool hasTotalCountHint() const; 

	bool hasPermissionData() const;

	bool hasEntryPermissionData() const;

	const EmaBuffer& getEntryPermissionData();

	UInt32 getTotalCountHint() const;

	MapEntry::MapAction getAction() const;

	const EmaBuffer& getHexBuffer();

	void clone( const MapDecoder& );

	bool decodingStarted() const;

	const RsslBuffer& getRsslBuffer() const;

	void setAtExit();

	OmmError::ErrorCode getErrorCode() const;

private :

	RsslMap						_rsslMap;

	mutable RsslBuffer			_rsslMapBuffer;
		
	RsslMapEntry				_rsslMapEntry;

	RsslDecodeIterator			_decodeIter;

	EmaBufferInt				_permissionData;

	NoDataImpl					_summary;

	NoDataImpl					_load;

	NoDataImpl					_key;

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

class MapDecoderPool : public DecoderPool< MapDecoder >
{
public :

	MapDecoderPool( unsigned int size = 5 ) : DecoderPool< MapDecoder >( size ) {};

	virtual ~MapDecoderPool() {}

private :

	MapDecoderPool( const MapDecoderPool& );
	MapDecoderPool& operator=( const MapDecoderPool& );
};

}

}

}

#endif // __refinitiv_ema_access_MapDecoder_h
