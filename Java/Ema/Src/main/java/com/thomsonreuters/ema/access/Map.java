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
 * Map is a homogeneous container of complex data type entries.
 * 
 * <p>Map is a collection which provides iterator over the elements in this collection.</p>
 * 
 * <p>Map entries are identified by a map key. All entries must have key of the
 * same primitive data type. All entries must have same complex data type (except for delete action).</p>
 * 
 * <p>Map supports two methods of adding containers; they are:
 * 	- adding of already populated containers, (e.g. complete() was called) and 
 *  - adding of clear containers (e.g. clear() was called) which would be populated after that.</p>
 *  
 * <p>The first method of adding of already populated containers allows for easy data
 *  manipulation but incurs additional memory copy. This method is useful in
 *  applications extracting data containers from some messages or containers and then
 *  setting them on other containers.</p>
 *  
 * <p>The second method allows for fast container population since it avoids additional
 *  memory copy incurred by the first method. This method is useful in source applications
 *  setting OMM data from native data formats.</p>
 */
public interface Map extends ComplexType, Collection<MapEntry>
{
	/**
	 * Iterates through a list of Data of any DataType. Typical usage is to
	 * extract the entry during each iteration via getEntry().
	 * 
	 * @return true at the end of Map; false otherwise
	 */
	public boolean forth();

	/**
	 * Resets iteration to start of container.
	 */
	public void reset();

	/**
	 * Returns Entry.
	 * 
	 * @throw OmmInvalidUsageException if forth() was not called first
	 * 
	 * @return MapEntry
	 */
	public MapEntry entry();

	/**
	 * Indicates presence of KeyFieldId.
	 * 
	 * @return true if key field id is set; false otherwise
	 */
	public boolean hasKeyFieldId();

	/**
	 * Indicates presence of TotalCountHint.
	 * 
	 * @return true if total count hint is set; false otherwise
	 */
	public boolean hasTotalCountHint();

	/**
	 * Returns KeyFieldId.
	 * 
	 * @throw OmmInvalidUsageException if hasKeyFieldId() returns false
	 * 
	 * @return key field id
	 */
	public int keyFieldId();

	/**
	 * Returns TotalCountHint.
	 * 
	 * @throw OmmInvalidUsageException if hasTotalCountHint() returns false
	 * 
	 * @return total count hint
	 */
	public int totalCountHint();

	/**
	 * Returns the contained summaryData Data based on the summaryData DataType.
	 * SummaryData contains no data if {@link com.thomsonreuters.ema.access.SummaryData#dataType()}
	 * returns {@link com.thomsonreuters.ema.access.DataType.DataTypes#NO_DATA}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.SummaryData}
	 */
	public SummaryData summaryData();

	/**
	 * Clears the Map. Invoking clear() method clears all the values and
	 * resets all the defaults
	 * 
	 * @return reference to this object
	 */
	public void clear();

	/**
	 * Specifies KeyFieldId.
	 * 
	 * @pram fieldId specifies key field id
	 * @return reference to this object
	 */
	public Map keyFieldId(int fieldId);

	/**
	 * Specifies TotalCountHint.
	 * 
	 * @param totalCountHint specifies total count hint
	 * @return reference to this object
	 */
	public Map totalCountHint(int totalCountHint);

