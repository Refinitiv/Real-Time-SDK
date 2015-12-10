/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_FilterListDecoder_h
#define __thomsonreuters_ema_access_FilterListDecoder_h

#include "FilterEntry.h"
#include "Decoder.h"
#include "OmmError.h"
#include "NoDataImpl.h"
#include "EmaBufferInt.h"
#include "EmaPool.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class FilterListDecoder : public Decoder
{
public :

	FilterListDecoder();

	virtual ~FilterListDecoder();

	void setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );

	void setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	void setRsslData( RsslDecodeIterator* , RsslBuffer* );

	bool getNextData();

	bool getNextData( UInt8 );

	void reset();

	bool hasTotalCountHint() const; 

	UInt32 getTotalCountHint() const;

	bool hasPermissionData() const;

	bool hasEntryPermissionData() const;

	const EmaBuffer& getEntryPermissionData();

	UInt8 getFilterId() const;

	const Data& getLoad() const;

	const EmaBuffer& getHexBuffer();

	FilterEntry::FilterAction getAction() const;

	void clone( const FilterListDecoder& );

	bool decodingStarted() const;

	void setAtExit();

private :

	RsslFilterList				_rsslFilterList;

	mutable RsslBuffer			_rsslFilterListBuffer;
		
	RsslFilterEntry				_rsslFilterEntry;

	RsslDecodeIterator			_decodeIter;

	EmaBufferInt				_permissionData;

	NoDataImpl					_load;

	EmaBufferInt				_hexBuffer;

	const RsslDataDictionary*	_pRsslDictionary;

	UInt8						_rsslMajVer;

	UInt8						_rsslMinVer;

	OmmError::ErrorCode			_errorCode;

	bool						_decodingStarted;

	bool						_atEnd;
};

class FilterListDecoderPool : public DecoderPool< FilterListDecoder >
{
public :

	FilterListDecoderPool( unsigned int size = 5 ) : DecoderPool< FilterListDecoder >( size ) {};

	virtual ~FilterListDecoderPool() {}

private :

	FilterListDecoderPool( const FilterListDecoderPool& );
	FilterListDecoderPool& operator=( const FilterListDecoderPool& );
};

}

}

}

#endif // __thomsonreuters_ema_access_FilterListDecoder_h
