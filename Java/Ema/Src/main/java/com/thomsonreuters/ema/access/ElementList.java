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
 * ElementList is a heterogeneous container of complex and primitive data
 * type entries. ElementList entries are identified by name.
 * 
 * <p>ElementList is a collection which provides iterator over the elements in this collection.</p>
 * 
 * <p>ElementList supports two methods of adding containers; they are:
 * - adding of already populated containers, and
 * - adding of clear containers which would be populated after that.</p>
 * 
 * <p>Note that these two methods apply to containers only: OmmArray, ElementList,
 * FieldList, FilterList, Map, Series, and Vector.</p>
 * 
 * <p>The first method of adding of already populated containers allows for easy data
 * manipulation but incurs additional memory copy. This method is useful in
 * applications extracting data containers from some other messages or containers
 * and then setting them on other containers without full decoding of the extracted
 * containers.</p>
 * 
 * <p>The second method allows for fast container population since it avoids additional
 * memory copy incurred in the first method. This method is useful in source applications
 * encoding OMM data from native data formats.</p>
 */
public interface ElementList extends ComplexType, Collection<ElementEntry>
{
	/**
	 * Indicates presence of Info.
	 * 
	 * @return true if ElementList Info is set; false otherwise
	 */
	public boolean hasInfo();

	/**
	 * Returns InfoElementListNum.
	 * 
	 * @throw OmmInvalidUsageException if hasInfo() returns false
	 * 
	 * @return ElementList Number
	 */
	public int infoElementListNum();

	/**
	 * Iterates through a list of Data of any DataType. Typical usage is to
	 * extract the entry during each iteration via entry().
	 * 
	 * @return true at the end of ElementList; false otherwise
	 */
	public boolean forth();

	/**
	 * Iterates through a list of Data matching the name. Typical usage is to
	 * extract the entry during each iteration via entry().
	 * 
	 * @param name - looked up ElementEntry's name
	 * @return true at the end of ElementList; false otherwise
	 */
	public boolean forth(String name);

	/**
	 * Iterates through a list of Data having the name matching the specified
	 * Data. Typical usage is for the Data to be a view, and thus extract each
	 * matched entry during each iteration via entry().
	 * 
	 * @param data - containing ElementList with a view definition specifying
	 *            looked up names
	 * @return true at the end of ElementList; false otherwise
	 */
	public boolean forth(Data data);

	/**
	 * Returns Entry.
	 * 
	 * @throw OmmInvalidUsageException if forth() was not called first
	 * 
	 * @return ElementEntry
	 */
	public ElementEntry entry();

	/**
	 * Resets iteration to start of container.
	 */
	public void reset();

	/**
	 * Clears the ElementList.
	 * Note: allows re-use of ElementList instance during encoding.
	 * 
	 * @return reference to this object
	 */
	public void clear();

	/**
	 * Specifies Info.
	 * The ElementList Info is optional.
	 * If used, it must be specified before adding anything to ElementList.
	 * 
	 * @param elementListNum - FieldList template number
	 * @return reference to this object
	 */
	public ElementList info(int elementListNum);

	/**
	 * Adds a complex type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added ReqMsg
	 * @return reference to this object
	 */
	public ElementList addReqMsg(String name, ReqMsg value);

	/**
	 * Adds a complex type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added RefreshMsg
	 * @return reference to this object
	 */
	public ElementList addRefreshMsg(String name, RefreshMsg value);

	/**
	 * Adds a complex type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added StatusMsg
	 * @return reference to this object
	 */
	public ElementList addStatusMsg(String name, StatusMsg value);

	/**
	 * Adds a complex type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added UpdateMsg
	 * @return reference to this object
	 */
	public ElementList addUpdateMsg(String name, UpdateMsg value);

	/**
	 * Adds a complex type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added PostMsg
	 * @return reference to this object
	 */
	public ElementList addPostMsg(String name, PostMsg value);

	/**
	 * Adds a complex type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added AckMsg
	 * @return reference to this object
	 */
	public ElementList addAckMsg(String name, AckMsg value);

