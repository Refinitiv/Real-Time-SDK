/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslRetCodes.h"
#include "rtr/rwfConvert.h"
#include "rtr/rsslDataUtils.h"
#include "rtr/decoderTools.h"
#include <ctype.h>
#include <time.h>


static const char * months[12] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

#define MAX_DECIMAL_DIGITS 9

/* Converts RsslDateTime to blank string only if the all portions of RsslDateTime are blank. 
 * Returns RSSL_FALSE if any one portion is not blank.
 */
RsslBool dateTimeIsBlankToStr(RsslBuffer * oBuffer, RsslDateTime * iDateTime)
{
	/* check for blank date and time */
	if (iDateTime->date.day == 0 && iDateTime->date.month == 0 && iDateTime->date.year == 0 &&
		iDateTime->time.hour == 255 && iDateTime->time.minute == 255 && iDateTime->time.second == 255 && 
		iDateTime->time.millisecond == 65535 && iDateTime->time.microsecond == 2047 && iDateTime->time.nanosecond == 2047)
	{
		memset(oBuffer->data, 0, oBuffer->length);
		oBuffer->length = 0;
		return RSSL_TRUE;
	}
	else
		return RSSL_FALSE; /*datetime not blank */
}

/* Converts RsslDate to blank string only if the all portions of RsslDate are blank. 
 * Returns RSSL_FALSE if any one portion is not blank.
 */
RsslBool dateIsBlankToStr(RsslBuffer * oBuffer, RsslDate *iDate)
{
	if (iDate->day == 0 && iDate->month == 0 && iDate->year == 0)
	{
		/* blank date */
		memset(oBuffer->data, 0, oBuffer->length);
		oBuffer->length = 0;
		return RSSL_TRUE;							
	}
	else
		return RSSL_FALSE; /* date not blank*/
}

/* Converts RsslTime to blank string only if the all portions of RsslTime are blank. 
 * Returns RSSL_FALSE if any one portion is not blank.
 */
RsslBool timeIsBlankToStr(RsslBuffer * oBuffer, RsslBool isTime, RsslTime * iTime)
{
	if (iTime->hour == 255 && iTime->minute == 255 && iTime->second == 255 && iTime->millisecond == 65535 && iTime->microsecond == 2047 && iTime->nanosecond == 2047)
	{
		if (isTime == RSSL_TRUE)
		{
			/* this is only time, it is not part of dateTime */
			memset(oBuffer->data, 0, oBuffer->length);
			oBuffer->length = 0;
			return RSSL_TRUE;
		}
		else
		{  /* Time portion of dateTime is blank */
			oBuffer->length = (RsslUInt32) strlen(oBuffer->data);
			return RSSL_TRUE;
		}				
	}
	else
		return RSSL_FALSE; /* time not blank*/
}

/* Converts RsslDate to string in ISO8601 'YYYY-MM-DD' format (e.g. 2003-06-01). */
RsslRet dateToStringIso8601(RsslBuffer * oBuffer, int *bufOffset, int *remainingLength, RsslDate *iDate )
{
	/* Iso8601 date */
	RsslRet ret = 0;
	int i;
	if (iDate->year)
	{
		ret = snprintf(oBuffer->data, *remainingLength, "%04d-", iDate->year);
		if (ret < 0 || ret >= *remainingLength)
			return RSSL_RET_FAILURE;	
	}
	else
	{
		ret = snprintf(oBuffer->data, *remainingLength, "--");
		if (ret < 0 || ret >= *remainingLength)
			return RSSL_RET_FAILURE;
	}
	*bufOffset += ret;
	*remainingLength -= ret;

	if (iDate->month)
	{
		ret = snprintf((oBuffer->data + *bufOffset), *remainingLength, "%02d-", iDate->month);
		if (ret < 0 || ret >= *remainingLength)
			return RSSL_RET_FAILURE;	
	}
	else
	{
		ret = snprintf((oBuffer->data + *bufOffset), *remainingLength, "  -");
		if (ret < 0 || ret >= *remainingLength)
			return RSSL_RET_FAILURE;
	}

	*bufOffset += ret;
	*remainingLength -= ret;
	if (iDate->day)
	{
		ret = snprintf((oBuffer->data + *bufOffset), *remainingLength, "%02d", iDate->day);
		if (ret < 0 || ret >= *remainingLength)
			return RSSL_RET_FAILURE;	
	}
	else
	{
		ret = snprintf((oBuffer->data + *bufOffset), *remainingLength, "  ");
		if (ret < 0 || ret >= *remainingLength)
			return RSSL_RET_FAILURE;
	}
	*bufOffset += ret;
	*remainingLength -= ret;
	i = *bufOffset -1;
	for(; i >= 0; --i, (*bufOffset)--, (*remainingLength)++)
	{ /* Trim trailing non digits */
		if(oBuffer->data[i] >= '0' && oBuffer->data[i] <= '9')  
		{
			oBuffer->data[i + 1] = '\0';
			break;
		}
	}
	return RSSL_RET_SUCCESS;
}

/* Converts RsslDate to string in RSSL "DD MON YYYY" format (e.g. 01 JUN 2003). */
RsslRet dateToStringRssl(RsslBuffer * oBuffer, int *bufOffset, int *remainingLength, RsslDate *iDate )
{
	/* normal date */
	/* put this into the same format as marketfeed uses where if any portion is blank, it is 
				   represented as spaces */
	RsslRet ret = 0;
	if (iDate->day)
	{
		ret = snprintf(oBuffer->data, *remainingLength, "%02d ", iDate->day);
		if (ret < 0 || ret >= *remainingLength)
			return RSSL_RET_FAILURE;

	}
	else
	{
		ret = snprintf(oBuffer->data, *remainingLength, "   ");
		if (ret < 0 || ret >= *remainingLength)
			return RSSL_RET_FAILURE;
	}
	*bufOffset += ret;
	*remainingLength -= ret;
				
	if (iDate->month)
	{
		ret = snprintf((oBuffer->data + *bufOffset), *remainingLength, "%s ", months[iDate->month - 1]);
		if (ret < 0 || ret >= *remainingLength)
			return RSSL_RET_FAILURE; 
	}
	else
	{
		ret = snprintf((oBuffer->data + *bufOffset), *remainingLength, "    ");
		if (ret < 0 || ret >= *remainingLength)
			return RSSL_RET_FAILURE;
	}
	*bufOffset += ret;
	*remainingLength -= ret;

	if (iDate->year)
	{
		ret = snprintf((oBuffer->data + *bufOffset), *remainingLength, "%4d", iDate->year);
		if (ret < 0 || ret >= *remainingLength)
			return RSSL_RET_FAILURE;
	}
	else
	{
		ret = snprintf((oBuffer->data + *bufOffset), *remainingLength, "    ");
		if (ret < 0 || ret >= *remainingLength)
			return RSSL_RET_FAILURE;
	}
	*bufOffset += ret;
	*remainingLength -= ret;

	return RSSL_RET_SUCCESS;
}

