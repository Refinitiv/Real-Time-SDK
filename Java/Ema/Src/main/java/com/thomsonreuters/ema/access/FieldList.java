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
 * FieldList is a heterogeneous container of complex and primitive data type entries.
 * 
 * <p>FieldList is a collection which provides iterator over the elements in this collection.</p>
 * 
 * <p>FieldList entries are identified by Field Id.
 * The meaning of the Field Id is conveyed by the RDMFieldDictionary.</p>
 *
 * <p>FieldList supports two methods of adding containers; they are:
 * 	- adding of already populated containers, (e.g. complete() was called) and
 * 	- adding of clear containers (e.g. clear() was called) which would be populated after that.</p>
 * 
 * <p>The first method of adding of already populated containers allows for easy data
 * manipulation but incurs additional memory copy. This method is useful in
 * applications extracting data containers from some messages or containers and then
 * setting them on other containers.</p>
 * 
 * <p>The second method allows for fast container population since it avoids additional
 * memory copy incurred by the first method. This method is useful in source applications
 * setting OMM data from native data formats.</p>
 */
public interface FieldList extends ComplexType, Collection<FieldEntry>
{
	/**
	 * Indicates presence of Info.
	 * 
	 * @return true if FieldList Info is set; false otherwise
	 */
	public boolean hasInfo();

	/**
	 * Returns InfoFieldListNum.
	 * 
	 * @throw OmmInvalidUsageException if hasInfo() returns false
	 * 
	 * @return FieldList Number
	 */
	public int infoFieldListNum();

	/**
	 * Returns InfoDictionaryId.
	 * 
	 * @throw OmmInvalidUsageException if hasInfo() returns false
	 * 
	 * @return DictionaryId associated with this FieldList
	 */
	public int infoDictionaryId();

	/**
	 * Iterates through a list of Data of any DataType. Typical usage is to
	 * extract the entry during each iteration via getEntry().
	 * 
	 * @return true at the end of FieldList; false otherwise
	 */
	public boolean forth();
	/**
	 * Iterates through a list of Data having the FieldId matching in the field
	 * dictionary. Typical usage is to extract the entry during each iteration
	 * via getEntry().
	 * 
	 * @param fieldId looked up FieldEntry's FieldId
	 * @return true at the end of FieldList; false otherwise
	 */
	public boolean forth(int fieldId);

	/**
	 * Iterates through a list of Data having the name matching the acronym in
	 * the field dictionary. Typical usage is to extract the entry during each
	 * iteration via getEntry().
	 * 
	 * @param name looked up FieldEntry's Field name (from
	 *            RdmFieldDictionary)
	 * @return true at the end of FieldList; false otherwise
	 */
	public boolean forth(String name);

	/**
	 * Iterates through a list of Data having the name matching the specified
	 * Data. Typical usage is for the Data to be a view, and thus extract each
	 * matched entry during each iteration via getEntry().
	 * 
	 * @param data containing ElementList with a view definition specifying
	 *            looked up field Ids
	 * @return true at the end of FieldList; false otherwise
	 */
	public boolean forth(Data data);

	/**
	 * Returns Entry.
	 * 
	 * @throw OmmInvalidUsageException if forth() was not called first
	 * 
	 * @return FieldEntry
	 */
	public FieldEntry entry();

	/**
	 * Resets iteration to start of container.
	 */
	public void reset();

	/**
	 * Clears the FieldList.
	 * Invoking clear() method clears all the values and resets all the defaults
	 * 
	 * @return reference to this object
	 */
	public void clear();

	/**
	 * Specifies Info.
	 * The FieldList Info is optional.
	 * If used, it must be set prior to adding anything to FieldList.
	 * 
	 * @param dictionaryId dictionary id of the RdmFieldDictioanry
	 *            associated with this FieldList
	 * @param fieldListNum FieldList template number
	 * @return reference to this object
	 */
	public FieldList info(int dictionaryId, int fieldListNum);

	/**
	 * Adds a complex type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added ReqMsg
	 * @return reference to this object
	 */
	public FieldList addReqMsg(int fieldId, ReqMsg value);

	/**
	 * Adds a complex type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added RefreshMsg
	 * @return reference to this object
	 */
	public FieldList addRefreshMsg(int fieldId, RefreshMsg value);

