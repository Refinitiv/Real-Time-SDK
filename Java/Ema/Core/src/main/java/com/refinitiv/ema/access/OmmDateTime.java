///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/**
 * OmmDateTime represents DateTime info in Omm.
 * <br>OmmDateTime encapsulates year, month, day, hour, minute, second, millisecond,
 * microsecond and nanosecond information.
 * 
 * OmmDateTime is a read only class.
 * 
 * The following code snippet shows extraction of dateTime from ElementList and use DateTimeStringFormat to
 * print DateTime in a specified format.
 * void decode(ElementList eList)
 *	{
 *      DateTimeStringFormat dateTimeStrFmt = EmaFactory.createDateTimeStringFormat();
 *		for (ElementEntry elementEntry : eList)
 *		{
 *			switch (elementEntry.load().dataType() )
 *			{
 *			case DataTypes.DATETIME:
 *				dateTimeStrFmt.format(DateTimeStringFormatTypes.STR_DATETIME_ISO8601);
 *              System.out.println(dateTimeStrFmt.dateTimeAsString((OmmDateTime)elementEntry.load()));
 *              break;
 *			}				
 *		}
 *	}
 *
 * @see Data
 * @see DateTimeStringFormat
 */
public interface OmmDateTime extends Data
{
	/**
	 * Returns Year. Range is 0 - 4095 where 0 indicates blank.
	 * @return value of year
	 */
	public int year();

	/**
	 * Returns Month. Range is 0 - 12 where 0 indicates blank.
	 * @return value of month
	 */
	public int month();

	/**
	 * Returns Day. Range is 0 - 31 where 0 indicates blank.
	 * @return value of day
	 */
	public int day();

	/**
	 * Returns Hour. Range is 0 - 23 where 255 indicates blank.
	 * @return value of hour
	 */
	public int hour();

	/**
	 * Returns Minute. Range is 0 - 59 where 255 indicates blank.
	 * @return value of minutes
	 */
	public int minute();

	/**
	 * Returns Second. Range is 0 - 60 where 255 indicates blank and 60 is to account for leap second.
	 * @return value of seconds
	 */
	public int second();

	/**
	 * Returns Millisecond. Range is 0 - 999 where 65535 indicates blank.
	 * @return value of milliseconds
	 */
	public int millisecond();

	/**
	 * Returns Microsecond. Range is 0 - 999 where 2047 indicates blank.
	 * @return value of microseconds
	 */
	public int microsecond();

	/**
	 * Returns Nanosecond. Range is 0 - 999 where 2047 indicates blank.
	 * @return value of nanoseconds
	 */
	public int nanosecond();
}
