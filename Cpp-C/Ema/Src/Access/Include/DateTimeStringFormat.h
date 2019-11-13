/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_DateTimeStringFormat_h
#define __thomsonreuters_ema_access_DateTimeStringFormat_h

/**
	@class thomsonreuters::ema::access::DateTimeStringFormat "Access/Include/DateTimeStringFormat.h"
	@brief DateTimeStringFormat is an interface to string conversion methods for OmmDate, OmmTime & OmmDateTime.

	\remark DateTimeStringFormat is single threaded.
	\remark If an application accesses a single object of DateTimeStringFormat via multiple threads,
	\remark it needs to implement its own locking mechanism.

	@see Data,
		OmmDate, 
		OmmTime, 
		OmmDateTime
*/

#include "Access/Include/Data.h"
#include "Access/Include/OmmDate.h"
#include "Access/Include/OmmTime.h"
#include "Access/Include/OmmDateTime.h"

namespace thomsonreuters {

namespace ema {

namespace access {
	
class EMA_ACCESS_API DateTimeStringFormat
{
public :	
		/** @enum DateTimeStringFormatTypes
		An enumeration representing item data state.
	*/
	enum DateTimeStringFormatTypes
	{
		STR_DATETIME_ISO8601 = 1,	/*!< Indicates Date/Time/DateTime to string output in ISO8601's dateTime format:
										"YYYY-MM-DDThour:minute:second.nnnnnnnnn" (e.g., 2010-11-30T15:24:54.627529436). */
		STR_DATETIME_RSSL = 2		/*!< Indicates Date/Time/DateTime to string output in to string output in the default format:
										"DD MON YYYY hour:minute:second:milli:micro:nano" (e.g., 30 NOV 2010 15:24:54:627:529:436).*/
	};

	DateTimeStringFormat(): _format(STR_DATETIME_RSSL){};

	/** Returns DateTimeStringFormatTypes.
		@return value of DateTimeStringFormatTypes
	*/
	DateTimeStringFormatTypes getDateTimeStringFormatType() ;

	/** Specifies DateTimeStringFormatType.
	@throw OmmOutOfRangeException if format is not any of the enumerations of DateTimeStringFormatTypes.
	@param[in] DateTimeStringFormatTypes specifies the format of the string output of Date/Time/DateTime
	@return format of the output string.
	*/
	void dateTimeStringFormatType(DateTimeStringFormatTypes format);

	/** Returns the OmmDate value as a string in a specified format.
		@param[in]  OmmDate that needs to be converted to string.
		@return string representation of this object OmmDate in the specified format
	*/
	const EmaString&	dateAsString(OmmDate &date);

	/** Returns the OmmTime value as a string in a specified format.
		@param[in]  OmmTime that needs to be converted to string.
		@return string representation of this object OmmTime in the specified format
	*/
	const EmaString&	timeAsString(OmmTime &time);	

	/** Returns the OmmDateTime value as a string in a specified format.
		@param[in]  OmmDateTime that needs to be converted to string.
		@return string representation of this object OmmDateTime in the specified format
	*/
	const EmaString&	dateTimeAsString(OmmDateTime &dateTime);

private:
	DateTimeStringFormatTypes _format;
};

}

}

}

#endif // __thomsonreuters_ema_access_DateTimeStringFormat_h
