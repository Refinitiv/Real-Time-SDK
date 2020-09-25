///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.util.Collection;

/**
 * FieldList is a heterogeneous container of complex and primitive data type
 * entries.
 * <br><br>
 * The following code snippet shows addition of containers to FieldList.
 * <pre>
 * FieldList fieldList = EmaFactory.createFieldList();
 * fieldList.add(EmaFactory.createFieldEntry().real(22, 34, OmmReal.MagnitudeType.EXPONENT_POS_1));
 * fieldList.add(EmaFactory.createFieldEntry().time(18, 11, 29, 30));
 * </pre>
 * 
 * The following code snippet shows extracting data from FieldList.
 * <pre>
 * for(MapEntry mEntry : map)
 * {
 *    if(mEntry.loadType() == DataTypes.FIELD_LIST)
 *    {
 *       FieldList fieldList = mEntry.fieldList();
 *       for(FieldEntry fieldEntry : fieldList)
 *       {
 *          switch(fieldEntry.loadType())
 *          {
 *             case DataTypes.INT :
 *                long value = fieldEntry.intValue();
 *                break;
 *             case DataTypes.REAL :
 *                double d = fieldEntry.real().asDouble();
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
	 * Clears the FieldList.<br>
	 * Invoking clear() method clears all the values and resets all the defaults
	 */
	public void clear();

	/**
	 * Specifies Info.<br>
	 * The FieldList Info is optional. If used, it must be set prior to adding anything to FieldList.
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