/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_access_OmmArray_h
#define __rtsdk_ema_access_OmmArray_h

/**
	@class rtsdk::ema::access::OmmArray OmmArray.h "Access/Include/OmmArray.h"
	@brief OmmArray is a homogeneous container of primitive data type entries.

	The following code snippet shows addition of primitive data type entries to OmmArray.

	\code

	OmmArray array;

	array.fixedWidth( 8 ).addInt( -16 ).addInt( 28 ).addInt( -35 ).complete();

	\endcode

	The following code snippet shows getting data from OmmArray.

	\code

	const ElementEntry& eEntry = eList.getEntry();

	if ( eEntry.getLoadType() == DataType::OmmArrayEnum &&
		eEntry.getCode() != Data::BlankEnum )
	{
		const OmmArray& array = eList.getArray();

		while ( array.forth() )
		{
			const OmmArrayEntry& aEntry = array.getEntry();
			switch ( aEntry.getLoadType() )
			{
				case DataType::IntEnum :
					Int64 value = aEntry.getInt();
					break;
				case DataType::RealEnum :
					const OmmReal& real = aEntry.getReal();
					break;

				...
			}
		}
	}

	\endcode

	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform setting and extracting of OmmArray and its content.
	\remark Objects of this class are not cache-able.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		OmmArrayEntry,
		OmmReal,
		OmmState,
		OmmQos,
		EmaString,
		EmaBuffer
*/

#include "Access/Include/Data.h"
#include "Access/Include/OmmReal.h"
#include "Access/Include/OmmState.h"
#include "Access/Include/OmmQos.h"
#include "Access/Include/OmmArrayEntry.h"

namespace rtsdk {

namespace ema {

namespace access {

class OmmArrayDecoder;
class OmmArrayEncoder;

class EMA_ACCESS_API OmmArray : public Data
{
public :

	///@name Constructor
	//@{
	/** Constructs OmmArray
	*/
	OmmArray();
	//@}

	///@name Destructor
	//@{
	/** Destructor
	*/
	virtual ~OmmArray();
	//@}

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::OmmArrayEnum
	*/
	DataType::DataTypeEnum getDataType() const;

	/** Returns the Code, which indicates a special state of a DataType.
		@return Data::BlankEnum if OmmArray is blank; Data::NoCodeEnum otherwise
	*/
	Data::DataCode getCode() const;

	/** Returns a buffer that in turn provides an alphanumeric null-terminated hexadecimal string representation.
		@return EmaBuffer with the message hex information
	*/
	const EmaBuffer& getAsHex() const;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EmaString& toString() const;

	/** Indicates presence of FixedWidth.
		@return true if fixed width is set; false otherwise
	*/
	bool hasFixedWidth() const;

	/** Returns FixedWidth.
		@return fixed width
	*/
	UInt16 getFixedWidth() const;

	/** Iterates through a list of Data of any DataType. Typical usage is
		to extract the entry during each iteration via getEntry().
		@return false at the end of Ommarray; true otherwise
	*/
	bool forth() const;

	/** Returns Entry.
		@throw OmmInvalidUsageException if forth() was not called first
		@return OmmArrayEntry
	*/
	const OmmArrayEntry& getEntry() const;

	/** Resets iteration to start of container.
	*/
	void reset() const; 
	//@}

	///@name Operations
	//@{
	/** Clears the OmmArray
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	OmmArray& clear();

	/** Specifies FixedWidth.
		Setting fixed-width implies each array entry has been populated with the same fixed size,
		to allow minimizing bandwidth. 
		Setting fixed-width as 0 implies the data contained in the array entries may be of different sizes,
		to allow the flexibility of different sized encoding;

		When using a fixed width, the application still passes in the base primitive type when
		encoding (e.g., if encoding fixed width DataType::IntEnum types, an Int is passed in
		regardless of fixed width). When encoding buffer types as fixed width:
		Any content that exceeds fixedWidth will be truncated.
		Any content that is shorter than fixedWidth will be padded with the \0 (NULL) character.

		Only specific types are allowed as fixed-width encodings. Here lists supported fixed widths and allowable data ranges: 
		DataType::IntEnum supports one byte(-2^7 to 2^7-1), two byte(-2^15 to 2^15-1), four byte((-2^31 to 2^31-1), or eight byte((-2^63 to 2^63-1).
		DataType::UIntEnum supports one byte(0 to 2^8-1), two byte(0 to 2^16-1), four byte(0 to 2^32-1), or eight byte(0 to 2^64-1).
        DataType::FloatEnum supports four byte, floating point type that represents the same range of values allowed by the system float type. Follows the IEEE 754 specification.
		DataType::DoubleEnum supports eight byte, floating point type that represents the same range of values allowed by the system double type. Follows the IEEE 754 specification.
        DataType::DateEnum supports four byte(year, month, day).
		DataType::TimeEnum supports:
			three byte(hour, minute, second);
			five byte(hour, minute, second, millisec);
			seven byte(hour, minute, second, millisec, microsec);
			eight byte(hour, minute, second, millisec, microsec, nanosec). 
		DataType::DateTimeEnum supports:
			seven byte(year, month, day, hour, minute, second);
			nine byte(year, month, day, hour, minute, second, millisec);
			eleven byte(year, month, day, hour, minute, second, millisec, microsec);
			twelve byte(year, month, day, hour, minute, second, millisec, microsec, nanosec);
		DataType::EnumEnum supports one byte(0 to 2^8-1) or two byte(0 to 2^16-1).
		DataType::BufferEnum, DataType::AsciiEnum, DataType::Utf8Enum, and
		DataType::RmtesEnum support any legal width value.
		 
		@throw OmmInvalidUsageException if an entry was already added
		@param[in] width specifies fixed width value
		@return reference to this object
	*/
	OmmArray& fixedWidth( UInt16 width );

