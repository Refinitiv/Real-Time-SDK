///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.math.BigInteger;
import java.nio.ByteBuffer;

/**
 * OmmArrayEntry represents an entry of OmmArray.
 * <p>OmmArrayEntry associates entry's data and its data type.</p>
 * 
 *  * Code snippet:
 * <pre>
 * void decode(OmmArray array)
 * {
 *     for(OmmArrayEntry arrayEntry : array)
 *     {
 *        System.out.print(" DataType: " + DataType.asString(arrayEntry.load().dataType()) + " Value: ");
 *
 *        if(Data.DataCode.BLANK == arrayEntry.code())
 *          System.out.println(" blank");
 *        else
 *            switch (arrayEntry.loadType())
 *            {
 *            case DataTypes.REAL:
 *                System.out.println(arrayEntry.real().asDouble());
 *                break;
 *            case DataTypes.DATE:
 *                System.out.println(arrayEntry.date().day() + " / " + arrayEntry.date().month() + " / "
 *                                   + arrayEntry.date().year());
 *                break;
 *            case DataTypes.TIME:
 *                System.out.println(arrayEntry.time().hour() + ":" + arrayEntry.time().minute() + ":"
 *                                   + arrayEntry.time().second() + ":" + arrayEntry.time().millisecond());
 *                break;
 *            case DataTypes.INT:
 *                System.out.println(arrayEntry.intValue());
 *                break;
 *
 *            ...
 *            }
 *     }
 * }
 * </pre>
 * 
 * Objects of this class are intended to be short lived or rather transitional.<br>
 * This class is designed to efficiently perform extracting of data from entry.<br>
 * Objects of this class are not cache-able.
 *
 * @see Data
 * @see OmmArray
 * @see OmmReal
 * @see OmmDate
 * @see OmmTime
 * @see OmmDateTime
 * @see OmmQos
 * @see OmmState
 * @see OmmError
 */
public interface OmmArrayEntry
{
	/**
	 * Returns the DataType of the entry's load.
	 * Return of {@link com.thomsonreuters.ema.access.DataType.DataTypes#ERROR} signifies error while extracting content of load.
	 * 
	 * @return data type of the contained object
	 */
	public int loadType();

	/**
	 * Returns the Code of the entry's load. The code indicates a special state of a Data.
	 * Attempts to extract data will cause OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK} is returned.
	 * 
	 * @return data code of the contained object
	 */
	public int code();
		
	/**
	 * Returns a string representation of the class instance.
	 * 
	 * @return string representation of the class instance
	 */
	public String toString();

	/**
	 * Returns the contained Data based on the DataType.
	 * 
	 * @return Data class reference to contained object
	 */
	public Data load();

	/** 
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmInt}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return long
	 */
	public long intValue();
	