	/**
	 * Adds a complex type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added StatusMsg
	 * @return reference to this object
	 */
	public FieldList addStatusMsg(int fieldId, StatusMsg value);

	/**
	 * Adds a complex type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added UpdateMsg
	 * @return reference to this object
	 */
	public FieldList addUpdateMsg(int fieldId, UpdateMsg value);

	/**
	 * Adds a complex type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added PostMsg
	 * @return reference to this object
	 */
	public FieldList addPostMsg(int fieldId, PostMsg value);

	/**
	 * Adds a complex type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @param fieldId field id value
	 * @param value added AckMsg
	 * @return reference to this object
	 */
	public FieldList addAckMsg(int fieldId, AckMsg value);

	/**
	 * Adds a complex type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added GenericMsg
	 * @return reference to this object
	 */
	public FieldList addGenericMsg(int fieldId, GenericMsg value);

	/**
	 * Adds a complex type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @param fieldId field id value
	 * @param value added FieldList
	 * @return reference to this object
	 */
	public FieldList addFieldList(int fieldId, FieldList value);

	/**
	 * Adds a complex type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added ElementList
	 * @return reference to this object
	 */
	public FieldList addElementList(int fieldId, ElementList value);

	/**
	 * Adds a complex type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added Map
	 * @return reference to this object
	 */
	public FieldList addMap(int fieldId, Map value);

	/**
	 * Adds a complex type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added Vector
	 * @return reference to this object
	 */
	public FieldList addVector(int fieldId, Vector value);

	/**
	 * Adds a complex type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added Series
	 * @return reference to this object
	 */
	public FieldList addSeries(int fieldId, Series value);

	/**
	 * Adds a complex type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added FilterList
	 * @return reference to this object
	 */
	public FieldList addFilterList(int fieldId, FilterList value);

	/**
	 * Adds a complex type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added OmmOpaque
	 * @return reference to this object
	 */
	public FieldList addOpaque(int fieldId, OmmOpaque value);

	/**
	 * Adds a complex type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added OmmXml
	 * @return reference to this object
	 */
	public FieldList addXml(int fieldId, OmmXml value);

