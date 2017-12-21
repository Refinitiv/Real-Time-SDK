
#ifndef __RSSL_DATETIME_H
#define __RSSL_DATETIME_H


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTypes.h"

/** 
 * @addtogroup RsslDateType
 * @{
 */

/**
 * @brief The RsslDate type allows for bandwidth optimized representation of a date value containing month, day, and year information.
 * @see RSSL_INIT_DATE, RSSL_BLANK_DATE, rsslClearDate, rsslBlankDate, rsslEncodeDate, rsslDecodeDate
 */
typedef struct
{
	RsslUInt8 day;		/*!< @brief Day of the month (0 - 31 where 0 indicates blank) */
	RsslUInt8 month;	/*!< @brief Month of the year (0 - 12 where 0 indicates blank) */
	RsslUInt16 year;	/*!< @brief Year (0 - 4095 where 0 indicates blank) */
} RsslDate;

/** 
 * @brief RsslDate, RsslTime and RsslDateTime to string conversion format enumeration values.
 * @see rsslDateTimeToStringFormat
 */
typedef enum {
	RSSL_STR_DATETIME_ISO8601	    = 1,	/*!< (1) Date/Time/DateTime to string output in ISO8601's dateTime format */
	RSSL_STR_DATETIME_RSSL 	= 2		/*!< (2) Date/Time/DateTime to string output in the format of the deprecated rsslDateTimeToString function.*/
} RsslDateTimeStringFormatTypes;

/**
 * @brief RsslDate static initializer
 * @see RsslDate, rsslClearDate
 */
#define RSSL_INIT_DATE {0, 0, 0}

/**
 * @brief A blank RsslDate, same as initialized or cleared RsslDate
 * @see RsslDate
 */
#define RSSL_BLANK_DATE {0, 0, 0}

/**
 * @brief Clears an RsslDate 
 * @param pDate Pointer to RsslDate structure to clear
 * @see RsslDate, RSSL_INIT_DATE
 */
RTR_C_INLINE void rsslClearDate( RsslDate *pDate )
{
	pDate->day = 0;
	pDate->month = 0;
	pDate->year = 0;
}

/** 
 * @brief Sets an RsslDate to blank 
 * @param pDate Pointer to RsslDate structure to set to blank
 * @see RsslDate, RSSL_BLANK_DATE
 */
RTR_C_INLINE void rsslBlankDate( RsslDate *pDate )
{
	pDate->day = 0;
	pDate->month = 0;
	pDate->year = 0;
}

/**
 * @}
 */



/** 
 * @addtogroup RsslTimeType
 * @{
 */

/**
 * @brief The RsslTime type allows for bandwidth optimized representation of a time value containing hour, minute, second, millisecond, microsecond, and nanosecond information.
 * @see RSSL_INIT_TIME, RSSL_BLANK_TIME, rsslClearTime, rsslBlankTime, rsslEncodeTime, rsslDecodeTime
 * RsslTime is always represented as GMT unless otherwise specified in product, data, or domain specific documentation
 */
typedef struct
{
	RsslUInt8 hour;			/*!<  @brief The hour of the day (0 - 23 where 255 indicates blank) */
	RsslUInt8 minute;		/*!<  @brief The minute of the hour (0 - 59 where 255 indicates blank) */
	RsslUInt8 second;		/*!<  @brief The second of the minute (0 - 60 where 255 indicates blank and 60 is to account for leap second) */
	RsslUInt16 millisecond;	/*!<  @brief The millisecond of the second (0 - 999 where 65535 indicates blank)*/
	RsslUInt16 microsecond;	/*!<  @brief The microsecond of the millisecond (0 - 999 where 2047 indicates blank)*/
	RsslUInt16 nanosecond;	/*!<  @brief The nanosecond of the microsecond (0 - 999 where 2047 indicates blank)*/
} RsslTime;


/**
 * @brief RsslTime static initializer
 * @see RsslTime, rsslClearTime
 */
#define RSSL_INIT_TIME {0, 0, 0, 0, 0, 0}

