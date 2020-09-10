///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;

/**
 * OmmTime represents Time info in Omm.<br>
 * OmmTime encapsulates hour, minute, second, millisecond, microsecond and nanosecond information.
 * 
 * OmmTime is a read only class.<br>
 * This class is used for extraction of Time info only.
 *
 * The following code snippet shows extraction of time from ElementList and use DateTimeStringFormat to
 * print time in a specified format.
 * void decode(ElementList eList)
 *	{
 *      DateTimeStringFormat dateTimeStrFmt = EmaFactory.createDateTimeStringFormat();
 *		for (ElementEntry elementEntry : eList)
 *		{
 *			switch (elementEntry.load().dataType() )
 *			{
 *			case DataTypes.TIME:
 *				dateTimeStrFmt.format(DateTimeStringFormatTypes.STR_DATETIME_ISO8601);
 *              System.out.println(dateTimeStrFmt.timeAsString((OmmTime)elementEntry.load()));
 *              break;
 *			}				
 *		}
 *	}
 *
 * @see Data
 * @see DateTimeStringFormat
 */
public interface OmmTime extends Data
{
	/**
	 * Returns Hour. Range is 0 - 23 where 255 indicates blank.
	 * 
	 * @return int
	 */
	public int hour();

	/**
	 * Returns Minute. Range is 0 - 59 where 255 indicates blank.
	 * 
	 * @return int
	 */
	public int minute();

	/**
	 * Returns Second. Range is 0 - 60 where 255 indicates blank and 60 is to account for leap second.
	 * 
	 * @return int
	 */
	public int second();

	/**
	 * Returns Millisecond. Range is 0 - 999 where 65535 indicates blank.
	 * 
	 * @return int
	 */
	public int millisecond();

	/**
	 * Returns Microsecond. Range is 0 - 999 where 2047 indicates blank.
	 * 
	 * @return int
	 */
	public int microsecond();

	/**
	 * Returns Nanosecond. Range is 0 - 999 where 2047 indicates blank.
	 * 
	 * @return int
	 */
	public int nanosecond();
}