/* Converts RsslTime to string in ISO8601 'HH:MM:SS.nnnnnnnnn' format & trims trailing 0s (e.g. '12:15:35.5006619' --Trimmed trail zeros after nano). */
RsslRet timeToStringIso8601Time(RsslBuffer * oBuffer, int *bufOffset, int *remainingLength, RsslTime *iTime )
{
	/* hour is always present */
	RsslRet ret = 0;
	int i = 0;

	if (iTime->minute != 255)
	{
		if (iTime->second != 255)
		{
			if (iTime->millisecond != 65535)
			{
				if (iTime->microsecond != 2047)
				{
					if (iTime->nanosecond != 2047)
					{
						/* Second, millisecond, microsecond, nanosecond */
						ret = snprintf((oBuffer->data + *bufOffset), *remainingLength, "%02d:%02d:%02d.%03d%03d%03d", iTime->hour, iTime->minute, iTime->second,
								iTime->millisecond, iTime->microsecond, iTime->nanosecond);
					}
					else
					{
						/* Second, millisecond, microsecond */
						ret = snprintf((oBuffer->data + *bufOffset), *remainingLength, "%02d:%02d:%02d.%03d%03d", iTime->hour, iTime->minute, iTime->second,
								iTime->millisecond, iTime->microsecond);
					}
				}
				else
				{
					/* Second, millisecond */
					ret = snprintf((oBuffer->data + *bufOffset), *remainingLength, "%02d:%02d:%02d.%03d", iTime->hour, iTime->minute, iTime->second,
							iTime->millisecond);
				}

				if (ret < 0 || ret >= *remainingLength)
					return RSSL_RET_FAILURE;

				*bufOffset += ret;
				*remainingLength -= ret;

				i = *bufOffset -1;
				for(; i >= 0; --i)
				{ /* Trim trailing zeros */
					if(oBuffer->data[i] >= '1' && oBuffer->data[i] <= '9')  
					{
						oBuffer->data[i + 1] = '\0';
						break;
					}
					else if (oBuffer->data[i] == '.' )
					{
						oBuffer->data[i] = '\0';
						break;
					}
				}

				return RSSL_RET_SUCCESS;
			}
			else
			{
				/* Second */
				ret = snprintf((oBuffer->data + *bufOffset), *remainingLength, "%02d:%02d:%02d", iTime->hour, iTime->minute, iTime->second);
			}
		}
		else
		{
			/* No seconds */
			ret = snprintf((oBuffer->data + *bufOffset), *remainingLength, "%02d:%02d", iTime->hour, iTime->minute);
		}
	}
	else
	{
		/* No minutes */
		ret = snprintf((oBuffer->data + *bufOffset), *remainingLength, "%02d", iTime->hour);
	}

	if (ret < 0 || ret >= *remainingLength)
		return RSSL_RET_FAILURE;

	*bufOffset += ret;
	*remainingLength -= ret;

	return RSSL_RET_SUCCESS;
}

/* Converts RsslTime to string in RSSL "HH:MM:SS:MIL:MIC:NAN" format (e.g. 12:15:35:500:661:900). */
RsslRet timeToStringRssltime(RsslBuffer * oBuffer, int *bufOffset, int *remainingLength, RsslTime *iTime )
{
	/* have to do this piece by piece to handle the various trailing portions being blank */
	/* hour is always present */
	RsslRet ret = 0;
	ret = snprintf((oBuffer->data + *bufOffset), *remainingLength, "%02d", 
				iTime->hour);
	if (ret < 0 || ret >= *remainingLength)
		return RSSL_RET_FAILURE;

	*bufOffset += ret;
	*remainingLength -= ret;
	/* minute */
	if (iTime->minute == 255)
		return RSSL_RET_SUCCESS;				
	ret = snprintf((oBuffer->data + *bufOffset), *remainingLength, ":%02d", 
				iTime->minute);
	if (ret < 0 || ret >= *remainingLength)
		return RSSL_RET_FAILURE;

	*bufOffset += ret;
	*remainingLength -= ret;
	/* second */
	if (iTime->second == 255)
		return RSSL_RET_SUCCESS;				
	ret = snprintf((oBuffer->data + *bufOffset), *remainingLength, ":%02d", 
				iTime->second);
	if (ret < 0 || ret >= *remainingLength)
		return RSSL_RET_FAILURE;

	*bufOffset += ret;
	*remainingLength -= ret;
	/* millisecond */
	if (iTime->millisecond == 65535)
		return RSSL_RET_SUCCESS;
				
	ret = snprintf((oBuffer->data + *bufOffset), *remainingLength, ":%03d", 
				iTime->millisecond);
	if (ret < 0 || ret >= *remainingLength)
		return RSSL_RET_FAILURE;

	*bufOffset += ret;
	*remainingLength -= ret;
	/* microsecond */
	if (iTime->microsecond == 2047)
		return RSSL_RET_SUCCESS;
				
	ret = snprintf((oBuffer->data + *bufOffset), *remainingLength, ":%03d", 
				iTime->microsecond);
	if (ret < 0 || ret >= *remainingLength)
		return RSSL_RET_FAILURE;

	*bufOffset += ret;
	*remainingLength -= ret;
	/* nanosecond */
	if (iTime->nanosecond == 2047)
		return RSSL_RET_SUCCESS;
				
	ret = snprintf((oBuffer->data + *bufOffset), *remainingLength, ":%03d", 
				iTime->nanosecond);
	if (ret < 0 || ret >= *remainingLength)
		return RSSL_RET_FAILURE;

	*bufOffset += ret;
	*remainingLength -= ret;

	return RSSL_RET_SUCCESS;
}

int translateMonth(const char* monthStr)
{
	int i, month = 0;

	for  (i = 0; i < 12; i++)
	{
		if (!strcmp(months[i], monthStr))
		{
			month = i + 1;
			break;
		}
	}

	return month;
}

RSSL_API RsslBool _rsslIsLeapYear(RsslUInt16 year)
{
	if ( (year % 4 == 0) && 
		 ((year % 100 != 0) || (year % 400 == 0)) )
		return RSSL_TRUE;
	else
		return RSSL_FALSE;
}

RSSL_API RsslBool rsslDateIsValid(const RsslDate * iDate)
{
	RsslDate blankDate = RSSL_BLANK_DATE;
	if (rsslDateIsEqual(iDate, &blankDate))
		return RSSL_TRUE;

	/* month or date or year of 0 is valid because marketfeed can send it */

	switch (iDate->month)
	{
	case 0:
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		if (iDate->day > 31)
			return RSSL_FALSE;
		break;
	case 4:
	case 6:
	case 9:
	case 11:
		if (iDate->day > 30)
			return RSSL_FALSE;
		break;
	case 2:
		if (iDate->day > 29)
			return RSSL_FALSE;
		else if ((iDate->day == 29) && !_rsslIsLeapYear(iDate->year))
			return RSSL_FALSE;
		break;
	default:
		return RSSL_FALSE;
	}

	return RSSL_TRUE;
}