/**
 * @brief RsslTime static blank initializer
 * @see RsslTime, rsslBlankTime
 */
#define RSSL_BLANK_TIME {255, 255, 255, 65535, 2047, 2047}

/**
 * @brief Clears an RsslTime structure
 * @param pTime RsslTime structure to clear
 * @see RsslTime, RSSL_INIT_TIME
 */
RTR_C_INLINE void rsslClearTime( RsslTime *pTime )
{
	pTime->hour = 0;
	pTime->minute = 0;
	pTime->second = 0;
	pTime->millisecond = 0;
	pTime->microsecond = 0;
	pTime->nanosecond = 0;
}

/**
 * @brief Sets an RsslTime structure to blank
 * @param pTime RsslTime structure to set to blank
 * @see RsslTime, RSSL_BLANK_TIME
 */
RTR_C_INLINE void rsslBlankTime( RsslTime *pTime )
{
	pTime->hour = 255;
	pTime->minute = 255;
	pTime->second = 255;
	pTime->millisecond = 65535;
	pTime->microsecond = 2047;
	pTime->nanosecond = 2047;
}

/**
 * @}
 */

/** 
 * @addtogroup RsslDateTimeType
 * @{
 */


/**
 * @brief The RsslDateTime type allows for bandwidth optimized representation of both a date and a time value containing month, day, year, hour, minute, second, millisecond, microsecond, and nanosecond information.
 * @see RSSL_INIT_DATETIME, RSSL_BLANK_DATETIME, rsslClearDateTime, rsslBlankDateTime, rsslEncodeDateTime, rsslDecodeDateTime
 */
typedef struct
{
	RsslDate		date;	/*!< @brief RsslDate portion of the RsslDateTime */
	RsslTime		time;	/*!< @brief RsslTime portion of the RsslDateTime */
} RsslDateTime;

/**
 * @brief RsslDateTime static initializer, sets all values to 0
 * @see RsslDateTime, rsslClearDateTime
 */
#define RSSL_INIT_DATETIME {RSSL_INIT_DATE, RSSL_INIT_TIME}

/**
 * @brief RsslDateTime static blank initializer function, sets all date values to 0, time hour/minute/second to 255 and millisecond to 65535.
 * @see RsslDateTime, rsslBlankDateTime
 */
#define RSSL_BLANK_DATETIME {RSSL_BLANK_DATE, RSSL_BLANK_TIME}


/**
 * @brief Clears an RsslDateTime 
 * @param pDateTime RsslDateTime structure to clear
 * @see RsslDateTime, RSSL_INIT_DATETIME
 */
RTR_C_INLINE void rsslClearDateTime( RsslDateTime *pDateTime )
{
	pDateTime->date.day = 0;
	pDateTime->date.month = 0;
	pDateTime->date.year = 0;
	pDateTime->time.hour = 0;
	pDateTime->time.minute = 0;
	pDateTime->time.second = 0;
	pDateTime->time.millisecond = 0;
	pDateTime->time.microsecond = 0;
	pDateTime->time.nanosecond = 0;
}

/**
 * @brief Sets an RsslDateTime structure to blank
 * @param pDateTime RsslDateTime structure to set to blank
 * @see RsslDateTime, RSSL_BLANK_DATETIME
 */
RTR_C_INLINE void rsslBlankDateTime( RsslDateTime *pDateTime )
{
	pDateTime->date.day = 0;
	pDateTime->date.month = 0;
	pDateTime->date.year = 0;
	pDateTime->time.hour = 255;
	pDateTime->time.minute = 255;
	pDateTime->time.second = 255;
	pDateTime->time.millisecond = 65535;
	pDateTime->time.microsecond = 2047;
	pDateTime->time.nanosecond = 2047;
}

/**
 * @}
 */

/**
 *	@addtogroup RsslDateUtils
 *	@{
 */

/**
 * @brief Verifies that day, month, and year information is valid (e.g. values that correspond to actual month or day information)
 * @param iDate RsslDate to check for validity
 * @return RsslBool RSSL_TRUE if validly populated, RSSL_FALSE otherwise
 * @see RsslDate
 */
