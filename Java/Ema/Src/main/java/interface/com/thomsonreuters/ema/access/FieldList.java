///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.util.Collection;

/**
 * FieldList is a heterogeneous container of complex and primitive data type
 * entries.
 * 
 * <p>
 * FieldList is a collection which provides iterator over the elements in this
 * collection.
 * </p>
 * 
 * <p>
 * FieldList entries are identified by Field Id. The meaning of the Field Id is
 * conveyed by the RDMFieldDictionary.
 * </p>
 *
 * <p>
 * FieldList supports two methods of adding containers; they are: <br>
 * - adding of already populated containers, (e.g. {@link #complete()} was
 * called) and <br>
 * - adding of clear containers (e.g. {@link #clear()} was called) which would
 * be populated after that.
 * </p>
 * 
 * <p>
 * The first method of adding of already populated containers allows for easy
 * data manipulation but incurs additional memory copy. This method is useful in
 * applications extracting data containers from some messages or containers and
 * then setting them on other containers.
 * </p>
 * 
 * <p>
 * The second method allows for fast container population since it avoids
 * additional memory copy incurred by the first method. This method is useful in
 * source applications setting OMM data from native data formats.
 * </p>
 * 
 * The following code snippet shows getting data from FieldList.
 * 
 * <pre>
 * MapEntry mEntry = map.entry();
 * if(DataTypes.FIELD_LIST == mapEntry.loadType())
 * {
 *   FieldList fList = map.fieldList();
 *   while(fList.forth())
 *   {
 *     FieldEntry fEntry = fList.entry();
 *     switch(fEntry.loadType())
 *     {
 *       case DataTypes.UINT :
 *       	long value = fEntry.uintValue();
 *       	break;
 *       case DataTypes.REAL :
 *       	double value = fEntry.real().asDouble();
 *       	break;
 *       ...
 *     }
 *   }
 * }
 * </pre>
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
	 * @throws OmmInvalidUsageException
	 *             if hasInfo() returns false
	 * 
	 * @return FieldList Number
	 */
	public int infoFieldListNum();

	/**
	 * Returns InfoDictionaryId.
	 * 
	 * @throws OmmInvalidUsageException
	 *             if hasInfo() returns false
	 * 
	 * @return DictionaryId associated with this FieldList
	 */
	public int infoDictionaryId();

	/**
	 * Clears the FieldList. Invoking clear() method clears all the values and
	 * resets all the defaults
	 */
	public void clear();

	/**
	 * Specifies Info. The FieldList Info is optional. If used, it must be set
	 * prior to adding anything to FieldList.
	 * 
	 * @param dictionaryId
	 *            dictionary id of the RdmFieldDictioanry associated with this
	 *            FieldList
	 * @param fieldListNum
	 *            FieldList template number
	 * @return reference to this object
	 */
	public FieldList info(int dictionaryId, int fieldListNum);
}