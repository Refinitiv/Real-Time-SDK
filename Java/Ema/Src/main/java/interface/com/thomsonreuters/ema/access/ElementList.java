///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.util.Collection;

/**
 * ElementList is a heterogeneous container of complex and primitive data type
 * entries. ElementList entries are identified by name.
 * 
 * <p>
 * ElementList is a collection which provides iterator over the elements in this
 * collection.
 * </p>
 * 
 * <p>
 * ElementList supports two methods of adding containers; they are: <br>
 * - adding of already populated containers, and <br>
 * - adding of clear containers which would be populated after that.
 * </p>
 * 
 * <p>
 * Note that these two methods apply to containers only: OmmArray, ElementList,
 * FieldList, FilterList, Map, Series, and Vector.
 * </p>
 * 
 * <p>
 * The first method of adding of already populated containers allows for easy
 * data manipulation but incurs additional memory copy. This method is useful in
 * applications extracting data containers from some other messages or
 * containers and then setting them on other containers without full decoding of
 * the extracted containers.
 * </p>
 * 
 * <p>
 * The second method allows for fast container population since it avoids
 * additional memory copy incurred in the first method. This method is useful in
 * source applications encoding OMM data from native data formats.
 * </p>
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
	 *             if hasInfo() returns false
	 * 
	 * @return ElementList Number
	 */
	public int infoElementListNum();

	/**
	 * Clears the ElementList. Note: allows re-use of ElementList instance
	 * during encoding.
	 */
	public void clear();

	/**
	 * Specifies Info. The ElementList Info is optional. If used, it must be
	 * specified before adding anything to ElementList.
	 * 
	 * @param elementListNum
	 *            FieldList template number
	 * @return reference to this object
	 */
	public ElementList info(int elementListNum);
}