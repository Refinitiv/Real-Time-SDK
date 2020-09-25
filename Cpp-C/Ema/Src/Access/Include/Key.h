/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_key_h
#define __refinitiv_ema_access_key_h

/**
	@class rtsdk::ema::access::Key Key.h "Access/Include/Key.h"
	@brief Key conveys MapEntry key information.

	Key contains objects of primitive type (e.g. they are not complex type)

	The following code snippet shows extracting of Key and its content while processing MapEntry.

	\code

	void decodeMap( const Map& map )
	{
		...

		while ( map.forth() )
		{
			const MapEntry& mEntry = map.getEntry();

			const Key& key = mEntry.getKey();

			switch ( key.getDataType() )
			{
			case DataType::OmmBufferEnum :
				const EmaBuffer& keyBuffer = key.getBuffer();
				break;
			case DataType::OmmAsciiEnum :
				const EmaString& keyString = key.getAscii();
				break;
			}
		}

		...
	}

	\endcode

	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform extracting of Key and its content.
	\remark Objects of this class are not cache-able.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data
		OmmInt,
		OmmUInt,
		OmmAscii,
		OmmBuffer,
		OmmRmtes,
		OmmUtf8,
		OmmEnum,
		OmmReal,
		OmmDate,
		OmmTime,
		OmmDateTime,
		OmmQos,
		OmmState,
		OmmError
*/

#include "Access/Include/Data.h"

namespace rtsdk {

namespace ema {

namespace access {

class OmmReal;
class OmmDate;
class OmmTime;
class OmmDateTime;
class OmmQos;
class OmmState;
class OmmError;
class RmtesBuffer;

class EMA_ACCESS_API Key
{
public :

	///@name Accessors
	//@{
	/** Returns the DataType of the contained data.
		\remark return of DataType::OmmErrorEnum signifies error while extracting content of Key
		@return data type of the contained object
	*/ 
	DataType::DataTypeEnum getDataType() const;

	/** Returns the simple type based on the DataType.
		@return Data class reference to the contained object
	*/ 
	const Data& getData() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmInt
		@return Int64
	*/ 
	Int64 getInt() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmUInt
		@return UInt64
	*/ 
	UInt64 getUInt() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmReal
		@return OmmReal class reference to the contained object
	*/ 
	const OmmReal& getReal() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmFloat
		@return float
	*/ 
	float getFloat() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmDouble
		@return double
	*/ 
	double getDouble() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmDate
		@return OmmDate class reference to the contained object
	*/ 
	const OmmDate& getDate() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmTime
		@return OmmTime class reference to the contained object
	*/ 
	const OmmTime& getTime() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmDateTime
		@return OmmDateTime class reference to the contained object
	*/ 
	const OmmDateTime& getDateTime() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmQos
		@return OmmQos class reference to the contained object
	*/ 
	const OmmQos& getQos() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmState
		@return OmmState class reference to the contained object
	*/ 
	const OmmState& getState() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmEnum
		@return UInt16
	*/ 
	UInt16 getEnum() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmBuffer
		@return EmaBuffer class reference to the contained object
	*/ 
	const EmaBuffer& getBuffer() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmAscii
		@return EmaString class reference to the contained object
	*/ 
	const EmaString& getAscii() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmUtf8
		@return EmaBuffer class reference to the contained object
	*/ 
	const EmaBuffer& getUtf8() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmRmtes
		@return RmtesBuffer class reference to the contained object
	*/ 
	const RmtesBuffer& getRmtes() const;

	/** Returns Error.
		@throw OmmInvalidUsageException if contained object is not OmmError
		@return OmmError class reference to contained object
	*/
	const OmmError& getError() const;
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~Key();
	//@}

private :

	friend class Map;
	friend class MapEntry;

	const Data*			_pData;

	Key();
	Key( const Key& );
	Key& operator=( const Key& );
};

}

}

}

#endif // __refinitiv_ema_access_key_h
