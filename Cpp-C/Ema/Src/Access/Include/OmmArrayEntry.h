/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_access_OmmArrayEntry_h
#define __rtsdk_ema_access_OmmArrayEntry_h

/**
	@class rtsdk::ema::access::OmmArrayEntry OmmArrayEntry.h "Access/Include/OmmArrayEntry.h"
	@brief OmmArrayEntry represents an entry of OmmArray.

	OmmArrayEntry associates entry's data and its data type.

	\code

	decodeArray( const OmmArray& array )
	{
		while ( array.forth() )
		{
			const ArrayEntry& aEntry = array.getEntry();

			if ( aEntry.getCode() != Data::BlankEnum )
				switch ( aEntry.getLoadType() )
				{
				case DataType::IntEnum :
					aEntry.getInt();
					break;

				}
		}
	}

	\endcode

	\remark Objects of this class are intended to be short lived or rather transitional.
			This class is designed to efficiently perform extracting of data from entry.
			Objects of this class are not cache-able.

	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		OmmArray,
		EmaString,
		EmaBuffer,
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

class OmmArrayDecoder;

class EMA_ACCESS_API OmmArrayEntry
{
public :

	///@name Accessors
	//@{
	/** Returns the DataType of the entry's load.
		\remark return of DataType::OmmErrorEnum signifies error while extracting content of load
		@return data type of the contained object
	*/
	DataType::DataTypeEnum getLoadType() const;

	/** Returns the Code of the entry's load. The code indicates a special state of a Data.
		\remark Attempts to extract data will cause OmmInvalidUsageException if Data::BlankEnum is returned.
		@return data code of the contained object
	*/
	Data::DataCode getCode() const;
		
	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EmaString& toString() const;

	/** Returns the contained Data based on the DataType.
		@return Data class reference to contained object
	*/
	const Data& getLoad() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmInt
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return Int64
	*/
	Int64 getInt() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmUInt
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return UInt64
	*/
	UInt64 getUInt() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmReal
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return OmmReal class reference to contained object
	*/
	const OmmReal& getReal() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmFloat
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return float
	*/
	float getFloat() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmDouble
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return double
	*/
	double getDouble() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmData
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return OmmDate class reference to contained object
	*/
	const OmmDate& getDate() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmTime
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return ommTime class reference to contained object
	*/
	const OmmTime& getTime() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmDateTime
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return OmmDateTime class reference to contained object
	*/
	const OmmDateTime& getDateTime() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmQos
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return OmmQos class reference to contained object
	*/
	const OmmQos& getQos() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmState
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return OmmState class reference to contained object
	*/
	const OmmState& getState() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmEnum
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return UInt16
	*/
	UInt16 getEnum() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmBuffer
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return EmaBuffer
	*/
	const EmaBuffer& getBuffer() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmAscii
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return EmaString
	*/
	const EmaString& getAscii() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmUtf8
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return EmaBuffer
	*/
	const EmaBuffer& getUtf8() const;

	/** Returns the current OMM data represented as a specific simple type.
		@throw OmmInvalidUsageException if contained object is not OmmRmtes
		@throw OmmInvalidUsageException if getCode() returns Data::BlankEnum
		@return RmtesBuffer
	*/
	const RmtesBuffer& getRmtes() const;

	/** Returns Error.
		@throw OmmInvalidUsageException if contained object is not OmmError
		@return OmmError class reference to contained object
	*/
	const OmmError& getError() const;
	//@}

private :

	friend class OmmArray;

	OmmArrayDecoder*		_pDecoder;
	const Data*				_pLoad;

	mutable EmaString		_toString;

	OmmArrayEntry();
	virtual ~OmmArrayEntry();

	OmmArrayEntry( const OmmArrayEntry& );
	OmmArrayEntry& operator=( const OmmArrayEntry& );
};

}

}

}

#endif //__rtsdk_ema_access_OmmArrayEntry_h
