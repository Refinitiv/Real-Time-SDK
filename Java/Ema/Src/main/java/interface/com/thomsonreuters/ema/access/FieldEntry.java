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
 * FieldEntry represents an entry of FieldList.
 * <br>FieldEntry associates entry's field id, name, data and its data type.
 * 
 * Code snippet:
 * <pre>
 * void decode(FieldList fieldList)
 * {
 *    for(FieldEntry fieldEntry : fieldList)
 *    {
 *       System.out.print(" Name = " + fieldEntry.name()
 *                                   + " DataType: " + DataType.asString(fieldEntry.load().dataType()) + " Value: ");
 *       if (Data.DataCode.BLANK == fieldEntry.code())
 *           System.out.println(" blank");
 *       else
 *           switch(fieldEntry.loadType())
 *           {
 *           case DataTypes.REAL :
 *               System.out.println(fieldEntry.real().asDouble());
 *               break;
 *           case DataTypes.DATE :
 *               System.out.println(fieldEntry.date().day() + " / " + fieldEntry.date().month() + " / " + fieldEntry.date().year());
 *               break;
 *           case DataTypes.TIME :
 *               System.out.println(fieldEntry.time().hour() + ":" + fieldEntry.time().minute()+ ":" 
 *                                  + fieldEntry.time().second() + ":" + fieldEntry.time().millisecond());
 *               break;
 *           case DataTypes.INT :
 *               System.out.println(fieldEntry.intValue());
 *               break;
 *           case DataTypes.UINT :
 *               System.out.println(fieldEntry.uintValue());
 *               break;
 *           case DataTypes.ASCII :
 *               System.out.println(fieldEntry.ascii());
 *               break;
 *           case DataTypes.ENUM :
 *               System.out.println(fieldEntry.enumValue());
 *               break;
 *           case DataTypes.ERROR :
 *               System.out.println(fieldEntry.error().errorCode() +" (" + fieldEntry.error().errorCodeAsString() + ")");
 *               break;
 *           default :
 *               System.out.println();
 *               break;
 *           }
 *    }
 * }
 * </pre>
 *
 * @see Data
 * @see ComplexType
 * @see ReqMsg
 * @see RefreshMsg
 * @see UpdateMsg
 * @see StatusMsg
 * @see GenericMsg
 * @see PostMsg
 * @see AckMsg
 * @see FieldList
 * @see ElementList
 * @see Map
 * @see Vector
 * @see Series
 * @see FilterList
 * @see OmmOpaque
 * @see OmmXml
 * @see OmmAnsiPage
 * @see OmmError
 */
public interface FieldEntry
{
	/**
	 * Returns the DataType of the entry's load.<br>
	 * Return of {@link com.thomsonreuters.ema.access.DataType.DataTypes#NO_DATA} signifies no data present in load.<br>
	 * Return of {@link com.thomsonreuters.ema.access.DataType.DataTypes#ERROR} signifies error while extracting content of load.
	 * 
	 * @return data type of the contained object
	 */
	public int loadType();

	/**
	 * Returns the Code of the entry's load.<br>
	 * The code indicates a special state of a Data.<br><br>
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
	 * Returns acronym field name associated to the FieldId from the field dictionary.<br>
	 * Returns empty string if FieldId is not found in field dictionary.
	 * 
	 * @return String containing name of the entry
	 */
	public String name();