RSSL_API RsslBool rsslTimeIsValid(const RsslTime * iTime)
{
	RsslTime blankTime = RSSL_BLANK_TIME;
		
	if (rsslTimeIsEqual(iTime, &blankTime))
		return RSSL_TRUE;

	/* make sure hours are between 0 and 23, minute/sec is between 0 and 59, and milli is between 0 and 999 */
	if ((iTime->hour != 255) && (iTime->hour > 23))
		return RSSL_FALSE;
	if ((iTime->minute != 255) && (iTime->minute > 59))
		return RSSL_FALSE;
	if ((iTime->second != 255) && (iTime->second > 60))
		return RSSL_FALSE;
	if ((iTime->millisecond != 65535) && (iTime->millisecond > 999))
		return RSSL_FALSE;
	if ((iTime->microsecond != 2047) && (iTime->microsecond > 999))
		return RSSL_FALSE;
	if ((iTime->nanosecond != 2047) && (iTime->nanosecond > 999))
		return RSSL_FALSE;
	
		if (iTime->nanosecond == 2047)
		{
			if (iTime->microsecond == 2047)
			{
				if (iTime->millisecond == 65535)
				{
					if (iTime->second == 255)
					{
						if (iTime->minute == 255)
							return RSSL_TRUE;
						else 
						{
							if (iTime->hour == 255)
								return RSSL_FALSE;
							else
								return RSSL_TRUE;
						}
					}
					else
					{
						if ((iTime->hour == 255) || (iTime->minute == 255))
							return RSSL_FALSE;
						else
							return RSSL_TRUE;					
					}
				}
				else
				{
					if ((iTime->hour == 255) || (iTime->minute == 255) || (iTime->second == 255))
						return RSSL_FALSE;
					else
						return RSSL_TRUE;
				}
			}
			else
			{
				if ((iTime->hour == 255) || (iTime->minute == 255) || (iTime->second == 255) || (iTime->millisecond == 65535))
					return RSSL_FALSE;
				else
					return RSSL_TRUE;
			}
		}
		else if ((iTime->hour == 255) || (iTime->minute == 255) || (iTime->second == 255) || (iTime->millisecond == 65535) || (iTime->microsecond == 2047))
			return RSSL_FALSE;

		return RSSL_TRUE;
}

RSSL_API RsslBool rsslDateTimeIsValid(const RsslDateTime * iDateTime)
{
	/* not checking for Blank datetime because the ValidDate and ValidTime check the individual parts */
	/* there is a slight risk here because date or time may be blank while the other part is not.  */

	return (rsslDateIsValid(&iDateTime->date) && rsslTimeIsValid(&iDateTime->time));
}

RSSL_API RsslRet rsslDateStringToDate(RsslDate * oDate, const RsslBuffer * iDateString)
{
	char * tmp;
	RsslUInt8 u8;
	RsslUInt16 u16;
	int a, b, c;
	int year = 0;
	int month = 0;
	int day = 0;
	int tmpInt = 0;
	char monthStr[32];
	int i = 0;
	int numberCount = 0;
	char* end;

	RSSL_ASSERT(oDate && iDateString, Invalid parameters or parameters passed in as NULL);

	if (iDateString->data == NULL || iDateString->length == 0)
	{
		oDate->day = 0;
		oDate->month = 0;
		oDate->year = 0;

		return RSSL_RET_BLANK_DATA;
	}

	tmp = iDateString->data;
	end = iDateString->data + iDateString->length;

	rsslClearDate(oDate);

	/* Check for yyyy year case first */
	while (tmp <= end && isdigit(*tmp))
	{
		tmpInt = tmpInt * 10 + (*tmp - '0');
		++numberCount;
		++tmp;
	}

	/* If it's a 4 digit year and nothing else, return the year */
	if (numberCount == 4 && (tmp == end || *tmp == '\0'))
	{
		oDate->year = tmpInt;
		return RSSL_RET_SUCCESS;
	}
	
	/* Covers following ISO 8601 cases:
		yyyy-mm-dd
		yyyy-mm */
	if (numberCount == 4 && !isspace(*tmp) && *tmp != '/')
	{
		year = tmpInt;
		/* Check for - */
		if (*tmp == '-')
		{
			numberCount = 0;
			tmpInt = 0;
			++tmp;

			/* Get the month */
			while (tmp <= end && isdigit(*tmp))
			{
				tmpInt = tmpInt * 10 + (*tmp - '0');
				++numberCount;
				++tmp;
			}

			if (numberCount != 2)
				return RSSL_RET_INVALID_DATA;

			month = tmpInt;

			/* If we're at the end, no date, return what we have. */
			if (tmp == end || *tmp == '\0')
			{
				oDate->year = year;
				oDate->month = month;
				return RSSL_RET_SUCCESS;
			}

			if (*tmp == '-')
			{
				numberCount = 0;
				tmpInt = 0;
				++tmp;

				/* Get the day */
				while (tmp <= end && isdigit(*tmp))
				{
					tmpInt = tmpInt * 10 + (*tmp - '0');
					++numberCount;
					++tmp;
				}

				if (numberCount != 2)
					return RSSL_RET_INVALID_DATA;

				day = tmpInt;

				/* Make sure we're a the end of the buffer */
				if (tmp == end || *tmp == '\0')
				{
					oDate->year = year;
					oDate->month = month;
					oDate->day = day;
					return RSSL_RET_SUCCESS;
				}
				else
					return RSSL_RET_INVALID_DATA;
			}
			else
				return RSSL_RET_INVALID_DATA;
		}
		else
			return RSSL_RET_INVALID_DATA;
	}
	/* Covers:
		yyyymmdd */
	else if (numberCount == 8)
	{
		tmp = iDateString->data;
		/* We've already verified that it is 8 consecutive digits, so parse the entire thing */
		for (i = 0; i < 4; i++)
		{
			oDate->year = oDate->year * 10 + (*tmp - '0');
			++tmp;
		}

		for (i = 0; i < 2; i++)
		{
			oDate->month = oDate->month * 10 + (*tmp - '0');
			++tmp;
		}

		for (i = 0; i < 2; i++)
		{
			oDate->day = oDate->day * 10 + (*tmp - '0');
			++tmp;
		}

		/* Check to make sure there's nothing left in the buffer */
		if (tmp == end || *tmp == '\0')
			return RSSL_RET_SUCCESS;
		else
		{
			rsslClearDate(oDate);
			return RSSL_RET_INVALID_DATA;
		}
	}
	/* Covers:
		--mm-dd
		--mmdd
	*/
	else if (numberCount == 0 && *tmp == '-')
	{
		/* Bounds check to make sure the first two characters are in the buffer */
		if ((tmp + 2) >= end)
			return RSSL_RET_INVALID_DATA;
		tmp++;
		if (*tmp != '-')
			return RSSL_RET_INVALID_DATA;

		tmp++;

		/* Get the month */
		tmpInt = 0;
		for(i = 0; i < 2; i++)
		{
			if (isdigit(*tmp))
				tmpInt = tmpInt * 10 + (*tmp - '0');
			else
				return RSSL_RET_INVALID_DATA;
			++tmp;
		}

		/* If there arne't 2 digits or this is the end of the buffer, error out */
		if (tmp == end || *tmp == '\0')
			return RSSL_RET_INVALID_DATA;

		month = tmpInt;

		if (isdigit(*tmp))
		{
			/* Get the day */
			numberCount = 0;
			tmpInt = 0;
			while (tmp <= end && isdigit(*tmp))
			{
				tmpInt = tmpInt * 10 + (*tmp - '0');
				++numberCount;
				++tmp;
			}

			/* Make sure that there were 2 digits, and that we're at the end of the buffer */
			if (numberCount != 2 && (tmp != end || *tmp != '\0'))
				return RSSL_RET_INVALID_DATA;

			oDate->day = tmpInt;
			oDate->month = month;

			return RSSL_RET_SUCCESS;
		}
		else if (*tmp == '-')
		{
			++tmp;
			/* Get the day */
			numberCount = 0;
			tmpInt = 0;
			while (tmp <= end && isdigit(*tmp))
			{
				tmpInt = tmpInt * 10 + (*tmp - '0');
				++numberCount;
				++tmp;
			}

			/* Make sure that there were 2 digits, and that we're at the end of the buffer */
			if (numberCount != 2 && (tmp != end || *tmp != '\0'))
				return RSSL_RET_INVALID_DATA;

			oDate->day = tmpInt;
			oDate->month = month;

			return RSSL_RET_SUCCESS;
		}
		else
			return RSSL_RET_INVALID_DATA;
	}

	if (sscanf(iDateString->data, "%d/%d/%d", &a, &b, &c) == 3)
	{
		// 1974/04/14
		//
		if (a > 255) // assume year here is greater than MAX UINT8
		{
			oDate->day = c;
			oDate->month = b;
			oDate->year = a;
		}
		// 04/14/74
		//
		else if (c < 100) // assume year here is less than 100, then add 1900
		{
			oDate->day = b;
			oDate->month = a;
			oDate->year = c + 1900;
		}
		// 04/14/1974
		//
		else
		{
			oDate->day = b;
			oDate->month = a;
			oDate->year = c;
		}
		return RSSL_RET_SUCCESS;
	}	

	if (isdigit(iDateString->data[3]))
	{
		if (sscanf(iDateString->data, "%d %d %d", &a, &b, &c) == 3)
		{
			if (a > 255) // assume year here is greater than MAX UINT
			{
				oDate->day = c;
				oDate->month = b;
				oDate->year = a;
			}
			else if (c < 100) // assume year here is less than 100, then add 1900
			{
				oDate->day = b;
				oDate->month = a;
				oDate->year = c + 1900;
			}
			else
			{
				oDate->day = b;
				oDate->month = a;
				oDate->year = c;
			}
			return RSSL_RET_SUCCESS;
		}
	}
	else if (isalpha(iDateString->data[3]))
	{
		if (sscanf(iDateString->data, "%d %3s %d", &a, monthStr, &c) == 3)
		{
			if (c < 100) // assume year here is less than 100, then add 1900
			{
				oDate->day = a;
				oDate->month = translateMonth(monthStr);
				oDate->year = c + 1900;
			}
			else
			{
				oDate->day = a;
				oDate->month = translateMonth(monthStr);
				oDate->year = c;
			}
			return RSSL_RET_SUCCESS;
		}
	}

	if (iDateString->length > 0)
	{
		tmp = iDateString->data;
		for (u8 = 0; isdigit(*tmp); tmp++)
			u8 = u8 * 10 + (*tmp - '0');
		oDate->day = u8;

		while (isspace(*tmp))
			tmp++; /* skip whitespace */
		
		if (tmp == iDateString->data + iDateString->length)
		{
			oDate->day = 0;
			oDate->month = 0;
			oDate->year = 0;

			return RSSL_RET_BLANK_DATA;
		}

		for (u8 = 1; u8 <= 12 && strncmp(months[u8-1], tmp, 3) ; u8++);
		if (u8 > 12)
        {
			return RSSL_RET_INVALID_DATA;
        }
		else
			oDate->month = u8;
		tmp += 3;

		tmp++; /* skip whitespace */

		for (u16 = 0; isdigit(*tmp); tmp++)
			u16 = u16 * 10 + (*tmp - '0');
		oDate->year = u16;
	}
	else
	{
		oDate->day = 0;
		oDate->month = 0;
		oDate->year = 0;

		return RSSL_RET_BLANK_DATA;
	}

	return RSSL_RET_SUCCESS;
}


