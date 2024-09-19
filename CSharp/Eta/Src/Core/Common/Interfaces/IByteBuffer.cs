/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Common
{
    /// <summary>
    /// This is interface contact for representing functionalities 
    /// </summary>
    public interface IByteBuffer
    {
        /// <summary>
        /// Gets the begining position
        /// </summary>
        int Begin { get; }

        /// <summary>
        /// Gets the capacity of underlying byte array.
        /// </summary>
        int Capacity { get;  }

        /// <summary>
        /// Checks whether this buffer has remaining number of bytes.
        /// </summary>
        bool HasRemaining { get; }

        /// <summary>
        /// Gets the limit
        /// </summary>
        int Limit { get; }

        /// <summary>
        /// Gets the current position of cursor.
        /// </summary>
        int Position { get;  }

        /// <summary>
        /// Gets the number of remaining bytes.
        /// </summary>
        int Remaining { get; }

        /// <summary>
        /// Resets the read or write position depending upon read or write mode respectively
        /// </summary>
        /// <returns>The object instance</returns>
        ByteBuffer Rewind();

        /// <summary>
        /// Reset to initial state.
        /// </summary>
        /// <returns>This object instance</returns>
        ByteBuffer Clear();

        /// <summary>
        /// Shift-left the already-read Contents.
        /// </summary>
        /// <returns>This object instance</returns>
        ByteBuffer Compact();

        /// <summary>
        /// Reserve additional bytes in underlying data without advancing Position.
        /// </summary>
        /// <param name="count">The number of bytes to reserve</param>
        /// <returns>The limit of this buffer</returns>
        long Reserve(long count);

        /// <summary>
        /// Shrink the Capacity to the WritePosition.
        /// </summary>
        /// <returns>This object instance</returns>
        ByteBuffer Truncate();

        /// <summary>
        /// Flip this buffer to for reading data at the beginning position.
        /// </summary>
        /// <returns>This object instance</returns>
        ByteBuffer Flip();
    }
}
