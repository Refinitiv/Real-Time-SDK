/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_StaticDecoder_h
#define __refinitiv_ema_access_StaticDecoder_h

#include "Data.h"
#include "OmmError.h"
#include "rtr/rsslMsg.h"
#include "rtr/rsslDataDictionary.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmQos;
class OmmState;
class RefreshMsg;
class StatusMsg;
class UpdateMsg;

class StaticDecoder
{
public :

	// entry point to decode any received message class
	//
	// Data*			-> Data class object to be morphed into a message class object
	// RsslMsg*			-> decoded RsslMsg (contains message class)
	// majVer / minVer	-> wire format version number
	// RsslDataDictionary* -> field dictionary used to decode FieldList
	static void setRsslData( Data* , RsslMsg* , UInt8 majVer, UInt8 minVer, const RsslDataDictionary* );

	// entry point to decode all containers
	//
	// Data*			-> Data class object to be morphed into message or container
	// RsslBuffer*		-> decoded RsslBuffer
	// RsslDataType		-> type of container pointed by the above RsslBuffer
	// majVer / minVer	-> wire format version number
	// RsslDataDictionary* -> field dictionary used to decode FieldList
	static void setRsslData( Data* , RsslBuffer* , RsslDataType , UInt8 majVer, UInt8 minVer, const RsslDataDictionary* );

	// Data* -> points to Data class object being morphed
	// OmmError::ErrorCode -> error code
	// UInt8 -> major rwf version
	// UInt8 -> minor rwf version
	// Rsslbuffer -> buffer containing actual wire format data
	static void setRsslData( Data* , OmmError::ErrorCode , UInt8 majVer, UInt8 minVer, RsslBuffer* );

	// helper method to convert decoded RsslQos struct into Qos object
	static void setRsslData( OmmQos* , RsslQos* );

	// helper method to convert decoded RsslState struct into State object
	static void setRsslData( OmmState* , RsslState* );

	// helper method allowing decoding of just encoded container
	static void setData( Data* pData, const RsslDataDictionary* );

	// helper utilities
	static void morph( Data* , DataType::DataTypeEnum );

	static void create( Data* , DataType::DataTypeEnum );
};

}

}

}

#endif // __refinitiv_ema_access_StaticDecoder_h
