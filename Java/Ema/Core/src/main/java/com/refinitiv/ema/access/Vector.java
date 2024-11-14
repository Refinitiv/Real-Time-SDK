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
 * Vector is a homogeneous container of complex data type entries.<br>
 * Vector is a collection which provides iterator over the elements in this collection.<br>
 * 
 * Vector entries are identified by index.
 * 
 * The following code snippet shows addition of entry and summaryData to Vector.
 * <pre>
 * Vector vector = EmaFactory.createVector();
 * FieldList fl = EmaFactory.createFieldList();
 * fl.add(EmaFactory.createFieldEntry().real(22, 34, OmmReal.MagnitudeType.EXPONENT_POS_1));
 * vector.summaryData(fl);
 * ByteBuffer permission = (ByteBuffer)ByteBuffer.wrap("PERMISSION DATA".getBytes());
 * fl.clear();
 * fl.add(EmaFactory.createFieldEntry().real(22, 34, OmmReal.MagnitudeType.EXPONENT_POS_1));
 * vector.add(EmaFactory.createVectorEntry().fieldList(1, VectorEntry.VectorAction.DELETE, fl, permission));
 * vector.add(EmaFactory.createVectorEntry().fieldList(1, VectorEntry.VectorAction.SET, fl, permission));
 * </pre>
 *
 * The following code snippet shows extracting data from Vector and its content.
 * <pre>
 * void decode(Vector vector)
 * {
 *    switch(vector.summaryData().dataType())
 *    {
 *    case DataTypes.FIELD_LIST:
 *         decode(vector.summaryData().fieldList());
 *         break;
 *    case DataTypes.NO_DATA:
 *         break;
 *    }
 *
 *    for(VectorEntry vectorEntry : vector)
 *    {
 *       System.out.println(" DataType: " + DataType.asString(vectorEntry.loadType()) + " Value: ");
 *
 *       switch(vectorEntry.loadType())
 *       {
 *       case DataTypes.FIELD_LIST:
 *           decode(vectorEntry.fieldList());
 *           break;
 *       case DataTypes.NO_DATA:
 *           break;
 *       }
 *    }
 * }
 * </pre>
 *
 * Objects of this class are intended to be short lived or rather transitional.<br>
 * This class is designed to efficiently perform setting and extracting of Vector and its content.<br>
 * Objects of this class are not cache-able.
 * 
 * @see Data
 * @see VectorEntry
 * @see SummaryData
 * @see ReqMsg
 * @see RefreshMsg
 * @see UpdateMsg
 * @see StatusMsg
 * @see GenericMsg
 * @see PostMsg
 * @see AckMsg
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
public interface Vector extends ComplexType, Collection<VectorEntry>
{
	/**
	 * Indicates presence of TotalCountHint.
	 * 
	 * @return true if total count hint is set; false otherwise
	 */
	public boolean hasTotalCountHint();

	/**
	 * Returns Sortable.
	 * 
	 * @return true if sortable flag is set; false otherwise
	 */
	public boolean sortable();

	/**
	 * Returns TotalCountHint.
	 * 
	 * @throws OmmInvalidUsageException
	 *             if {@link #hasTotalCountHint()} returns false
	 * 
	 * @return total count hint
	 */
	public int totalCountHint();

	/**
	 * Returns the contained summaryData Data based on the summaryData DataType.
	 * <br>
	 * SummaryData contains no data if {@link SummaryData#dataType()} returns
	 * {@link com.refinitiv.ema.access.DataType.DataTypes#NO_DATA}
	 * 
	 * @return {@link SummaryData}
	 */
	public SummaryData summaryData();

	/**
	 * Clears the Vector. <br>
	 * Invoking clear() method clears all the values and resets all the
	 * defaults.
	 */
	public void clear();

	/**
	 * Specifies Sortable.
	 * 
	 * @param sortable
	 *            specifies if this object is sortable
	 * 
	 * @return reference to this object
	 */
	public Vector sortable(boolean sortable);

	/**
	 * Specifies TotalCountHint.
	 * 
	 * @param totalCountHint
	 *            specifies total count hint
	 * 
	 * @return reference to this object
	 */
	public Vector totalCountHint(int totalCountHint);

	/**
	 * Specifies the SummaryData OMM Data. <br>
	 * Call to summaryData() must happen prior to calling the add() method
	 * 
	 * @throws OmmInvalidUsageException
	 *             if an error is detected (exception will specify the cause of
	 *             the error)
	 * 
	 * @param data
	 *            specifies complex type as summaryData
	 * @return reference to this object
	 */
	public Vector summaryData(ComplexType data);

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
	 * Returns an iterator over a single decoded Vector. This does not return a copy of this data,
	 * but rather a reference to it that can be read and used before being moved to the next decoded VectorEntry
	 * when hasNext() is called, and returning the entry with next() on this iterator. hasNext() is required to be called
	 * before each next() call to return the following entry.
	 * 
	 * @return iterator for a reference of a single decoded VectorEntry.
	 */
	public Iterator<VectorEntry> iteratorByRef();
}