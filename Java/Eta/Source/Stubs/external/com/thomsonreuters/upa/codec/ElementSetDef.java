package com.thomsonreuters.upa.codec;

/**
 * Represents a single element set definition, and may define content for
 * multiple entries in an {@link ElementList}.
 * 
 * @see LocalElementSetDefDb
 * @see ElementSetDefEntry
 */
public interface ElementSetDef
{
    /**
     * Clears members from an {@link ElementSetDef}. Useful for object reuse.
     */
    public void clear();

    /**
     * The identifier value associated with this element set definition. Any
     * element list content that leverages this definition should have the
     * {@link ElementList#setId} matching this identifier. 0 - 15 are the only
     * values valid for local set definition content. Values can be higher for
     * global set definition content.
     * 
     * @return the setId
     */
    public int setId();

    /**
     * The identifier value associated with this element set definition. Any
     * element list content that leverages this definition should have the
     * {@link ElementList#setId} matching this identifier. 0 - 15 are the only
     * values valid for local set definition content. Values can be higher for
     * global set definition content. Must be in the range of 0 - 32767.
     * 
     * @param setId the setId to set
     */
    public void setId(int setId);

    /**
     * The number of {@link ElementSetDefEntry} structures contained in this
     * definition. Each entry defines how to encode or decode an {@link ElementEntry}.
     * 
     * @return the count
     */
    public int count();

    /**
     * The number of {@link ElementSetDefEntry} structures contained in this
     * definition. Each entry defines how to encode or decode an
     * {@link ElementEntry}. Must be in the range of 0 - 255.
     * 
     * @param count the count to set 
     */
    public void count(int count);

    /**
     * The array of entries this definition will use. Each entry defines how to
     * encode or decode an {@link ElementEntry}.
     * 
     * @return the entries array
     */
    public ElementSetDefEntry[] entries();

    /**
     * The array of entries this definition will use. Each entry defines how to
     * encode or decode an {@link ElementEntry}.
     * 
     * @param entries the entries array to set
     */
    public void entries(ElementSetDefEntry entries[]);
    
    /**
     * Performs a shallow copy from this ElementSetDef into the designated ElementSetDef.
     * 
     * @param destSetDef
     */
    public void copy(ElementSetDef destSetDef);
}