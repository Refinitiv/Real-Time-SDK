/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/**
 * ETA Supported Compression types.
 */
public class CompressionTypes
{
    
    /**
     * Instantiates a new compression types.
     */
    // CompressionTypes class cannot be instantiated
    private CompressionTypes()
    {
        throw new AssertionError();
    }

    /** No compression is desired on the connection. */
    public static final int NONE = 0;

    /**
     * Use of zlib compression is desired on the connection. Zlib, an open
     * source utility, employs a variation of the LZ77 algorithm while
     * compressing and decompressing data.
     */
    public static final int ZLIB = 1;
    
    /**
     * Use of lz4 compression is desired on the connection. Lz4 is a lossless 
     * data compression algorithm that is focused on compression and decompression speed.
     * It belongs to the LZ77 family of byte-oriented compression schemes.
     */
    public static final int LZ4 = 2;
    
    /** max defined compressionType */
    static final int MAX_DEFINED = LZ4;
    
    /**
     * Provide string representation for a compression type value.
     *
     * @param type the type
     * @return string representation for a compression type value
     */
    public static String toString(int type)
    {
        switch (type)
        {
            case NONE:
                return "none";
            case ZLIB:
                return "zlib";
            case LZ4:
                return "lz4";
            default:
                return Integer.toString(type);
        }
    }    
}
