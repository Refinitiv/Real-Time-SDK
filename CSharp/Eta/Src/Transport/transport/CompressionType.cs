/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.Transports
{
    /// <summary>
    /// ETA Supported Compression types.
    /// </summary>
    public enum CompressionType : byte
    {
        /// <summary>
        /// No compression is desired on the connection.
        /// </summary>
        NONE = 0,

       /// <summary>
       /// Use of zlib compression is desired on the connection. Zlib, an open
       /// source utility, employs a variation of the LZ77 algorithm while
       /// compressing and decompressing data.
       /// </summary>
        ZLIB = 1,

        /// <summary>
        /// Use of lz4 compression is desired on the connection. Lz4 is a lossless 
        /// data compression algorithm that is focused on compression and decompression speed.
        /// It belongs to the LZ77 family of byte-oriented compression schemes.
        /// </summary>
        LZ4 = 2,

        /// <summary>
        /// Max defined Compression type.
        /// </summary>
        MAX_DEFINED = LZ4
    }
}
