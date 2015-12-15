/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_VectorEncoder_h
#define __thomsonreuters_ema_access_VectorEncoder_h

#include "Encoder.h"
#include "ComplexType.h"
#include "EmaPool.h"
#include "VectorEntry.h"
#include "rtr/rsslVector.h"

namespace thomsonreuters {

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

	void add( UInt32 position, VectorEntry::VectorAction action, 
				const ComplexType& complexType, const EmaBuffer& permission );

	void complete();

private :

	void initEncode( UInt8 dataType );

	void addEncodedEntry( UInt32 position, UInt8 action, const EmaBuffer& permission, const char* , RsslBuffer& );

	void startEncodingEntry( UInt32 position, UInt8 action, const EmaBuffer& permission, const char* );

	void endEncodingEntry() const;

	RsslVector				_rsslVector;

	RsslVectorEntry			_rsslVectorEntry;

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

#endif // __thomsonreuters_ema_access_VectorEncoder_h
