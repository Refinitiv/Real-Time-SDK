/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Common
{
    /// <summary>
    /// The interface contacts for writing data from a buffer implementation.
    /// </summary>
    public interface IDataBufferWriter
    {
        /// <summary>
        /// Gets the write limit of the buffer
        /// </summary>
        int Limit { get; }
      

        #region Direct Writes into Contents

        /// <summary>
        /// Copies the content from the byte array
        /// </summary>
        /// <param name="source">The source byte array</param>
        /// <param name="offset">The offset of the source</param>
        /// <param name="length">The length to copy from</param>
        /// <returns>This object</returns>
        ByteBuffer Put(byte[] source, int offset, int length);

        /// <summary>
        /// Copy content from a byte array.
        /// </summary>
        /// <param name="source">The byte array to copy content from</param>
        /// <returns>This object</returns>
        ByteBuffer Put(byte[] source);

        /// <summary>
        /// Copy content from a <see cref="ByteBuffer"/>
        /// </summary>
        /// <param name="sourceBuffer">The <see cref="ByteBuffer"/> to copy content from</param>
        /// <returns>This object</returns>
        ByteBuffer Put(ByteBuffer sourceBuffer);

        #endregion

        #region Writer - Serialization

        /// <summary>
        /// Write a <see cref="byte"/> value to the current position
        /// </summary>
        /// <param name="value">A <see cref="byte"/> to write</param>
        /// <returns>This object</returns>
        ByteBuffer Write(byte value);

        /// <summary>
        /// Write a <see cref="float"/> value to the current position
        /// </summary>
        /// <param name="value">A <see cref="float"/> to write</param>
        /// <returns>This object</returns>
        ByteBuffer Write(float value);

        /// <summary>
        /// Write a <see cref="System.Int32"/> value to the current position
        /// </summary>
        /// <param name="value">A <see cref="System.Int32"/> to write</param>
        /// <returns>This object</returns>
        ByteBuffer Write(int value);

        /// <summary>
        /// Write a <see cref="System.Int64"/> value to the current position
        /// </summary>
        /// <param name="value">A <see cref="System.Int64"/> to write</param>
        /// <returns>This object</returns>
        ByteBuffer Write(long value);

        /// <summary>
        /// Write a <see cref="System.Int16"/> value to the current position
        /// </summary>
        /// <param name="value">A <see cref="System.Int16"/> to write</param>
        /// <returns>This object</returns>
        ByteBuffer Write(short value);

        /// <summary>
        /// Write a <see cref="byte"/> value to a specific postion
        /// </summary>
        /// <param name="index">The position to write data to</param>
        /// <param name="value">The <see cref="byte"/> value</param>
        /// <returns>This object</returns>
        ByteBuffer WriteAt(int index, byte value);

        /// <summary>
        /// Write a <see cref="double"/> value to a specific postion
        /// </summary>
        /// <param name="index">The position to write data to</param>
        /// <param name="value">The <see cref="double"/> value</param>
        /// <returns>This object</returns>
        ByteBuffer WriteAt(int index, double value);

        /// <summary>
        /// Write a <see cref="float"/> value to a specific postion
        /// </summary>
        /// <param name="index">The position to write data to</param>
        /// <param name="value">The <see cref="float"/> value</param>
        /// <returns>This object</returns>
        ByteBuffer WriteAt(int index, float value);

        /// <summary>
        /// Write a <see cref="System.Int32"/> value to a specific postion
        /// </summary>
        /// <param name="index">The position to write data to</param>
        /// <param name="value">The <see cref="System.Int32"/> value</param>
        /// <returns>This object</returns>
        ByteBuffer WriteAt(int index, int value);

        /// <summary>
        /// Write a <see cref="System.Int64"/> value to a specific postion
        /// </summary>
        /// <param name="index">The position to write data to</param>
        /// <param name="value">The <see cref="System.Int64"/> value</param>
        /// <returns>This object</returns>
        ByteBuffer WriteAt(int index, long value);

        /// <summary>
        /// Write a <see cref="System.Int16"/> value to a specific postion
        /// </summary>
        /// <param name="index">The position to write data to</param>
        /// <param name="value">The <see cref="System.Int16"/> value</param>
        /// <returns>This object</returns>
        ByteBuffer WriteAt(int index, short value);

        #endregion
    }
}