/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using System.Text;

namespace LSEG.Eta.Codec
{
    /// <summary>
    /// This buffer is used by the RMTES Decoder to store and display the decoded strings.
    /// <see cref="RmtesCacheBuffer"/>
    /// <see cref="RmtesDecoder"/>
    /// </summary>
    sealed public class RmtesBuffer
    {
        /// <summary>
        /// The ByteBuffer containing data
        /// </summary>
        public ByteBuffer Data { get; set; }

        /// <summary>
        /// The length of the contents currently stored in the buffer
        /// </summary>
        public int Length { get; set; }

        /// <summary>
        /// The capacity of the buffer
        /// </summary>
        public int AllocatedLength { get; set; }

        /// <summary>
        /// Constructor for RmtesBuffer
        /// </summary>
        /// <param name="dataLength">The length of the data</param>
        /// <param name="byteData">ByteBuffer that contains data</param>
        /// <param name="allocLength">The capacity of the buffer</param>
        public RmtesBuffer(int dataLength, ByteBuffer byteData, int allocLength)
        {
            Length = dataLength;
            Data = byteData;
            AllocatedLength = allocLength;
        }

        /// <summary>
        /// Constructor for RmtesBuffer
        /// </summary>
        /// <param name="x">Buffer capacity</param>
        public RmtesBuffer(int x)
        {
            Data = new ByteBuffer(x);
            AllocatedLength = x;
            Length = 0;
        }

        /// <summary>
        /// String representation of RmtesBuffer
        /// </summary>
        /// <returns>string object that represents current RmtesBuffer instance
        /// </returns>
        override public string ToString()
        {
            return Encoding.Unicode.GetString(Data.Contents, 0, Length);
        }

        /// <summary>
        /// Clears current RmtesBuffer instance and sets all values to default
        /// </summary>
        public void Clear()
        {
            Data.Clear();
            Length = 0;
            AllocatedLength = Data.BufferLimit();
        }
    }
}