	/**
	 * Adds a complex type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added GenericMsg
	 * @return reference to this object
	 */
	public ElementList addGenericMsg(String name, GenericMsg value);

	/**
	 * Adds a complex type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added FieldList
	 * @return reference to this object
	 */
	public ElementList addFieldList(String name, FieldList value);

	/**
	 * Adds a complex type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added ElementList
	 * @return reference to this object
	 */
	public ElementList addElementList(String name, ElementList value);

	/**
	 * Adds a complex type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added Map
	 * @return reference to this object
	 */
	public ElementList addMap(String name, Map value);

	/**
	 * Adds a complex type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added Vector
	 * @return reference to this object
	 */
	public ElementList addVector(String name, Vector value);

	/**
	 * Adds a complex type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added Series
	 * @return reference to this object
	 */
	public ElementList addSeries(String name, Series value);

	/**
	 * Adds a complex type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added FilterList
	 * @return reference to this object
	 */
	public ElementList addFilterList(String name, FilterList value);

	/**
	 * Adds a complex type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added OmmOpaque
	 * @return reference to this object
	 */
	public ElementList addOpaque(String name, OmmOpaque value);

	/**
	 * Adds a complex type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added OmmXml
	 * @return reference to this object
	 */
	public ElementList addXml(String name, OmmXml value);

