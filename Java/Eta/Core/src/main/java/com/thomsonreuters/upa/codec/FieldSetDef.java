package com.thomsonreuters.upa.codec;

/**
 * Represents a single field set definition and can define the contents of multiple entries in a {@link FieldList}.
 * 
 * @see LocalFieldSetDefDb
 * @see FieldSetDefEntry
 */
public interface FieldSetDef
{
    /**
     * Clears members from a {@link FieldSetDef}. Useful for object reuse.
     */
    public void clear();

    /**
     * The set id. The identifier value associated with this field set
     * definition. Any field list content that leverages this definition should
     * have {@link FieldList#setId()} match this identifier. Only values 0 - 15
     * are valid for local set definition content. Values can be higher for global set definition content.
     * 
     * @return the setId
     */
    public long setId();

    /**
     * The set id. The identifier value associated with this field set
     * definition. Any field list content that leverages this definition should
     * have {@link FieldList#setId()} match this identifier. Only values 0 - 15
     * are valid for local set definition content. Values can be higher for
     * global set definition content. Must be in the range of 0 - 32767.
     * 
     * @param setId the setId to set
     */
    public void setId(long setId);

    /**
     * The number of entries.
     * 
     * @return the count
     */
    public int count();

    /**
     * The number of entries. Must be in the range of 0 - 65535.
     * 
     * @param tmpInt the count to set
     */
    public void count(int tmpInt);

    /**
     * The array of entries this definition will use. Each entry defines how a
     * {@link FieldEntry} is encoded or decoded.
     * 
     * @return the entries array
     */
    public FieldSetDefEntry[] entries();

    /**
     * The array of entries this definition will use.Each entry defines how a
     * {@link FieldEntry} is encoded or decoded.
     * 
     * @param entries the entries array to set
     */
    public void entries(FieldSetDefEntry[] entries);

    /**
     * Performs a shallow copy from this FieldSetDef into the designated FieldSetDef.
     *
     * @param destSetDef the dest set def
     */
    public void copy(FieldSetDef destSetDef);
}