///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/**
 * OmmDate represents Date info in Omm.
 * <br>OmmDate encapsulates year, month and day information.
 * 
 * OmmDate is a read only class.
 * 
 * The following code snippet shows extraction of date from ElementList and use DateTimeStringFormat to
 * print date in a specified format.
 * void decode(ElementList eList)
 *	{
 *      DateTimeStringFormat dateTimeStrFmt = EmaFactory.createDateTimeStringFormat();
 *		for (ElementEntry elementEntry : eList)
 *		{
 *			switch (elementEntry.load().dataType() )
 *			{
 *			case DataTypes.DATE:
 *				dateTimeStrFmt.format(DateTimeStringFormatTypes.STR_DATETIME_ISO8601);
 *              System.out.println(dateTimeStrFmt.dateAsString((OmmDate)elementEntry.load()));
 *              break;
 *			}				
 *		}
 *	}
 *
 * @see Data
 * @see DateTimeStringFormat
 */
public interface OmmDate extends Data
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
}
