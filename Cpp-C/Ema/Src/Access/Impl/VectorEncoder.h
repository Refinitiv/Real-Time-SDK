/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_VectorEncoder_h
#define __refinitiv_ema_access_VectorEncoder_h

#include "Encoder.h"
#include "ComplexType.h"
#include "EmaPool.h"
#include "VectorEntry.h"
#include "rtr/rsslVector.h"

namespace rtsdk {

namespace ema {

namespace access {

class OmmArray;
class ElementList;
class FieldList;
class FilterList;
class Map;
class Series;
class Vector;
class AckMsg;
class GenericMsg;
class PostMsg;
class ReqMsg;
class RespMsg;
class OmmAnsiPage;
class OmmOpaque;
class OmmXml;

class VectorEncoder : public Encoder
{
public :

	VectorEncoder();

	virtual ~VectorEncoder();

	void clear();

	void totalCountHint( UInt32 totalCountHint );

	void summaryData( const ComplexType& data );

	void sortable( bool sortable );

	void add( UInt32 position, VectorEntry::VectorAction action, 
				const ComplexType& complexType, const EmaBuffer& permission );

	void add( UInt32 position, VectorEntry::VectorAction action,
		const EmaBuffer& permission );

	void complete();

private :

	void initEncode( UInt8 rsslDataType, DataType::DataTypeEnum );

	void addEncodedEntry( UInt32 position, UInt8 action, const EmaBuffer& permission, const char* , const RsslBuffer& );

	void startEncodingEntry( UInt32 position, UInt8 action, const EmaBuffer& permission, const char* );

	void endEncodingEntry() const;

	RsslVector				_rsslVector;

	RsslVectorEntry			_rsslVectorEntry;

	DataType::DataTypeEnum	_emaDataType;

	bool					_containerInitialized;
};


class VectorEncoderPool : public EncoderPool< VectorEncoder >
{
public :

	VectorEncoderPool( unsigned int size = 5 ) : EncoderPool< VectorEncoder >( size ) {};

	virtual ~VectorEncoderPool() {}

private :

	VectorEncoderPool( const VectorEncoderPool& );
	VectorEncoderPool& operator=( const VectorEncoderPool& );
};

}

}

}

#endif // __refinitiv_ema_access_VectorEncoder_h