	/**
	 * Adds a complex type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added OmmAnsiPage
	 * @return reference to this object
	 */
	public FieldList addAnsiPage(int fieldId, OmmAnsiPage value);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added Int64
	 * @return reference to this object
	 */
	public FieldList addInt(int fieldId, long value);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added long
	 * @return reference to this object
	 */
	public FieldList addUInt(int fieldId, long value);
	
	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added BigInteger
	 * @return reference to this object
	 */
	public FieldList addUInt(int fieldId, BigInteger value);
	
	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
 	 * @param mantissa - added OmmReal mantissa   
	 * @param magnitudeType - added {@link com.thomsonreuters.ema.access.OmmReal.MagnitudeType}
	 * @return reference to this object
	 */
	public FieldList addReal(int fieldId, long mantissa, int magnitudeType);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added double to be converted to {@link com.thomsonreuters.ema.access.OmmReal}
	 * @return reference to this object
	 */
	public FieldList addRealFromDouble(int fieldId, double value);
	
	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added double to be converted to {@link com.thomsonreuters.ema.access.OmmReal}
	 * @param magnitudeType {@link com.thomsonreuters.ema.access.OmmReal.MagnitudeType} used for the conversion
	 *            (default value is {@link com.thomsonreuters.ema.access.OmmReal.MagnitudeType#EXPONENT_0})
	 * @return reference to this object
	 */
	public FieldList addRealFromDouble(int fieldId, double value, int magnitudeType);
	

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added float
	 * @return reference to this object
	 */
	public FieldList addFloat(int fieldId, float value);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added double
	 * @return reference to this object
	 */
	public FieldList addDouble(int fieldId, double value);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmDate is invalid
	 * 
	 * @param fieldId field id value
	 * @param year added OmmDate year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDate month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDate day (0 - 31 where 0 indicates blank)
	 * @return reference to this object
	 */
	public FieldList addDate(int fieldId, int year, int month, int day);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param fieldId field id value
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmTime millisecond (0 - 999 where 65535 indicates blank)
	 * @param microsecond added OmmTime microsecond (0 - 999 where 2047 indicates blank)
	 * @param nanosecond added OmmTime nanosecond (0 - 999 where 2047 indicates blank)
	 * @return reference to this object
	 */
	public FieldList addTime(int fieldId);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param fieldId field id value
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @return reference to this object
	 */
	public FieldList addTime(int fieldId, int hour);
	
	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param fieldId field id value
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @return reference to this object
	 */
	public FieldList addTime(int fieldId, int hour, int minute);
	
	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param fieldId field id value
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * @return reference to this object
	 */
	public FieldList addTime(int fieldId, int hour, int minute, int second);
	
	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param fieldId field id value
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmTime millisecond (0 - 999 where 65535 indicates blank)
	 * @return reference to this object
	 */
	public FieldList addTime(int fieldId, int hour, int minute, int second,	int millisecond);
	
	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param fieldId field id value
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmTime millisecond (0 - 999 where 65535 indicates blank)
	 * @param microsecond added OmmTime microsecond (0 - 999 where 2047 indicates blank)
	 * @return reference to this object
	 */
	public FieldList addTime(int fieldId, int hour, int minute, int second,	int millisecond, int microsecond);
	
	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @throw OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param fieldId field id value
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmTime millisecond (0 - 999 where 65535 indicates blank)
	 * @param microsecond added OmmTime microsecond (0 - 999 where 2047 indicates blank)
	 * @param nanosecond added OmmTime nanosecond (0 - 999 where 2047 indicates blank)
	 * @return reference to this object
	 */
	public FieldList addTime(int fieldId, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond);
	
	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates  blank)
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
	public FieldList addDateTime(int fieldId, int year, int month, int day);
	
	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates  blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @return reference to this object
	 */
	public FieldList addDateTime(int fieldId, int year, int month, int day,	int hour);
	
	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates  blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * @return reference to this object
	 */
	public FieldList addDateTime(int fieldId, int year, int month, int day,	int hour, int minute);
	
	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates  blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * @param minute added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @return reference to this object
	 */
	public FieldList addDateTime(int fieldId, int year, int month, int day,	int hour, int minute, int second);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates  blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * @param minute added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmDateTime second (0 - 60 where 255 indicates blank)
	 * @return reference to this object
	 */
	public FieldList addDateTime(int fieldId, int year, int month, int day, int hour, int minute,
									int second , int millisecond);
	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates  blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * @param minute added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmDateTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmDateTime millisecond (0 - 999 where 65535 indicates blank)
	 * @return reference to this object
	 */
	public FieldList addDateTime(int fieldId, int year, int month, int day, int hour, int minute, 
									int second, int millisecond, int microsecond);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates  blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * @param minute added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmDateTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmDateTime millisecond (0 - 999 where 65535 indicates blank)
	 * @param microsecond added OmmDateTime microsecond (0 - 999 where 2047 indicates blank)
	 * @return reference to this object
	 */
	public FieldList addDateTime(int fieldId, int year, int month, int day, int hour, int minute,
									int second, int millisecond, int microsecond, int nanosecond);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @return reference to this object
	 */
	public FieldList addQos(int fieldId);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
 	 * @param timeliness - added {@link com.thomsonreuters.ema.access.OmmQos.Timeliness}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmQos.Timeliness#REALTIME})
	 * @return reference to this object
	 */
	public FieldList addQos(int fieldId, int timeliness);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
 	 * @param timeliness - added {@link com.thomsonreuters.ema.access.OmmQos.Timeliness}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmQos.Timeliness#REALTIME})
	 * @param rate - added {@link com.thomsonreuters.ema.access.OmmQos.Rate}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmQos.Rate#TICK_BY_TICK})
	 * @return reference to this object
	 */
	public FieldList addQos(int fieldId, int timeliness, int rate);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @return reference to this object
	 */
	public FieldList addState(int fieldId);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param streamState added {@link com.thomsonreuters.ema.access.OmmState.StreamState} 
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StreamState#OPEN})
	 * @return reference to this object
	 */
	public FieldList addState(int fieldId, int streamState);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param streamState added {@link com.thomsonreuters.ema.access.OmmState.StreamState} 
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StreamState#OPEN})
	 * @param dataState added {@link com.thomsonreuters.ema.access.OmmState.DataState}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.DataState#OK})
	 * @return reference to this object
	 */
	public FieldList addState(int fieldId, int streamState, int dataState);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param streamState added {@link com.thomsonreuters.ema.access.OmmState.StreamState} 
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StreamState#OPEN})
	 * @param dataState added {@link com.thomsonreuters.ema.access.OmmState.DataState}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.DataState#OK})
	 * @param statusCode added {@link com.thomsonreuters.ema.access.OmmState.StatusCode}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StatusCode#NONE})
	 * @return reference to this object
	 */
	public FieldList addState(int fieldId, int streamState, int dataState, int statusCode);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param streamState added {@link com.thomsonreuters.ema.access.OmmState.StreamState} 
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StreamState#OPEN})
	 * @param dataState added {@link com.thomsonreuters.ema.access.OmmState.DataState}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.DataState#OK})
	 * @param statusCode added {@link com.thomsonreuters.ema.access.OmmState.StatusCode}
	 *        (default value is {@link com.thomsonreuters.ema.access.OmmState.StatusCode#NONE})
	 * @param statusText added OmmState text (default value is 'empty string')
	 * @return reference to this object
	 */
	public FieldList addState(int fieldId, int streamState, int dataState, int statusCode, String statusText);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added Enum
	 * @return reference to this object
	 */
	public FieldList addEnum(int fieldId, int value);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added ByteBuffer as OmmBuffer
	 * @return reference to this object
	 */
	public FieldList addBuffer(int fieldId, ByteBuffer value);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added String as OmmASCII
	 * @return reference to this object
	 */
	//java doc
	public FieldList addAscii(int fieldId, String value);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added ByteBuffer as OmmUtf8
	 * @return reference to this object
	 */
	public FieldList addUtf8(int fieldId, ByteBuffer value);

	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added String as OmmUtf8 (String has to be Utf8 charset)
	 * @return reference to this object
	 */
	public FieldList addUtf8(int fieldId, String value);
	
	/**
	 * Adds a specific simple type of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added ByteBuffer as OmmRmtes
	 * @return reference to this object
	 */
	public FieldList addRmtes(int fieldId, ByteBuffer value);

	/**
	 * Adds an OmmArray of OMM data to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added OmmArray
	 * @return reference to this object
	 */
	public FieldList addArray(int fieldId, OmmArray value);

	/**
	 * Adds a blank data code to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public FieldList addCodeInt(int fieldId);

	/**
	 * Adds a blank data code to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public FieldList addCodeUInt(int fieldId);

	/**
	 * Adds a blank data code to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public FieldList addCodeReal(int fieldId);

	/**
	 * Adds a blank data code to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public FieldList addCodeFloat(int fieldId);

	/**
	 * Adds a blank data code to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public FieldList addCodeDouble(int fieldId);

	/**
	 * Adds a blank data code to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public FieldList addCodeDate(int fieldId);

	/**
	 * Adds a blank data code to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 * @return reference to this object
	 */
	public FieldList addCodeTime(int fieldId);

	/**
	 * Adds a blank data code to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public FieldList addCodeDATETIME(int fieldId);

	/**
	 * Adds a blank data code to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public FieldList addCodeQos(int fieldId);

	/**
	 * Adds a blank data code to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public FieldList addCodeState(int fieldId);

	/**
	 * Adds a blank data code to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public FieldList addCodeEnum(int fieldId);

	/**
	 * Adds a blank data code to the FieldList.
	 * 
	 * @return reference to this object
	 */
	public FieldList addCodeBuffer(int fieldId);

	/**
	 * Adds a blank data code to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public FieldList addCodeASCII(int fieldId);

	/**
	 * Adds a blank data code to the FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public FieldList addCodeUtf8(int fieldId);

	/**
	 * Adds a blank data code to the FieldList.
	 * 
	 * @return reference to this object
	 */
	public FieldList addCodeRmtes(int fieldId);

	/**
	 * Completes encoding of FieldList.
	 * 
	 * @throw OmmInvalidUsageException if an error is detected (exception will
	 *        specify the cause of the error)
	 *        
	 * @return reference to this object
	 */
	public FieldList complete();
}