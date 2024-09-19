
/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using LSEG.Eta.Internal.Interfaces;
using LSEG.Eta.Transports.Internal;
using LSEG.Eta.Common;
using System.Net.Sockets;
using LSEG.Eta.Internal;

namespace LSEG.Eta.Transports
{
    class RipcProtocolFunctions : IProtocolFunctions
    {
        private ChannelBase m_ChannelBase;
        private ReadBufferStateMachine m_ReadBufferStateMachine;

        public RipcProtocolFunctions(ChannelBase channelBase)
        {
            m_ChannelBase = channelBase;
        }

        public int EstimateHeaderLength()
        {
            return RipcDataMessage.HeaderSize;
        }

        public int EntireHeaderLength()
        {
            return RipcDataMessage.HeaderSize;
        }

        public void SetReadBufferStateMachine(ReadBufferStateMachine readBufferStateMachine)
        {
            m_ReadBufferStateMachine = readBufferStateMachine;
        }

        public ITransportBuffer GetBigBuffer(int size, out Error error)
        {
            TransportBuffer buffer;
            error = null;

            try
            {
                buffer = (TransportBuffer)m_ChannelBase.m_BigBuffersPool.Poll(size, true);
            }
            catch(Exception exp) // Handle OutOfMemoryExcpetion
            {
                error = new Error()
                {
                    ErrorId = TransportReturnCode.NO_BUFFERS,
                    SysError = 0,
                    Text = exp.Message
                };

                return null;
            }

            if (error != null)
            {
                return null;
            }

            SocketBuffer sBuffer = m_ChannelBase.GetSocketBuffer();

            if (sBuffer != null)
            {
                buffer.FirstBuffer = sBuffer.GetBufferSliceForFragment(m_ChannelBase.InternalFragmentSize);
                buffer.NextID();
                buffer.IsOwnedByApp = true;
                buffer.StartPosition = buffer.Data.Begin;
                buffer.Data.Limit = size;
                // Sets the RIPC vesion from the handshake
                buffer.SetRipcVersion(m_ChannelBase.RipcVersion);
            }
            else
            {
                buffer.ReturnToPool();
                error = new Error()
                {
                    ErrorId = TransportReturnCode.NO_BUFFERS,
                    SysError = 0,
                    Text = "Failed to get buffer slice for fragment"
                };

                return null;
            }

            return buffer;
        }

        public int PrependTransportHdr(TransportBuffer buffer, short ripcFlags, bool addHeaderLength)
        {
            buffer.TotalLength  = RipcDataMessage.PopulateRipcHeader(buffer, (byte)ripcFlags, addHeaderLength);

            return 0; // Returns additional header for a specific connection type if any
        }

        public bool WriteAsFragmentedMessage(TransportBuffer buffer)
        {
           return buffer.Data.Position + EstimateHeaderLength() > m_ChannelBase.InternalFragmentSize;
        }

        public int RemaingBytesAfterPausing(TransportBuffer bigBuffer)
        {
            return bigBuffer.Data.Remaining;
        }

