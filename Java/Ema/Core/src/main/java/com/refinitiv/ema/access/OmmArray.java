///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019, 2024 Refinitiv. All rights reserved.        --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import com.refinitiv.ema.rdm.DataDictionary;

import java.util.Collection;
import java.util.Iterator;

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
	 * Specifies FixedWidth. Setting fixed-width implies each array entry has
	 * been populated with the same fixed size, to allow minimizing bandwidth.<br>
	 * Setting fixed-width as 0 implies the data contained in the array entries
	 * may be of different sizes, to allow the flexibility of different sized
	 * encoding;<br>
	 * 
	 * When using a fixed width, the application still passes in the base
	 * primitive type when encoding (e.g., if encoding fixed width
	 * DataTypes.INT types, an Int is passed in regardless of itemLength).<br>
	 * When encoding buffer types as fixed width: Any content that exceeds
	 * fixedWidth will be truncated Any content that is shorter than fixedWidth
	 * will be padded with the \0 (NULL) character.<br>
	 * 
	 * Only specific types are allowed as fixed-width encodings. Here lists supported fixed widths and allowable data ranges: <br>
	 * <pre>
	 * DataType::IntEnum supports one byte(-2^7 to 2^7-1), two byte(-2^15 to 2^15-1), four byte((-2^31 to 2^31-1), or eight byte((-2^63 to 2^63-1).
	 * DataType::UIntEnum supports one byte(0 to 2^8-1), two byte(0 to 2^16-1), four byte(0 to 2^32-1), or eight byte(0 to 2^64-1).
	 * DataType::FloatEnum supports four byte, floating point type that represents the same range of values allowed by the system float type. Follows the IEEE 754 specification.
	 * DataType::DoubleEnum supports eight byte, floating point type that represents the same range of values allowed by the system double type. Follows the IEEE 754 specification.
     * DataType::DateEnum supports four byte(year, month, day).
	 * DataType::TimeEnum supports:
	 *		three byte(hour, minute, second);
	 *		five byte(hour, minute, second, millisec);
	 *		seven byte(hour, minute, second, millisec, microsec);
	 *		eight byte(hour, minute, second, millisec, microsec, nanosec). 
	 * DataType::DateTimeEnum supports:
	 *		seven byte(year, month, day, hour, minute, second);
	 *		nine byte(year, month, day, hour, minute, second, millisec);
	 *		eleven byte(year, month, day, hour, minute, second, millisec, microsec);
	 *		twelve byte(year, month, day, hour, minute, second, millisec, microsec, nanosec);
	 * DataType::EnumEnum supports one byte(0 to 2^8-1) or two byte(0 to 2^16-1).
     * DataType::BufferEnum, DataType::AsciiEnum, DataType::Utf8Enum, and
	 *		DataType::RmtesEnum support any legal width value.
	 * </pre>
	 * 
	 * @throws OmmInvalidUsageException
	 *             if an entry was already added
	 * 
	 * @param width
	 *            specifies fixed width value
	 * @return reference to this object
	 */
	public OmmArray fixedWidth(int width);

	/**
	 *  Returns a string representation of the class instance.
	 * @param dictionary use for toString() conversion
	 * @return string representation of the class instance
	 */
	public String toString(DataDictionary dictionary);
	
	/**
	 * A more efficient and performant iterator call, which eliminates using a collection and
	 * having an iterator over the collection.
	 * 
	 * Returns an iterator over a single decoded OmmArray. This does not return a copy of this data,
	 * but rather a reference to it that can be read and used before being moved to the next decoded OmmArrayEntry
	 * when hasNext() is called, and returning the entry with next() on this iterator. hasNext() is required to be called
	 * before each next() call to return the following entry.
	 * 
	 * @return iterator for a reference of a single decoded OmmArrayEntry.
	 */
	public Iterator<OmmArrayEntry> iteratorByRef();
}