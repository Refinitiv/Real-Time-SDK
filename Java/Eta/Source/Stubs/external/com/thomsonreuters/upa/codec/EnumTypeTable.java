package com.thomsonreuters.upa.codec;

/** 
 * A table of enumerated types.  A field that uses this table will contain a value
 * corresponding to an enumerated type in this table.
 */
public interface EnumTypeTable 
{
    /**
     * The highest value type present. This also indicates the length of the enumTypes list.
     * 
     * @return the maxValue
     */
    public int maxValue();
	
    /**
     * The list of enumerated types.
     * 
     * @return the enumTypes
     */
    public EnumType[] enumTypes();
	
    /**
     * The number of fields in the fidReferences list.
     * 
     * @return fidReferenceCount
     */
    public int fidReferenceCount();
	
    /**
     * A list of fieldId's representing fields that reference this table.
     * 
     * @return fidReferences
     */
    public int[] fidReferences();
}