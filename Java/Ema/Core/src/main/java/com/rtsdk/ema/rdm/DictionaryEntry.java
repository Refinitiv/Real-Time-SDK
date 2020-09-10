///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.rdm;

import com.rtsdk.ema.access.OmmInvalidUsageException;

/**
 * A data dictionary entry, containing field information and an enumeration table reference if present.
 * 
 * @see MfFieldTypes
 * @see com.rtsdk.ema.access.DataType.DataTypes
 */
public interface DictionaryEntry
{
	  /**
     * Acronym.
     * 
     * @return the acronym
     */
    public String acronym();

    /**
     * DDE Acronym.
     * 
     * @return the ddeAcronym
     */
    public String ddeAcronym();

    /**
     * The fieldId the entry corresponds to.
     * 
     * @return the fid
     */
    public int fid();

    /**
     * The field to ripple data to.
     * 
     * @return the rippleToField
     */
    public int rippleToField();

    /**
     * Marketfeed Field Type. 
     * This type is defined in {@link MfFieldTypes}
     * 
     * @return the fieldType
     */
    public int fieldType();

    /**
     * Marketfeed length.
     * 
     * @return the length
     */
    public int length();

    /**
     * Marketfeed enum length.
     * 
     * @return the enumLength
     */
    public int enumLength();

    /**
     * RWF type.
     * This type is defined in {@link com.rtsdk.ema.access.DataType.DataTypes} 
     * and less than 256.
     * 
     * @return the rwfType
     */
    public int rwfType();

    /**
     * RWF Length.
     * 
     * @return the rwfLength
     */
    public int rwfLength();
    
	/**
	* Check whether the EnumType exists
	*
	* @param value the value of the enumerated type to check
	*
	* @return the enumerated type if it exists
	*/
	public boolean hasEnumType(int value);
    
    /**
     * Returns the corresponding enumerated type in the dictionary entry's
     * table, if the type exists.
     * 
     * @param value the value of the enumerated type to get
     * @throws OmmInvalidUsageException if the entry does not exist.
     * @return the enumerated type if it exists
     */
    public EnumType enumType(int value);
    
    /**
	* Check whether the EnumTypeTable exists
	*
	* @return true if EnumTypeTable exists, otherwise false
	*/
	public boolean hasEnumTypeTable();

	/**
	* Returns the list of EnumTypeTable that is used by this DictionaryEntry,
	* if the type exists.
	* @throws OmmInvalidUsageException if the entry does not exist.
	* @return the array of EnumTypeTable if it exists
	*/
	public EnumTypeTable enumTypeTable();
	
	/**
    * Convert information contained in the dictionary entry to a string.
    * 
    * @return the string representation of this {@link DictionaryEntry}
    */
    public String toString();
}
