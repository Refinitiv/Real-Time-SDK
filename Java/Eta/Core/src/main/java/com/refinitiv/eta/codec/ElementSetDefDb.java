package com.refinitiv.eta.codec;

/**
 * Message Field List Set Definitions Database that can groups ElementListSet definitions together.
 * <p>
 * Using a database can be helpful when the content leverages multiple
 * definitions; the database provides an easy way to pass around all set
 * definitions necessary to encode or decode information. For instance, a
 * {@link Vector} can contain multiple set definitions via a set definition
 * database with the contents of each {@link VectorEntry} requiring a different
 * definition from the database.
 * 
 * @see ElementSetDef
 */
public interface ElementSetDefDb
{
    /**
     * Clears {@link ElementSetDefDb} and all entries in it. Useful for object reuse.
     */
    public void clear();

    /**
     * The list of definitions, indexed by ID.
     * 
     * @return the definitions
     */
    public ElementSetDef[] definitions();
    
    /**
     * The maximum setId
     * 
     * @return maxSetId
     */
    int maxSetId();

    /**
     * Set the maximum setId
     * 
     * @param setMaxSetId set maxSetId
     */
    void maxSetId(int setMaxSetId);
}
