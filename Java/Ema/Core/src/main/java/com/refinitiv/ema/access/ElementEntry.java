///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.math.BigInteger;
import java.nio.ByteBuffer;

/**
 * ElementEntry represents an entry of ElementList.<br>
 * ElementEntry associates entry's name, data and its data type.<br><br>
 * 
 * Code snippet:
 * <pre>
 * void decode(ElementList elementList)
 * {
 *    for(ElementEntry elementEntry : elementList)
 *    {
 *       System.out.print(" Name = " + elementEntry.name()
 *                                   + " DataType: " + DataType.asString(elementEntry.load().dataType()) + " Value: ");
 *       if (Data.DataCode.BLANK == elementEntry.code())
 *           System.out.println(" blank");
 *       else
 *           switch(elementEntry.loadType())
 *           {
 *           case DataTypes.REAL :
 *               System.out.println(elementEntry.real().asDouble());
 *               break;
 *           case DataTypes.DATE :
 *               System.out.println(elementEntry.date().day() + " / " + elementEntry.date().month() + " / " + elementEntry.date().year());
 *               break;
 *           case DataTypes.TIME :
 *               System.out.println(elementEntry.time().hour() + ":" + elementEntry.time().minute()+ ":" 
 *                                  + elementEntry.time().second() + ":" + elementEntry.time().millisecond());
 *               break;
 *           case DataTypes.INT :
 *               System.out.println(elementEntry.intValue());
 *               break;
 *           case DataTypes.UINT :
 *               System.out.println(elementEntry.uintValue());
 *               break;
 *           case DataTypes.ASCII :
 *               System.out.println(elementEntry.ascii());
 *               break;
 *           case DataTypes.ENUM :
 *               System.out.println(elementEntry.enumValue());
 *               break;
 *           case DataTypes.ERROR :
 *               System.out.println(elementEntry.error().errorCode() +" (" + elementEntry.error().errorCodeAsString() + ")");
 *               break;
 *           default :
 *               System.out.println();
 *               break;
 *           }
 *    }
 * }
 * </pre>
 * 
 * Objects of this class are intended to be short lived or rather transitional.<br>
 * Objects of this class are not cache-able.
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
public interface ElementEntry
{
	public int loadType();

	/**
	 * Returns the Code of the entry's load.<br>
	 * The code indicates a special state of a Data.<br>
	 * Attempts to extract data will cause an
	 * OmmInvalidUsageException if {@link com.refinitiv.ema.access.Data.DataCode#BLANK} is returned.
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
	 * Returns name of the entry.
	 * 
	 * @return String containing name of the entry
	 */
	public String name();

	/**
	 * Returns the current OMM Data.
	 * 
	 * @return Data class reference to contained object
	 */
	public Data load();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.ReqMsg}
	 * 
	 * @return {@link com.refinitiv.ema.access.ReqMsg} class reference to contained object
	 */
	public ReqMsg reqMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.RefreshMsg}
	 * 
	 * @return {@link com.refinitiv.ema.access.RefreshMsg} class reference to contained object
	 */
	public RefreshMsg refreshMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.UpdateMsg}
	 * 
	 * @return {@link com.refinitiv.ema.access.UpdateMsg} class reference to contained object
	 */
	public UpdateMsg updateMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.StatusMsg}
	 * 
	 * @return {@link com.refinitiv.ema.access.StatusMsg} class reference to contained object
	 */
	public StatusMsg statusMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.PostMsg}
	 * 
	 * @return {@link com.refinitiv.ema.access.PostMsg} class reference to contained object
	 */
	public PostMsg postMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.AckMsg}
	 * 
	 * @return {@link com.refinitiv.ema.access.AckMsg} class reference to contained object
	 */
	public AckMsg ackMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.GenericMsg}
	 * 
	 * @return {@link com.refinitiv.ema.access.GenericMsg} class reference to contained object
	 */
	public GenericMsg genericMsg();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.FieldList}
	 * 
	 * @return {@link com.refinitiv.ema.access.FieldList} class reference to contained object
	 */
	public FieldList fieldList();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.ElementList}
	 * 
	 * @return {@link com.refinitiv.ema.access.ElementList} class reference to contained object
	 */
	public ElementList elementList();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.Map}
	 * 
	 * @return {@link com.refinitiv.ema.access.Map} class reference to contained object
	 */
	public Map map();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.Vector}
	 * 
	 * @return {@link com.refinitiv.ema.access.Vector} class reference to contained object
	 */
	public Vector vector();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.Series}
	 * 
	 * @return {@link com.refinitiv.ema.access.Series} class reference to contained object
	 */
	public Series series();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.FilterList}
	 * 
	 * @return {@link com.refinitiv.ema.access.FilterList} class reference to contained object
	 */
	public FilterList filterList();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmOpaque}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmOpaque} class reference to contained object
	 */
	public OmmOpaque opaque();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmXml}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmXml} class reference to contained object
	 */
	public OmmXml xml();

	/**
	 * Returns the current OMM data represented as a specific complex type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmAnsiPage}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmAnsiPage} class reference to contained object
	 */
	public OmmAnsiPage ansiPage();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmInt}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return long
	 */
	public long intValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmInt}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmInt} class reference to contained object
	 */
	public OmmInt ommIntValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmUInt}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return long
	 */
	public long uintValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmUInt}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return OmmUInt class reference to contained object
	 */
	public OmmUInt ommUIntValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmReal}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmReal} class reference to contained object
	 */
	public OmmReal real();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmFloat}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return float
	 */
	public float floatValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmFloat}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmFloat} class reference to contained object
	 */
	public OmmFloat ommFloatValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmDouble}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return double
	 */
	public double doubleValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmDouble}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmDouble} class reference to contained object
	 */
	public OmmDouble ommDoubleValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmDate}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmDate} class reference to contained object
	 */
	public OmmDate date();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmTime}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmTime} class reference to contained object
	 */
	public OmmTime time();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmDateTime}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmDateTime} class reference to contained object
	 */
	public OmmDateTime dateTime();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmQos}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmQos} class reference to contained object
	 */
	public OmmQos qos();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmState}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmState} class reference to contained object
	 */
	public OmmState state();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmEnum}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return int
	 */
	public int enumValue();
	
	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmEnum}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmEnum} class reference to contained object
	 */
	public OmmEnum ommEnumValue();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmBuffer}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmBuffer} class reference to contained object
	 */
	public OmmBuffer buffer();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmAscii}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmAscii} class reference to contained object
	 */
	public OmmAscii ascii();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmUtf8}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmUtf8} class reference to contained object
	 */
	public OmmUtf8 utf8();

	/**
	 * Returns the current OMM data represented as a specific simple type.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmRmtes}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmRmtes} class reference to contained object
	 */
	public OmmRmtes rmtes();

	/**
	 * Returns current OMM data represented as an OmmArray.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmArray}
	 * @throws OmmInvalidUsageException if {@link #code()} returns {@link com.refinitiv.ema.access.Data.DataCode#BLANK}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmArray} class reference to contained object
	 */
	public OmmArray array();

	/**
	 * Returns Error.
	 * 
	 * @throws OmmInvalidUsageException if contained object is not {@link com.refinitiv.ema.access.OmmError}
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmError} class reference to contained object
	 */
	public OmmError error();
	
	/**
	 * Adds a complex type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added ReqMsg
	 * 
	 * @return reference to this object
	 */
	public ElementEntry reqMsg(String name, ReqMsg value);

	/**
	 * Adds a complex type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added RefreshMsg
	 * 
	 * @return reference to this object
	 */
	public ElementEntry refreshMsg(String name, RefreshMsg value);

	/**
	 * Adds a complex type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added StatusMsg
	 * 
	 * @return reference to this object
	 */
	public ElementEntry statusMsg(String name, StatusMsg value);

	/**
	 * Adds a complex type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added UpdateMsg
	 * 
	 * @return reference to this object
	 */
	public ElementEntry updateMsg(String name, UpdateMsg value);

	/**
	 * Adds a complex type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added PostMsg
	 * 
	 * @return reference to this object
	 */
	public ElementEntry postMsg(String name, PostMsg value);

	/**
	 * Adds a complex type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added AckMsg
	 * 
	 * @return reference to this object
	 */
	public ElementEntry ackMsg(String name, AckMsg value);

	/**
	 * Adds a complex type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added GenericMsg
	 * 
	 * @return reference to this object
	 */
	public ElementEntry genericMsg(String name, GenericMsg value);

	/**
	 * Adds a complex type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added FieldList
	 * 
	 * @return reference to this object
	 */
	public ElementEntry fieldList(String name, FieldList value);

	/**
	 * Adds a complex type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added ElementList
	 * 
	 * @return reference to this object
	 */
	public ElementEntry elementList(String name, ElementList value);

	/**
	 * Adds a complex type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added Map
	 * 
	 * @return reference to this object
	 */
	public ElementEntry map(String name, Map value);

	/**
	 * Adds a complex type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added Vector
	 * 
	 * @return reference to this object
	 */
	public ElementEntry vector(String name, Vector value);

	/**
	 * Adds a complex type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added Series
	 * 
	 * @return reference to this object
	 */
	public ElementEntry series(String name, Series value);

	/**
	 * Adds a complex type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added FilterList
	 * 
	 * @return reference to this object
	 */
	public ElementEntry filterList(String name, FilterList value);

	/**
	 * Adds a complex type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added OmmOpaque
	 * 
	 * @return reference to this object
	 */
	public ElementEntry opaque(String name, OmmOpaque value);

	/**
	 * Adds a complex type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added OmmXml
	 * 
	 * @return reference to this object
	 */
	public ElementEntry xml(String name, OmmXml value);

	/**
	 * Adds a complex type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added OmmAnsiPage
	 * 
	 * @return reference to this object
	 */
	public ElementEntry ansiPage(String name, OmmAnsiPage value);

	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added long
	 * 
	 * @return reference to this object
	 */
	public ElementEntry intValue(String name, long value);

	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added long
	 * 
	 * @return reference to this object
	 */
	public ElementEntry uintValue(String name, long value);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added BigInteger
	 * 
	 * @return reference to this object
	 */
	public ElementEntry uintValue(String name, BigInteger value);
	

	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param mantissa added OmmReal mantissa   
	 * @param magnitudeType added {@link com.refinitiv.ema.access.OmmReal.MagnitudeType}
	 * 
	 * @return reference to this object
	 */
	public ElementEntry real(String name, long mantissa, int magnitudeType);

	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.
	 * Default magnitudeType is {@link com.refinitiv.ema.access.OmmReal.MagnitudeType#EXPONENT_0}
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added double to be converted to OmmReal
	 * 
	 * @return reference to this object
	 */
	public ElementEntry realFromDouble(String name, double value);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added double to be converted to OmmReal
	 * @param magnitudeType {@link com.refinitiv.ema.access.OmmReal.MagnitudeType} used for the conversion
	 * 
	 * @return reference to this object
	 */
	public ElementEntry realFromDouble(String name, double value, int magnitudeType);

	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added float
	 * 
	 * @return reference to this object
	 */
	public ElementEntry floatValue(String name, float value);

	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added double
	 * 
	 * @return reference to this object
	 */
	public ElementEntry doubleValue(String name, double value);

	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 * @throws OmmOutOfRangeException if passed in OmmDate is invalid
	 * 
	 * @param name String object containing ElementEntry name
	 * @param year added OmmDate year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDate month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDate day (0 - 31 where 0 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public ElementEntry date(String name, int year, int month, int day);

	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.<br>
	 * Defaults: second=0, millisecond=0, microsecond=0, nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 * @throws OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param name String object containing ElementEntry name
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public ElementEntry time(String name, int hour, int minute);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.<br>
	 * Defaults: millisecond=0, microsecond=0, nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 * @throws OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param name String object containing ElementEntry name
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public ElementEntry time(String name, int hour, int minute, int second);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.<br>
	 * Defaults: microsecond=0, nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 * @throws OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param name String object containing ElementEntry name
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond - added OmmTime millisecond (0 - 999 where 65535 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public ElementEntry time(String name, int hour, int minute, int second, int millisecond);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.<br>
	 * Default: nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 * @throws OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param name String object containing ElementEntry name
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmTime millisecond (0 - 999 where 65535 indicates blank)
	 * @param microsecond added OmmTime microsecond (0 - 999 where 2047 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public ElementEntry time(String name, int hour, int minute, int second, int millisecond, int microsecond);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 * @throws OmmOutOfRangeException if passed in OmmTime is invalid
	 * 
	 * @param name String object containing ElementEntry name
	 * @param hour added OmmTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmTime second (0 - 60 where 255 indicates blank)
	 * @param millisecond added OmmTime millisecond (0 - 999 where 65535 indicates blank)
	 * @param microsecond added OmmTime microsecond (0 - 999 where 2047 indicates blank)
	 * @param nanosecond added OmmTime nanosecond (0 - 999 where 2047 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public ElementEntry time(String name, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.<br>
	 * Defaults: hour=0, minute=0, second=0, millisecond=0, microsecond=0, nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param name String object containing ElementEntry name
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public ElementEntry dateTime(String name, int year, int month, int day);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.<br>
	 * Defaults: minute=0, second=0, millisecond=0, microsecond=0, nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param name String object containing ElementEntry name
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 23 where 255 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public ElementEntry dateTime(String name, int year, int month, int day, int hour);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.<br>
	 * Defaults: second=0, millisecond=0, microsecond=0, nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param name String object containing ElementEntry name
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public ElementEntry dateTime(String name, int year, int month, int day, int hour, int minute);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.<br>
	 * Defaults: millisecond=0, microsecond=0, nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param name String object containing ElementEntry name
	 * @param year added OmmDateTime year (0 - 4095 where 0 indicates blank)
	 * @param month added OmmDateTime month (0 - 12 where 0 indicates blank)
	 * @param day added OmmDateTime day (0 - 31 where 0 indicates blank)
	 * @param hour added OmmDateTime hour (0 - 23 where 255 indicates blank)
	 * @param minute added OmmDateTime minute (0 - 59 where 255 indicates blank)
	 * @param second added OmmDateTime second (0 - 60 where 255 indicates blank)
	 * 
	 * @return reference to this object
	 */
	public ElementEntry dateTime(String name, int year, int month, int day, int hour, int minute, 
									int second);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.<br>
	 * Defaults: microsecond=0, nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param name String object containing ElementEntry name
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
	public ElementEntry dateTime(String name, int year, int month, int day, int hour, int minute,
									int second , int millisecond);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.<br>
	 * Defaults: nanosecond=0
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param name String object containing ElementEntry name
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
	public ElementEntry dateTime(String name, int year, int month, int day, int hour, int minute, 
									int second, int millisecond, int microsecond);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 * @throws OmmOutOfRangeException if passed in OmmDateTime is invalid
	 * 
	 * @param name String object containing ElementEntry name
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
	public ElementEntry dateTime(String name, int year, int month, int day, int hour, int minute,
									int second, int millisecond, int microsecond, int nanosecond);

	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.<br>
	 * Default rate is {@link com.refinitiv.ema.access.OmmQos.Rate#TICK_BY_TICK}
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param timeliness added {@link com.refinitiv.ema.access.OmmQos.Timeliness}
	 * 
	 * @return reference to this object
	 */
	public ElementEntry qos(String name, int timeliness);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param timeliness added {@link com.refinitiv.ema.access.OmmQos.Timeliness}
	 * @param rate added {@link com.refinitiv.ema.access.OmmQos.Rate}
	 * 
	 * @return reference to this object
	 */
	public ElementEntry qos(String name, int timeliness, int rate);

	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.<br>
	 * Default dataState is {@link com.refinitiv.ema.access.OmmState.DataState#OK}<br>
	 * Default statusCode is {@link com.refinitiv.ema.access.OmmState.StatusCode#NONE}<br>
	 * Default statusText is an empty String<br>
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param streamState added {@link com.refinitiv.ema.access.OmmState.StreamState} 
	 * 
	 * @return reference to this object
	 */
	public ElementEntry state(String name, int streamState);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.<br>
	 * Default statusCode is {@link com.refinitiv.ema.access.OmmState.StatusCode#NONE}<br>
	 * Default statusText is an empty String<br>
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param streamState added {@link com.refinitiv.ema.access.OmmState.StreamState}
	 * @param dataState added {@link com.refinitiv.ema.access.OmmState.DataState}
	 * 
	 * @return reference to this object
	 */
	public ElementEntry state(String name, int streamState, int dataState);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.<br>
	 * Default statusText is an empty String
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param streamState added {@link com.refinitiv.ema.access.OmmState.StreamState}
	 * @param dataState added {@link com.refinitiv.ema.access.OmmState.DataState}
	 * @param statusCode added {@link com.refinitiv.ema.access.OmmState.StatusCode}
	 * 
	 * @return reference to this object
	 */
	public ElementEntry state(String name, int streamState, int dataState, int statusCode);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param streamState added {@link com.refinitiv.ema.access.OmmState.StreamState}
	 * @param dataState added {@link com.refinitiv.ema.access.OmmState.DataState}
	 * @param statusCode added {@link com.refinitiv.ema.access.OmmState.StatusCode}
	 * @param statusText added OmmState text
	 * 
	 * @return reference to this object
	 */
	public ElementEntry state(String name, int streamState, int dataState, int statusCode, String statusText);

	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added int
	 * 
	 * @return reference to this object
	 */
	public ElementEntry enumValue(String name, int value);

	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added ByteBuffer as OmmBuffer
	 * 
	 * @return reference to this object
	 */
	public ElementEntry buffer(String name, ByteBuffer value);

	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added String as OmmAscii
	 * 
	 * @return reference to this object
	 */
	public ElementEntry ascii(String name, String value);

	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added ByteBuffer as OmmUtf8
	 * 
	 * @return reference to this object
	 */
	public ElementEntry utf8(String name, ByteBuffer value);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added String as OmmUtf8
	 * 
	 * @return reference to this object
	 */
	public ElementEntry utf8(String name, String value);
	
	/**
	 * Adds a specific simple type of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added ByteBuffer as OmmRmtes
	 * 
	 * @return reference to this object
	 */
	public ElementEntry rmtes(String name, ByteBuffer value);

	/**
	 * Adds an OmmArray of OMM data to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name String object containing ElementEntry name
	 * @param value added OmmArray
	 * 
	 * @return reference to this object
	 */
	public ElementEntry array(String name, OmmArray value);

	/**
	 * Adds a blank data code to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *
	 * @param name string identifying blank data
	 * @return reference to this object
	 */
	public ElementEntry codeInt(String name);

	/**
	 * Adds a blank data code to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name string identifying blank data
	 * @return reference to this object
	 */
	public ElementEntry codeUInt(String name);

	/**
	 * Adds a blank data code to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name string identifying blank data
	 * @return reference to this object
	 */
	public ElementEntry codeReal(String name);

	/**
	 * Adds a blank data code to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name string identifying blank data
	 * @return reference to this object
	 */
	public ElementEntry codeFloat(String name);

	/**
	 * Adds a blank data code to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name string identifying blank data
	 * @return reference to this object
	 */
	public ElementEntry codeDouble(String name);

	/**
	 * Adds a blank data code to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name string identifying blank data
	 * @return reference to this object
	 */
	public ElementEntry codeDate(String name);

	/**
	 * Adds a blank data code to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name string identifying blank data
	 * @return reference to this object
	 */
	public ElementEntry codeTime(String name);

	/**
	 * Adds a blank data code to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name string identifying blank data
	 * @return reference to this object
	 */
	public ElementEntry codeDateTime(String name);

	/**
	 * Adds a blank data code to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name string identifying blank data
	 * @return reference to this object
	 */
	public ElementEntry codeQos(String name);

	/**
	 * Adds a blank data code to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name string identifying blank data
	 * @return reference to this object
	 */
	public ElementEntry codeState(String name);

	/**
	 * Adds a blank data code to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name string identifying blank data
	 * @return reference to this object
	 */
	public ElementEntry codeEnum(String name);

	/**
	 * Adds a blank data code to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name string identifying blank data
	 * @return reference to this object
	 */
	public ElementEntry codeBuffer(String name);

	/**
	 * Adds a blank data code to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name string identifying blank data
	 * @return reference to this object
	 */
	public ElementEntry codeAscii(String name);

	/**
	 * Adds a blank data code to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name string identifying blank data
	 * @return reference to this object
	 */
	public ElementEntry codeUtf8(String name);

	/**
	 * Adds a blank data code to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name string identifying blank data
	 * @return reference to this object
	 */
	public ElementEntry codeRmtes(String name);
	
	/**
	 * Adds no payload to the ElementEntry.
	 * 
	 * @throws OmmInvalidUsageException if an error is detected (exception will
	 *         specify the cause of the error)
	 *        
	 * @param name string identifying no payload entry
	 * @return reference to this object
	 */
	public ElementEntry noData(String name);
}
