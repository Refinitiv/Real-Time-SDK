/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license      --
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
*|                See the project's LICENSE.md for details.                  --
*|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
*|-----------------------------------------------------------------------------
*/

using System;

using LSEG.Eta.Common;
using LSEG.Eta.Transports.Internal;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.Transports
{
    internal class TransportBuffer : EtaNode, ITransportBuffer
    {
        private static ushort s_fragmentID = 0;

        internal static readonly int PACKED_HDR = 2;
        
        internal bool IsBigBuffer { get; set; }

        internal bool IsOwnedByApp { get; set; }

        internal bool IsReadMode { get; set; }

        internal int HeaderLength { get; set; }

        internal bool IsPacked { get; set; }

        internal int PackedMsgOffSetPosition { get; set; }

        internal bool IsWritePaused { get; set; }

        internal RipcVersions RipcVersion { get; private set; }

        internal int FirstFragmentHeaderLength { get; private set; }

        internal int NextFragmentHeaderLength { get; private set; }
     
        internal ushort FragmentID { get; private set; }

        internal TransportBuffer FirstBuffer { get; set; }

        /// <summary>
        /// Gets or sets this position is set in the write buffer when the buffer is obtained via the GetBuffer() method.
        /// </summary>
        internal int StartPosition { get; set; }

        internal TransportBuffer(bool isBigBuffer = false)
        {
            if (!isBigBuffer)
            {
                IsBigBuffer = false;
                IsReadMode = false;
                StartPosition = 0;
                HeaderLength = RipcLengths.HEADER;
            }            
        }

        internal TransportBuffer(int capacity, bool isBigBuffer = false)
        {
            Data = new ByteBuffer(capacity);
            if (!isBigBuffer)
            { 
                IsBigBuffer = false;
                IsReadMode = false;
                StartPosition = 0;
                HeaderLength = RipcLengths.HEADER;
            } else
            {
                NextID(); // Get next fragmented ID for this buffer
                IsBigBuffer = true;
                IsWritePaused = false;
            }
                
        }       

        internal TransportBuffer(Pool pool, ByteBuffer dataBuffer, bool isBigBuffer = false)
        {
            Pool = pool;
            Data = dataBuffer;
            IsBigBuffer = isBigBuffer;
            IsReadMode = false;
            HeaderLength = 0;
            HeaderLength = RipcLengths.HEADER;
        }

        internal TransportBuffer(ByteBuffer data, int headerLength, bool skipHeader = true)
        {
            Data = data;
            IsBigBuffer = false;
            IsReadMode = false;
            HeaderLength = headerLength;

            if (skipHeader)
                Data.WritePosition += HeaderLength;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void SetData(ByteBuffer data, int headerLength, bool skipHeader = true)
        {
            Data = data;
            HeaderLength = headerLength;

            if (skipHeader)
                Data.WritePosition += HeaderLength;
        }

        internal void Clear()
        {
            Data = null;
            IsReadMode = false;
            HeaderLength = 0;
            StartPosition = 0;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal int PackedLen()
        {
            return Data.Position - StartPosition - HeaderLength;
        }

        internal static TransportBuffer Load(ref ByteBuffer ioBuffer)
        {
            TransportBuffer transportBuffer = null;

            //----------------------------------------------------------------------
            // Attempt to parse th ioBuffer for a RipcMessage
            RipcDataMessage dataMessage = default(RipcDataMessage);
            if (ioBuffer.ReadAt(ref dataMessage, ioBuffer.ReadPosition, false))
            {
                //------------------------------------------------------------------
                // A complete RipcMessage is available; peel a new TransportBuffer
                // out of the IO Buffer, but...

                ushort messageLength = 0;
                ByteBuffer dataBuffer;

                if (dataMessage.IsPacked)
                {   // ...is it a Packed Message (an envelope and a set of payloads), ...

                    if (dataMessage.MessageLength == RipcDataMessage.HeaderSize)
                        // Empty Packed Message; treat it like a 'ping' packet.
                        dataBuffer = ExtractDataMessage(ref ioBuffer, dataMessage, out messageLength);
                    else
                        // Extract the lead sub-message from the envelope; compact the IO Buffer.
                        dataBuffer = ExtractPackedMessage(ref ioBuffer, dataMessage, out messageLength);

                }
                else
                {   // ...or is it a Normal Message?
                    dataBuffer = ExtractDataMessage(ref ioBuffer, dataMessage, out messageLength);
                }

                transportBuffer = new TransportBuffer(dataBuffer, RipcDataMessage.HeaderSize, false);
            }
            else
            {
                //----------------------------------------------------------------------
                // The IO Buffer contains a partial message; compact the IO Buffer
                // to allow it to be sent around again the RecvBegin/LoadReadBuffer cycle.
                ioBuffer.Compact();
            }

            return transportBuffer;
        }

        private static ByteBuffer ExtractPackedMessage(ref ByteBuffer ioBuffer, RipcDataMessage dataMessage, out ushort messageLength)
        {
            // Remember where we started
            int readPosition = ioBuffer.ReadPosition;
            // Skip past the header
            ioBuffer.ReadPosition += RipcDataMessage.HeaderSize;

            // Read the 1st payload into the dataBuffer
            messageLength = (ushort)ioBuffer.ReadShort();

            ByteBuffer dataBuffer = new ByteBuffer(messageLength + RipcDataMessage.HeaderSize);
            dataBuffer.Write((short)(messageLength + RipcDataMessage.HeaderSize));
            dataBuffer.Write((byte)RipcFlags.DATA);
            dataBuffer.Put(ioBuffer.Contents, ioBuffer.ReadPosition, dataBuffer.Limit - RipcDataMessage.HeaderSize);

            //------------------------------------------------------------
            // Rewrite the IO Buffer
            // Shift the first message to the head of the payload area
            Buffer.BlockCopy(ioBuffer.Contents,
                             ioBuffer.ReadPosition + messageLength,
                             ioBuffer.Contents,
                             ioBuffer.ReadPosition - sizeof(ushort),
                             ioBuffer.WritePosition - (ioBuffer.ReadPosition + messageLength));

            // Decrement the IO Buffer's MessageLength
            dataMessage.MessageLength -= (ushort)(messageLength + sizeof(ushort));
            ioBuffer.WriteAt(readPosition, (short)dataMessage.MessageLength);

            // Decrement the IO Buffer Write Position, as well
            ioBuffer.WritePosition -= (ushort)(messageLength + sizeof(ushort));

            // Backfill the abandoned portion of the IO Buffer
            int writePosition = ioBuffer.WritePosition;
            while (ioBuffer.WritePosition < ioBuffer.Capacity)
                ioBuffer.Write((byte)0xAA);
            ioBuffer.WritePosition = writePosition;

            // Reset the Read Position to it's original value
            ioBuffer.ReadPosition = (ioBuffer.WritePosition == readPosition + RipcDataMessage.HeaderSize)
                                            ? ioBuffer.WritePosition
                                            : readPosition;

            return dataBuffer;
        }

        private static ByteBuffer ExtractDataMessage(ref ByteBuffer ioBuffer, RipcDataMessage dataMessage, out ushort messageLength)
        {
            messageLength = dataMessage.MessageLength;
            ByteBuffer dataBuffer = new ByteBuffer(ioBuffer.Contents, false)
            {
                ReadPosition = ioBuffer.ReadPosition,
                WritePosition = messageLength
            };

            ioBuffer.ReadPosition += dataBuffer.WritePosition;
            return dataBuffer;
        }

        public ByteBuffer Data { get; set; }

        /* Should be called only if buffer is packed. */
        internal int PackedLength()
        {
            return Data.WritePosition - StartPosition - HeaderLength;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int Length()
        {
            if (!IsBigBuffer)
            {
                if (!IsReadMode)
                {
                    if (!IsPacked)
                    {
                        int length = Data.WritePosition - StartPosition - HeaderLength;
                        if (length > 0)
                        {
                            return length;
                        }
                        else
                        {
                            return Capacity();
                        }
                    }
                    else
                    {
                        int length = Data.WritePosition - StartPosition - HeaderLength - PACKED_HDR;

                        if (length > 0)
                        {
                            return length;
                        }
                        else
                        {
                            return Capacity();
                        }
                    }
                }
                else
                {
                    return Data.WritePosition - Data.ReadPosition;
                }
            } 
            else
            {
                if (Data.Position - StartPosition > 0)
                {
                    return Data.Position - StartPosition;
                }
                else
                {
                    return Data.Limit - StartPosition;
                }
            }
            
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int Capacity()
        {
            if (!IsBigBuffer)
            {
                if (!IsReadMode)
                {
                    return Data.Limit - StartPosition - HeaderLength;
                }
                else
                {
                    return Length();
                }
            } 
            else
            {
                return Data.Capacity;
            }
                
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int GetDataStartPosition()
        {
            if (!IsBigBuffer)
            {
                if (!IsReadMode)
                {
                    return Data.ReadPosition + StartPosition + HeaderLength;
                }
                else
                {
                    return Data.ReadPosition;
                }
            }
            else
            {
                return Data.ReadPosition;
            }
        }

        public int Copy(ByteBuffer destination)
        {
            if (destination is null)
                throw new ArgumentNullException(nameof(destination));

            if (destination.Capacity < Length())
                throw new InsufficientMemoryException(nameof(destination));

            TransportReturnCode result = TransportReturnCode.FAILURE;

            try
            {
                destination.Put(Data.Contents, destination.WritePosition, Length());
                result = TransportReturnCode.SUCCESS;
            }
            catch (Exception)
            {
            }

            return (int)result;
        }

        public override string ToString()
        {
            return $"Data: {{{Data}}}";
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal bool GetIsRipcMessage()
        {
            if (!IsBigBuffer)
            {
                // Data contains at least a Header
                bool isRipcMessage = Data.WritePosition >= RipcDataMessage.HeaderSize;
                if (isRipcMessage)
                {
                    short msgLength = Data.ReadShortAt(0);
                    // Payload is complete
                    isRipcMessage = isRipcMessage && (msgLength == Length() + RipcDataMessage.HeaderSize);

                    // RipcFlags.Data 
                    RipcFlags flags = (RipcFlags)Data.ReadByteAt(sizeof(short));
                    isRipcMessage = isRipcMessage && ((flags & RipcFlags.DATA) == RipcFlags.DATA);
                }

                return isRipcMessage;
            }
            else
            {
                return false;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void PopulateRipcHeader(byte flags)
        {
            int lastPosition = Data.Position;
            TotalLength = lastPosition - StartPosition;
            Data.WritePosition = StartPosition;
            Data.Write((short)TotalLength);
            Data.Write(flags);
            Data.ReadPosition = StartPosition;
            Data.WritePosition = lastPosition;
        }

        /// <summary>
        /// Gets or sets the total message length including the header.
        /// </summary>
        internal int TotalLength { get; set; }

        TransportReturnCode IsPackedBuffer(IChannel chnl, out Error error)
        {
            if (!IsPacked)
            {
                error = new Error()
                {
                    Channel = chnl,
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = "Cannot pack a buffer with packing disabled"
                };
                return TransportReturnCode.FAILURE;
            }

            error = null;
            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Update the packed header with the length of the message being packed.
        /// </summary>
        /// <param name="reserveNextPackedHdr">if true, this method will attempt to reserve space (i.e. the packed header) for another packed message</param>
        /// <param name="chnl">current <see cref="IChannel"/> instance</param>
        /// <param name="error">output <see cref="Error"/> parameter</param>
        /// <returns>the amount of user available bytes remaining for packing</returns>
        internal int Pack(bool reserveNextPackedHdr, IChannel chnl, out Error error)
        {
            error = null;
            // immediately return failure if packing not enabled on buffer
            if (!IsPacked)
            {
                error = new Error()
                {
                    Channel = chnl,
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = "Cannot pack a buffer with packing disabled."
                };
                return -1;
            }

            if (Data.Position > (PackedMsgOffSetPosition + PACKED_HDR))
            {
                // write the length of the message being packed
                Data.WriteAt(PackedMsgOffSetPosition, (short)(Data.Position - PackedMsgOffSetPosition - PACKED_HDR));

                // set new mark for the next message
                PackedMsgOffSetPosition = Data.Position;
            }

            if (reserveNextPackedHdr)
            {
                // attempt to reserve space for the next packed_hdr.
                if ((PackedMsgOffSetPosition + PACKED_HDR) > Data.Limit)
                {
                    error = new Error()
                    {
                        Channel = chnl,
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = "Failed to reserve space for the next packed header."
                    };
                    return -1; // FAILURE
                }

                SetDataPosition(PackedMsgOffSetPosition + PACKED_HDR);
            }
            else
            {
                // don't reserve space for the next packed_hdr, we're done packing.
                SetDataPosition(PackedMsgOffSetPosition);
                return 0; //SUCCESS
            }

            return Data.Limit - Data.Position;
        }

        internal static int PackBuffer(TransportBuffer packedBuffer, bool reserveNextPackedHdr, IChannel chnl, out Error error)
        {
            if (packedBuffer.IsPackedBuffer(chnl, out error) != TransportReturnCode.SUCCESS)
            {
                return -1; // TransportReturnCode.FAILURE
            }

            if (packedBuffer.Data.Position > packedBuffer.PackedMsgOffSetPosition + PACKED_HDR)
            {
                // write the length of the message being packed
                packedBuffer.Data.WriteAt(packedBuffer.PackedMsgOffSetPosition, (short)(packedBuffer.Data.Position - packedBuffer.PackedMsgOffSetPosition - PACKED_HDR));

                // set new mark for the next message
                packedBuffer.PackedMsgOffSetPosition = packedBuffer.Data.Position;
            }

            if (reserveNextPackedHdr)
            {
                // attempt to reserve space for the next packed_hdr.
                if (packedBuffer.PackedMsgOffSetPosition + PACKED_HDR > packedBuffer.Data.Limit)
                {
                    error = new Error()
                    {
                        Channel = chnl,
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = "Failed to reserve space for the next packed header."
                    };
                    return -1; // FAILURE
                }

                packedBuffer.SetDataPosition(packedBuffer.PackedMsgOffSetPosition + PACKED_HDR);
            }
            else
            {
                // don't reserve space for the next packed_hdr, we're done packing.
                packedBuffer.SetDataPosition(packedBuffer.PackedMsgOffSetPosition);
                return 0;
            }

            return packedBuffer.Data.Limit - packedBuffer.Data.Position;
        }

        internal void SetDataPosition(int position)
        {
            if (IsReadMode)
            {
                Data.ReadPosition = position;
            }
            else
            {
                Data.WritePosition = position;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void NextID()
        {
            ++s_fragmentID;

            if (s_fragmentID == 0)
            {
                s_fragmentID = 1;
            }

            FragmentID = s_fragmentID;
        }

        internal void SetRipcVersion(RipcVersions ripcVersion)
        {
            RipcVersion = ripcVersion;

            if (ripcVersion >= RipcVersions.VERSION13)
            {
                FirstFragmentHeaderLength = 10;
                NextFragmentHeaderLength = 6;
            }
            else
            {
                FirstFragmentHeaderLength = 9;
                NextFragmentHeaderLength = 5;
            }
        }

        public override void ReturnToPool()
        {
            if (!IsBigBuffer)
            {
                base.ReturnToPool();
            } else
            {
                if (!InPool)
                {
                    if (FirstBuffer != null)
                        FirstBuffer.ReturnToPool();
                    FirstBuffer = null;
                    IsWritePaused = false;
                    Data.Clear();
                    Pool.Add(this);
                }
            }
        }
    }
}

