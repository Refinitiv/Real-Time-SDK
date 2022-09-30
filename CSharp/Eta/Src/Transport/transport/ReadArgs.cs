/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Transports.Interfaces;

namespace Refinitiv.Eta.Transports
{
    /// <summary>
    /// ETA Read Arguments used in the <see cref="IChannel.Read(ReadArgs, out Error)"/> call.
    /// </summary>
    /// <seealso cref="IChannel"/>

    public class ReadArgs
    {
        /// <summary>
        /// Default constructor sets all property to defaults.
        /// </summary>
        public ReadArgs()
        {
            Clear();
        }

        /// <summary>
        /// Gets ETA Read Return Value used in the <see cref="IChannel.Read(ReadArgs, out Error)"/>
        /// call. A value more than of the <see cref="TransportReturnCode.SUCCESS"/> value if there is more data to read,
        /// <see cref="TransportReturnCode.READ_WOULD_BLOCK"/> if the read call is blocked, or a failure code.
        /// </summary>
        /// <value>The read return value</value>
        public TransportReturnCode ReadRetVal { get; internal set; }

        /// <summary>
        /// Gets the number of bytes read, including any transport header overhead,
        /// excluding any decompression, with this read call.
        /// </summary> 
        /// <value>The number of bytes read</value>
        public int BytesRead { get; internal set; }

        /// <summary>
        /// Gets the number of bytes read, including any transport header overhead and
        /// including any decompression, with this read call.
        /// </summary> 
        /// <value>The number of uncompressed bytes read</value>
        public int UncompressedBytesRead { get; internal set; }

        /// <summary>
        ///  Gets or sets read flags for <see cref="IChannel.Read(ReadArgs, out Error)"/>.
        /// </summary>
        /// <value>The read flags</value>
        public ReadFlags Flags;

        /// <summary>
        /// Clear the ReadArgs to defaults.
        /// </summary>
        public void Clear()
        {
            BytesRead = 0;
            UncompressedBytesRead = 0;
            ReadRetVal = TransportReturnCode.SUCCESS;
            Flags = ReadFlags.NO_FLAGS;
        }
    }
}
