/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.Common.Interfaces
{
    /// <summary>
    /// This is interface contact for representing functioanlities 
    /// </summary>
    public interface IByteBuffer
    {
        /// <summary>
        /// 
        /// </summary>
        int Begin { get; }

        /// <summary>
        /// 
        /// </summary>
        int Capacity { get;  }

        /// <summary>
        /// 
        /// </summary>
        byte[] Contents { get; set; }

        /// <summary>
        /// 
        /// </summary>
        bool HasRemaining { get; }

        /// <summary>
        /// 
        /// </summary>
        int Limit { get; }

        /// <summary>
        /// 
        /// </summary>
        int Position { get;  }

        /// <summary>
        /// 
        /// </summary>
        int Remaining { get; }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        ByteBuffer Rewind();

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        ByteBuffer Clear();

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        ByteBuffer Compact();

        /// <summary>
        /// 
        /// </summary>
        /// <param name="count"></param>
        /// <returns></returns>
        long Reserve(long count);

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        ByteBuffer Truncate();

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        ByteBuffer Flip();
    }
}