        public int PopulateFragment(TransportBuffer bigBuffer, bool firstFragment, TransportBuffer buffer, byte flags, WriteArgs writeArgs)
        {
            int bytesCopied = 0;
            int position = bigBuffer.Data.Position; // Returns the current read position
            int limit = bigBuffer.Data.Limit; // Returns the write position
            int uncompressedBytesWritten = writeArgs.UncompressedBytesWritten;
            int bytesWritten = writeArgs.BytesWritten;
            int length;

            if(firstFragment)
            {
                buffer.HeaderLength -= buffer.GetDataStartPosition(); // Moves the cursor back before the RIPC header
                RipcDataMessage.PopulateRipcHeader(bigBuffer, buffer, true, flags, (byte)RipcOpCode.FRAGMENT_HEADER, buffer.Capacity(), limit);

                bytesCopied = buffer.Capacity() - bigBuffer.FirstFragmentHeaderLength;
                bigBuffer.Data.ReadPosition = 0;
                bigBuffer.Data.WritePosition = bytesCopied;
                length = buffer.Capacity();
                buffer.TotalLength = length;

                writeArgs.UncompressedBytesWritten = (uncompressedBytesWritten + bytesCopied + bigBuffer.FirstFragmentHeaderLength);
            }
            else
            {
                buffer.HeaderLength -= buffer.GetDataStartPosition(); // Moves the cursor back before the RIPC header
                int bytesToCopy = limit - position;
                if(buffer.Capacity() <= (bytesToCopy + bigBuffer.NextFragmentHeaderLength))
                {
                    bytesCopied = buffer.Capacity() - bigBuffer.NextFragmentHeaderLength;
                }
                else
                {
                    bytesCopied = bytesToCopy;
                }

                RipcDataMessage.PopulateRipcHeader(bigBuffer, buffer, false, flags, (byte)RipcOpCode.FRAGMENT, bytesCopied + bigBuffer.NextFragmentHeaderLength, 0);

                length = bytesCopied + bigBuffer.NextFragmentHeaderLength;
                buffer.TotalLength = length;
                bigBuffer.Data.ReadPosition = position;
                bigBuffer.Data.WritePosition = position + bytesCopied;

                writeArgs.UncompressedBytesWritten = (uncompressedBytesWritten + bytesCopied + bigBuffer.NextFragmentHeaderLength);
            }

            buffer.Data.Put(bigBuffer.Data);
            buffer.Data.Limit = buffer.Data.WritePosition;
            bigBuffer.Data.ReadPosition += bytesCopied;
            bigBuffer.Data.WritePosition = limit;
            writeArgs.BytesWritten = bytesWritten + length;

            return bytesCopied;
        }

        public int MessageLength()
        {
            return m_ReadBufferStateMachine.CurrentMsgRipcLen;
        }

        public void UnsetMessageLength()
        {
            m_ReadBufferStateMachine.CurrentMsgRipcLen = ReadBufferStateMachine.UNKNOWN_LENGTH;
        }

        public bool IsRWFProtocol()
        {
            return true;
        }

        public int AdditionalHeaderLength()
        {
            return 0;
        }

        public bool IsPingMessage()
        {
            return (m_ReadBufferStateMachine.CurrentMsgRipcLen == RipcDataMessage.HeaderSize);
        }

        public int PrependInitChnlHdr(ByteBuffer sourceData, ByteBuffer destinationData)
        {
            if(destinationData != null && ReferenceEquals(sourceData, destinationData) == false)
            {
                destinationData.Put(sourceData);
                sourceData.Clear();
            }

            return 0;
        }

        public int InitChnlReadFromChannel(ByteBuffer sourceData, out Error error)
        {
            error = null;

            int bytesRead = m_ChannelBase.SocketChannel.Receive(sourceData, out SocketError socketError);

            if(bytesRead > 0)
            {
                if(sourceData.Position > (m_ChannelBase.HTTP_HEADER4 + 2))
                {
                    int messageLength = sourceData.ReadUShortAt(m_ChannelBase.HTTP_HEADER4);
                    if(sourceData.Position >= (m_ChannelBase.HTTP_HEADER4 + messageLength + m_ChannelBase.CHUNKEND_SIZE))
                    {
                        // we have at least one complete message
                        return sourceData.Position;
                    }
                }
            }
            else if (bytesRead == -1)
            {
                m_ChannelBase.InitChannelState = ChannelBase.InitChnlState.RECONNECTING;
                m_ChannelBase.SocketChannel.Socket.Disconnect(false);
            }

            // we don't have a complete message, or no bytes were read.
            return 0;
        }

        public int InitChnlReadFromChannelProvider(ByteBuffer dst, out Error error)
        {
            error = null;

            int bytesRead = m_ChannelBase.SocketChannel.Receive(dst, out SocketError socketError);

            if (bytesRead > 0)
            {
                if (dst.Position > (m_ChannelBase.HTTP_HEADER4 + 2))
                {
                    if (m_ChannelBase.CheckIsProviderHTTP(dst))
                    {
                        m_ChannelBase.IsProviderHTTP = true;
                    }

                    if (m_ChannelBase.IsProviderHTTP != true)
                    {
                        int messageLength = dst.ReadUShortAt(m_ChannelBase.HTTP_HEADER4);
                        if (dst.Position >= (m_ChannelBase.HTTP_HEADER4 + messageLength + m_ChannelBase.CHUNKEND_SIZE))
                        {
                            // we have at least one complete message
                            return dst.Position;
                        }
                    }
                    else
                    {
                        return dst.Position;
                    }
                }
            }
            else if (bytesRead == -1)
            {
                m_ChannelBase.InitChannelState = ChannelBase.InitChnlState.RECONNECTING;
                return -1;
            }

            return 0;
        }