	/**
	 * Specifies the SummaryData OMM Data. 
	 * Call to summaryData() must happen prior to calling any add***() method
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param summaryData specifies complex type as summaryData
	 * @return reference to this object
	 */
	public Map summaryData(ComplexType summaryData);

	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param key containing long key information
	 * @param action specifies action to be applied to the entry
	 * @param value specifies complex type associated with this entry
	 * @return reference to this object
	 */
	public Map addKeyInt(long key, int action, ComplexType value);
	
	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param key containing long key information
	 * @param action specifies action to be applied to the entry
	 * @param value specifies complex type associated with this entry
	 * @param permissionData ByteBuffer containing permission data related to this entry
	 * @return reference to this object
	 */
	public Map addKeyInt(long key, int action, ComplexType value, ByteBuffer permissionData);

	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param key containing UInt key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @return reference to this object
	 */
	public Map addKeyUInt(long key, int action, ComplexType value);		
	
	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param key containing UInt key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @param permissionData ByteBuffer containing permission data related to this entry
	 * @return reference to this object
	 */
	public Map addKeyUInt(long key, int action, ComplexType value, ByteBuffer permissionData);
	
	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param key containing UInt key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @return reference to this object
	 */
	public Map addKeyUInt(BigInteger key, int action, ComplexType value);
	
	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param key containing UInt key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @param permissionData ByteBuffer containing permission data related to this entry
	 * @return reference to this object
	 */
	public Map addKeyUInt(BigInteger key, int action, ComplexType value, ByteBuffer permissionData);									

	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param mantissa specifies OmmReal mantissa part of key information
	 * @param magnitudeType specifies OmmReal magnitudeType part of key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @return reference to this object
	 */
	public Map addKeyReal(long mantissa, int magnitudeType, int action,	ComplexType value);
	
	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param mantissa specifies OmmReal mantissa part of key information
	 * @param magnitudeType specifies OmmReal magnitudeType part of key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @param permissionData ByteBuffer containing permission data related to this entry
	 * @return reference to this object
	 */
	public Map addKeyReal(long mantissa, int magnitudeType, int action, ComplexType value,
							ByteBuffer permissionData);

	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param key specifies double to be converted to {@link com.thomsonreuters.ema.access.OmmReal}
	 * @param magnitudeType OmmReal magnitudeType used for the conversion
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmReal.MagnitudeType#EXPONENT_0})
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @return reference to this object
	 */
	public Map addKeyReal(double key, int action, ComplexType value, int magnitudeType);
	
	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param key specifies double to be converted to {@link com.thomsonreuters.ema.access.OmmReal}
	 * @param magnitudeType OmmReal magnitudeType used for the conversion
	 *            (default value is {@link com.thomsonreuters.ema.access.OmmReal.MagnitudeType#EXPONENT_0})
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @param permissionData ByteBuffer containing permission data related to  this entry
	 * @return reference to this object
	 */
	public Map addKeyReal(double key, int action, ComplexType value, int magnitudeType,
							ByteBuffer permissionData);

	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param key containing float key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @return reference to this object
	 */
	public Map addKeyFloat(float key, int action, ComplexType value);
	
	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param key containing float key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @param permissionData ByteBuffer containing permission data related to this entry
	 * @return reference to this object
	 */
	public Map addKeyFloat(float key, int action, ComplexType value, ByteBuffer permissionData);

	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @param key containing double key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @return reference to this object
	 */
	public Map addKeyDouble(double key, int action, ComplexType value);
	
	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @param key containing double key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @param permissionData ByteBuffer containing permission data related to this entry
	 * @return reference to this object
	 */
	public Map addKeyDouble(double key, int action, ComplexType value, ByteBuffer permissionData);

	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmDate is invalid
	 * 
	 * @param year specifies OmmDate year part of key information (0 - 4095)
	 * @param month specifies OmmDate month part of key information (0 - 12)
	 * @param day specifies OmmDate day part of key information (0 - 31)
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @return reference to this object
	 */
	public Map addKeyDate(int year, int month, int day, int action, ComplexType value);
	
	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmDate is invalid
	 * 
	 * @param year specifies OmmDate year part of key information (0 - 4095)
	 * @param month specifies OmmDate month part of key information (0 - 12)
	 * @param day specifies OmmDate day part of key information (0 - 31)
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @param permissionData ByteBuffer containing permission data related to this entry
	 * @return reference to this object
	 */
	public Map addKeyDate(int year, int month, int day, int action, ComplexType value,
			ByteBuffer permissionData);

	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param hour specifies OmmTime hour part of key information (0 - 23)
	 * @param minute specifies OmmTime minute part of key information (0 - 59)
	 * @param second specifies OmmTime second part of key information (0 - 60)
	 * @param millisecond specifies OmmTime millisecond part of key information (0 - 999)
	 * @param microsecond specifies OmmTime microsecond part of key information (0 - 999)
	 * @param nanosecond specifies OmmTime nanosecond part of key information (0 - 999)
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @return reference to this object
	 */
	public Map addKeyTime(int hour, int minute, int second, int millisecond,
			int microsecond, int nanosecond, int action, ComplexType value);
	
	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param hour specifies OmmTime hour part of key information (0 - 23)
	 * @param minute specifies OmmTime minute part of key information (0 - 59)
	 * @param second specifies OmmTime second part of key information (0 - 60)
	 * @param millisecond specifies OmmTime millisecond part of key information (0 - 999)
	 * @param microsecond specifies OmmTime microsecond part of key information (0 - 999)
	 * @param nanosecond specifies OmmTime nanosecond part of key information (0 - 999)
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @param permissionData ByteBuffer containing permission data related to this entry
	 * @return reference to this object
	 */
	public Map addKeyTime(int hour, int minute, int second, int millisecond,
			int microsecond, int nanosecond, int action, ComplexType value,
			ByteBuffer permissionData);

	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param year specifies OmmDateTime year part of key information (0 - 4095)
	 * @param month specifies OmmDateTime month part of key information (0 - 12)
	 * @param day specifies OmmDateTime day part of key information (0 - 31)
	 * @param hour specifies OmmDateTime hour part of key information (0 - 23)
	 * @param minute specifies OmmDateTime minute part of key information (0 - 59)
	 * @param second specifies OmmDateTime second part of key information (0 - 60)
	 * @param millisecond specifies OmmDateTime millisecond part of key information (0 - 999)
	 * @param microsecond specifies OmmDateTime microsecond part of key information (0 - 999)
	 * @param nanosecond specifies OmmDateTime nanosecond part of key information (0 - 999)
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @return reference to this object
	 */
	public Map addKeyDateTime(int year, int month, int day, int hour,
			int minute, int second, int millisecond, int microsecond,
			int nanosecond, int action, ComplexType value);
	
	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param year specifies OmmDateTime year part of key information (0 - 4095)
	 * @param month specifies OmmDateTime month part of key information (0 - 12)
	 * @param day specifies OmmDateTime day part of key information (0 - 31)
	 * @param hour specifies OmmDateTime hour part of key information (0 - 23)
	 * @param minute specifies OmmDateTime minute part of key information (0 - 59)
	 * @param second specifies OmmDateTime second part of key information (0 - 60)
	 * @param millisecond specifies OmmDateTime millisecond part of key information (0 - 999)
	 * @param microsecond specifies OmmDateTime microsecond part of key information (0 - 999)
	 * @param nanosecond specifies OmmDateTime nanosecond part of key information (0 - 999)
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @param permissionData ByteBuffer containing permission data related to this entry
	 * @return reference to this object
	 */
	public Map addKeyDateTime(int year, int month, int day, int hour,
			int minute, int second, int millisecond, int microsecond,
			int nanosecond, int action, ComplexType value,
			ByteBuffer permissionData);

	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param timeliness specifies {@link com.thomsonreuters.ema.access.OmmQos.Timeliness} part of key information
	 * @param rate specifies {@link com.thomsonreuters.ema.access.OmmQos.Rate} part of key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @return reference to this object
	 */
	public Map addKeyQos(int timeliness, int rate, int action, ComplexType value);
	
	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param timeliness specifies {@link com.thomsonreuters.ema.access.OmmQos.Timeliness} part of key information
	 * @param rate specifies {@link com.thomsonreuters.ema.access.OmmQos.Rate} part of key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @param permissionData ByteBuffer containing permission data related to  this entry
	 * @return reference to this object
	 */
	public Map addKeyQos(int timeliness, int rate, int action, ComplexType value,
			ByteBuffer permissionData);

	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param streamState specifies {@link com.thomsonreuters.ema.access.OmmState.StreamState} part of key information
	 * @param dataState specifies {@link com.thomsonreuters.ema.access.OmmState.DataState} part of key information
	 * @param statusCode specifies {@link com.thomsonreuters.ema.access.OmmState.StatusCode} part of key information
	 * @param statusText specifies OmmState text part of key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @return reference to this object
	 */
	public Map addKeyState(int streamState, int dataState, int statusCode,
			String statusText, int action, ComplexType value);
	
	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param streamState specifies {@link com.thomsonreuters.ema.access.OmmState.StreamState} part of key information
	 * @param dataState specifies {@link com.thomsonreuters.ema.access.OmmState.DataState} part of key information
	 * @param statusCode specifies {@link com.thomsonreuters.ema.access.OmmState.StatusCode} part of key information
	 * @param statusText specifies OmmState text part of key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @param permissionData ByteBuffer containing permission data related to this entry
	 * @return reference to this object
	 */
	public Map addKeyState(int streamState, int dataState, int statusCode,
			String statusText, int action, ComplexType value,
			ByteBuffer permissionData);

	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param key int containing Enum key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @return reference to this object
	 */
	public Map addKeyEnum(int key, int action, ComplexType value);
	
	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param key int containing Enum key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @param permissionData ByteBuffer containing permission data related to this entry
	 * @return reference to this object
	 */
	public Map addKeyEnum(int key, int action, ComplexType value,
			ByteBuffer permissionData);

	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param key ByteBuffer containing Buffer key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @return reference to this object
	 */
	public Map addKeyBuffer(ByteBuffer key, int action, ComplexType value);
	
	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param key ByteBuffer containing Buffer key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @param permissionData ByteBuffer containing permission data related to this entry
	 * @return reference to this object
	 */
	public Map addKeyBuffer(ByteBuffer key, int action, ComplexType value,
			ByteBuffer permissionData);

	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param key String containing Ascii key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @return reference to this object
	 */
	public Map addKeyAscii(String key, int action, ComplexType value);
	
	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param key String containing Ascii key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @param permissionData ByteBuffer containing permission data related to this entry
	 * @return reference to this object
	 */
	public Map addKeyAscii(String key, int action, ComplexType value,
			ByteBuffer permissionData);

	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param key ByteBuffer containing Utf8 key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @return reference to this object
	 */
	public Map addKeyUtf8(ByteBuffer key, int action, ComplexType value);
	
	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param key ByteBuffer containing Utf8 key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @param permissionData ByteBuffer containing permission data related to this entry
	 * @return reference to this object
	 */
	public Map addKeyUtf8(ByteBuffer key, int action, ComplexType value,
			ByteBuffer permissionData);

	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param key ByteBuffer containing Rmtes key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @return reference to this object
	 */
	public Map addKeyRmtes(ByteBuffer key, int action, ComplexType value);
	
	/**
	 * Adds complex OMM data identified by a specific simple type of OMM data.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param key ByteBuffer containing Rmtes key information
	 * @param action specifies action to apply to the entry
	 * @param value complex type contained in this entry
	 * @param permissionData ByteBuffer containing permission data related to this entry
	 * @return reference to this object
	 */
	public Map addKeyRmtes(ByteBuffer key, int action, ComplexType value,
			ByteBuffer permissionData);

	/**
	 * Completes encoding of Map.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public Map complete();
}