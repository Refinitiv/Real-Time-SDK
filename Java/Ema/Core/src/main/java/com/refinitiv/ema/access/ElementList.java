///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019, 2024 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.util.Collection;
import com.refinitiv.ema.rdm.DataDictionary;
import java.util.Iterator;


/**
 * ElementList is a heterogeneous container of complex and primitive data type entries.<br>
 * ElementList entries are identified by name.<br><br>
 * 
 * The following code snippet shows addition of containers to ElementList.
 * <pre>
 * ElementList elementList = EmaFactory.createElementList();
 * elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_APP_ID, "127"));
 * elementList.add(EmaFactory.createElementEntry().intValue("value", 23));
 * </pre>
 * 
 * The following code snippet shows extracting data from ElementList.
 * <pre>
 * for(MapEntry mEntry : map)
 * {
 *    if(mEntry.loadType() == DataTypes.ELEMENT_LIST)
 *    {
 *       ElementList elementList = mEntry.elementList();
 *       for(ElementEntry elementEntry : elementList)
 *       {
 *          switch(elementEntry.loadType())
 *          {
 *             case DataTypes.INT :
 *                long value = elementEntry.intValue();
 *                break;
 *             case DataTypes.REAL :
 *                double d = elementEntry.real().asDouble();
 *                break;
 *
 *             ...
 *          }
 *       }
 *    }
 * }
 * </pre>
 * 
 * Objects of this class are intended to be short lived or rather transitional.<br>
 * This class is designed to efficiently perform setting and extracting of ElementList and its content.<br>
 * Objects of this class are not cache-able.
 *
 * @see Data
 * @see ElementEntry
 * @see ReqMsg
 * @see RefreshMsg
 * @see UpdateMsg
 * @see StatusMsg
 * @see GenericMsg
 * @see PostMsg
 * @see AckMsg
 * @see FieldList
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
	 * @throws OmmInvalidUsageException
	 *             if {@link #hasInfo()} returns false
	 * 
	 * @return ElementList Number
	 */
	public int infoElementListNum();

	/**
	 * Clears the ElementList.<br>
	 * Note: allows re-use of ElementList instance during encoding.
	 */
	public void clear();

	/**
	 * Specifies Info.<br>
	 * The ElementList Info is optional. If used, it must be specified before adding anything to ElementList.
	 * 
	 * @param elementListNum
	 *            FieldList template number
	 * @return reference to this object
	 */
	public ElementList info(int elementListNum);

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
	 * Returns an iterator over a single decoded ElementList. This does not return a copy of this data,
	 * but rather a reference to it that can be read and used before being moved to the next decoded ElementEntry
	 * when hasNext() is called, and returning the entry with next() on this iterator. hasNext() is required to be called
	 * before each next() call to return the following entry.
	 * 
	 * @return iterator for a reference of a single decoded ElementEntry.
	 */
	public Iterator<ElementEntry> iteratorByRef();
}