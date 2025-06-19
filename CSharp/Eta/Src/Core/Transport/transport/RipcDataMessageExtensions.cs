/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using LSEG.Eta.Common;
using LSEG.Eta.Transports.Internal;

namespace LSEG.Eta.Transports
{
    internal static class RipcDataMessageExtensions
    {

        internal static bool HasMoreMessage(this ByteBuffer buffer, ref RipcDataMessage msgRef, int readPosition)
        {
            int remainingToReadLength = buffer.WritePosition - buffer.ReadPosition;
            if (remainingToReadLength < RipcDataMessage.HeaderSize)
            {   // Not enuff data
                return false;
            }

            if (msgRef.Equals(default(RipcDataMessage)))
                msgRef = new RipcDataMessage();

            msgRef.MessageLength = (ushort)buffer.ReadShortAt(readPosition);

            return remainingToReadLength >= msgRef.MessageLength;
        }

        public static bool ReadAt(this ByteBuffer buffer, ref RipcDataMessage msgRef, int readPosition, bool loadPayload = false)
        {
            int remainingToReadLength = buffer.WritePosition - buffer.ReadPosition;
            if (remainingToReadLength < RipcDataMessage.HeaderSize)
            {   // Not enuff data
                return false;
            }

            if (msgRef.Equals(default(RipcDataMessage)))
                msgRef = new RipcDataMessage();

            msgRef.MessageLength = (ushort)buffer.ReadShortAt(readPosition);
            msgRef.Flags = (RipcFlags)buffer.ReadByteAt(readPosition + sizeof(ushort));

            if ((msgRef.Flags & RipcFlags.DATA) != RipcFlags.DATA)
            {
                // Not a Connection request
                throw new InvalidOperationException("Incorrect layout for RipcDataMessage");
            }

            if (loadPayload)
            {
                int bufferReadReposition = buffer.ReadPosition;

                try
                {
                    buffer.ReadPosition += RipcDataMessage.HeaderSize;
                    if (msgRef.IsEmptyPayload)
                        msgRef.Payload = new byte[msgRef.MessageLength - RipcDataMessage.HeaderSize];
                    buffer.ReadBytesInto(msgRef.Payload,
                                     0, msgRef.MessageLength - RipcDataMessage.HeaderSize);
                }
                finally
                {
                    buffer.ReadPosition = bufferReadReposition;
                }
            }

            // Return whether an entire RipcMessage was read.
            return remainingToReadLength >=
                        msgRef.MessageLength;
        }

        public static bool Read(this ByteBuffer buffer, ref RipcDataMessage msgRef, bool loadPayload = false)
        {
            var message = buffer.ReadAt(ref msgRef, buffer.ReadPosition, loadPayload);
            buffer.ReadPosition += (loadPayload) ? msgRef.MessageLength : RipcDataMessage.HeaderSize;
            return message;
        }

    }
}
