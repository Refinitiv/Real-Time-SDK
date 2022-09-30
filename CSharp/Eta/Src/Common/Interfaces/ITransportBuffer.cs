/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Common;

namespace Refinitiv.Common.Interfaces
{
    /// <summary>
    /// ETA buffer used by transport layer.
    /// </summary>
   public interface ITransportBuffer
   {
        /// <summary>
        /// Gets the underlying byte buffer.
        /// </summary>
        /// <value>The underlying byte buffer</value>
        ByteBuffer Data { get; }

        /// <summary>
        /// Gets the length of the buffer.
        /// <para>If the TransportBuffer is a read buffer (i.e.a TransportBuffer that contains
        /// data read from network, typically associated with DecodeIterator), the length is 
        /// the difference between <see cref="ByteBuffer.Limit"/> and <see cref="ByteBuffer.Position"/>
        /// of the Buffer's ByteBuffer data.
        /// </para>
        /// </summary>
        /// <value>The length of the buffer</value>
        int Length { get; }

        /// <summary>
        /// Copies the buffer contents into memory passed by the user.
        /// </summary>
        /// <returns>The status of this operation</returns> 
        int Copy(ByteBuffer destination);

        /// <summary>
        /// This is maximum number of bytes ETA fills in to the buffer.
        /// </summary>
        /// <value>The capacity</value>
        int Capacity { get; }

        /// <summary>
        /// Return the offset to start of content.
        /// </summary>
        /// <value>The start position of the data</value>
        int DataStartPosition { get; }
   }
}
