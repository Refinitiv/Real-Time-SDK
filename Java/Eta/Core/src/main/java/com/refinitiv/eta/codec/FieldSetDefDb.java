/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;
/**
 * Message Field List Set Definitions Database that can group
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
public interface FieldSetDefDb
{
    /**
     * Clears {@link FieldSetDefDb} and all entries in it.
     * Useful for object reuse.
     */
    public void clear();

    /**
     * The list of definitions, indexed by ID.
     * 
     * @return the definitions
     */
    public FieldSetDef[] definitions();
    
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