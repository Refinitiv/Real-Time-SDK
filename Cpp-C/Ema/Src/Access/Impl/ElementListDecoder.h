/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_ElementListDecoder_h
#define __refinitiv_ema_access_ElementListDecoder_h

#include "Decoder.h"
#include "EmaStringInt.h"
#include "EmaBufferInt.h"
#include "EmaVector.h"
#include "EmaPool.h"
#include "OmmError.h"
#include "NoDataImpl.h"
#include "rtr/rsslSetData.h"

namespace refinitiv {
	
namespace ema {

namespace access {

class ElementListDecoder : public Decoder
{
public :

	ElementListDecoder();

	virtual ~ElementListDecoder();

	bool setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );

	bool setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	bool setRsslData( RsslDecodeIterator* , RsslBuffer* );

	bool getNextData();

	bool getNextData( const EmaString& );

	bool getNextData( const Data& );

	void reset();

	const EmaString& getName();

	const Data& getLoad() const;

	Data** getLoadPtr();

	const EmaBuffer& getHexBuffer();
	
	void clone( const ElementListDecoder& );

	bool hasInfo() const; 

	Int16 getInfoElementListNum() const;

	bool decodingStarted() const;

	const RsslBuffer& getRsslBuffer() const;

	void setAtExit();

	OmmError::ErrorCode getErrorCode() const;

private :

	bool getNextData( const EmaVector< EmaString >& );

	void decodeViewList( RsslBuffer* , RsslDataType& , EmaVector< EmaString >& );

	RsslElementList				_rsslElementList;

	mutable RsslBuffer			_rsslElementListBuffer;
		
	RsslElementEntry			_rsslElementEntry;

	RsslDecodeIterator			_decodeIter;

	Data**						_pLoadPool;

	Data*						_pLoad;

	const RsslDataDictionary*	_pRsslDictionary;

	RsslLocalElementSetDefDb*	_rsslLocalELSetDefDb;

	EmaStringInt				_name;

	EmaBufferInt				_hexBuffer;

	UInt8						_rsslMajVer;

	UInt8						_rsslMinVer;

	OmmError::ErrorCode			_errorCode;

	bool						_decodingStarted;

	bool						_atEnd;
};

class ElementListDecoderPool : public DecoderPool< ElementListDecoder >
{
public :

	ElementListDecoderPool( unsigned int size = 5 ) : DecoderPool< ElementListDecoder >( size ) {};

	virtual ~ElementListDecoderPool() {}

private :

	ElementListDecoderPool( const ElementListDecoderPool& );
	ElementListDecoderPool& operator=( const ElementListDecoderPool& );
};

}

}

}

#endif // __refinitiv_ema_access_ElementListDecoder_h