	/**
	 * Returns a ripple FieldId if the current entry has a ripple field.<br>
	 * A subsequent call using the former non-zero return value as a formal parameter,
	 * returns the next ripple field in a ripple sequence.<br>
	 * Returns zero if no ripple field or the final ripple field of a ripple sequence.
	 *
	 * @param fieldId field id value
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
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.ReqMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.ReqMsg} class reference to contained object
	 */
	public ReqMsg reqMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.RefreshMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.RefreshMsg} class reference to contained object
	 */
	public RefreshMsg refreshMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.UpdateMsg}
	 * @return {@link com.thomsonreuters.ema.access.UpdateMsg} class reference to contained object
	 */
	public UpdateMsg updateMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.StatusMsg}
	 * @return {@link com.thomsonreuters.ema.access.StatusMsg} class reference to contained object
	 */
	public StatusMsg statusMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.PostMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.PostMsg} class reference to contained object
	 */
	public PostMsg postMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.AckMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.AckMsg} class reference to contained object
	 */
	public AckMsg ackMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.GenericMsg}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.GenericMsg} class reference to contained object
	 */
	public GenericMsg genericMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.FieldList}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.FieldList} class reference to contained object
	 */
	public FieldList fieldList();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.ElementList}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.ElementList} class reference to contained object
	 */
	public ElementList elementList();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.Map}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.Map} class reference to contained object
	 */
	public Map map();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.Vector}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.Vector} class reference to contained object
	 */
	public Vector vector();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.Series}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.Series} class reference to contained object
	 */
	public Series series();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.FilterList}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.FilterList} class reference to contained object
	 */
	public FilterList filterList();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmOpaque}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmOpaque} class reference to contained object
	 */
	public OmmOpaque opaque();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmXml}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmXml} class reference to contained object
	 */
	public OmmXml xml();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmAnsiPage}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmAnsiPage} class reference to contained object
	 */
	public OmmAnsiPage ansiPage();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmInt}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return long
	 */
	public long intValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmInt}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmInt}
	 */
	public OmmInt ommIntValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmUInt}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return long
	 */
	public long uintValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmUInt}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmUInt}
	 */
	public OmmUInt ommUIntValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmReal}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmReal} class reference to contained object
	 */
	public OmmReal real();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmFloat}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return float
	 */
	public float floatValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmFloat}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmFloat}
	 */
	public OmmFloat ommFloatValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmDouble}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return double
	 */
	public double doubleValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmDouble}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmDouble}
	 */
	public OmmDouble ommDoubleValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmDate}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmDate} class reference to contained object
	 */
	public OmmDate date();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmTime}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmTime} class reference to contained object
	 */
	public OmmTime time();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmDateTime}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmDateTime} class reference to contained object
	 */
	public OmmDateTime dateTime();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmQos}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmQos} class reference to contained object
	 */
	public OmmQos qos();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmState}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmState} class reference to contained object
	 */
	public OmmState state();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmEnum}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return int
	 */
	public int enumValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmEnum}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmEnum}
	 */
	public OmmEnum ommEnumValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmBuffer}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmBuffer}
	 */
	public OmmBuffer buffer();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmAscii}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmAscii}
	 */
	public OmmAscii ascii();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmUtf8}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmUtf8}
	 */
	public OmmUtf8 utf8();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmRmtes}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmRmtes}
	 */
	public OmmRmtes rmtes();

	/**
	 * Returns current OMM data represented as an OmmArray.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmArray}
	 * @throws OmmInvalidUsageException if {@link com.thomsonreuters.ema.access.Data#code()} returns {@link com.thomsonreuters.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmArray} class reference to contained object
	 */
	public OmmArray array();

	/**
	 * Returns Error.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.thomsonreuters.ema.access.OmmError}
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmError} class reference to contained object
	 */
	public OmmError error();
	
	/**
	 * Adds a complex type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added ReqMsg
	 * 
	 * @return reference to this object
	 */
	public FieldEntry reqMsg(int fieldId, ReqMsg value);

	/**
	 * Adds a complex type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added RefreshMsg
	 * 
	 * @return reference to this object
	 */
	public FieldEntry refreshMsg(int fieldId, RefreshMsg value);

	/**
	 * Adds a complex type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added StatusMsg
	 * 
	 * @return reference to this object
	 */
	public FieldEntry statusMsg(int fieldId, StatusMsg value);

	/**
	 * Adds a complex type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added UpdateMsg
	 * 
	 * @return reference to this object
	 */
	public FieldEntry updateMsg(int fieldId, UpdateMsg value);

	/**
	 * Adds a complex type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added PostMsg
	 * 
	 * @return reference to this object
	 */
	public FieldEntry postMsg(int fieldId, PostMsg value);

	/**
	 * Adds a complex type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *         
	 * @param fieldId field id value
	 * @param value added AckMsg
	 * 
	 * @return reference to this object
	 */
	public FieldEntry ackMsg(int fieldId, AckMsg value);

	/**
	 * Adds a complex type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added GenericMsg
	 * 
	 * @return reference to this object
	 */
	public FieldEntry genericMsg(int fieldId, GenericMsg value);

	/**
	 * Adds a complex type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *         
	 * @param fieldId field id value
	 * @param value added FieldList
	 * 
	 * @return reference to this object
	 */
	public FieldEntry fieldList(int fieldId, FieldList value);

	/**
	 * Adds a complex type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added ElementList
	 * 
	 * @return reference to this object
	 */
	public FieldEntry elementList(int fieldId, ElementList value);

	/**
	 * Adds a complex type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added Map
	 * 
	 * @return reference to this object
	 */
	public FieldEntry map(int fieldId, Map value);

	/**
	 * Adds a complex type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *
	 * @param fieldId field id value
	 * @param value added Vector
	 * 
	 * @return reference to this object
	 */
	public FieldEntry vector(int fieldId, Vector value);

	/**
	 * Adds a complex type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added Series
	 * 
	 * @return reference to this object
	 */
	public FieldEntry series(int fieldId, Series value);

	/**
	 * Adds a complex type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added FilterList
	 * 
	 * @return reference to this object
	 */
	public FieldEntry filterList(int fieldId, FilterList value);

	/**
	 * Adds a complex type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added OmmOpaque
	 * 
	 * @return reference to this object
	 */
	public FieldEntry opaque(int fieldId, OmmOpaque value);

	/**
	 * Adds a complex type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added OmmXml
	 * 
	 * @return reference to this object
	 */
	public FieldEntry xml(int fieldId, OmmXml value);

	/**
	 * Adds a complex type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added OmmAnsiPage
	 * 
	 * @return reference to this object
	 */
	public FieldEntry ansiPage(int fieldId, OmmAnsiPage value);

	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added int
	 * 
	 * @return reference to this object
	 */
	public FieldEntry intValue(int fieldId, long value);

	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added long
	 * 
	 * @return reference to this object
	 */
	public FieldEntry uintValue(int fieldId, long value);
		
	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added BigInteger
	 * 
	 * @return reference to this object
	 */
	public FieldEntry uintValue(int fieldId, BigInteger value);
		
	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
 	 * @param mantissa added OmmReal mantissa   
	 * @param magnitudeType added {@link com.thomsonreuters.ema.access.OmmReal.MagnitudeType}
	 * 
	 * @return reference to this object
	 */
	public FieldEntry real(int fieldId, long mantissa, int magnitudeType);

	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * Default magnitudeType is {@link com.thomsonreuters.ema.access.OmmReal.MagnitudeType#EXPONENT_0}
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added double to be converted to {@link com.thomsonreuters.ema.access.OmmReal}
	 * 
	 * @return reference to this object
	 */
	public FieldEntry realFromDouble(int fieldId, double value);
		
	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added double to be converted to {@link com.thomsonreuters.ema.access.OmmReal}
	 * @param magnitudeType {@link com.thomsonreuters.ema.access.OmmReal.MagnitudeType} used for the conversion
	 * 
	 * @return reference to this object
	 */
	public FieldEntry realFromDouble(int fieldId, double value, int magnitudeType);
		

	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added float
	 * 
	 * @return reference to this object
	 */
	public FieldEntry floatValue(int fieldId, float value);

	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added double
	 * 
	 * @return reference to this object
	 */
	public FieldEntry doubleValue(int fieldId, double value);

	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 * @throws OmmOutOfRangeException if passed in OmmDate is invalid
	 * 
	 * @param fieldId field id value
	 * @param year added OmmDate year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDate month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDate day (0 - 31 where 0 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public FieldEntry date(int fieldId, int year, int month, int day);

	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * Defaults: second=0, millisecond=0, microsecond=0, nanosecond=0
	 *
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 * @throws OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param fieldId field id value
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public FieldEntry time(int fieldId, int hour, int minute);
		
	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * Defaults: millisecond=0, microsecond=0, nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 * @throws OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param fieldId field id value
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public FieldEntry time(int fieldId, int hour, int minute, int second);
		
	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * Defaults: microsecond=0, nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 * @throws OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param fieldId field id value
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmTime millisecond (0 - 999 where 65535 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public FieldEntry time(int fieldId, int hour, int minute, int second, int millisecond);
		
	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * Defaults: nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 * @throws OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param fieldId field id value
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmTime millisecond (0 - 999 where 65535 indicates blank)
	 * @param microsecond added OmmTime microsecond (0 - 999 where 2047 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public FieldEntry time(int fieldId, int hour, int minute, int second, int millisecond, int microsecond);
		
	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 * @throws OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param fieldId field id value
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmTime millisecond (0 - 999 where 65535 indicates blank)
	 * @param microsecond added OmmTime microsecond (0 - 999 where 2047 indicates blank)
	 * @param nanosecond added OmmTime nanosecond (0 - 999 where 2047 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public FieldEntry time(int fieldId, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond);
		
	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.<br>
	 * Defaults: hour=0, minute=0, second=0, millisecond=0, microsecond=0, nanosecond=0
	 * 
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates  blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public FieldEntry dateTime(int fieldId, int year, int month, int day);
		
	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.<br>
	 * Defaults: minute=0, second=0, millisecond=0, microsecond=0, nanosecond=0
	 * 
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates  blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public FieldEntry dateTime(int fieldId, int year, int month, int day, int hour);
		
	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.<br>
	 * Defaults: second=0, millisecond=0, microsecond=0, nanosecond=0
	 * 
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates  blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * @param minute added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public FieldEntry dateTime(int fieldId, int year, int month, int day, int hour, int minute);
		
	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.<br>
	 * Defaults: millisecond=0, microsecond=0, nanosecond=0
	 * 
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates  blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * @param minute added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmDateTime second (0 - 60 where 255 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public FieldEntry dateTime(int fieldId, int year, int month, int day, int hour, int minute, int second);

	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.<br>
	 * Defaults: microsecond=0, nanosecond=0
	 * 
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates  blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 59 where 255 indicates blank)
	 * @param minute added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmDateTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmDateTime millisecond (0 - 999 where 65535 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public FieldEntry dateTime(int fieldId, int year, int month, int day, int hour, int minute, int second , int millisecond);
	
	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.<br>
	 * Defaults: nanosecond=0
	 * 
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
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
	 * 
	 * @return reference to this object
	 */
	public FieldEntry dateTime(int fieldId, int year, int month, int day, int hour, int minute, int second, int millisecond, int microsecond);

	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
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
	 * 
	 * @return reference to this object
	 */
	public FieldEntry dateTime(int fieldId, int year, int month, int day, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond);

	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * Default rate is {@link com.thomsonreuters.ema.access.OmmQos.Rate#TICK_BY_TICK}
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
 	 * @param timeliness added {@link com.thomsonreuters.ema.access.OmmQos.Timeliness}
 	 * 
	 * @return reference to this object
	 */
	public FieldEntry qos(int fieldId, int timeliness);

	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
 	 * @param timeliness added {@link com.thomsonreuters.ema.access.OmmQos.Timeliness}
	 * @param rate added {@link com.thomsonreuters.ema.access.OmmQos.Rate}
	 * @return reference to this object
	 */
	public FieldEntry qos(int fieldId, int timeliness, int rate);

	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.<br>
	 * Default dataState is {@link com.thomsonreuters.ema.access.OmmState.DataState#OK}<br>
	 * Default statusCode is {@link com.thomsonreuters.ema.access.OmmState.StatusCode#NONE}<br>
	 * Default statusText is an empty String
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param streamState added {@link com.thomsonreuters.ema.access.OmmState.StreamState}
	 * 
	 * @return reference to this object
	 */
	public FieldEntry state(int fieldId, int streamState);

	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.<br>
	 * Default statusCode is {@link com.thomsonreuters.ema.access.OmmState.StatusCode#NONE}<br>
	 * Default statusText is an empty String
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param streamState added {@link com.thomsonreuters.ema.access.OmmState.StreamState} 
	 * @param dataState added {@link com.thomsonreuters.ema.access.OmmState.DataState}
	 * 
	 * @return reference to this object
	 */
	public FieldEntry state(int fieldId, int streamState, int dataState);

	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.<br>
	 * Default statusText is an empty String
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param streamState added {@link com.thomsonreuters.ema.access.OmmState.StreamState} 
	 * @param dataState added {@link com.thomsonreuters.ema.access.OmmState.DataState}
	 * @param statusCode added {@link com.thomsonreuters.ema.access.OmmState.StatusCode}
	 * 
	 * @return reference to this object
	 */
	public FieldEntry state(int fieldId, int streamState, int dataState, int statusCode);

	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param streamState added {@link com.thomsonreuters.ema.access.OmmState.StreamState} 
	 * @param dataState added {@link com.thomsonreuters.ema.access.OmmState.DataState}
	 * @param statusCode added {@link com.thomsonreuters.ema.access.OmmState.StatusCode}
	 * @param statusText added OmmState text
	 * 
	 * @return reference to this object
	 */
	public FieldEntry state(int fieldId, int streamState, int dataState, int statusCode, String statusText);

	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added Enum
	 * 
	 * @return reference to this object
	 */
	public FieldEntry enumValue(int fieldId, int value);

	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added ByteBuffer as OmmBuffer
	 * 
	 * @return reference to this object
	 */
	public FieldEntry buffer(int fieldId, ByteBuffer value);

	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added String as OmmASCII
	 * 
	 * @return reference to this object
	 */
	public FieldEntry ascii(int fieldId, String value);

	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added ByteBuffer as OmmUtf8
	 * 
	 * @return reference to this object
	 */
	public FieldEntry utf8(int fieldId, ByteBuffer value);

	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added String as OmmUtf8 (String has to be Utf8 charset)
	 * 
	 * @return reference to this object
	 */
	public FieldEntry utf8(int fieldId, String value);
		
	/**
	 * Adds a specific simple type of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added ByteBuffer as OmmRmtes
	 * 
	 * @return reference to this object
	 */
	public FieldEntry rmtes(int fieldId, ByteBuffer value);

	/**
	 * Adds an OmmArray of OMM data to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @param value added OmmArray
	 * 
	 * @return reference to this object
	 */
	public FieldEntry array(int fieldId, OmmArray value);

	/**
	 * Adds a blank data code to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *
	 * @param fieldId field id value
	 * @return reference to this object
	 */
	public FieldEntry codeInt(int fieldId);

	/**
	 * Adds a blank data code to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @return reference to this object
	 */
	public FieldEntry codeUInt(int fieldId);

	/**
	 * Adds a blank data code to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @return reference to this object
	 */
	public FieldEntry codeReal(int fieldId);

	/**
	 * Adds a blank data code to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @return reference to this object
	 */
	public FieldEntry codeFloat(int fieldId);

	/**
	 * Adds a blank data code to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @return reference to this object
	 */
	public FieldEntry codeDouble(int fieldId);

	/**
	 * Adds a blank data code to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @return reference to this object
	 */
	public FieldEntry codeDate(int fieldId);

	/**
	 * Adds a blank data code to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *
	 * @param fieldId field id value
	 * @return reference to this object
	 */
	public FieldEntry codeTime(int fieldId);

	/**
	 * Adds a blank data code to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @return reference to this object
	 */
	public FieldEntry codeDateTime(int fieldId);

	/**
	 * Adds a blank data code to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @return reference to this object
	 */
	public FieldEntry codeQos(int fieldId);

	/**
	 * Adds a blank data code to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @return reference to this object
	 */
	public FieldEntry codeState(int fieldId);

	/**
	 * Adds a blank data code to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @return reference to this object
	 */
	public FieldEntry codeEnum(int fieldId);

	/**
	 * Adds a blank data code to the FieldEntry.
	 * 
	 * @param fieldId field id value
	 * @return reference to this object
	 */
	public FieldEntry codeBuffer(int fieldId);

	/**
	 * Adds a blank data code to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @return reference to this object
	 */
	public FieldEntry codeAscii(int fieldId);

	/**
	 * Adds a blank data code to the FieldEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param fieldId field id value
	 * @return reference to this object
	 */
	public FieldEntry codeUtf8(int fieldId);

	/**
	 * Adds a blank data code to the FieldEntry.
	 * 
	 * @param fieldId field id value
	 * @return reference to this object
	 */
	public FieldEntry codeRmtes(int fieldId);
}