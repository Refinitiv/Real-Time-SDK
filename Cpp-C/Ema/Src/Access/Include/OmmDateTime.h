/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmDateTime_h
#define __refinitiv_ema_access_OmmDateTime_h

/**
	@class refinitiv::ema::access::OmmDateTime OmmDateTime.h "Access/Include/OmmDateTime.h"
	@brief OmmDateTime represents DateTime info in Omm.
	
	OmmDateTime encapsulates year, month, day, hour, minute, second, millisecond,
	microsecond and nanosecond information.

	The following code snippet shows extraction of DateTime from ElementList.

	\code

	void decodeElementList( const ElementList& eList )
	{
		while ( eList.forth() )
		{
			const ElementEntry& eEntry = eList.getEntry();

			if ( eEntry.getCode() != Data::BlankEnum )
				switch ( eEntry.getDataType() )
				{
					case DataType::DateTimeEnum :
						const OmmDateTime& ommDateTime = eEntry.getDateTime();
						UInt16 year = ommDateTime.getYear();
						...
						UInt8 hour = ommDateTime.getHour();
						break;
				}
		}
	}

	\endcode
	
	\remark OmmDateTime is a read only class.
	\remark This class is used for extraction of DateTime info only.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		EmaString,
		EmaBuffer
*/

#include "Access/Include/Data.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmDateTimeDecoder;

class EMA_ACCESS_API OmmDateTime : public Data
{
public :

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::DateTimeEnum
	*/
	DataType::DataTypeEnum getDataType() const;

	/** Returns the Code, which indicates a special state of a DataType.
		@return Data::BlankEnum if received data is blank; Data::NoCodeEnum otherwise
	*/
	Data::DataCode getCode() const;

	/** Returns a buffer that in turn provides an alphanumeric null-terminated hexadecimal string representation.
		@return EmaBuffer with the object hex information
	*/
	const EmaBuffer& getAsHex() const;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EmaString& toString() const;

	/** Returns Year. Range is 0 - 4095 where 0 indicates blank.
		@return value of year
	*/
	UInt16 getYear() const;

	/** Returns Month. Range is 0 - 12 where 0 indicates blank.
		@return value of month
	*/
	UInt8 getMonth() const;

	/** Returns Day. Range is 0 - 31 where 0 indicates blank.
		@return value of day
	*/
	UInt8 getDay() const;

	/** Returns Hour. Range is 0 - 23 where 255 indicates blank.
		@return value of hour
	*/
	UInt8 getHour() const;

	/** Returns Minute. Range is 0 - 59 where 255 indicates blank.
		@return value of minute
	*/
	UInt8 getMinute() const;

	/** Returns Second. Range is 0 - 60 where 255 indicates blank and 60 is to account for leap second.
		@return value of second
	*/
	UInt8 getSecond() const;

	/** Returns Millisecond. Range is 0 - 999 where 65535 indicates blank.
		@return value of millisecond
	*/
	UInt16 getMillisecond() const;

	/** Returns Microsecond. Range is 0 - 999 where 2047 indicates blank.
		@return value of microsecond
	*/
	UInt16 getMicrosecond() const;

	/** Returns Nanosecond. Range is 0 - 999 where 2047 indicates blank.
		@return value of nanosecond
	*/
	UInt16 getNanosecond() const;
	//@}

private :

	friend class Decoder;
	friend class StaticDecoder;
	friend class DateTimeStringFormat;

	Decoder& getDecoder();
	bool hasDecoder() const;

	const EmaString& toString( UInt64 ) const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	OmmDateTime();
	virtual ~OmmDateTime();
	OmmDateTime( const OmmDateTime& );
	OmmDateTime& operator=( const OmmDateTime& );

	OmmDateTimeDecoder*			_pDecoder;
	UInt64						_space[17];
};

}

}

}

#endif // __refinitiv_ema_access_OmmDateTime_h
