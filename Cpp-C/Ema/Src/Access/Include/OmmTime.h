/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmTime_h
#define __thomsonreuters_ema_access_OmmTime_h

/**
	@class thomsonreuters::ema::access::OmmTime OmmTime.h "Access/Include/OmmTime.h"
	@brief OmmTime represents Time info in Omm.

	OmmTime encapsulates hour, minute, second, millisecond, microsecond and nanosecond information.

	The following code snippet shows setting of time in ElementList.

	\code

	ElementList eList;
	eList.addTime( "my time", 23, 59, 59, 999, 999, 999 ).complete();

	\endcode

	The following code snippet sows extraction of time from ElementList.

	\code

	void decodeElementList( const ElementList& eList )
	{
		while ( eList.forth() )
		{
			const ElementEntry& eEntry = eList.getEntry();

			if ( eEntry.getCode() != Data::BlankEnum )
				switch ( eEntry.getDataType() )
				{
					case DataType::TimeEnum :
						const OmmTime& ommTime = eEntry.getTime();
						UInt8 hour = ommTime.getHour();
						break;
				}
		}
	}

	\endcode

	\remark OmmTime is a read only class.
	\remark This class is used for extraction of Time info only.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		EmaString,
		EmaBuffer
*/

#include "Access/Include/Data.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmTimeDecoder;

class EMA_ACCESS_API OmmTime : public Data
{
public :

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::TimeEnum
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

	OmmTime();
	virtual ~OmmTime();
	OmmTime( const OmmTime& );
	OmmTime& operator=( const OmmTime& );

	OmmTimeDecoder*		_pDecoder;
	UInt64				_space[13];
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmTime_h