/* This function handles converting both ETA formatted time strings and ISO8601 formats.  Time zones for ISO format are currently ignored.*/

RSSL_API RsslRet rsslTimeStringToTime(RsslTime * oTime, const RsslBuffer * iTimeString)
{
	char * tmp;
	RsslUInt8 u8;
	RsslUInt16 u16;
	int tmpInt = 0;
	int min = 0;
	int sec = 0;
	int hour = 0;
	int milli = 0;
	int micro = 0;
	int nano = 0;
	int numberCount = 0;
	char* end;
	int placeValue = 0;
	int i;
	
	RSSL_ASSERT(oTime && iTimeString, Invalid parameters or parameters passed in as NULL);

	if (iTimeString->data == NULL || iTimeString->length == 0)
	{
		oTime->hour = 255;
		oTime->minute = 255;
		oTime->second = 255;
		oTime->millisecond = 65535;
		oTime->microsecond = 2047;
		oTime->nanosecond = 2047;

		return RSSL_RET_BLANK_DATA;
	}

	tmp = iTimeString->data;
	end = iTimeString->data + iTimeString->length;

	rsslClearTime(oTime);
	
	/* Check for ISO case of hh:mm:ss first*/
	while (tmp <= end && isdigit(*tmp))
	{
		tmpInt = tmpInt * 10 + (*tmp - '0');
		++numberCount;
		++tmp;
	}

	/* If there are 2 characters in the first number, this is minimally hh:mm.
		It can also be:
			hh:mm:ss
			hh:mm:ss:mmm
			hh:mm:ss:mmm:mmm
			hh:mm:ss:mmm:mmm:nnn
			hh:mm:ss.nnnnnnnnn
			hh:mm:ss,nnnnnnnnn */
	if (numberCount == 2 && *tmp == ':')
	{
		hour = tmpInt;
		tmpInt = 0;

		tmp++;
		if (tmp == end || *tmp == '\0')
			return RSSL_RET_INVALID_DATA;

		tmpInt = 0;
		numberCount = 0;
		
		while (tmp <= end && isdigit(*tmp))
		{
			tmpInt = tmpInt * 10 + (*tmp - '0');
			++numberCount;
			++tmp;
			
		}
		min = tmpInt;
		
		if (numberCount != 2)
			return RSSL_RET_INVALID_DATA;
		/* Check to see if we're at the end or if there's a time zone(which will be dropped) */
		if (tmp == end || *tmp == '\0' || *tmp == 'Z' || *tmp == '+' || *tmp == '-')
		{
			oTime->hour = hour;
			oTime->minute = min;
			return RSSL_RET_SUCCESS;
		}

		if (tmp == end || *tmp == '\0' || *tmp != ':')
			return RSSL_RET_INVALID_DATA;
		tmp++;
		tmpInt = 0;
		numberCount = 0;

		while (tmp <= end && isdigit(*tmp))
		{
			tmpInt = tmpInt * 10 + (*tmp - '0');
			++numberCount;
			++tmp;
		}
		sec = tmpInt;

		if (numberCount != 2)
			return RSSL_RET_INVALID_DATA;
		/* Check to see if we're at the end or if there's a time zone(which will be dropped) */
		if (tmp == end || *tmp == '\0' || *tmp == 'Z' || *tmp == '+' || *tmp == '-')
		{
			oTime->hour = hour;
			oTime->minute = min;
			oTime->second = sec;
			return RSSL_RET_SUCCESS;
		}

		tmpInt = 0;
		numberCount = 0;
		if (tmp == end)
			return RSSL_RET_INVALID_DATA;
		else if (*tmp == ':')
		{
			tmpInt = 0;

			while (tmp <= end && isdigit(*tmp))
			{
				tmpInt = tmpInt * 10 + (*tmp - '0');
				++numberCount;
				++tmp;
			}
			milli = tmpInt;

			if (numberCount != 3)
				return RSSL_RET_INVALID_DATA;
			if (tmp == end )
			{
				oTime->hour = hour;
				oTime->minute = min;
				oTime->second = sec;
				oTime->millisecond = milli;
				return RSSL_RET_SUCCESS;
			}

			tmp++;
			tmpInt = 0;
			numberCount = 0;
			if (tmp == end || *tmp != ':')
				return RSSL_RET_INVALID_DATA;

			while (tmp <= end && isdigit(*tmp))
			{
				tmpInt = tmpInt * 10 + (*tmp - '0');
				++numberCount;
				++tmp;
			}
			micro = tmpInt;

			if (numberCount != 3)
				return RSSL_RET_INVALID_DATA;
			if (tmp == end)
			{
				oTime->hour = hour;
				oTime->minute = min;
				oTime->second = sec;
				oTime->millisecond = milli;
				oTime->microsecond = micro;
				return RSSL_RET_SUCCESS;
			}

			tmp++;
			tmpInt = 0;
			numberCount = 0;
			if (tmp == end || *tmp != ':')
				return RSSL_RET_INVALID_DATA;

			while (tmp <= end && isdigit(*tmp))
			{
				tmpInt = tmpInt * 10 + (*tmp - '0');
				++numberCount;
				++tmp;
			}
			nano = tmpInt;

			if (numberCount != 3)
				return RSSL_RET_INVALID_DATA;
			else
			{
				oTime->hour = hour;
				oTime->minute = min;
				oTime->second = sec;
				oTime->millisecond = milli;
				oTime->microsecond = micro;
				oTime->nanosecond = nano;
				return RSSL_RET_SUCCESS;
			}
		}
		else if (*tmp == '.' || *tmp == ',')
		{
			++tmp;
			if (tmp == end)
				return RSSL_RET_INVALID_DATA;

			oTime->hour = hour;
			oTime->minute = min;
			oTime->second = sec;

			placeValue = 100;
			for (i = 0; i < 3; ++i)
			{
				if (tmp == end || *tmp == '\0' || *tmp == 'Z' || *tmp == '+' || *tmp == '-')
					return RSSL_RET_SUCCESS;
				else if (isdigit(*tmp))
				{
					oTime->millisecond = oTime->millisecond + (*tmp - '0') * placeValue;
					placeValue = placeValue / 10;
				}
				else
				{
					memset(oTime, 0, sizeof(RsslTime));
					return RSSL_RET_INVALID_DATA;
				}
				tmp++;
			}

			placeValue = 100;
			for (i = 0; i < 3; ++i)
			{
				if (tmp == end || *tmp == '\0' || *tmp == 'Z' || *tmp == '+' || *tmp == '-')
					return RSSL_RET_SUCCESS;
				else if (isdigit(*tmp))
				{
					oTime->microsecond = oTime->microsecond + (*tmp - '0') * placeValue;
					placeValue = placeValue / 10;
				}
				else
				{
					memset(oTime, 0, sizeof(RsslTime));
					return RSSL_RET_INVALID_DATA;
				}
				tmp++;
			}

			placeValue = 100;
			for (i = 0; i < 3; ++i)
			{
				if (tmp == end || *tmp == '\0' || *tmp == 'Z' || *tmp == '+' || *tmp == '-')
					return RSSL_RET_SUCCESS;
				else if (isdigit(*tmp))
				{
					oTime->nanosecond = oTime->nanosecond + (*tmp - '0') * placeValue;
					placeValue = placeValue / 10;
				}
				else
				{
					memset(oTime, 0, sizeof(RsslTime));
					return RSSL_RET_INVALID_DATA;
				}
				tmp++;
			}

			/* We've parsed all available data, and are ignoring any time zone specification */
			return RSSL_RET_SUCCESS;
		}
		else
			return RSSL_RET_INVALID_DATA;
	}
	/* This covers: 
		hhmm
		hhmmss
		hhmmss.nnnnnnnnn
		hhmmss,nnnnnnnnn */
	else if (numberCount == 6 || numberCount == 4)
	{
		
		/* reset tmp to the beginning */
		tmp = iTimeString->data;
		/* Since we've verified that this is exactly 6 digits, we can just parse hhmmss */
		for (i = 0; i < 2; ++i)
		{
			oTime->hour = oTime->hour * 10 + (*tmp - '0');
			tmp++;
		}
		
		for (i = 0; i < 2; ++i)
		{
			oTime->minute = oTime->minute * 10 + (*tmp - '0');
			tmp++;
		}

		if(numberCount == 6)
		{
			for (i = 0; i < 2; ++i)
			{
				oTime->second = oTime->second * 10 + (*tmp - '0');
				tmp++;
			}

			/* Ignoring ISO time zone formatting */
			if (tmp == end || *tmp == '\0' || *tmp == 'Z' || *tmp == '+' || *tmp == '-')
			{
				return RSSL_RET_SUCCESS;
			}
			else if (*tmp == '.' || *tmp == ',')
			{
				++tmp;
				if (tmp == end)
					return RSSL_RET_INVALID_DATA;

				placeValue = 100;
				for (i = 0; i < 3; ++i)
				{
					if (tmp == end || *tmp == '\0' || *tmp == 'Z' || *tmp == '+' || *tmp == '-')
						return RSSL_RET_SUCCESS;
					else if (isdigit(*tmp))
					{
						oTime->millisecond = oTime->millisecond + (*tmp - '0') * placeValue;
						placeValue = placeValue / 10;
					}
					else
					{
						memset(oTime, 0, sizeof(RsslTime));
						return RSSL_RET_INVALID_DATA;
					}
					tmp++;
				}

				placeValue = 100;
				for (i = 0; i < 3; ++i)
				{
					if (tmp == end || *tmp == '\0' || *tmp == 'Z' || *tmp == '+' || *tmp == '-')
						return RSSL_RET_SUCCESS;
					else if (isdigit(*tmp))
					{
						oTime->microsecond = oTime->microsecond + (*tmp - '0') * placeValue;
						placeValue = placeValue / 10;
					}
					else
					{
						memset(oTime, 0, sizeof(RsslTime));
						return RSSL_RET_INVALID_DATA;
					}
					tmp++;
				}

				placeValue = 100;
				for (i = 0; i < 3; ++i)
				{
					if (tmp == end || *tmp == '\0' || *tmp == 'Z' || *tmp == '+' || *tmp == '-')
						return RSSL_RET_SUCCESS;
					else if (isdigit(*tmp))
					{
						oTime->nanosecond = oTime->nanosecond + (*tmp - '0') * placeValue;
						placeValue = placeValue / 10;
					}
					else
					{
						memset(oTime, 0, sizeof(RsslTime));
						return RSSL_RET_INVALID_DATA;
					}
					tmp++;
				}

				/* We've parsed all available data, and are ignoring any time zone specification */
				return RSSL_RET_SUCCESS;
			}
			else
				return RSSL_RET_INVALID_DATA;
		}
		else if (tmp == end || *tmp == '\0' || *tmp == 'Z' || *tmp == '+' || *tmp == '-')
		{
			return RSSL_RET_SUCCESS;
		}
		else
			return RSSL_RET_INVALID_DATA;
	}

	/* Catchall for remaining alternate formats */
	tmp = iTimeString->data;

	while (isspace(*tmp))
		tmp++; /* skip whitespace */
		
	/* if all whitespaces init to blank */
	if (tmp == end || *tmp == '\0')
	{
		oTime->hour = 255;
		oTime->minute = 255;
		oTime->second = 255;
		oTime->millisecond = 65535;
		oTime->microsecond = 2047;
		oTime->nanosecond = 2047;
		//printf("9\n");
		return RSSL_RET_BLANK_DATA;
	}

	for (u8 = 0; isdigit(*tmp); tmp++)
		u8 = u8 * 10 + (*tmp - '0');
	oTime->hour = u8;
	
	while ((*tmp == ':') || isspace(*tmp))
		tmp++;

	if (tmp == end || *tmp == '\0')
		return RSSL_RET_INVALID_DATA;

	for (u8 = 0; isdigit(*tmp); tmp++)
		u8 = u8 * 10 + (*tmp - '0');
	oTime->minute = u8;

	while ((*tmp == ':') || isspace(*tmp))
		tmp++;

	if ((tmp < end))
	{
		for (u8 = 0; isdigit(*tmp); tmp++)
			u8 = u8 * 10 + (*tmp - '0');
		oTime->second = u8;
	}

	while ((*tmp == ':') || isspace(*tmp))
		tmp++;

	if ((tmp < end))
	{
		for (u16 = 0; isdigit(*tmp); tmp++)
			u16 = u16 * 10 + (*tmp - '0');
		oTime->millisecond =u16;
	}

	while ((*tmp == ':') || isspace(*tmp))
		tmp++;

	if ((tmp < end))
	{
		for (u16 = 0; isdigit(*tmp); tmp++)
			u16 = u16 * 10 + (*tmp - '0');
		oTime->microsecond =u16;
	}

	while ((*tmp == ':') || isspace(*tmp))
		tmp++;

	if ((tmp < end))
	{
		for (u16 = 0; isdigit(*tmp); tmp++)
			u16 = u16 * 10 + (*tmp - '0');
		oTime->nanosecond =u16;
	}		
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslDateTimeStringToDateTime(RsslDateTime *oDateTime, const RsslBuffer *iDateTimeString)
{
	int hour = 0;
	int minute = 0;
	int second = 0;
	int millisecond = 0;
	int micro = 0;
	int nano = 0;
	int a, b, c;
	char monthStr[32];
	char* tmp;
	char* end;
	int curr = 0;
	int tlocation = -1;
	RsslBuffer tmpBuf;
	RsslRet ret;

	if (iDateTimeString->data == NULL || iDateTimeString->length == 0)
	{
		oDateTime->date.day = 0;
		oDateTime->date.month = 0;
		oDateTime->date.year = 0;

		oDateTime->time.hour = 255;
		oDateTime->time.minute = 255;
		oDateTime->time.second = 255;
		oDateTime->time.millisecond = 65535;
		oDateTime->time.microsecond = 2047;
		oDateTime->time.nanosecond = 2047;
		
		return RSSL_RET_BLANK_DATA;
	}

	rsslClearDateTime(oDateTime);

	tmp = iDateTimeString->data;
	end = iDateTimeString->data + iDateTimeString->length;

	/* Find out if the string is ISO 8601 encoded.  This is the case if the string has a 'T' in it, and no spaces. */
	while (tmp < end)
	{
		if (*tmp == 'T')
		{
			tlocation = curr;
			break;
		}
		else if (isspace(*tmp))
		{
			tlocation = -1;
			break;
		}
		tmp++;
		curr++;
	}

	/* If there is a 'T' character and no space in the buffer, then setup the temp buffer for the beginning to the 'T' character for date */
	if (tlocation != -1)
	{
		/* Get the date out of the string. */
		tmpBuf.data = iDateTimeString->data;
		tmpBuf.length = tlocation;
		ret = rsslDateStringToDate(&(oDateTime->date), &tmpBuf);
		if (ret != RSSL_RET_SUCCESS)
		{
			rsslClearDateTime(oDateTime);
			return ret;
		}

		/* Get the time out of the string */
		/* check to make sure there is anything left in the string after the 'T' */
		if ((RsslUInt32)tlocation + 1 >= iDateTimeString->length)
		{
			rsslClearDateTime(oDateTime);
			return RSSL_RET_INVALID_DATA;
		}

		tmpBuf.data = iDateTimeString->data + tlocation + 1;
		tmpBuf.length = iDateTimeString->length - tlocation - 1;

		ret = rsslTimeStringToTime(&(oDateTime->time), &tmpBuf);
		if (ret != RSSL_RET_SUCCESS)
		{
			rsslClearDateTime(oDateTime);
			return ret;
		}

		return RSSL_RET_SUCCESS;
	}


	if (sscanf(iDateTimeString->data, "%d/%d/%d %d:%d:%d:%d:%d:%d", &a, &b, &c, &hour, &minute, &second, &millisecond, &micro, & nano) >= 5)
	{
		// 1974/04/14
		//
		if (a > 255) // assume year here is greater than MAX UINT8
		{
			oDateTime->date.day = c;
			oDateTime->date.month = b;
			oDateTime->date.year = a;

			oDateTime->time.hour = hour;
			oDateTime->time.minute = minute;
			oDateTime->time.second = second;
			oDateTime->time.millisecond = millisecond;
			oDateTime->time.microsecond = micro;
			oDateTime->time.nanosecond = nano;
		}
		// 04/14/74
		//
		else if (c < 100) // assume year here is less than 100, then add 1900
		{
			oDateTime->date.day = b;
			oDateTime->date.month = a;
			oDateTime->date.year = c + 1900;

			oDateTime->time.hour = hour;
			oDateTime->time.minute = minute;
			oDateTime->time.second = second;
			oDateTime->time.millisecond = millisecond;
			oDateTime->time.microsecond = micro;
			oDateTime->time.nanosecond = nano;
		}
		// 04/14/1974
		//
		else
		{
			oDateTime->date.day = b;
			oDateTime->date.month = a;
			oDateTime->date.year = c;

			oDateTime->time.hour = hour;
			oDateTime->time.minute = minute;
			oDateTime->time.second = second;
			oDateTime->time.millisecond = millisecond;
			oDateTime->time.microsecond = micro;
			oDateTime->time.nanosecond = nano;
		}
		return RSSL_RET_SUCCESS;
	}	

	if (isdigit(iDateTimeString->data[3]))
	{
		if (sscanf(iDateTimeString->data, "%d %d %d %d %d %d %d %d %d", &a, &b, &c, &hour, &minute, &second, &millisecond, &micro, &nano) >= 5)
		{
			if (a > 255) // assume year here is greater than MAX UINT8
			{
				oDateTime->date.day = c;
				oDateTime->date.month = b;
				oDateTime->date.year = a;

				oDateTime->time.hour = hour;
				oDateTime->time.minute = minute;
				oDateTime->time.second = second;
				oDateTime->time.millisecond = millisecond;
				oDateTime->time.microsecond = micro;
				oDateTime->time.nanosecond = nano;
			}
			else if (c < 100) // assume year here is less than 100, then add 1900
			{
				oDateTime->date.day = b;
				oDateTime->date.month = a;
				oDateTime->date.year = c + 1900;

				oDateTime->time.hour = hour;
				oDateTime->time.minute = minute;
				oDateTime->time.second = second;
				oDateTime->time.millisecond = millisecond;
				oDateTime->time.microsecond = micro;
				oDateTime->time.nanosecond = nano;
			}
			else
			{
				oDateTime->date.day = b;
				oDateTime->date.month = a;
				oDateTime->date.year = c;

				oDateTime->time.hour = hour;
				oDateTime->time.minute = minute;
				oDateTime->time.second = second;
				oDateTime->time.millisecond = millisecond;
				oDateTime->time.microsecond = micro;
				oDateTime->time.nanosecond = nano;
			}
			return RSSL_RET_SUCCESS;
		}	
	}
	else if (isalpha(iDateTimeString->data[3]))
	{
		if (sscanf(iDateTimeString->data, "%d %3s %d %d %d %d %d %d %d", &a, monthStr, &c, &hour, &minute, &second, &millisecond, &micro, &nano) >= 5)
		{
			if (c < 100) // assume year here is less than 100, then add 1900
			{
				oDateTime->date.day = a;
				oDateTime->date.month = translateMonth(monthStr);
				oDateTime->date.year = c + 1900;

				oDateTime->time.hour = hour;
				oDateTime->time.minute = minute;
				oDateTime->time.second = second;
				oDateTime->time.millisecond = millisecond;
				oDateTime->time.microsecond = micro;
				oDateTime->time.nanosecond = nano;
			}
			else
			{
				oDateTime->date.day = a;
				oDateTime->date.month = translateMonth(monthStr);
				oDateTime->date.year = c;

				oDateTime->time.hour = hour;
				oDateTime->time.minute = minute;
				oDateTime->time.second = second;
				oDateTime->time.millisecond = millisecond;
				oDateTime->time.microsecond = micro;
				oDateTime->time.nanosecond = nano;
			}
			return RSSL_RET_SUCCESS;
		}
	}

	if (sscanf(iDateTimeString->data, "%d/%d/%d %d %d %d %d %d %d", &a, &b, &c, &hour, &minute, &second, &millisecond, &micro, &nano) >= 5)
	{
		// 1974/04/14
		//
		if (a > 255) // assume year here is greater than MAX UINT8
		{
			oDateTime->date.day = c;
			oDateTime->date.month = b;
			oDateTime->date.year = a;

			oDateTime->time.hour = hour;
			oDateTime->time.minute = minute;
			oDateTime->time.second = second;
			oDateTime->time.millisecond = millisecond;
			oDateTime->time.microsecond = micro;
			oDateTime->time.nanosecond = nano;
		}
		// 04/14/74
		//
		else if (c < 100) // assume year here is less than 100, then add 1900
		{
			oDateTime->date.day = b;
			oDateTime->date.month = a;
			oDateTime->date.year = c + 1900;

			oDateTime->time.hour = hour;
			oDateTime->time.minute = minute;
			oDateTime->time.second = second;
			oDateTime->time.millisecond = millisecond;
			oDateTime->time.microsecond = micro;
			oDateTime->time.nanosecond = nano;
		}
		// 04/14/1974
		//
		else
		{
			oDateTime->date.day = b;
			oDateTime->date.month = a;
			oDateTime->date.year = c;

			oDateTime->time.hour = hour;
			oDateTime->time.minute = minute;
			oDateTime->time.second = second;
			oDateTime->time.millisecond = millisecond;
			oDateTime->time.microsecond = micro;
			oDateTime->time.nanosecond = nano;
		}
		return RSSL_RET_SUCCESS;
	}	

	if (isdigit(iDateTimeString->data[3]))
	{
		if (sscanf(iDateTimeString->data, "%d %d %d %d:%d:%d:%d:%d:%d", &a, &b, &c, &hour, &minute, &second, &millisecond, &micro, &nano) >= 5)
		{
			if (a > 255) // assume year here is greater than MAX UINT8
			{
				oDateTime->date.day = c;
				oDateTime->date.month = b;
				oDateTime->date.year = a;

				oDateTime->time.hour = hour;
				oDateTime->time.minute = minute;
				oDateTime->time.second = second;
				oDateTime->time.millisecond = millisecond;
				oDateTime->time.microsecond = micro;
				oDateTime->time.nanosecond = nano;
			}
			else if (c < 100) // assume year here is less than 100, then add 1900
			{
				oDateTime->date.day = b;
				oDateTime->date.month = a;
				oDateTime->date.year = c + 1900;

				oDateTime->time.hour = hour;
				oDateTime->time.minute = minute;
				oDateTime->time.second = second;
				oDateTime->time.millisecond = millisecond;
				oDateTime->time.microsecond = micro;
				oDateTime->time.nanosecond = nano;
			}
			else
			{
				oDateTime->date.day = b;
				oDateTime->date.month = a;
				oDateTime->date.year = c;

				oDateTime->time.hour = hour;
				oDateTime->time.minute = minute;
				oDateTime->time.second = second;
				oDateTime->time.millisecond = millisecond;
				oDateTime->time.microsecond = micro;
				oDateTime->time.nanosecond = nano;
			}
			return RSSL_RET_SUCCESS;
		}	
	}
	else if (isalpha(iDateTimeString->data[3]))
	{
		if (sscanf(iDateTimeString->data, "%d %3s %d %d:%d:%d:%d:%d:%d", &a, monthStr, &c, &hour, &minute, &second, &millisecond, &micro, &nano) >= 5)
		{
			if (c < 100) // assume year here is less than 100, then add 1900
			{
				oDateTime->date.day = a;
				oDateTime->date.month = translateMonth(monthStr);
				oDateTime->date.year = c + 1900;

				oDateTime->time.hour = hour;
				oDateTime->time.minute = minute;
				oDateTime->time.second = second;
				oDateTime->time.millisecond = millisecond;
				oDateTime->time.microsecond = micro;
				oDateTime->time.nanosecond = nano;
			}
			else
			{
				oDateTime->date.day = a;
				oDateTime->date.month = translateMonth(monthStr);
				oDateTime->date.year = c;

				oDateTime->time.hour = hour;
				oDateTime->time.minute = minute;
				oDateTime->time.second = second;
				oDateTime->time.millisecond = millisecond;
				oDateTime->time.microsecond = micro;
				oDateTime->time.nanosecond = nano;
			}
			return RSSL_RET_SUCCESS;
		}
	}

	if (iDateTimeString->length > 0)
	{
		char* tmp = iDateTimeString->data;

		while (isspace(*tmp))
			tmp++; /* skip whitespace */
		
		/* if all whitespaces init to blank */
		if (tmp == iDateTimeString->data + iDateTimeString->length)
		{
			oDateTime->date.day = 0;
			oDateTime->date.month = 0;
			oDateTime->date.year = 0;

			oDateTime->time.hour = 255;
			oDateTime->time.minute = 255;
			oDateTime->time.second = 255;
			oDateTime->time.millisecond = 65535;
			oDateTime->time.microsecond = 2047;
			oDateTime->time.nanosecond = 2047;

			return RSSL_RET_BLANK_DATA;
		}

		if (rwf_stodatetime(oDateTime, iDateTimeString->data) < 0)
		{
			return RSSL_RET_INVALID_DATA;
		}
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslDateTimeGmtTime(RsslDateTime * oDateTime)
{
	time_t now;
	struct tm t;
	time(&now);
#if defined(_WIN32) || defined(WIN32)
	t = *gmtime( &now );
#else
	t = *gmtime_r( &now, &t);
#endif
	oDateTime->date.day = t.tm_mday;
	oDateTime->date.month = t.tm_mon + 1;
	oDateTime->date.year = 1900 + t.tm_year;
	oDateTime->time.hour = t.tm_hour;
	oDateTime->time.minute = t.tm_min;
	oDateTime->time.second = t.tm_sec;
	oDateTime->time.millisecond = 0;
	oDateTime->time.microsecond = 0;
	oDateTime->time.nanosecond = 0;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslDateTimeLocalTime(RsslDateTime * oDateTime)
{
	time_t now;
	struct tm t;
	time(&now);
#if defined(_WIN32) || defined(WIN32)
	t = *localtime( &now );
#else
	t = *localtime_r( &now, &t);
#endif
	oDateTime->date.day = t.tm_mday;
	oDateTime->date.month = t.tm_mon + 1;
	oDateTime->date.year = 1900 + t.tm_year;
	oDateTime->time.hour = t.tm_hour;
	oDateTime->time.minute = t.tm_min;
	oDateTime->time.second = t.tm_sec;
	oDateTime->time.millisecond = 0;
	oDateTime->time.microsecond = 0;
	oDateTime->time.nanosecond = 0;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslDateTimeToStringFormat(RsslBuffer * oBuffer, RsslUInt8 dataType, RsslDateTime * iDateTime, RsslUInt8 format)
{
	RsslRet ret = 0;
	int remainingLength = (int)oBuffer->length;
	int bufOffset = 0;
	
	switch(dataType)
	{
		case RSSL_DT_DATETIME:
		{
			/* check for blank date and time */
			if(dateTimeIsBlankToStr( oBuffer, iDateTime) == RSSL_TRUE)
				return RSSL_RET_SUCCESS;

			/* Date portion of the DateTime to string conversion */
			if( dateIsBlankToStr(oBuffer, &(iDateTime->date)) == RSSL_FALSE)
			{ /* Date is not blank */
				if (!rsslDateIsValid(&iDateTime->date))
				{
					ret = snprintf(oBuffer->data, remainingLength, "Invalid date");
					if (ret < 0 || ret >= remainingLength)
						return RSSL_RET_FAILURE;
					bufOffset += ret;
					remainingLength -= ret;
				}
				else 
				{ /* Valid date -convert to string according to the format */
					if( format == RSSL_STR_DATETIME_ISO8601 )
					{
						if(dateToStringIso8601(oBuffer, &bufOffset, &remainingLength, &(iDateTime->date)) ==  RSSL_RET_FAILURE)
							return RSSL_RET_FAILURE;
					}
					else if( format == RSSL_STR_DATETIME_RSSL )
					{
						if(dateToStringRssl(oBuffer, &bufOffset, &remainingLength, &(iDateTime->date)) ==  RSSL_RET_FAILURE)
							return RSSL_RET_FAILURE;

					}
					else
					{
						ret = snprintf(oBuffer->data, remainingLength, "Invalid DateTime format value %d", format);
						if (ret < 0 || ret >= remainingLength)
							return RSSL_RET_FAILURE;
						bufOffset += ret;
						remainingLength -= ret;
					}
				}
			}	

			/* Time portion of the DateTime to string conversion according to the format */
			if( format == RSSL_STR_DATETIME_ISO8601 )
			{ 
				if(timeIsBlankToStr(oBuffer, RSSL_FALSE, &(iDateTime->time)) == RSSL_TRUE)
					return RSSL_RET_SUCCESS;

				/* Put in Time delimiter 'T' for ISO8601 first */
				ret = snprintf((oBuffer->data + bufOffset), remainingLength, "T");
				if (ret < 0 || ret >= remainingLength)
					return RSSL_RET_FAILURE;

				bufOffset += ret;
				remainingLength -= ret;
				if(timeToStringIso8601Time(oBuffer, &bufOffset, &remainingLength, &(iDateTime->time)) ==  RSSL_RET_FAILURE)
					return RSSL_RET_FAILURE;

			}
			else if( format == RSSL_STR_DATETIME_RSSL )
			{
				/* Put in a space first */
				ret = snprintf((oBuffer->data + bufOffset), remainingLength, " "); 
				if (ret < 0 || ret >= remainingLength)
					return RSSL_RET_FAILURE;

				bufOffset += ret;
				remainingLength -= ret;
				if(timeIsBlankToStr(oBuffer, RSSL_FALSE, &(iDateTime->time)) == RSSL_TRUE)
					return RSSL_RET_SUCCESS;

				if(timeToStringRssltime(oBuffer, &bufOffset, &remainingLength, &(iDateTime->time)) ==  RSSL_RET_FAILURE)
					return RSSL_RET_FAILURE;
			}
			else 
			{
				ret = snprintf(oBuffer->data, remainingLength, "Invalid DateTime format value %d", format);
				if (ret < 0 || ret >= remainingLength)
					return RSSL_RET_FAILURE;

			}

			bufOffset += ret;
			remainingLength -= ret;
		}
		break;

		case RSSL_DT_DATE:
		{	
			if( dateIsBlankToStr(oBuffer, &(iDateTime->date)) ==  RSSL_TRUE)
				return RSSL_RET_SUCCESS;

			if (!rsslDateIsValid(&iDateTime->date))
			{
				ret = snprintf(oBuffer->data, remainingLength, "Invalid date");
				if (ret < 0 || ret >= remainingLength)
					return RSSL_RET_FAILURE;

				bufOffset += ret;
				remainingLength -= ret;
			}
			else
			{ /* convert date to string according to the format */
				if( format == RSSL_STR_DATETIME_ISO8601 )
				{
					if(dateToStringIso8601(oBuffer, &bufOffset, &remainingLength, &(iDateTime->date)) ==  RSSL_RET_FAILURE)
						return RSSL_RET_FAILURE;
				}
				else if( format == RSSL_STR_DATETIME_RSSL )
				{
					if(dateToStringRssl(oBuffer, &bufOffset, &remainingLength, &(iDateTime->date)) ==  RSSL_RET_FAILURE)
						return RSSL_RET_FAILURE;

				}
				else
				{
					ret = snprintf(oBuffer->data, remainingLength, "Invalid date format value %d", format);
					if (ret < 0 || ret >= remainingLength)
						return RSSL_RET_FAILURE;
					bufOffset += ret;
					remainingLength -= ret;
				}
			}					
		}
		break;

		case RSSL_DT_TIME:
		{
			if(timeIsBlankToStr(oBuffer, RSSL_TRUE, &(iDateTime->time)) == RSSL_TRUE)
				return RSSL_RET_SUCCESS;
			
			/* Convert Time to string according to the format */
			/* have to do this piece by piece to handle the various trailing portions being blank */
			if( format == RSSL_STR_DATETIME_ISO8601 )
			{ 
				if(timeToStringIso8601Time(oBuffer, &bufOffset, &remainingLength, &(iDateTime->time)) ==  RSSL_RET_FAILURE)
					return RSSL_RET_FAILURE;

			}
			else if( format == RSSL_STR_DATETIME_RSSL )
			{
				if(timeToStringRssltime(oBuffer, &bufOffset, &remainingLength, &(iDateTime->time)) ==  RSSL_RET_FAILURE)
					return RSSL_RET_FAILURE;
			}
			else 
			{
				ret = snprintf(oBuffer->data, remainingLength, "Invalid DateTime format value %d", format);
				if (ret < 0 || ret >= remainingLength)
					return RSSL_RET_FAILURE;

			}

			bufOffset += ret;
			remainingLength -= ret;
		}
		break;

		default:
			snprintf(oBuffer->data, remainingLength, "Invalid dataType %d ", dataType);
			return RSSL_RET_UNSUPPORTED_DATA_TYPE;
		break;
	}

	/* set length on out buffer */
	oBuffer->length = (RsslUInt32) strlen(oBuffer->data);

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslDateTimeToString(RsslBuffer * oBuffer, RsslDataType type, RsslDateTime * iDateTime)
{
	return rsslDateTimeToStringFormat(oBuffer, type, iDateTime, RSSL_STR_DATETIME_RSSL);
}
