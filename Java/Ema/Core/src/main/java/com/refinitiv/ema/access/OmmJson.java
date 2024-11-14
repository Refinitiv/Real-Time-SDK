///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2024 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;

/**
 * OmmJson represents JSON data format in Omm.
 *
 * <p>Method {@link #string()} can have performance decrease, since it will trigger garbage collection.</p>
 * <p>Objects of this class are intended to be short lived or rather transitional.<br>
 * <br>This class is designed to efficiently perform setting and extracting of JSON and its content.<br>
 * <br>Objects of this class are not cache-able.</p>
 *
 * @see Data
 */
public interface OmmJson extends ComplexType
{
    /**
     * Returns contained JSON buffer.
     *
     * @return ByteBuffer containing the JSON data
     */
    ByteBuffer buffer();

    /**
     * Returns a string representation of the class instance.
     *
     * @return string representation of the class instance
     */
    String string();

    /**
     * Clears the OmmJson.
     * Invoking clear() method clears all the values and resets all the defaults.
     *
     * @return reference to this object
     */
    OmmJson clear();

    /**
     * Specifies Set.
     *
     * @param value specifies JSON data using String
     * @return reference to this object
     */
    OmmJson string(String value);

    /**
     * Specifies Set.
     *
     * @param value specifies JSON data using ByteBuffer
     * @return reference to this object
     */
    OmmJson buffer(ByteBuffer value);
}