        public int PackBuffer(TransportBuffer packedBuffer, bool reserveNextPackedHdr, IChannel chnl, out Error error)
        {
            return TransportBuffer.PackBuffer(packedBuffer, reserveNextPackedHdr, chnl, out error);
        }    

        /* Takes the uncompressed buffer, compressed the data and write it.
         * One or two RIPC messages can be created from the compressed data.
         * This method handles a normal message, not big buffers for fragmentation.
         * 
         * Returns count of bytes queued if successful, or TransportReturnCode for error scenarios.
         */
        public int WriteCompressed(TransportBuffer buffer, WriteArgs writeArgs, out Error error)
        {
            error = null;
            int retVal = 0;
            int bytesForBuffer = 0;
            int totalBytes = 0;
            short ripcHdrFlags = (short)RipcFlags.COMPRESSION;
            int msgLen = buffer.Length();
            int hdrLen = m_ChannelBase.ProtocolFunctions.EstimateHeaderLength();
            int MAX_BYTES_FOR_BUFFER = m_ChannelBase.InternalFragmentSize - hdrLen;

            // An extra buffer might be needed: get it now before compression
            TransportBuffer compFragmentBuffer = m_ChannelBase.GetBufferInternal(MAX_BYTES_FOR_BUFFER, false, hdrLen);

            if (compFragmentBuffer == null)
            {
                retVal = m_ChannelBase.FlushInternal(out error);
                if (retVal < (int)TransportReturnCode.SUCCESS)
                    return retVal;
                compFragmentBuffer = m_ChannelBase.GetBufferInternal(MAX_BYTES_FOR_BUFFER, false, hdrLen);
                if (compFragmentBuffer == null)
                {
                    error = new Error
                    {
                        Channel = m_ChannelBase,
                        ErrorId = TransportReturnCode.NO_BUFFERS,
                        SysError = 0,
                        Text = "channel out of buffers"
                    };

                    return (int)TransportReturnCode.NO_BUFFERS;
                }
            }

            compFragmentBuffer.HeaderLength = hdrLen;

            if (buffer.IsPacked)
            {
                ripcHdrFlags |= (int)RipcFlags.PACKING;
                msgLen = buffer.PackedLen();
            }

            int compressedBytesLen = m_ChannelBase.m_Compressor.Compress(buffer, buffer.GetDataStartPosition(), msgLen);

            byte[] compressedBytes = m_ChannelBase.m_Compressor.CompressedData;

            if (compressedBytesLen > MAX_BYTES_FOR_BUFFER)
            {
                // The compressed data will be split into two ripc messages since the compressed size exceeded the buffer size.
                // This is possible when the uncompressed data is near the buffer size, and the data compresses poorly (data size grows).
                bytesForBuffer = MAX_BYTES_FOR_BUFFER;
                ripcHdrFlags |= (short)RipcFlags.COMP_FRAGMENT;
            }
            else
            {
                bytesForBuffer = compressedBytesLen;
            }

            // Transfer compressed bytes to the transport buffer
            buffer.Data.WritePosition = buffer.GetDataStartPosition();
            buffer.Data.Limit = buffer.GetDataStartPosition() + bytesForBuffer;
            buffer.Data.Put(compressedBytes, 0, bytesForBuffer);
            // Do this last (before write), so that internal buffer length and position set

            int additionalHdrLen = m_ChannelBase.ProtocolFunctions.PrependTransportHdr(buffer, ripcHdrFlags);

            retVal = (int)m_ChannelBase.WriteBuffersQueued(buffer, writeArgs, out error);

            // First part stats: User data bytes + overhead (ignore compression)
            writeArgs.UncompressedBytesWritten  = msgLen + additionalHdrLen + RipcLengths.HEADER;
            totalBytes = buffer.TotalLength;

            // Send extra message if there are bytes that did not fit in the first part
            if (retVal >= (int)TransportReturnCode.SUCCESS && compressedBytesLen > MAX_BYTES_FOR_BUFFER)
            {
                // Remaining compressed bytes to be sent
                bytesForBuffer = compressedBytesLen - MAX_BYTES_FOR_BUFFER;

                // Populate second message
                compFragmentBuffer.Data.WritePosition = compFragmentBuffer.GetDataStartPosition();
                compFragmentBuffer.Data.Limit = compFragmentBuffer.GetDataStartPosition() + bytesForBuffer;
                compFragmentBuffer.Data.Put(compressedBytes, MAX_BYTES_FOR_BUFFER, bytesForBuffer);

                additionalHdrLen = m_ChannelBase.ProtocolFunctions.PrependTransportHdr(compFragmentBuffer, (short)RipcFlags.COMPRESSION);

                writeArgs.UncompressedBytesWritten = writeArgs.UncompressedBytesWritten + additionalHdrLen + RipcLengths.HEADER;
                totalBytes += compFragmentBuffer.TotalLength;

                retVal = (int)m_ChannelBase.WriteBuffersQueued(compFragmentBuffer, writeArgs, out error);
            }
            else
            {
                m_ChannelBase.ReleaseBufferInternal(compFragmentBuffer);
            }

            writeArgs.BytesWritten = totalBytes;

            return retVal;
        }

