///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.util.Collection;

/**
 * OmmArray is a homogeneous container of primitive data type entries.
 * 
 * <p>OmmArray is a collection which provides iterator over the elements in this collection.</p>
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
	 * Iterates through a list of Data of any DataType.
	 * Typical usage is to extract the entry during each iteration via getEntry().
	 */
	public boolean forth();

	/**
	 * Returns Entry.
	 * 
	 * @throw OmmInvalidUsageException if forth() was not called first
	 * 
	 * @return OmmArrayEntry
	 */
	public OmmArrayEntry entry();

	/**
	 * Resets iteration to start of container.
	 */
	public void reset();

	/**
	 * Clears the OmmArray.
	 * Invoking clear() method clears all the values and resets all the defaults.
	 * 
	 * @return reference to this object
	 */
	public void clear();

	/**
	 * Specifies FixedWidth.
	 * 
	 * @throw OmmInvalidUsageException if an entry was already added
	 * 
	 * @param width specifies fixed width value
	 * @return reference to this object
	 */
	public OmmArray fixedWidth(int width);

	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value specifies added long
	 * @return reference to this object
	 */
	public OmmArray addInt(long value);

	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 *        
	 * @param value specifies added long
	 * @return reference to this object
	 */
	public OmmArray addUInt(long value);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 *        
	 * @param value specifies added BigInteger
	 * @return reference to this object
	 */
	public OmmArray addUInt(BigInteger value);

	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param mantissa added OmmReal mantissa
	 * @param magnitudeType added OmmReal magnitudeType
	 * @return reference to this object
	 */
	public OmmArray addReal(long mantissa, int magnitudeType);

	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value added double to be converted to OmmReal
	 * @return reference to this object
	 */
	public OmmArray addRealFromDouble(double value);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value added double to be converted to OmmReal
	 * @param magnitudeType OmmReal magnitudeType used for the conversion
	 *            (default value is {@link com.thomsonreuters.ema.access.OmmReal.MagnitudeType#EXPONENT_0})
	 * @return reference to this object
	 */
	public OmmArray addRealFromDouble(double value, int magnitudeType);

	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value added float
	 * @return reference to this object
	 */
	public OmmArray addFloat(float value);

	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value added double
	 * @return reference to this object
	 */
	public OmmArray addDouble(double value);

	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * @throw OmmOutOfRangeException if passed in OmmDate is invalid
	 * 
	 * @param year added OmmDate year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDate month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDate day (0 - 31 where 0 indicates blank)
	 * @return reference to this object
	 */
	public OmmArray addDate(int year, int month, int day);

	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @return reference to this object
	 */
	public OmmArray addTime();
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @return reference to this object
	 */
	public OmmArray addTime(int hour);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @return reference to this object
	 */
	public OmmArray addTime(int hour, int minute);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * @return reference to this object
	 */
	public OmmArray addTime(int hour, int minute, int second);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmTime millisecond (0 - 999 where 65535 indicates blank)
	 * @return reference to this object
	 */
	public OmmArray addTime(int hour, int minute, int second, int millisecond);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmTime millisecond (0 - 999 where 65535 indicates blank)
	 * @param microsecond added OmmTime microsecond (0 - 999 where 2047 indicates blank)
	 * @return reference to this object
	 */
	public OmmArray addTime(int hour, int minute, int second, int millisecond, int microsecond);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmTime millisecond (0 - 999 where 65535 indicates blank)
	 * @param microsecond added OmmTime microsecond (0 - 999 where 2047 indicates blank)
	 * @param nanosecond added OmmTime nanosecond (0 - 999 where 2047 indicates blank)
	 * @return reference to this object
	 */
	public OmmArray addTime(int hour, int minute, int second, int millisecond, int microsecond, int nanosecond);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @return reference to this object
	 */
	public OmmArray addDateTime(int year, int month, int day);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * @return reference to this object
	 */
	public OmmArray addDateTime(int year, int month, int day, int hour);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * @param minute added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @return reference to this object
	 */
	public OmmArray addDateTime(int year, int month, int day, int hour, int minute);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * @param minute added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmDateTime second (0 - 60 where 255 indicates blank)
	 * @return reference to this object
	 */
	public OmmArray addDateTime(int year, int month, int day, int hour, int minute, int second);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * @param minute added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmDateTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmDateTime millisecond (0 - 999 where 65535 indicates blank)
	 * @return reference to this object
	 */
	public OmmArray addDateTime(int year, int month, int day, int hour, int minute,	int second, int millisecond);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * @param minute added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmDateTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmDateTime millisecond (0 - 999 where 65535 indicates blank)
	 * @param microsecond added OmmDateTime microsecond (0 - 999 where 2047 indicates blank)
	 * @return reference to this object
	 */
	public OmmArray addDateTime(int year, int month, int day, int hour, int minute, 
									int second, int millisecond, int microsecond);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * @param minute added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmDateTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmDateTime millisecond (0 - 999 where 65535 indicates blank)
	 * @param microsecond added OmmDateTime microsecond (0 - 999 where 2047 indicates blank)
	 * @param nanosecond added OmmDateTime nanosecond (0 - 999 where 2047 indicates blank)
	 * @return reference to this object
	 */
	public OmmArray addDateTime(int year, int month, int day, int hour, int minute,
									int second, int millisecond, int microsecond, int nanosecond);

	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArray addQos();
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param timeliness added OmmQos timeliness
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmQos.Timeliness#REALTIME})
	 * @return reference to this object
	 */
	public OmmArray addQos(int timeliness);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param timeliness added OmmQos timeliness
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmQos.Timeliness#REALTIME})
	 * @param rate added OmmQos rate
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmQos.Rate#TICK_BY_TICK})
	 * @return reference to this object
	 */
	public OmmArray addQos(int timeliness, int rate);

	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param streamState added {@link com.thomsonreuters.ema.access.OmmState.StreamState}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StreamState#OPEN})
	 * @param dataState added {@link com.thomsonreuters.ema.access.OmmState.DataState}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.DataState#OK})
	 * @param statusCode added {@link com.thomsonreuters.ema.access.OmmState.StatusCode}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StatusCode#NONE})
	 * @param statusText added OmmState text (default value is 'empty string')
	 * @return reference to this object
	 */
	public OmmArray addState();
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param streamState added {@link com.thomsonreuters.ema.access.OmmState.StreamState}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StreamState#OPEN})
	 * @return reference to this object
	 */
	public OmmArray addState(int streamState);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param streamState added {@link com.thomsonreuters.ema.access.OmmState.StreamState}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StreamState#OPEN})
	 * @param dataState added {@link com.thomsonreuters.ema.access.OmmState.DataState}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.DataState#OK})
	 * @return reference to this object
	 */
	public OmmArray addState(int streamState, int dataState);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param streamState added {@link com.thomsonreuters.ema.access.OmmState.StreamState}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StreamState#OPEN})
	 * @param dataState added {@link com.thomsonreuters.ema.access.OmmState.DataState}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.DataState#OK})
	 * @param statusCode added {@link com.thomsonreuters.ema.access.OmmState.StatusCode}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StatusCode#NONE})
	 * @return reference to this object
	 */
	public OmmArray addState(int streamState, int dataState, int statusCode);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param streamState added {@link com.thomsonreuters.ema.access.OmmState.StreamState}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StreamState#OPEN})
	 * @param dataState added {@link com.thomsonreuters.ema.access.OmmState.DataState}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.DataState#OK})
	 * @param statusCode added {@link com.thomsonreuters.ema.access.OmmState.StatusCode}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StatusCode#NONE})
	 * @param statusText added {@link OmmState#statusText()} (default value is 'empty string')
	 * @return reference to this object
	 */
	public OmmArray addState(int streamState, int dataState, int statusCode, String statusText);

	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value added Enum
	 * @return reference to this object
	 */
	public OmmArray addEnum(int value);

	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value added ByteBuffer as OmmBuffer
	 * @return reference to this object
	 */
	public OmmArray addBuffer(ByteBuffer value);

	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value added String as OmmAscii
	 * @return reference to this object
	 */
	public OmmArray addAscii(String value);

	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value added ByteBuffer as OmmUtf8
	 * @return reference to this object
	 */
	public OmmArray addUtf8(ByteBuffer value);
	
	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value added String as OmmUtf8
	 * @return reference to this object
	 */
	public OmmArray addUtf8(String value);
	

	/**
	 * Adds a specific simple type of OMM data to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @param value added ByteBuffer as OmmRmtes
	 * @return reference to this object
	 */
	public OmmArray addRmtes(ByteBuffer value);

	/**
	 * Adds a blank data code to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArray addCodeInt();

	/**
	 * Adds a blank data code to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArray addCodeUInt();

	/**
	 * Adds a blank data code to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArray addCodeReal();

	/**
	 * Adds a blank data code to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArray addCodeFloat();

	/**
	 * Adds a blank data code to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArray addCodeDouble();

	/**
	 * Adds a blank data code to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArray addCodeDate();

	/**
	 * Adds a blank data code to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArray addCodeTime();

	/**
	 * Adds a blank data code to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArray addCodeDateTime();

	/**
	 * Adds a blank data code to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArray addCodeQos();

	/**
	 * Adds a blank data code to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArray addCodeState();

	/**
	 * Adds a blank data code to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArray addCodeEnum();

	/**
	 * Adds a blank data code to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArray addCodeBuffer();

	/**
	 * Adds a blank data code to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * @return reference to this object
	 * 
	 */
	public OmmArray addCodeASCII();

	/**
	 * Adds a blank data code to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArray addCodeUtf8();

	/**
	 * Adds a blank data code to the OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if first addition was of different data type
	 * 
	 * @return reference to this object
	 */
	public OmmArray addCodeRmtes();

	/**
	 * Completes encoding of OmmArray.
	 * 
	 * @return reference to this object
	 */
	public OmmArray complete();
}