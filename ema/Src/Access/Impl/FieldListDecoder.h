/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_FieldListDecoder_h
#define __thomsonreuters_ema_access_FieldListDecoder_h

#include "Decoder.h"
#include "EmaStringInt.h"
#include "EmaBufferInt.h"
#include "EmaVector.h"
#include "EmaPool.h"
#include "rtr/rsslSetData.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class FieldListDecoder : public Decoder
{
public :

	FieldListDecoder();

	virtual ~FieldListDecoder();

	void setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );

	void setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	void setRsslData( RsslDecodeIterator* , RsslBuffer* );

	bool getNextData();

	bool getNextData( Int16 );

	bool getNextData( const EmaString& );

	bool getNextData( const Data& );

	void reset();

	const EmaString& getName();

	Int16 getFieldId() const;

	Int16 getRippleTo( Int16 fieldId = 0 ) const;

	const Data& getLoad() const;

	Data** getLoadPtr();

	const EmaBuffer& getHexBuffer();
	
	void clone( const FieldListDecoder& );

	bool hasInfo() const; 

	Int16 getInfoFieldListNum() const;

	Int16 getInfoDictionaryId() const; 

	bool decodingStarted() const;

	void setAtExit();

private :

	bool getNextData( const EmaVector< Int16 >& );

	bool getNextData( const EmaVector< EmaString >& );

	void decodeViewList( RsslBuffer* , RsslDataType& , EmaVector< Int16 >& , EmaVector< EmaString >& );

	RsslFieldList				_rsslFieldList;

	mutable RsslBuffer			_rsslFieldListBuffer;
		
	RsslFieldEntry				_rsslFieldEntry;

	RsslDecodeIterator			_decodeIter;

	Data**						_pLoadPool;

	Data*						_pLoad;

	const RsslDataDictionary*	_pRsslDictionary;

	const RsslDictionaryEntry*	_rsslDictionaryEntry;

	RsslLocalFieldSetDefDb*		_rsslLocalFLSetDefDb;

	EmaStringInt				_name;

	EmaBufferInt				_hexBuffer;

	UInt8						_rsslMajVer;

	UInt8						_rsslMinVer;

	OmmError::ErrorCode			_errorCode;

	bool						_decodingStarted;

	bool						_atEnd;
};

class FieldListDecoderPool : public DecoderPool< FieldListDecoder >
{
public :

	FieldListDecoderPool( unsigned int size = 5 ) : DecoderPool< FieldListDecoder >( size ) {};

	virtual ~FieldListDecoderPool() {}

private :

	FieldListDecoderPool( const FieldListDecoderPool& );
	FieldListDecoderPool& operator=( const FieldListDecoderPool& );
};

}

}

}

#endif // __thomsonreuters_ema_access_FieldListDecoder_h