        /* Builds the fragment by taking the next chunk of data from the big buffer,
        * compressing it, and populating the fragment.
        *
        * bigBuffer is the source of data to be fragmented
        *
        * fragment is the fragment to be built with data compressed from bigBuffer
        *
        * writeArgs is the firstFragment if this is the first fragment being built for
        *            the bigBuffer message
        *
        * Returns the number of bytes from the bigBuffer which were encoded in this fragment,
        * or TransportReturnCodes if there is a failure.
        */
        public int WriteFragmentCompressed(TransportBuffer bigBuffer, TransportBuffer fragment, WriteArgs writeArgs, bool firstFragment, out Error error)
        {
            error = null;
            int userBytesForFragment = 0;
            int position = bigBuffer.Data.Position; // Returns the current read position
            int limit = bigBuffer.Data.Limit; // Returns the write position
            byte flags = (byte)(RipcFlags.HAS_OPTIONAL_FLAGS | RipcFlags.COMPRESSION);
            byte optFlags = 0;
            int headerLength = 0;
            int compressedLen = 0;
            int maxPayloadSize = 0;
            byte[] compressedBytes;
            int extraBytes = 0;
            int extraTotalLength = 0;
            int extraHeaderLength = 0;
            int totalLength = 0;
            int retVal = (int)TransportReturnCode.SUCCESS;
            int MAX_BYTES_FOR_BUFFER = m_ChannelBase.InternalFragmentSize - RipcLengths.HEADER;

            // An extra buffer might be needed: get it now before compression
            TransportBuffer compFragmentBuffer = m_ChannelBase.GetBufferInternal(MAX_BYTES_FOR_BUFFER, false, RipcLengths.HEADER);

            if (compFragmentBuffer == null)
            {
                retVal = m_ChannelBase.FlushInternal(out error);
                if (retVal < (int)TransportReturnCode.SUCCESS)
                    return retVal;
                compFragmentBuffer = m_ChannelBase.GetBufferInternal(MAX_BYTES_FOR_BUFFER, false, RipcLengths.HEADER);
                if (compFragmentBuffer == null)
                {
                    error = new Error
                    {
                        Channel = m_ChannelBase,
                        ErrorId = TransportReturnCode.NO_BUFFERS,
                        SysError = 0,
                        Text = "channel out of buffers"
                    };

                    return (int)TransportReturnCode.NO_BUFFERS;
                }
            }

            if (firstFragment)
            {
                totalLength = limit;
                optFlags = (byte)RipcOpCode.FRAGMENT_HEADER;
                headerLength = bigBuffer.FirstFragmentHeaderLength;
                maxPayloadSize = fragment.Data.Limit - headerLength;

                bigBuffer.Data.ReadPosition = 0; // start at the beginning
                userBytesForFragment = fragment.Data.Limit - headerLength;
                bigBuffer.Data.WritePosition = userBytesForFragment;
            }
            else
            {
                optFlags = (byte)RipcOpCode.FRAGMENT;
                headerLength = bigBuffer.NextFragmentHeaderLength;
                maxPayloadSize = fragment.Data.Limit - headerLength;

                int bytesRemaining = limit - position; // bytes remaining in big buffer
                if (fragment.Data.Limit <= (bytesRemaining + headerLength))
                {
                    userBytesForFragment = fragment.Data.Limit - headerLength;
                }
                else
                {
                    userBytesForFragment = bytesRemaining;
                }
            }

            // Compress the selected number of bytes (userBytesForFragment) for the fragment
            compressedLen = m_ChannelBase.m_Compressor.Compress(bigBuffer,
                    bigBuffer.Data.ReadPosition, // big buffer position points at data to be sent
                    userBytesForFragment); // number of bytes to compress

            compressedBytes = m_ChannelBase.m_Compressor.CompressedData;

            if (compressedLen > maxPayloadSize)
            {
                // There is going to be an extra message after this, so set the COMP_FRAGMENT flag
                flags |= (byte)RipcFlags.COMP_FRAGMENT;
                if (firstFragment)
                {
                    RipcDataMessage.PopulateFirstFragment(bigBuffer, fragment, flags, optFlags, totalLength, compressedBytes, 0, maxPayloadSize);
                }
                else
                {
                    RipcDataMessage.PopulateNextFragment(bigBuffer, fragment, flags, optFlags, compressedBytes, 0, maxPayloadSize);
                }

                extraBytes = compressedLen - maxPayloadSize;
            }
            else
            {
                if (firstFragment)
                {
                    RipcDataMessage.PopulateFirstFragment(bigBuffer, fragment, flags, optFlags, totalLength, compressedBytes, 0, compressedLen);
                }
                else
                {
                    RipcDataMessage.PopulateNextFragment(bigBuffer, fragment, flags, optFlags, compressedBytes, 0, compressedLen);
                }
            }

            // add to the priority queues
            m_ChannelBase.WriteFragment(fragment, writeArgs);

            // If there are extra bytes that could not fit in the fragment, write the remainder of the compressed bytes into an extra message.
            // Extra bytes start at position maxPayloadSize (after data sent in previous message)
            if (extraBytes > 0)
            {
                // Populate second message
                compFragmentBuffer.Data.WritePosition = compFragmentBuffer.GetDataStartPosition();
                compFragmentBuffer.Data.Limit = compFragmentBuffer.GetDataStartPosition() + extraBytes;
                compFragmentBuffer.Data.Put(compressedBytes, maxPayloadSize, extraBytes);
                compFragmentBuffer.PopulateRipcHeader((byte)RipcFlags.COMPRESSION);

                m_ChannelBase.WriteFragment(compFragmentBuffer, writeArgs);

                extraTotalLength = RipcLengths.HEADER + extraBytes; // actual length on wire
                extraHeaderLength = RipcLengths.HEADER; // overhead (header) from sending extra part
            }
            else
            {
                m_ChannelBase.ReleaseBufferInternal(compFragmentBuffer);
            }

            // Actual bytes on wire is total length of first fragment, plus total length on wire of extra bytes (if sent)
            writeArgs.BytesWritten = writeArgs.BytesWritten + fragment.TotalLength + extraTotalLength;

            // Uncompressed bytes is the number of bytes taken from the big buffer before
            // compression, plus overhead for the one (or two) messages sent on wire
            writeArgs.UncompressedBytesWritten = writeArgs.UncompressedBytesWritten +
                                                     userBytesForFragment + headerLength + extraHeaderLength;

            // Adjust big buffer for next call
            // -- set the limit to end of big buffer user data
            bigBuffer.Data.WritePosition = limit;
            // -- new position will be set just after the data inserted in this
            // fragment
            bigBuffer.Data.ReadPosition = bigBuffer.Data.ReadPosition + userBytesForFragment;

            // Tell the caller how many payload bytes were put in this fragment (uncompressed)
            return userBytesForFragment;
        }

        public bool CheckCompressionFragmentedMsg(int messageSize)
        {
            return (messageSize < m_ChannelBase.m_SessionCompLowThreshold);
        }
    }
}