RSSL_API RsslBool rsslDateIsValid(const RsslDate * iDate);

/**
 * @brief Checks equality of two RsslDate structures
 * @param lhs Pointer to the left hand side RsslDate structure.
 * @param rhs Pointer to the right hand side RsslDate structure.
 * @return RsslBool RSSL_TRUE if both RsslDate structures are equal, RSSL_FALSE otherwise
 * @see RsslDate
 */
RTR_C_ALWAYS_INLINE RsslBool rsslDateIsEqual( const RsslDate *lhs, const RsslDate *rhs )
{
	return ((lhs->day == rhs->day) && (lhs->month == rhs->month) &&
			(lhs->year == rhs->year) ? RSSL_TRUE : RSSL_FALSE);
}


/**
 * @brief Converts string date in strftime()'s "%d %b %Y" format (e.g. 01 JUN 2003) or "%m/%d/%y" to RsslDate Or in ISO 8601's "YYYY-MM-DD' format (e.g. 2003-06-01) 
 * @param oDate RsslDate structure to populate from string
 * @param iDateString RsslBuffer containing an appropriately formatted string to convert from.  RsslBuffer::data should point to date string, RsslBuffer::length should indicate the number of bytes pointed to.
 * @return RsslRet RSSL_RET_SUCCESS if successful, RSSL_RET_FAILURE otherwise  
 * @see RsslDate, RsslBuffer
 */
RSSL_API RsslRet rsslDateStringToDate(RsslDate * oDate, const RsslBuffer * iDateString);



/**
 * @}
 */


/**
 *	@addtogroup RsslTimeUtils
 *	@{
 */


/**
 * @brief Verifies that hour, minute, second, and millisecond information is valid (e.g. values that correspond to actual time information).  Allows seconds to be set up to 60 to account for periodic leap seconds.
 * @param iTime RsslTime to check validity
 * @return RsslBool RSSL_TRUE if validly populated, RSSL_FALSE otherwise
 * @see RsslTime
 */
RSSL_API RsslBool rsslTimeIsValid(const RsslTime * iTime);


/**
 * @brief Checks equality of two RsslTime structures
 * @param lhs Pointer to the left hand side RsslTime structure.
 * @param rhs Pointer to the right hand side RsslTime structure.
 * @return RsslBool RSSL_TRUE if both RsslTime structures are equal, RSSL_FALSE otherwise
 * @see RsslTime
 */
RTR_C_ALWAYS_INLINE RsslBool rsslTimeIsEqual( const RsslTime *lhs, const RsslTime *rhs )
{
	return ((lhs->second == rhs->second) && (lhs->minute == rhs->minute) &&
			(lhs->hour == rhs->hour) && (lhs->millisecond == rhs->millisecond) &&
			(lhs->microsecond == rhs->microsecond) && (lhs->nanosecond == rhs->nanosecond) ?
				RSSL_TRUE : RSSL_FALSE);
}


/**
 * @brief Converts string time in strftime()'s "%H:%M" or "%H:%M:%S" format (e.g. 13:01 or 15:23:54) to RsslTime Or in ISO 8601 format (e.g 15:23:54.3549) to RsslTime
 * @param oTime RsslTime structure to have populated from string
 * @param iTimeString RsslBuffer containing an appropriately formatted string to convert from.  RsslBuffer::data should point to time string, RsslBuffer::length should indicate the number of bytes pointed to.
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_FAILURE otherwise
 * @see RsslTime, RsslBuffer
 */
RSSL_API RsslRet rsslTimeStringToTime(RsslTime * oTime, const RsslBuffer * iTimeString);



/**
 * @}
 */

/**
 *	@addtogroup RsslDateTimeUtils
 *	@{
 */


/**
 * @brief Verifies that day, month, year, hour, minute, second, and millisecond information is valid (e.g. values that correspond to actual date and time information)
 * @param iDateTime RsslDateTime to check validity
 * @return RsslBool RSSL_TRUE if validly populated, RSSL_FALSE otherwise
 * @see RsslDateTime
 */