	/** Adds a specific simple type of OMM data to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@param[in] value specifies added Int64
		@return reference to this object
	*/
	OmmArray& addInt( Int64 value );

	/** Adds a specific simple type of OMM data to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@param[in] value specifies added UInt64
		@return reference to this object
	*/
	OmmArray& addUInt( UInt64 value );

	/** Adds a specific simple type of OMM data to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@param[in] mantissa added OmmReal::mantissa
		@param[in] magnitudeType added OmmReal::magnitudeType
		@return reference to this object
	*/
	OmmArray& addReal( Int64 mantissa, OmmReal::MagnitudeType magnitudeType );

	/** Adds a specific simple type of OMM data to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@param[in] value added double to be converted to OmmReal
		@param[in] magnitudeType OmmReal::magnitudeType used for the conversion (default value is OmmReal::Exponent0Enum)
		@return reference to this object
	*/
	OmmArray& addRealFromDouble( double value, OmmReal::MagnitudeType magnitudeType = OmmReal::Exponent0Enum );

	/** Adds a specific simple type of OMM data to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@param[in] value added float
		@return reference to this object
	*/
	OmmArray& addFloat( float value );

	/** Adds a specific simple type of OMM data to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@param[in] value added double
		@return reference to this object
	*/
	OmmArray& addDouble( double value );

	/** Adds a specific simple type of OMM data to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@throw OmmOutOfRangeException if passed in OmmDate is invalid
		@param[in] year added OmmDate::year (0 - 4095 where 0 indicates blank)
		@param[in] month added OmmDate::month (0 - 12 where 0 indicates blank)
		@param[in] day added OmmDate::day (0 - 31 where 0 indicates blank)
		@return reference to this object
	*/
	OmmArray& addDate( UInt16 year, UInt8 month, UInt8 day );

	/** Adds a specific simple type of OMM data to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@throw OmmOutOfRangeException if passed in OmmTime is invalid
		@param[in] hour added OmmTime::hour (0 - 23 where 255 indicates blank)
		@param[in] minute added OmmTime::minute (0 - 59 where 255 indicates blank)
		@param[in] second added OmmTime::second (0 - 60 where 255 indicates blank)
		@param[in] millisecond added OmmTime::millisecond (0 - 999 where 65535 indicates blank)
		@param[in] microsecond added OmmTime::microsecond (0 - 999 where 2047 indicates blank)
		@param[in] nanosecond added OmmTime::nanosecond (0 - 999 where 2047 indicates blank)
		@return reference to this object
	*/
	OmmArray& addTime( UInt8 hour = 0, UInt8 minute = 0, UInt8 second = 0,
					UInt16 millisecond = 0, UInt16 microsecond = 0, UInt16 nanosecond = 0 );

	/** Adds a specific simple type of OMM data to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@throw OmmOutOfRangeException if passed in OmmDate is invalid
		@param[in] year added OmmDateTime::year (0 - 4095 where 0 indicates blank)
		@param[in] month added OmmDateTime::month (0 - 12 where 0 indicates blank)
		@param[in] day added OmmDateTime::day (0 - 31 where 0 indicates blank)
		@param[in] hour added OmmDateTime::hour (0 - 23 where 255 indicates blank)
		@param[in] minute added OmmDateTime::minute (0 - 59 where 255 indicates blank)
		@param[in] second added OmmDateTime::second (0 - 60 where 255 indicates blank)
		@param[in] millisecond added OmmDateTime::millisecond (0 - 999 where 65535 indicates blank)
		@param[in] microsecond added OmmDateTime::microsecond (0 - 999 where 2047 indicates blank)
		@param[in] picoseconds added OmmDateTime::nanosecond (0 - 999 where 2047 indicates blank)
		@return reference to this object
	*/
	OmmArray& addDateTime( UInt16 year, UInt8 month, UInt8 day,
					   UInt8 hour = 0, UInt8 minute = 0, UInt8 second = 0,
					   UInt16 millisecond = 0, UInt16 microsecond = 0, UInt16 nanosecond = 0 );

