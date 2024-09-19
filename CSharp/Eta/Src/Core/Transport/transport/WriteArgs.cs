/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.Transports
{
    /// <summary>
    /// ETA Write Arguments used in the <see cref="IChannel.Write(ITransportBuffer, WriteArgs, out Error)"/> call.
    ///
    /// <seealso cref="IChannel"/>
    /// <seealso cref="WritePriorities"/>
    /// <seealso cref="WriteFlags"/>
    /// </summary>
    sealed public class WriteArgs
    {
        /// <summary>
        /// Priority to flush the message for <see cref="IChannel.Write(ITransportBuffer, WriteArgs, out Error)"/>.
        /// <seealso cref="WritePriorities"/>
        /// </summary>
        /// <vaue>The priority</vaue>
        public WritePriorities Priority { get; set; }

       /// <summary>
       /// Write flags used for <see cref = "IChannel.Write(ITransportBuffer, WriteArgs, out Error)" />.
       /// <seealso cref="WriteFlags"/>
       /// </summary>
       /// <value>The flags</value>
        public WriteFlags Flags { get; set; }

        /// <summary>
        /// The number of bytes to be written, including any transport header overhead
        /// and taking into account any compression, for this write call.
        /// </summary>
        /// <value>The bytes written</value>
        public int BytesWritten { get; internal set; }

        /// <summary>
        /// The number of bytes to be written, including any transport header
        /// overhead but not taking into account any compression, for this write call.
        /// </summary>
        /// <value>The uncompressed bytes written</value>
        public int UncompressedBytesWritten { get; internal set; }

        /// <summary>
        /// The sequence number of the message for <see cref="IChannel.Write(ITransportBuffer, WriteArgs, out Error)"/>.
        /// </summary>
        /// <value>The sequeuce number</value>
        public long SeqNum { get; set; }

        /// <summary>
        /// Clears ETA Write Arguments.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Clear()
        {
            Priority = WritePriorities.HIGH;
            Flags = WriteFlags.NO_FLAGS;
            BytesWritten = 0;
            UncompressedBytesWritten = 0;
            SeqNum = 0;
        }

        /// <summary>
        /// Gets the string representation of this object. 
        /// </summary>
        /// <returns>The string value</returns>
        public override string ToString()
        {
            return $"Flags: {Flags}, BytesWritten: {BytesWritten}, SeqNum: {SeqNum}";
        }
    }
}

