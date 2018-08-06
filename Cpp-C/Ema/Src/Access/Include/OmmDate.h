/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmDate_h
#define __thomsonreuters_ema_access_OmmDate_h

/**
	@class thomsonreuters::ema::access::OmmDate OmmDate.h "Access/Include/OmmDate.h"
	@brief OmmDate represents Date info in Omm.

	OmmDate encapsulates year, month and day information.

	The following code snippet shows setting of date in ElementList;

	\code

	ElementList eList;
	eList.addDate( "my date", 1999, 12, 31 ).complete();

	\endcode

	The following code snippet shows extraction of date from ElementList.

	\code

	void decodeElementList( const ElementList& eList )
	{
		while ( eList.forth() )
		{
			const ElementEntry& eEntry = eList.getEntry();

			if ( eEntry.getCode() != Data::BlankEnum )
				switch ( eEntry.getDataType() )
				{
					case DataType::DateEnum :
						const OmmDate& ommDate = eEntry.getDate();
						UInt16 year = ommDate.getYear();
						break;
				}
		}
	}

	\endcode

	\remark OmmDate is a read only class.
	\remark This class is used for extraction of Date info only.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		EmaString,
		EmaBuffer
*/

#include "Access/Include/Data.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmDateDecoder;

class EMA_ACCESS_API OmmDate : public Data
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
	//@}

private :

	friend class Decoder;
	friend class StaticDecoder;
	friend class DateTimeStringFormat;

	Decoder& getDecoder();
	bool hasDecoder() const;

	const EmaString& toString( UInt64 indent ) const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	OmmDate();
	virtual ~OmmDate();
	OmmDate( const OmmDate& );
	OmmDate& operator=( const OmmDate& );

	OmmDateDecoder*		_pDecoder;
	UInt64				_space[12];
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmDate_h
