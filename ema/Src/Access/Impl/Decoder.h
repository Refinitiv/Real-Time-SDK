/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_Decoder_h
#define __thomsonreuters_ema_access_Decoder_h

#include "OmmError.h"
#include "rtr/rsslMsg.h"
#include "rtr/rsslDataDictionary.h"

namespace thomsonreuters {
	
namespace ema {

namespace access {

class Encoder;

class Decoder
{
public :

	// this method is used by messages only (e.g. refresh, request, etc)
	//
	// UInt8 -> major rwf version
	// UInt8 -> minor rwf version
	// RsslMsg -> RsslMsg containing rssl decoded message
	// RsslDataDictionary -> dictionary used for FieldList decoding
	// todo ... make sure that the RsslDataDictionary is able to pass dictionary family
	virtual void setRsslData( UInt8 majVer, UInt8 minVer, RsslMsg* , const RsslDataDictionary* ) = 0;

	// this method is used by iter-able containers only (e.g. FieldList, Map, etc)
	//
	// UInt8 -> major rwf version
	// UInt8 -> minor rwf version
	// RsslBuffer -> buffer containing actual wire data
	// RsslDataDictionary -> dictionary used for FieldList decoding
	// void -> local set defined data
	// todo ... make sure that the RsslDataDictionary is able to pass dictionary family
	virtual void setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* , const RsslDataDictionary* , void* ) = 0;

	// this method is used by primitive data type (not iter-able data types) plus Array
	// RsslDecodeIterator -> current decode iterator in which this primitive is contained
	// RsslBuffer -> buffer containing actual wire data
	virtual void setRsslData( RsslDecodeIterator* , RsslBuffer* ) = 0;

	virtual void setServiceName( const char* , UInt32 , bool nullTerm = true ) {}

	virtual const Data* getSummaryData() const { return 0; }

protected :

	// Data* -> points to Data class object being morphed
	// RsslDataType -> rssl data type to morph the above Data class object into
	// UInt8 -> major rwf version
	// UInt8 -> minor rwf version
	// Rsslbuffer -> buffer containing actual wire format data whose type is RsslDataType
	// RsslDataDictionary -> dictionary used for decoding of passed in FieldList
	// void -> local set defined data
	// todo ... make sure that the RsslDataDictionary is able to pass dictionary family
	void setRsslData( Data* , RsslDataType rsslType, RsslDecodeIterator* , RsslBuffer* , const RsslDataDictionary* , void* localDb ) const;

	// Data* -> points to Data class object being morphed
	// OmmError::ErrorCode -> error code
	// UInt8 -> major rwf version
	// UInt8 -> minor rwf version
	// Rsslbuffer -> buffer containing actual wire format data
	Data* setRsslData( Data* , OmmError::ErrorCode , RsslDecodeIterator* , RsslBuffer* ) const;

	void create( Data* , DataType::DataTypeEnum ) const;

	void createLoadPool( Data**& );

	void destroyLoadPool( Data**& );

	Data* setRsslData( Data** , RsslDataType rsslType, RsslDecodeIterator* , RsslBuffer* , const RsslDataDictionary* , void* localDb ) const;
};

}

}

}

#endif // __thomsonreuters_ema_access_Decoder_h