	/** Adds a specific simple type of OMM data to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@param[in] timeliness added OmmQos::timeliness (default value is OmmQos::RealTimeEnum)
		@param[in] rate added OmmQos::rate (default value is OmmQos::TickByTickEnum)
		@return reference to this object
	*/
	OmmArray& addQos( UInt32 timeliness = OmmQos::RealTimeEnum, UInt32 rate = OmmQos::TickByTickEnum );

	/** Adds a specific simple type of OMM data to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@param[in] streamState added OmmState::streamState (default value is OmmState::OpenEnum)
		@param[in] dataState added OmmState::dataState (default value is OmmState::OkEnum)
		@param[in] statusCode added OmmState::statusCode (default value is OmmState::NoneEnum)
		@param[in] statusText added OmmState::text (default value is 'empty string')
		@return reference to this object
	*/
	OmmArray& addState( OmmState::StreamState streamState = OmmState::OpenEnum,
					OmmState::DataState dataState = OmmState::OkEnum,
					UInt8 statusCode = OmmState::NoneEnum,
					const EmaString& statusText = EmaString() );

	/** Adds a specific simple type of OMM data to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@param[in] value added Enum
		@return reference to this object
	*/
	OmmArray& addEnum( UInt16 value );

	/** Adds a specific simple type of OMM data to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@param[in] value added EmaBuffer as OmmBuffer
		@return reference to this object
	*/
	OmmArray& addBuffer( const EmaBuffer& value );

	/** Adds a specific simple type of OMM data to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@param[in] value added EmaString as OmmAscii
		@return reference to this object
	*/
	OmmArray& addAscii( const EmaString& value );

	/** Adds a specific simple type of OMM data to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@param[in] value added EmaBuffer as OmmUtf8
		@return reference to this object
	*/
	OmmArray& addUtf8( const EmaBuffer& value );

	/** Adds a specific simple type of OMM data to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@param[in] value added EmaBuffer as OmmRmtes
		@return reference to this object
	*/
	OmmArray& addRmtes( const EmaBuffer& value );

	/** Adds a blank data code to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@return reference to this object
	*/
	OmmArray& addCodeInt();

	/** Adds a blank data code to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@return reference to this object
	*/
	OmmArray& addCodeUInt();

	/** Adds a blank data code to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@return reference to this object
	*/
	OmmArray& addCodeReal();

	/** Adds a blank data code to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@return reference to this object
	*/
	OmmArray& addCodeFloat();

	/** Adds a blank data code to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@return reference to this object
	*/
	OmmArray& addCodeDouble();

	/** Adds a blank data code to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@return reference to this object
	*/
	OmmArray& addCodeDate();

	/** Adds a blank data code to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@return reference to this object
	*/
	OmmArray& addCodeTime();

	/** Adds a blank data code to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@return reference to this object
	*/
	OmmArray& addCodeDateTime();

	/** Adds a blank data code to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@return reference to this object
	*/
	OmmArray& addCodeQos();

	/** Adds a blank data code to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@return reference to this object
	*/
	OmmArray& addCodeState();

	/** Adds a blank data code to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@return reference to this object
	*/
	OmmArray& addCodeEnum();

	/** Adds a blank data code to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@return reference to this object
	*/
	OmmArray& addCodeBuffer();

	/** Adds a blank data code to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@return reference to this object
	*/
	OmmArray& addCodeAscii();

	/** Adds a blank data code to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@return reference to this object
	*/
	OmmArray& addCodeUtf8();

	/** Adds a blank data code to the OmmArray.
		@throw OmmInvalidUsageException if first addition was of different data type
		@return reference to this object
	*/
	OmmArray& addCodeRmtes();

	/** Completes encoding of OmmArray.
		@return const reference to this object
	*/
	const OmmArray& complete();
	//@}

private :

	Decoder& getDecoder();
	bool hasDecoder() const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	const EmaString& toString( UInt64 indent ) const;

	mutable EmaString			_toString;
	OmmArrayEntry				_entry;
	OmmArrayDecoder*			_pDecoder;
	mutable OmmArrayEncoder*	_pEncoder;

	OmmArray( const OmmArray& );
	OmmArray& operator=( const OmmArray& );
};

}

}

}

#endif // __rtsdk_ema_access_OmmArray_h
