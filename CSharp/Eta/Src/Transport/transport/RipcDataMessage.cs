/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Linq;

using Refinitiv.Eta.Transports.Internal;

namespace Refinitiv.Eta.Transports
{
    internal struct RipcDataMessage
    {
        public ushort MessageLength;

        public RipcFlags Flags;

        public byte[] Payload;

        public static ushort HeaderSize = sizeof(ushort) + sizeof(byte);

        public static ushort PackedHeaderSize = sizeof(ushort);

        public bool IsEmptyPayload { get => (Payload == null || Payload.Length == 0); }

        public bool IsPacked { get => (Flags & RipcFlags.PACKING) == RipcFlags.PACKING; }

        public RipcDataMessage(ushort capacity)
        {
            MessageLength = (ushort)(HeaderSize + capacity);
            Flags = RipcFlags.DATA;
            Payload = new byte[capacity];
        }

        internal RipcDataMessage(byte[] payload, RipcFlags flags = RipcFlags.DATA)
        {
            MessageLength = (ushort)(HeaderSize + payload.Length);
            Flags = flags;
            Payload = payload;
        }

        internal static int PopulateRipcHeader(TransportBuffer buffer, byte ripcFlags, bool addHeaderSize)
        {
            int entireMsgLength = buffer.Length;

            if(addHeaderSize)
            {
                if (!buffer.IsPacked)
                {
                    entireMsgLength += buffer.HeaderLength;
                }
                else
                {
                    entireMsgLength += buffer.HeaderLength + PackedHeaderSize;
                }
            }

            buffer.Data.WriteAt(buffer.StartPosition, (short)entireMsgLength);  // Populate RIPC message length
            buffer.Data.WriteAt(buffer.StartPosition + 2, ripcFlags);  // Populate RIPC message header flags

            return entireMsgLength;
        }

        internal static void PopulateRipcHeader(BigTransportBuffer bigBuffer, TransportBuffer buffer, bool firstFragment, byte flags, 
            byte optFlags, int fragmentedMsgLength, int entirePayloadLength)
        {
            if(firstFragment)
            {
                buffer.Data.WritePosition = buffer.StartPosition;
                buffer.Data.Write((short)fragmentedMsgLength);
                buffer.Data.Write(flags); // add flags in the third byte of header
                buffer.Data.Write(optFlags); // add Ext flags (08) in the fourth byte of header
                buffer.Data.Write(entirePayloadLength); // add the entire payload length

                // add fragment ID in the ninth byte of header
                if (bigBuffer.RipcVersion >= RipcVersions.VERSION13)
                {
                    // two byes fragment ID
                    buffer.Data.Write((short)bigBuffer.FragmentID);
                }
                else
                {
                    // one bye fragment ID
                    buffer.Data.Write((byte)bigBuffer.FragmentID);
                }
            }
            else
            {
                buffer.Data.WritePosition = buffer.StartPosition;
                buffer.Data.Write((short)fragmentedMsgLength);
                buffer.Data.Write(flags); // add flags in the third byte of header
                buffer.Data.Write(optFlags); // add Ext flags (04) in the fourth byte of header

                // add fragment ID in the ninth byte of header
                if (bigBuffer.RipcVersion >= RipcVersions.VERSION13)
                {
                    // two byes fragment ID
                    buffer.Data.Write((short)bigBuffer.FragmentID);
                }
                else
                {
                    // one bye fragment ID
                    buffer.Data.Write((byte)bigBuffer.FragmentID);
                }
            }
        }

