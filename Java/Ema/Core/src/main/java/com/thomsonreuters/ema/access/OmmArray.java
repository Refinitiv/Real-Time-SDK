///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.util.Collection;

/**
 * OmmArray is a homogeneous container of primitive data type entries.
 * <p>
 * OmmArray is a collection which provides iterator over the elements in this
 * collection.</p>
 *
 *  The following code snippet shows additions to OmmArray.
 * <pre>
 * OmmArray array = EmaFactory.createOmmArray();
 * array.fixedWidth(2);
 * array.add(EmaFactory.createOmmArrayEntry().intValue(22));
 * array.add(EmaFactory.createOmmArrayEntry().intValue(25));
 * </pre>
 * 
 * The following code snippet shows extracting data from OmmArray.
 * <pre>
 * for(OmmArrayEntry arrayEntry : array)
 * {
 *    System.out.print(" DataType: " + DataType.asString(arrayEntry.load().dataType()) + " Value: ");
 *
 *    if(Data.DataCode.BLANK == arrayEntry.code())
 *      System.out.println(" blank");
 *    else
 *        switch (arrayEntry.loadType())
 *        {
 *        case DataTypes.REAL:
 *            System.out.println(arrayEntry.real().asDouble());
 *            break;
 *        case DataTypes.DATE:
 *            System.out.println(arrayEntry.date().day() + " / " + arrayEntry.date().month() + " / "
 *                               + arrayEntry.date().year());
 *            break;
 *        case DataTypes.TIME:
 *            System.out.println(arrayEntry.time().hour() + ":" + arrayEntry.time().minute() + ":"
 *                               + arrayEntry.time().second() + ":" + arrayEntry.time().millisecond());
 *            break;
 *        case DataTypes.INT:
 *            System.out.println(arrayEntry.intValue());
 *            break;
 *
 *        ...
 *        }
 * }
 * </pre>
 * 
 * Objects of this class are intended to be short lived or rather transitional.<br>
 * This class is designed to efficiently perform setting and extracting of OmmArray and its content.<br>
 * Objects of this class are not cache-able.
 *
 * @see Data
 * @see OmmArrayEntry
 * @see	OmmReal
 * @see	OmmState
 * @see	OmmQos
 */
public interface OmmArray extends Data, Collection<OmmArrayEntry>
{
	/**
	 * Indicates presence of FixedWidth.
	 * 
	 * @return true if fixed width is set; false otherwise
	 */
	public boolean hasFixedWidth();

	/**
	 * Returns FixedWidth.
	 * 
	 * @return fixed width
	 */
	public int fixedWidth();

	/**
	 * Clears the OmmArray. Invoking clear() method clears all the values and
	 * resets all the defaults.
	 */
	public void clear();

	/**
	 * Specifies FixedWidth.
	 * 
	 * @throws OmmInvalidUsageException
	 *             if an entry was already added
	 * 
	 * @param width
	 *            specifies fixed width value
	 * @return reference to this object
	 */
	public OmmArray fixedWidth(int width);
}