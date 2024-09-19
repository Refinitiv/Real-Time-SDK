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
    /// The interface contacts for reading data from a buffer implementation.
    /// </summary>
    public interface IDataBufferReader
    {
        /// <summary>
        /// Gets the read limit of a buffer
        /// </summary>
        int Limit { get; }

        /// <summary>
        /// Gets the read position
        /// </summary>
        int ReadPosition { get; set; }

        /// <summary>
        /// Copies the content to the byte array.
        /// </summary>
        /// <param name="dst">The destination byte array</param>
        /// <param name="offset">The offset of the destination</param>
        /// <param name="length">The copy length</param>
        /// <returns>This object</returns>
        ByteBuffer ReadBytesInto(byte[] dst, int offset, int length);

        #region Deserialization

        /// <summary>
        /// Read data at the current position as <see cref="System.Byte"/>
        /// </summary>
        /// <returns>The <see cref="System.Byte"/> value</returns>
        byte ReadByte();

        /// <summary>
        /// Read data at the specific position as <see cref="System.Byte"/>
        /// </summary>
        /// <param name="index">The position to read data</param>
        /// <returns>The <see cref="System.Byte"/> value</returns>
        int ReadByteAt(int index);

        /// <summary>
        /// Read data at the current position as <see cref="double"/>
        /// </summary>
        /// <returns>The <see cref="double"/> value</returns>
        double ReadDouble();

        /// <summary>
        /// Read data at the specific position as <see cref="double"/>
        /// </summary>
        /// <param name="index">The position to read data</param>
        /// <returns>The <see cref="double"/> value</returns>
        double ReadDoubleAt(int index);

        /// <summary>
        /// Read data at the current position as <see cref="float"/>
        /// </summary>
        /// <returns>The <see cref="float"/> value</returns>
        float ReadFloat();

        /// <summary>
        /// Read data at the specific position as <see cref="float"/>
        /// </summary>
        /// <param name="index">The position to read data</param>
        /// <returns>The <see cref="float"/> value</returns>
        float ReadFloatAt(int index);

        /// <summary>
        /// Read data at the current position as <see cref="System.Int32"/>
        /// </summary>
        /// <returns>The <see cref="System.Int32"/> value</returns>
        int ReadInt();

        /// <summary>
        /// Read data at the specific position as <see cref="System.Int32"/>
        /// </summary>
        /// <param name="index">The position to read data</param>
        /// <returns>The <see cref="System.Int32"/> value</returns>
        int ReadIntAt(int index);

        /// <summary>
        /// Read data at the current position as <see cref="System.Int64"/>
        /// </summary>
        /// <returns>The <see cref="System.Int64"/> value</returns>
        long ReadLong();

        /// <summary>
        /// Read data at the specific position as <see cref="System.Int64"/>
        /// </summary>
        /// <param name="index">The position to read data</param>
        /// <returns>The <see cref="System.Int64"/> value</returns>
        long ReadLongAt(int index);

        /// <summary>
        /// Read data at the current position as <see cref="System.Int16"/>
        /// </summary>
        /// <returns>The <see cref="System.Int16"/> value</returns>
        short ReadShort();

        /// <summary>
        /// Read data at the specific position as <see cref="System.Int16"/>
        /// </summary>
        /// <param name="index">The position to read data</param>
        /// <returns>The <see cref="System.Int16"/> value</returns>
        short ReadShortAt(int index);

        #endregion
    }
}