        /* Populates the first (header) fragment with the given data.
        * bigBuffer is the BigTransportBuffer which holds the entire user's data
        * buffer is the fragment buffer
        * flags is the flags for the RipcFlags field
        * optFlags is the flags for the RipcOptionalFlags field
        * totalLength is the total (uncompressed) length as written in the first fragment
        * inData is the array containing the source data to be added to this fragment
        * offset is the offset in the source data array
        * inDataLength is the number of bytes to be copied from the inData array (starting from offset) and added to this fragment
        *
        * Returns the number of payload bytes copied to the fragment.
        * Returns TransportReturnCodes.FAILURE before any data is copied, if the input data will not fit in the fragment.
        */
        internal static int PopulateFirstFragment(BigTransportBuffer bigBuffer, TransportBuffer buffer, byte flags, byte optFlags, int totalLength, 
            byte[] inData, int offset, int inDataLength)
        {
            int msgLength = bigBuffer.FirstFragmentHeaderLength + inDataLength;

            if (msgLength > buffer.Data.Limit)
                return (int)TransportReturnCode.FAILURE;

            buffer.Data.WritePosition = buffer.StartPosition;
            buffer.Data.Write((short)msgLength);
            buffer.Data.Write(flags);
            buffer.Data.Write(optFlags); // add Ext flags (08) in the fourth byte of header
            buffer.Data.Write(totalLength); // add the payload length of all fragments

            // add fragment ID in the ninth byte of header
            if(bigBuffer.RipcVersion >= RipcVersions.VERSION13)
            {
                buffer.Data.Write((short)bigBuffer.FragmentID);
            }
            else
            {
                buffer.Data.Write((byte)bigBuffer.FragmentID);
            }

            buffer.Data.Put(inData, offset, inDataLength);

            buffer.TotalLength = msgLength;
            buffer.Data.ReadPosition = buffer.StartPosition;
            buffer.Data.Limit = buffer.StartPosition + msgLength;

            return (msgLength - bigBuffer.FirstFragmentHeaderLength);
        }

        /* Populates the Nth fragment in the set (N>1) with the given data.
        * 
        * bigBuffer is the BigTransportBuffer which holds the entire user's data
        * buffer is the fragment buffer
        * flags is the flags for the RipcFlags field
        * optFlags is the flags for the RipcOptionalFlags field
        * inData is the array containing the source data to be added to this fragment
        * offset is the starting position of input data in the inData array
        * inDataLength is the number of bytes of input data to be added to the fragment
        *
        * Returns the number of payload bytes copied to the fragment.
        * Returns TransportReturnCodes.FAILURE before any data is copied, if the input data will not fit in the fragment.
        */
        internal static int PopulateNextFragment(BigTransportBuffer bigBuffer, TransportBuffer buffer, byte flags, byte optFlags, byte[] inData, 
            int offset, int inDataLength)
        {
            int msgLength = bigBuffer.NextFragmentHeaderLength + inDataLength;

            if (msgLength > buffer.Data.Limit)
                return (int)TransportReturnCode.FAILURE;

            buffer.Data.WritePosition = buffer.StartPosition;
            buffer.Data.Write((short)msgLength);
            buffer.Data.Write(flags); // add flags in the third byte of header
            buffer.Data.Write(optFlags); // add Ext flags (04) in the fourth byte of header

            // add fragment ID in the ninth byte of header
            if (bigBuffer.RipcVersion >= RipcVersions.VERSION13)
            {
                buffer.Data.Write((short)bigBuffer.FragmentID);
            }
            else
            {
                buffer.Data.Write((byte)bigBuffer.FragmentID);
            }

            buffer.Data.Put(inData, offset, inDataLength);

            buffer.TotalLength = msgLength;
            buffer.Data.ReadPosition = buffer.StartPosition;
            buffer.Data.Limit = buffer.StartPosition + msgLength;

            return (msgLength - bigBuffer.NextFragmentHeaderLength);
        }

        public override string ToString()
        {
            var byteStrings = (Payload.Take((Payload.Length < 32)?Payload.Length:32).Select(b => $"{b:X2}"));
            string payload = String.Join(", ", byteStrings);
            return $"MessageLength: {MessageLength}, Flags: {Flags}, HeaderSize: {HeaderSize}, Payload: byte[{Payload?.Length}] {{ {payload} }}";
        }
    }
}