	/** Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmInt}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmInt}
	 */
	public OmmInt ommIntValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmUInt}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return long
	 */
	public long uintValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmUInt}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmUInt}
	 */
	public OmmUInt ommUIntValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmReal}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmReal} class reference to contained object
	 */
	public OmmReal real();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmFloat}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return float
	 */
	public float floatValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmFloat}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmFloat}
	 */
	public OmmFloat ommFloatValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmDouble}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return double
	 */
	public double doubleValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmDouble}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmDouble}
	 */
	public OmmDouble ommDoubleValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmDate}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmDate} class reference to contained object
	 */
	public OmmDate date();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmTime}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmTime} class reference to contained object
	 */
	public OmmTime time();

	/** 
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmDateTime}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmDateTime} class reference to contained object
	 */
	public OmmDateTime dateTime();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmQos}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmQos} class reference to contained object
	 */
	public OmmQos qos();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmState}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmState} class reference to contained object
	 */
	public OmmState state();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmEnum}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return int
	 */
	public int enumValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmEnum}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmEnum}
	 */
	public OmmEnum ommEnumValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmBuffer}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmBuffer}
	 */
	public OmmBuffer buffer();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmAscii}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmAscii}
	 */
	public OmmAscii ascii();

	/** 
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmUtf8}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmUtf8}
	 */
	public OmmUtf8 utf8();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmRmtes}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmRmtes}
	 */
	public OmmRmtes rmtes();

	/** Returns Error.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmError}
	 * @return {@link com.thomsonreuters.ema.access.OmmError} class reference to contained object
	 */
	public OmmError error();
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value specifies added long
	 * @return reference to this object
	 */
	public OmmArrayEntry intValue(long value);

	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 *        
	 * @param value specifies added long
	 * @return reference to this object
	 */
	public OmmArrayEntry uintValue(long value);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 *        
	 * @param value specifies added BigInteger
	 * @return reference to this object
	 */
	public OmmArrayEntry uintValue(BigInteger value);

	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param mantissa added OmmReal mantissa
	 * @param magnitudeType added OmmReal magnitudeType
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry real(long mantissa, int magnitudeType);

	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value added double to be converted to OmmReal
	 * @return reference to this object
	 */
	public OmmArrayEntry realFromDouble(double value);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value added double to be converted to OmmReal
	 * @param magnitudeType OmmReal magnitudeType used for the conversion
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry realFromDouble(double value, int magnitudeType);

	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value added float
	 * @return reference to this object
	 */
	public OmmArrayEntry floatValue(float value);

	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value added double
	 * @return reference to this object
	 */
	public OmmArrayEntry doubleValue(double value);

	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * @throws OmmOutOfRangeException if passed in OmmDate is invalid
	 * 
	 * @param year added OmmDate year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDate month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDate day (0 - 31 where 0 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry date(int year, int month, int day);

	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * Defaults: second=0, millisecond=0, microsecond=0, nanosecond=0
	 *
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * @throws OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry time(int hour, int minute);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * Defaults: millisecond=0, microsecond=0, nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * @throws OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry time(int hour, int minute, int second);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * Defaults: microsecond=0, nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * @throws OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmTime millisecond (0 - 999 where 65535 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry time(int hour, int minute, int second, int millisecond);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * Default: nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * @throws OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmTime millisecond (0 - 999 where 65535 indicates blank)
	 * @param microsecond added OmmTime microsecond (0 - 999 where 2047 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry time(int hour, int minute, int second, int millisecond, int microsecond);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * @throws OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmTime millisecond (0 - 999 where 65535 indicates blank)
	 * @param microsecond added OmmTime microsecond (0 - 999 where 2047 indicates blank)
	 * @param nanosecond added OmmTime nanosecond (0 - 999 where 2047 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry time(int hour, int minute, int second, int millisecond, int microsecond, int nanosecond);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * Defaults: hour=0, minute=0, second=0, millisecond=0, microsecond=0, nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry dateTime(int year, int month, int day);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * Defaults:  minute=0, second=0, millisecond=0, microsecond=0, nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 23 where 255 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry dateTime(int year, int month, int day, int hour);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * Defaults: second=0, millisecond=0, microsecond=0, nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry dateTime(int year, int month, int day, int hour, int minute);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * Defaults: millisecond=0, microsecond=0, nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmDateTime second (0 - 60 where 255 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry dateTime(int year, int month, int day, int hour, int minute, int second);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * Defaults: microsecond=0, nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmDateTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmDateTime millisecond (0 - 999 where 65535 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry dateTime(int year, int month, int day, int hour, int minute, int second, int millisecond);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * Default: nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmDateTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmDateTime millisecond (0 - 999 where 65535 indicates blank)
	 * @param microsecond added OmmDateTime microsecond (0 - 999 where 2047 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry dateTime(int year, int month, int day, int hour, int minute, 
									int second, int millisecond, int microsecond);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmDateTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmDateTime millisecond (0 - 999 where 65535 indicates blank)
	 * @param microsecond added OmmDateTime microsecond (0 - 999 where 2047 indicates blank)
	 * @param nanosecond added OmmDateTime nanosecond (0 - 999 where 2047 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry dateTime(int year, int month, int day, int hour, int minute,
									int second, int millisecond, int microsecond, int nanosecond);

	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * Default rate is {@link com.thomsonreuters.ema.access.OmmQos.Rate#TICK_BY_TICK}
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param timeliness added OmmQos timeliness
	 * @return reference to this object
	 */
	public OmmArrayEntry qos(int timeliness);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param timeliness added OmmQos timeliness
	 * @param rate added OmmQos rate
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry qos(int timeliness, int rate);

	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * Default dataState is {@link com.thomsonreuters.ema.access.OmmState.DataState#OK}
	 * Default statusCode is {@link com.thomsonreuters.ema.access.OmmState.StatusCode#NONE}
	 * Default statusText is an empty String
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param streamState added {@link com.thomsonreuters.ema.access.OmmState.StreamState}
	 * @return reference to this object
	 */
	public OmmArrayEntry state(int streamState);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * Default statusCode is {@link com.thomsonreuters.ema.access.OmmState.StatusCode#NONE}
	 * Default statusText is an empty String
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param streamState added {@link com.thomsonreuters.ema.access.OmmState.StreamState}
	 * @param dataState added {@link com.thomsonreuters.ema.access.OmmState.DataState}
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry state(int streamState, int dataState);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * Default statusText is an empty String
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param streamState added {@link com.thomsonreuters.ema.access.OmmState.StreamState}
	 * @param dataState added {@link com.thomsonreuters.ema.access.OmmState.DataState}
	 * @param statusCode added {@link com.thomsonreuters.ema.access.OmmState.StatusCode}
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry state(int streamState, int dataState, int statusCode);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param streamState added {@link com.thomsonreuters.ema.access.OmmState.StreamState}
	 * @param dataState added {@link com.thomsonreuters.ema.access.OmmState.DataState}
	 * @param statusCode added {@link com.thomsonreuters.ema.access.OmmState.StatusCode}
	 * @param statusText added {@link OmmState#statusText()}
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry state(int streamState, int dataState, int statusCode, String statusText);

	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value added Enum
	 * @return reference to this object
	 */
	public OmmArrayEntry enumValue(int value);

	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value added ByteBuffer as OmmBuffer
	 * @return reference to this object
	 */
	public OmmArrayEntry buffer(ByteBuffer value);

	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value added String as OmmAscii
	 * @return reference to this object
	 */
	public OmmArrayEntry ascii(String value);

	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value added ByteBuffer as OmmUtf8
	 * @return reference to this object
	 */
	public OmmArrayEntry utf8(ByteBuffer value);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value added String as OmmUtf8
	 * @return reference to this object
	 */
	public OmmArrayEntry utf8(String value);
	

	/**
	 * Adds a specific simple type of OMM data to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value added ByteBuffer as OmmRmtes
	 * @return reference to this object
	 */
	public OmmArrayEntry rmtes(ByteBuffer value);

	/**
	 * Adds a blank data code to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry codeInt();

	/**
	 * Adds a blank data code to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry codeUInt();

	/**
	 * Adds a blank data code to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry codeReal();

	/**
	 * Adds a blank data code to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry codeFloat();

	/**
	 * Adds a blank data code to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry codeDouble();

	/**
	 * Adds a blank data code to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry codeDate();

	/**
	 * Adds a blank data code to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry codeTime();

	/**
	 * Adds a blank data code to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry codeDateTime();

	/**
	 * Adds a blank data code to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry codeQos();

	/**
	 * Adds a blank data code to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry codeState();

	/**
	 * Adds a blank data code to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry codeEnum();

	/**
	 * Adds a blank data code to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry codeBuffer();

	/**
	 * Adds a blank data code to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * @return reference to this object
	 * 
	 */
	public OmmArrayEntry codeAscii();

	/**
	 * Adds a blank data code to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry codeUtf8();

	/**
	 * Adds a blank data code to the OmmArrayEntry.
	 * 
	 * @throws OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArrayEntry codeRmtes();
}