	/**
	 * Adds a complex type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added OmmAnsiPage
	 * @return reference to this object
	 */
	public ElementList addAnsiPage(String name, OmmAnsiPage value);

	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added Int64
	 * @return reference to this object
	 */
	public ElementList addInt(String name, long value);

	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added long
	 * @return reference to this object
	 */
	public ElementList addUInt(String name, long value);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added BigInteger
	 * @return reference to this object
	 */
	public ElementList addUInt(String name, BigInteger value);
	

	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param mantissa - added OmmReal mantissa   
	 * @param magnitudeType - added {@link com.thomsonreuters.ema.access.OmmReal.MagnitudeType}
	 * @return reference to this object
	 */
	public ElementList addReal(String name, long mantissa, int magnitudeType);

	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added double to be converted to OmmReal
	 * @return reference to this object
	 */
	public ElementList addRealFromDouble(String name, double value);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added double to be converted to OmmReal
	 * @param magnitudeType - {@link com.thomsonreuters.ema.access.OmmReal.MagnitudeType} used for the conversion
	 *            (default value is {@link com.thomsonreuters.ema.access.OmmReal.MagnitudeType#EXPONENT_0})
	 * @return reference to this object
	 */
	public ElementList addRealFromDouble(String name, double value, int magnitudeType);

	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added float
	 * @return reference to this object
	 */
	public ElementList addFloat(String name, float value);

	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added double
	 * @return reference to this object
	 */
	public ElementList addDouble(String name, double value);

	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmDate is invalid
	 * 
	 * @param name - String object containing ElementEntry name
	 * @param year - added OmmDate year (0 - 4095 where 0 indicates blank)
	 * @param month - added OmmDate month (0 - 12 where 0 indicates blank)
	 * @param day - added OmmDate day (0 - 31 where 0 indicates blank)
	 * @return reference to this object
	 */
	public ElementList addDate(String name, int year, int month, int day);

	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param name - String object containing ElementEntry name
	 * @return reference to this object
	 */
	public ElementList addTime(String name);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param name - String object containing ElementEntry name
	 * @param hour - added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @return reference to this object
	 */
	public ElementList addTime(String name, int hour);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param name - String object containing ElementEntry name
	 * @param hour - added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute - added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @return reference to this object
	 */
	public ElementList addTime(String name, int hour, int minute);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param name - String object containing ElementEntry name
	 * @param hour - added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute - added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second - added OmmTime second (0 - 60 where 255 indicates blank)
	 * @return reference to this object
	 */
	public ElementList addTime(String name, int hour, int minute, int second);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param name - String object containing ElementEntry name
	 * @param hour - added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute - added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second - added OmmTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond - added OmmTime millisecond (0 - 999 where 65535 indicates blank)
	 * @return reference to this object
	 */
	public ElementList addTime(String name, int hour, int minute, int second, int millisecond);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param name - String object containing ElementEntry name
	 * @param hour - added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute - added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second - added OmmTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond - added OmmTime millisecond (0 - 999 where 65535 indicates blank)
	 * @param microsecond - added OmmTime microsecond (0 - 999 where 2047 indicates blank)
	 * @return reference to this object
	 */
	public ElementList addTime(String name, int hour, int minute, int second, int millisecond, int microsecond);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param name - String object containing ElementEntry name
	 * @param hour - added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute - added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second - added OmmTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond - added OmmTime millisecond (0 - 999 where 65535 indicates blank)
	 * @param microsecond - added OmmTime microsecond (0 - 999 where 2047 indicates blank)
	 * @param nanosecond - added OmmTime nanosecond (0 - 999 where 2047 indicates blank)
	 * @return reference to this object
	 */
	public ElementList addTime(String name, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param name - String object containing ElementEntry name
	 * @param year - added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month - added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day - added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @return reference to this object
	 */
	public ElementList addDateTime(String name, int year, int month, int day);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param name - String object containing ElementEntry name
	 * @param year - added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month - added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day - added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour - added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * @return reference to this object
	 */
	public ElementList addDateTime(String name, int year, int month, int day, int hour);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param name - String object containing ElementEntry name
	 * @param year - added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month - added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day - added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour - added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * @param minute - added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @return reference to this object
	 */
	public ElementList addDateTime(String name, int year, int month, int day, int hour, int minute);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param name - String object containing ElementEntry name
	 * @param year - added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month - added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day - added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour - added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * @param minute - added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @param second - added OmmDateTime second (0 - 60 where 255 indicates blank)
	 * @return reference to this object
	 */
	public ElementList addDateTime(String name, int year, int month, int day, int hour, int minute, 
									int second);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param name - String object containing ElementEntry name
	 * @param year - added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month - added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day - added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour - added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * @param minute - added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @param second - added OmmDateTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond - added OmmDateTime millisecond (0 - 999 where 65535 indicates blank)
	 * @return reference to this object
	 */
	public ElementList addDateTime(String name, int year, int month, int day, int hour, int minute,
									int second , int millisecond);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param name - String object containing ElementEntry name
	 * @param year - added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month - added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day - added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour - added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * @param minute - added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @param second - added OmmDateTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond - added OmmDateTime millisecond (0 - 999 where 65535 indicates blank)
	 * @param microsecond - added OmmDateTime microsecond (0 - 999 where 2047 indicates blank)
	 * @return reference to this object
	 */
	public ElementList addDateTime(String name, int year, int month, int day, int hour, int minute, 
									int second, int millisecond, int microsecond);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param name - String object containing ElementEntry name
	 * @param year - added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month - added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day - added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour - added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * @param minute - added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @param second - added OmmDateTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond - added OmmDateTime millisecond (0 - 999 where 65535 indicates blank)
	 * @param microsecond - added OmmDateTime microsecond (0 - 999 where 2047 indicates blank)
	 * @param nanosecond - added OmmDateTime nanosecond (0 - 999 where 2047 indicates blank)
	 * @return reference to this object
	 */
	public ElementList addDateTime(String name, int year, int month, int day, int hour, int minute,
									int second, int millisecond, int microsecond, int nanosecond);

	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @return reference to this object
	 */
	public ElementList addQos(String name);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param timeliness - added {@link com.thomsonreuters.ema.access.OmmQos.Timeliness}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmQos.Timeliness#REALTIME})
	 * @return reference to this object
	 */
	public ElementList addQos(String name, int timeliness);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param timeliness - added {@link com.thomsonreuters.ema.access.OmmQos.Timeliness}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmQos.Timeliness#REALTIME})
	 * @param rate - added {@link com.thomsonreuters.ema.access.OmmQos.Rate}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmQos.Rate#TICK_BY_TICK})
	 * @return reference to this object
	 */
	public ElementList addQos(String name, int timeliness, int rate);

	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @return reference to this object
	 */
	public ElementList addState(String name);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param streamState - added {@link com.thomsonreuters.ema.access.OmmState.StreamState} 
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StreamState#OPEN})
	 * @return reference to this object
	 */
	public ElementList addState(String name, int streamState);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param streamState - added {@link com.thomsonreuters.ema.access.OmmState.StreamState}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StreamState#OPEN})
	 * @param dataState - added {@link com.thomsonreuters.ema.access.OmmState.DataState}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.DataState#OK})
	 * @return reference to this object
	 */
	public ElementList addState(String name, int streamState, int dataState);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param streamState - added {@link com.thomsonreuters.ema.access.OmmState.StreamState}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StreamState#OPEN})
	 * @param dataState - added {@link com.thomsonreuters.ema.access.OmmState.DataState}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.DataState#OK})
	 * @param statusCode - added {@link com.thomsonreuters.ema.access.OmmState.StatusCode}
	 *       (default value is {@link com.thomsonreuters.ema.access.OmmState.StatusCode#NONE})
	 * @return reference to this object
	 */
	public ElementList addState(String name, int streamState, int dataState, int statusCode);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param streamState - added {@link com.thomsonreuters.ema.access.OmmState.StreamState}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StreamState#OPEN})
	 * @param dataState - added {@link com.thomsonreuters.ema.access.OmmState.DataState}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.DataState#OK})
	 * @param statusCode - added {@link com.thomsonreuters.ema.access.OmmState.StatusCode}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StatusCode#NONE})
	 * @param statusText - added OmmState text (default value is 'empty string')
	 * @return reference to this object
	 */
	public ElementList addState(String name, int streamState, int dataState, int statusCode, String statusText);

	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added int
	 * @return reference to this object
	 */
	public ElementList addEnum(String name, int value);

	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added ByteBuffer as OmmBuffer
	 * @return reference to this object
	 */
	public ElementList addBuffer(String name, ByteBuffer value);

	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added String as OmmAscii
	 * @return reference to this object
	 */
	public ElementList addAscii(String name, String value);

	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added ByteBuffer as OmmUtf8
	 * @return reference to this object
	 */
	public ElementList addUtf8(String name, ByteBuffer value);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added String as OmmUtf8
	 * @return reference to this object
	 */
	public ElementList addUtf8(String name, String value);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added ByteBuffer as OmmRmtes
	 * @return reference to this object
	 */
	public ElementList addRmtes(String name, ByteBuffer value);

	/**
	 * Adds an OmmArray of OMM data to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param name - String object containing ElementEntry name
	 * @param value - added OmmArray
	 * @return reference to this object
	 */
	public ElementList addArray(String name, OmmArray value);

	/**
	 * Adds a blank data code to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public ElementList addCodeInt(String name);

	/**
	 * Adds a blank data code to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public ElementList addCodeUInt(String name);

	/**
	 * Adds a blank data code to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public ElementList addCodeReal(String name);

	/**
	 * Adds a blank data code to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public ElementList addCodeFloat(String name);

	/**
	 * Adds a blank data code to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public ElementList addCodeDouble(String name);

	/**
	 * Adds a blank data code to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public ElementList addCodeDate(String name);

	/**
	 * Adds a blank data code to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public ElementList addCodeTime(String name);

	/**
	 * Adds a blank data code to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public ElementList addCodeDATETIME(String name);

	/**
	 * Adds a blank data code to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public ElementList addCodeQos(String name);

	/**
	 * Adds a blank data code to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public ElementList addCodeState(String name);

	/**
	 * Adds a blank data code to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public ElementList addCodeEnum(String name);

	/**
	 * Adds a blank data code to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public ElementList addCodeBuffer(String name);

	/**
	 * Adds a blank data code to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public ElementList addCodeASCII(String name);

	/**
	 * Adds a blank data code to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public ElementList addCodeUtf8(String name);

	/**
	 * Adds a blank data code to the ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public ElementList addCodeRmtes(String name);

	/**
	 * Completes encoding of ElementList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public ElementList complete();
}