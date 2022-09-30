/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Common;
using System.Text;

namespace Refinitiv.Eta.Codec
{
    /// <summary>
    /// This buffer is used by the RMTES Decoding interface to store data that needs 
    /// to be decoded, and used for partial data storage.
    /// </summary>
    public class RmtesCacheBuffer
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
        /// Constructor for RmtesCacheBuffer
        /// </summary>
        /// <param name="dataLength">The length of the input data</param>
        /// <param name="byteData">ByteBuffer that contains input data</param>
        /// <param name="allocLength">The capacity of the RmtesCacheBuffer instance</param>
        public RmtesCacheBuffer(int dataLength, ByteBuffer byteData, int allocLength)
        {
            Length = dataLength;
            Data = byteData;
            AllocatedLength = allocLength;
        }

        /// <summary>
        /// Constructor for RmtesCacheBuffer
        /// </summary>
        /// <param name="x">The allocated length of the buffer</param>
        public RmtesCacheBuffer(int x)
        {
            Data = new ByteBuffer(x);
            AllocatedLength = x;
            Length = 0;
        }

        /// <summary>
        /// String representation of the current RmtesCacheBuffer
        /// </summary>
        /// <returns>string object that represents this instance of RmtesCacheBuffer</returns>
        override public string ToString()
        {
            return Encoding.Unicode.GetString(Data.Contents, 0, Length);
        }

        /// <summary>
        /// Clears current RmtesCacheBuffer instance
        /// </summary>
        public void Clear()
        {
            Data.Clear();
            Length = 0;
            AllocatedLength = Data.Limit;
        }
    }
}
