/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access
{
    /// <summary>
    /// Compression types.
    /// </summary>
    public enum CompressionType
    {
        /// <summary>
        /// No compression will be negotiated
        /// </summary>
        NONE = 0x00,

        /// <summary>
        /// Will attempt to use Zlib compression
        /// </summary>
        ZLIB = 0x01,

        /// <summary>
        /// Will attempt to use LZ4 compression
        /// </summary>
        LZ4 = 0x02
    }
}
