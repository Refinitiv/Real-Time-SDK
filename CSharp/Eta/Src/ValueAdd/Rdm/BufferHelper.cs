/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Helper class for handling operations with <see cref="Buffer"/> objects
    /// </summary>
    sealed public class BufferHelper
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
