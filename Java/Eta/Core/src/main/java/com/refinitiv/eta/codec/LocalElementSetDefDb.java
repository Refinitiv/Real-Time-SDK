/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/**
 * Local Message Field List Set Definitions Database that can groups ElementListSet
 * definitions together.
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
public interface LocalElementSetDefDb extends ElementSetDefDb
{
    /**
     * Clears {@link LocalElementSetDefDb} and all entries in it.
     * Useful for object reuse.
     */
    public void clear();

    /**
     * Decode set definitions contained on {@link Map}, {@link Vector}, or
     * {@link Series} into setDefDB database.
     * 
     * Typical use:<BR>
     * 1. Call Map.decode(), Series.decode() or Vector.decode()<BR>
     * 2. Call LocalElementSetDefDb.decode()<BR>
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
     * Encode ElementList set definitions database.
     * 
     * @param iter encode iterator
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encode(EncodeIterator iter);

    /**
     * The list of entries, indexed by ID.
     * 
     * @return the entries
     */
    public ElementSetDefEntry[][] entries();
}
