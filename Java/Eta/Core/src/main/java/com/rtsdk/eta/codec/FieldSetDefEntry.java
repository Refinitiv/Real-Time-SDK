package com.rtsdk.eta.codec;

/**
 * A UPA entry that defines how to encode or decode a {@link FieldEntry}.
 * 
 * @see FieldSetDef
 */
public interface FieldSetDefEntry
{
    /**
     * Clears members from a {@link FieldSetDefEntry}. Useful for object reuse.
     */
    public void clear();

    /**
     * The field identifier. The fieldId value that corresponds to this entry in
     * the set-defined {@link FieldList} content. Refers to specific name and
     * type information defined by an external field dictionary, such as the
     * RDMFieldDictionary. Negative fieldId values typically refer to user-defined
     * values while positive fieldId values typically refer to Refinitiv
     * -defined values.
     * 
     * @return the fieldId
     */
    public int fieldId();

    /**
     * The field identifier. The fieldId value that corresponds to this entry in
     * the set-defined {@link FieldList} content. Refers to specific name and
     * type information defined by an external field dictionary, such as the
     * RDMFieldDictionary. Negative fieldId values typically refer to user-defined
     * values while positive fieldId values typically refer to Refinitiv
     * -defined values. Must be in the range of -32768 - 32767.
     * 
     * @param fieldId the fieldId to set
     */
    public void fieldId(int fieldId);

    /**
     * The field data type. Defines the {@link DataTypes} of the entry as it
     * encodes or decodes when using this set definition.
     * 
     * @return the dataType
     * 
     * @see DataTypes
     */
    public int dataType();

    /**
     * The field data type. Defines the {@link DataTypes} of the entry as it
     * encodes or decodes when using this set definition. Must be in the range
     * of {@link DataTypes#INT} - 255.
     * 
     * @param dataType the dataType to set
     * 
     * @see DataTypes
     */
    public void dataType(int dataType);
}
