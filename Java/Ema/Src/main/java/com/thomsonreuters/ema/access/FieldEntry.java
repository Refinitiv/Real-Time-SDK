///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/**
 * FieldEntry represents an entry of FieldList.
 * FieldEntry associates entry's field id, name, data and its data type.
 */
public interface FieldEntry
{
	/**
	 * Returns the DataType of the entry's load.
	 * Return of {@link com.thomsonreuters.ema.access.DataType.DataTypes#NO_DATA} signifies no data present in load.
	 * Return of {@link com.thomsonreuters.ema.access.DataType.DataTypes#ERROR}
	 * 
	 * signifies error while extracting content of load.
	 * 
	 * @return data type of the contained object
	 */
	public int loadType();

	/**
	 * Returns the Code of the entry's load.
	 * The code indicates a special state of a Data.
	 * 
	 * Attempts to extract data will cause OmmInvalidUsageException
	 * if {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK} is returned.
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
	 * Returns FieldId.
	 * 
	 * @return field id
	 */
	public int fieldId();

	/**
	 * Returns acronym field name associated to the FieldId from the field dictionary.
	 * Returns empty string if FieldId is not found in field dictionary.
	 * 
	 * @return String containing name of the entry
	 */
	public String name();

	/**
	 * Returns a ripple FieldId if the current entry has a ripple field.
	 * A subsequent call using the former non-zero return value as a formal parameter,
	 * returns the next ripple field in a ripple sequence.
	 * Returns zero if no ripple field or the final ripple field of a ripple sequence.
	 * 
	 * @param fieldId
	 * @return field id
	 */
	public int rippleTo(int fieldId);

	/**
	 * Returns the contained Data based on the DataType.
	 * 
	 * @return Data class reference to contained object
	 */
	public Data load();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.ReqMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.ReqMsg} class reference to contained object
	 */
	public ReqMsg reqMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.RefreshMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.RefreshMsg} class reference to contained object
	 */
	public RefreshMsg refreshMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.UpdateMsg}
	 * @return {@link com.thomsonreuters.ema.access.UpdateMsg} class reference to contained object
	 */
	public UpdateMsg updateMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.StatusMsg}
	 * @return {@link com.thomsonreuters.ema.access.StatusMsg} class reference to contained object
	 */
	public StatusMsg statusMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.PostMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.PostMsg} class reference to contained object
	 */
	public PostMsg postMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.AckMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.AckMsg} class reference to contained object
	 */
	public AckMsg ackMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.GenericMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.GenericMsg} class reference to contained object
	 */
	public GenericMsg genericMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.FieldList}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.FieldList} class reference to contained object
	 */
	public FieldList fieldList();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.ElementList}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.ElementList} class reference to contained object
	 */
	public ElementList elementList();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.Map}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.Map} class reference to contained object
	 */
	public Map map();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.Vector}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.Vector} class reference to contained object
	 */
	public Vector vector();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.Series}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.Series} class reference to contained object
	 */
	public Series series();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.FilterList}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.FilterList} class reference to contained object
	 */
	public FilterList filterList();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmOpaque}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmOpaque} class reference to contained object
	 */
	public OmmOpaque opaque();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmXml}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmXml} class reference to contained object
	 */
	public OmmXml xml();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmAnsiPage}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmAnsiPage} class reference to contained object
	 */
	public OmmAnsiPage ansiPage();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmInt}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return long
	 */
	public long intValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmInt}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmInt}
	 */
	public OmmInt ommIntValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmUInt}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return long
	 */
	public long uintValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmUInt}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmUInt}
	 */
	public OmmUInt ommUIntValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmReal}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmReal} class reference to contained object
	 */
	public OmmReal real();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmFloat}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return float
	 */
	public float floatValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmFloat}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmFloat}
	 */
	public OmmFloat ommFloatValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmDouble}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return double
	 */
	public double doubleValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmDouble}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmDouble}
	 */
	public OmmDouble ommDoubleValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmDate}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmDate} class reference to contained object
	 */
	public OmmDate date();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmTime}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmTime} class reference to contained object
	 */
	public OmmTime time();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmDateTime}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmDateTime} class reference to contained object
	 */
	public OmmDateTime dateTime();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmQos}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmQos} class reference to contained object
	 */
	public OmmQos qos();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmState}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmState} class reference to contained object
	 */
	public OmmState state();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmEnum}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return int
	 */
	public int enumValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmEnum}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmEnum}
	 */
	public OmmEnum ommEnumValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmBuffer}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmBuffer}
	 */
	public OmmBuffer buffer();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmAscii}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmAscii}
	 */
	public OmmAscii ascii();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmUtf8}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmUtf8}
	 */
	public OmmUtf8 utf8();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmRmtes}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmRmtes}
	 */
	public OmmRmtes rmtes();

	/**
	 * Returns current OMM data represented as an OmmArray.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmArray}
	 * @throw OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmArray} class reference to contained object
	 */
	public OmmArray array();

	/**
	 * Returns Error.
	 * 
	 * @throw OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmError}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmError} class reference to contained object
	 */
	public OmmError error();
}