RSSL_API RsslBool rsslDateTimeIsValid(const RsslDateTime * iDateTime);


/**
 * @brief Checks equality of two RsslDateTime structures
 * @param lhs Pointer to the left hand side RsslDateTime structure.
 * @param rhs Pointer to the right hand side RsslDateTime structure.
 * @return RsslBool RSSL_TRUE if both RsslDateTime structures are equal, RSSL_FALSE otherwise
 * @see RsslDateTime
 */
RTR_C_ALWAYS_INLINE RsslBool rsslDateTimeIsEqual( const RsslDateTime *lhs, const RsslDateTime *rhs )
{
	return ((lhs->time.second == rhs->time.second) &&
			(lhs->time.minute == rhs->time.minute) &&
			(lhs->time.hour == rhs->time.hour) &&
			(lhs->time.millisecond == rhs->time.millisecond) &&
			(lhs->time.microsecond == rhs->time.microsecond) &&
			(lhs->time.nanosecond == rhs->time.nanosecond) &&
			(lhs->date.day == rhs->date.day) &&
			(lhs->date.month == rhs->date.month) &&
			(lhs->date.year == rhs->date.year) ?  RSSL_TRUE : RSSL_FALSE);
}


/**
 * @brief Converts DateTime string to RsslDateTime, expects date before time, where date is formatted in in strftime()'s "%d %b %Y" format (e.g. 01 JUN 2003) or "%m/%d/%y" and time is formatted in strftime()'s "%H:%M" or "%H:%M:%S" format (e.g. 13:01 or 15:23:54), Or in ISO 8601's dateTime format
 * @param oDateTime RsslDateTime structure to populate from string
 * @param iDateTimeString RsslBuffer containing an appropriately formatted string to convert from
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_FAILURE otherwise
 * @see RsslDateTime, RsslBuffer
 */
RSSL_API RsslRet rsslDateTimeStringToDateTime(RsslDateTime *oDateTime, const RsslBuffer *iDateTimeString);

/**
 * @brief DEPRECATED: Converts a populated RsslDateTime structure to a string. Users should migrate to use rsslDateTimeToStringFormat.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param dataType Either RSSL_DT_DATE, RSSL_DT_TIME or RSSL_DT_DATETIME depending on which portion(s) to convert to string
 * @param iDateTime RsslDateTime structure to convert to string
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_FAILURE otherwise
 */
RSSL_API RsslRet rsslDateTimeToString(RsslBuffer * oBuffer, RsslUInt8 dataType, RsslDateTime * iDateTime);

/**
 * @brief Converts a populated RsslDateTime structure to a string in the specifed format. 
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param dataType Either RSSL_DT_DATE, RSSL_DT_TIME or RSSL_DT_DATETIME depending on which portion(s) to convert to string
 * @param iDateTime RsslDateTime structure to convert to string
 * @param format RsslUInt8 format of output string.
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_FAILURE otherwise
 * @see RsslDateTimeStringFormatTypes
 */
RSSL_API RsslRet rsslDateTimeToStringFormat(RsslBuffer * oBuffer, RsslUInt8 dataType, RsslDateTime * iDateTime, RsslUInt8 format);

/**
 * @brief Set the given RsslDateTime to now/current time and date in the local time zone.
 * @param oDateTime RsslDateTime structure to populate with local time
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_FAILURE otherwise  
 * @see RsslDateTime
 */
RSSL_API RsslRet rsslDateTimeLocalTime(RsslDateTime * oDateTime);

/**
 * @brief Set the given RsslDateTime to now/current time and date in GMT.
 * @param oDateTime RsslDateTime structure to populate with GMT time
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_FAILURE otherwise  
 * @see RsslDateTime
 */
RSSL_API RsslRet rsslDateTimeGmtTime(RsslDateTime * oDateTime);

/**
 * @}
 */
 


#ifdef __cplusplus
}
#endif 

#endif

