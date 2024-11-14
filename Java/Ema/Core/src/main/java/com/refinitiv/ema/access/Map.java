///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019, 2024 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import com.refinitiv.ema.rdm.DataDictionary;

import java.util.Collection;
import java.util.Iterator;

/**
 * Map is a homogeneous container of complex data type entries.
 * <p>
 * Map is a collection which provides iterator over the elements in this
 * collection.</p>
 * <p>
 * Map entries are identified by a map key.<br> All entries must have key of the
 * same primitive data type.<br> All entries must have same complex data type
 * (except for delete action).</p>
 * 
 * The following code snippet shows addition of containers to Map.
 * <pre>
 * Map map = EmaFactory.createMap();
 * FieldList fl = EmaFactory.createFieldList();
 * fl.add(EmaFactory.createFieldEntry().real(22, 34, OmmReal.MagnitudeType.EXPONENT_POS_1));
 * map.summaryData(fl);
 * ByteBuffer permission = (ByteBuffer)ByteBuffer.wrap("PERMISSION DATA".getBytes());
 * ByteBuffer orderBuf =(ByteBuffer)ByteBuffer.wrap("ABCD".getBytes());
 * map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.ADD, fl, permission));
 * </pre>
 * 
 * The following code snippet shows extracting data from Map.
 * <pre>
 * void decode(Map map)
 * {
 *    switch(map.summaryData().dataType())
 *    {
 *    case DataTypes.FIELD_LIST :
 *        decode(map.summaryData().fieldList());
 *        break;
 *    case DataTypes.MAP :
 *        decode(map.summaryData().map());
 *        break;
 *    case DataTypes.REFRESH_MSG :
 *        decode(map.summaryData().refreshMsg());
 *        break;
 *    case DataTypes.UPDATE_MSG :
 *        decode(map.summaryData().updateMsg());
 *        break;
 *    }
 *
 *    for(MapEntry mapEntry : map)
 *    {
 *       switch(mapEntry.key().dataType())
 *       {
 *       case DataTypes.ASCII :
 *           System.out.println("Action = " + mapEntry.mapActionAsString() + ", key = " + mapEntry.key().ascii());
 *           break;
 *       case DataTypes.BUFFER :
 *           System.out.println("Action = " + mapEntry.mapActionAsString() + ", key = " + mapEntry.key().buffer());
 *           break;
 *       }
 *
 *       switch(mapEntry.loadType())
 *       {
 *          case DataTypes.FIELD_LIST :
 *              decode(mapEntry.fieldList());
 *              break;
 *          case DataTypes.MAP :
 *              decode(mapEntry.map());
 *              break;
 *          case DataTypes.REFRESH_MSG :
 *              decode(mapEntry.refreshMsg());
 *              break;
 *          case DataTypes.UPDATE_MSG :
 *              decode(mapEntry.updateMsg());
 *              break;
 *       }
 *    }
 * }
 * </pre>
 *
 * Objects of this class are intended to be short lived or rather transitional.
 * This class is designed to efficiently perform setting and extracting of Map and its content.
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
 * @see OmmJson
 * @see OmmAnsiPage
 * @see OmmError
 */
public interface Map extends ComplexType, Collection<MapEntry>
{
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
	 * @throws OmmInvalidUsageException
	 *             if hasKeyFieldId() returns false
	 * 
	 * @return key field id
	 */
	public int keyFieldId();

	/**
	 * Returns TotalCountHint.
	 * 
	 * @throws OmmInvalidUsageException
	 *             if hasTotalCountHint() returns false
	 * 
	 * @return total count hint
	 */
	public int totalCountHint();

	/**
	 * Returns the contained summaryData Data based on the summaryData DataType.
	 * SummaryData contains no data if
	 * {@link com.refinitiv.ema.access.SummaryData#dataType()} returns
	 * {@link com.refinitiv.ema.access.DataType.DataTypes#NO_DATA}
	 * 
	 * @return {@link com.refinitiv.ema.access.SummaryData}
	 */
	public SummaryData summaryData();

	/**
	 * Clears the Map. Invoking clear() method clears all the values and resets
	 * all the defaults
	 */
	public void clear();

	/**
	 * Specifies KeyFieldId.
	 * 
	 * @param fieldId
	 *            specifies key field id
	 * @return reference to this object
	 */
	public Map keyFieldId(int fieldId);

	/**
	 * Specifies TotalCountHint.
	 * 
	 * @param totalCountHint
	 *            specifies total count hint
	 * @return reference to this object
	 */
	public Map totalCountHint(int totalCountHint);

	/**
	 * Specifies the SummaryData OMM Data. Call to summaryData() must happen
	 * prior to calling any add***() method
	 * 
	 * @throws OmmInvalidUsageException
	 *             if an error is detected (exception will specify the cause of
	 *             the error)
	 * 
	 * @param summaryData
	 *            specifies complex type as summaryData
	 * @return reference to this object
	 */
	public Map summaryData(ComplexType summaryData);
	
	/**
	 * Specifies a primitive type for Map Entry key. This is used to override the 
	 * default {@link com.refinitiv.ema.access.DataType.DataTypes#BUFFER} type. 
	 * Call this method or any add**() method can override the default key type.
	 * 
	 * @throws OmmInvalidUsageException
	 *             if an error is detected (exception will specify the cause of
	 *             the error)
	 * 
	 * @param keyPrimitiveType specifies a key primitive type defined in
	 * {@link com.refinitiv.ema.access.DataType.DataTypes}
	 *  
	 * @return reference to this object
	 */
	public Map keyType(int keyPrimitiveType);

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
	 * Returns an iterator over a single decoded Map. This does not return a copy of this data,
	 * but rather a reference to it that can be read and used before being moved to the next decoded MapEntry
	 * when hasNext() is called, and returning the entry with next() on this iterator. hasNext() is required to be called
	 * before each next() call to return the following entry.
	 * 
	 * @return iterator for a reference of a single decoded MapEntry.
	 */
	public Iterator<MapEntry> iteratorByRef();
}