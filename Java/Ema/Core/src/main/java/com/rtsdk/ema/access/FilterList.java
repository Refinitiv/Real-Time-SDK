///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;

import java.util.Collection;


/**
 * FilterList is a heterogeneous container of complex data type entries.
 * <br><br>
 * The following code snippet shows addition of containers to FilterList.
 * <pre>
 * FilterList filterList = EmaFactory.createFilterList();
 * FieldList fl = EmaFactory.createFieldList();
 * fl.add(EmaFactory.createFieldEntry().real(22, 34, OmmReal.MagnitudeType.EXPONENT_POS_1));
 * filterList.add(EmaFactory.createFilterEntry().fieldList(2, FilterEntry.FilterAction.SET, fl));
 * </pre>
 * 
 * The following code snippet shows extracting data from FilterList.
 * <pre>
 * for(FilterEntry filterEntry : filterList)
 * {
 *    System.out.println("ID: " + filterEntry.filterId() + " Action = " + filterEntry.filterActionAsString() 
 *                              + " DataType: " + DataType.asString(filterEntry.loadType()) + " Value: ");
 *    switch(filterEntry.loadType())
 *    {
 *    case DataTypes.MAP :
 *        decode(filterEntry.map());
 *        break;
 *    case DataTypes.ELEMENT_LIST :
 *        decode(filterEntry.elementList());
 *        break;
 *    case DataTypes.FIELD_LIST :
 *        decode(filterEntry.fieldList());
 *        break;
 *
 *        ...
 *    }
 * }
 * </pre>
 * 
 * Objects of this class are intended to be short lived or rather transitional.<br>
 * This class is designed to efficiently perform setting and extracting of FieldList and its content.<br>
 * Objects of this class are not cache-able.
 * 
 * @see Data
 * @see FieldEntry
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
 * @see OmmAnsiPage
 * @see OmmError
 */
public interface FilterList extends ComplexType, Collection<FilterEntry>
{
	/**
	 * Indicates presence of TotalCountHint.
	 *
	 * @return true if total count hint is set; false otherwise
	 */
	public boolean hasTotalCountHint();

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
	 * Clears the FilterList.<br>
	 * Invoking clear() method clears all the values and resets all the
	 * defaults.
	 */
	public void clear();

	/**
	 * Specifies TotalCountHint.
	 * 
	 * @param totalCountHint
	 *            specifies estimated total number of entries
	 * @return reference to this object
	 */
	public FilterList totalCountHint(int totalCountHint);
}