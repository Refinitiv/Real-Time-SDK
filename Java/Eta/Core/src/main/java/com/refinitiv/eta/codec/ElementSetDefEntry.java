/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Buffer;

/**
 * A ETA entry that defines how to encode or decode an {@link ElementEntry}.
 * 
 * @see ElementSetDef
 */
public interface ElementSetDefEntry
{
    /**
     * Clears members from an {@link ElementSetDefEntry}. Useful for object reuse.
     */
    public void clear();

    /**
     * The name that corresponds to this set-defined element; contained in the
     * structure as a {@link Buffer}. Element names are defined outside of ETA,
     * typically as part of a domain model specification or dictionary. When
     * encoding, you can optionally populate {@link ElementEntry#name()} with
     * the name expected in the set definition. If name is not used, validation
     * checking is not provided and information might be encoded that does not
     * properly correspond to the definition. When decoding,
     * {@link ElementEntry#name()} is populated with the information indicated
     * in the set definition.
     * 
     * @return the name
     */
    public Buffer name();

    /**
     * The name that corresponds to this set-defined element; contained in the
     * structure as a {@link Buffer}. Element names are defined outside of ETA,
     * typically as part of a domain model specification or dictionary. When
     * encoding, you can optionally populate {@link ElementEntry#name()} with
     * the name expected in the set definition. If name is not used, validation
     * checking is not provided and information might be encoded that does not
     * properly correspond to the definition. When decoding,
     * {@link ElementEntry#name()} is populated with the information indicated
     * in the set definition.
     * 
     * @param name the name to set
     */
    public void name(Buffer name);

    /**
     * The element data type. When encoding or decoding an entry using this set
     * definition, dataType defines the entry's {@link DataTypes}. This can be a
     * base primitive type, a set-defined primitive type, or a container type.
     *
     * @return the dataType
     */
    public int dataType();

    /**
     * The element data type. When encoding or decoding an entry using this set
     * definition, dataType defines the entry's {@link DataTypes}. This can be a
     * base primitive type, a set-defined primitive type, or a container type.
     * Must be in the range of {@link DataTypes#INT} - 255.
     * 
     * @param dataType the dataType to set
     */
    public void dataType(int dataType);
}