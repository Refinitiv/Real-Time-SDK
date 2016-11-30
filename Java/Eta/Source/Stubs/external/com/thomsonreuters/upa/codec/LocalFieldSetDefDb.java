package com.thomsonreuters.upa.codec;

/**
 * Local Message Field List Set Definitions Database that can groups
 * FieldListSet definitions together.
 * <p>
 * Using a database can be helpful when the content leverages multiple
 * definitions; the database provides an easy way to pass around all set
 * definitions necessary to encode or decode information. For instance, a
 * {@link Vector} can contain multiple set definitions via a set definition
 * database with the contents of each {@link VectorEntry} requiring a different
 * definition from the database.
 * 
 * @see FieldSetDef
 */
public interface LocalFieldSetDefDb extends FieldSetDefDb
{
    /**
     * Clears {@link LocalFieldSetDefDb} and all entries in it.
     * Useful for object reuse.
     */
    public void clear();
    
    /**
     * The list of entries, indexed by ID.
     * 
     * @return the entries
     */
    public FieldSetDefEntry[][] entries();

    /**
     * Initialize decoding iterator for a field list set definitions database.
     * 
     * Typical use:<BR>
     * 1. Call Map.decode(), Series.decode() or Vector.decode()<BR>
     * 2. Call LocalFieldSetDefDb.decode()<BR>
     * 
     * @param iter The decoding iterator.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see DecodeIterator
     * @see Map
     * @see Series
     * @see Vector
     */
    public int decode(DecodeIterator iter);

    /**
     * Encode FieldList set definitions database.
     * 
     * @param iter encode iterator
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encode(EncodeIterator iter);
}