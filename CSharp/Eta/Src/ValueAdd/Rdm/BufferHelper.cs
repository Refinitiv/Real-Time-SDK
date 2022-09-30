/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Common;
using Buffer = Refinitiv.Eta.Codec.Buffer;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Helper class for handling operations with <see cref="Buffer"/> objects
    /// </summary>
    public class BufferHelper
    {
        /// <summary>
        /// Copies data between two Buffer instances.
        /// </summary>
        /// <param name="source">The source <see cref="Buffer"/> from which bytes are copied</param>
        /// <param name="dest">The destination <see cref="Buffer"/> to which data is copied</param>
        public static void CopyBuffer(Buffer source, Buffer dest)
        {
            if (source != null && source.Length > 0)
            {
                ByteBuffer byteBuffer = new ByteBuffer(source.Length);
                source.Copy(byteBuffer);
                byteBuffer.Flip();
                dest.Data(byteBuffer);
            }
        }
    }
}
