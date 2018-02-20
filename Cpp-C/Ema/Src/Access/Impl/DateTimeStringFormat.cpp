/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2017. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ExceptionTranslator.h"
#include "OmmDateDecoder.h"
#include "OmmTimeDecoder.h"
#include "OmmDateTimeDecoder.h"
using namespace thomsonreuters::ema::access;

void DateTimeStringFormat::dateTimeStringFormatType(DateTimeStringFormatTypes format )
{
	if(format >= STR_DATETIME_ISO8601 && format <= STR_DATETIME_RSSL)
		_format = format;	
	else
	{
		EmaString temp( "Failed to set format is out of range, format: " );
		temp.append(format);
		throwOorException( temp );
	}
	return;
}

DateTimeStringFormat::DateTimeStringFormatTypes DateTimeStringFormat::getDateTimeStringFormatType()
{
	return _format;
}

const EmaString&	DateTimeStringFormat::dateAsString(OmmDate &date)
{
	OmmDateDecoder& dateDecder = static_cast<OmmDateDecoder&>(date.getDecoder());
	DateTimeStringFormat::DateTimeStringFormatTypes originalFormat = dateDecder.getDateTimeStringFormatType();
	dateDecder.setDateTimeStringFormatType(_format);
	const EmaString& dateStr = dateDecder.toString();
	dateDecder.setDateTimeStringFormatType(originalFormat);
	return dateStr;
}

const EmaString&	DateTimeStringFormat::timeAsString(OmmTime &time)
{
	OmmTimeDecoder& timeDecder = static_cast<OmmTimeDecoder&>(time.getDecoder());
	DateTimeStringFormat::DateTimeStringFormatTypes originalFormat = timeDecder.getDateTimeStringFormatType();
	timeDecder.setDateTimeStringFormatType(_format);
	const EmaString& dateStr = timeDecder.toString();
	timeDecder.setDateTimeStringFormatType(originalFormat);
	return dateStr;
}

const EmaString&	DateTimeStringFormat::dateTimeAsString(OmmDateTime &dtTime)
{
	OmmDateTimeDecoder& dtTimeDecder = static_cast<OmmDateTimeDecoder&>(dtTime.getDecoder());
	DateTimeStringFormat::DateTimeStringFormatTypes originalFormat = dtTimeDecder.getDateTimeStringFormatType();
	dtTimeDecder.setDateTimeStringFormatType(_format);
	const EmaString& dateStr = dtTimeDecder.toString();
	dtTimeDecder.setDateTimeStringFormatType(originalFormat);
	return dateStr;
}
