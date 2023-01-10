/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Net.Sockets;
using System.Diagnostics;
using LSEG.Eta.Internal.Interfaces;
using LSEG.Eta.Transports.Internal;
using LSEG.Eta.Common;
using System.Collections.Generic;

namespace LSEG.Eta.Transports
{
    /// <summary>
    /// This class is used to parse RIPC messages from the read buffer and maintains the state of the buffer.
    /// </summary>
    internal class ReadBufferStateMachine : ResultObject
    {
        public enum BufferState
        {
            /// <summary>
            /// The buffer contains no data.
            /// </summary>
            NO_DATA,

            /// <summary>
            /// The length of the next message is unknown.
            /// </summary>
            UNKNOWN_INCOMPLETE,

            /// <summary>
            /// The length of the next message is unknown and there is insufficient space in the buffer
            /// to read the messge length.
            /// </summary>
            UNKNOWN_INSUFFICIENT,

            /// <summary>
            /// The length of the next message is known, but the message is incomplete.
            /// </summary>
            KNOWN_INCOMPLETE,

            /// <summary>
            /// The length of the next message is known, but there is insufficient space to read it completely.
            /// </summary>
            KNOWN_INSUFFICENT,

            /// <summary>
            /// A complete message is ready to read
            /// </summary>
            KNOWN_COMPLETE,

            /// <summary>
            /// The reader reached the end of stream
            /// </summary>
            END_OF_STREAM
        }

        public enum BufferSubState
        {
            /// <summary>
            /// No special processing for a single message
            /// </summary>
            NORMAL,

            /// <summary>
            /// It is processing a packed message
            /// </summary>
            PROCESSING_PACKED_MESSAGE,

            /// <summary>
            /// It is processing a complete fragmented message 
            /// </summary>
            PROCESSING_COMPLETE_FRAGMENTED_MESSAGE,

            /// <summary>
            /// It is processing a compressed message
            /// </summary>
            PROCESSING_COMPRESSED_MESSAGE,

            /// <summary>
            /// It is processing a fragmented compressed message
            /// </summary>
            PROCESSING_FRAGMENTED_COMPRESSED_MESSAGE,

            /// <summary>
            /// It is processing a packed compressed message
            /// </summary>
            PROCESSING_PACKED_COMPRESSED_MESSAGE,

            /// <summary>
            /// It is processing a fragmented message
            /// </summary>
            PROCESSING_FRAGMENTED_MESSAGE,
        }

        internal static class ReadReturnCodes
        {
            // No data was read from the network
            public const int NO_DATA_READ = 0;

            // The last invocation of Receive indicated we reached the end of the data stream 
            public const int END_OF_STREAM = -1;
        }

        public const int UNKNOWN_LENGTH = -1;

        public BufferState State { get; internal set; }

        public BufferSubState SubState { get; protected set; }

        public int CurrentMsgStartPos { get; set; }

        internal int CurrentMsgRipcLen { get; set; }

        internal RipcFlags CurrentMsgRipcFlags { get; set; }

        private IProtocolFunctions m_ProtocolFunctions;

        private Dictionary<int, ByteBuffer> m_FragmentedMessagesDict = new Dictionary<int, ByteBuffer>(16);

        /* The fragment ID of the last reassembled fragmented message returned to the user. */
        private int m_LastReassembledFragmentId;

        /* The length of the current data. */
        private int m_DataLength;

        /* The position of the current data. */
        private int m_DataPosition;

        private RipcVersions m_RipcVersion;

        private int m_FlagIdLength;

        private int m_FragmentationHeaderLength = 0;

        /* For message decompression. */
        protected ByteBuffer m_DecompressBuffer;

        /* Compressed fragment assembly buffer. */
        private ByteBuffer m_CompressedFragmentAssemblyBuffer;

        /* A compressed fragment is defined by two ripc messages:
        * (1) The first part is indicated by RipcFlag CompFragment (0x8)
        * (2) The final part is indicated by RipcFlag CompressedData (0x4)
        * The compressed fragment can only be decompressed when the two parts have been re-assembled.
        * Since other (non-compressed) messages can be received between parts 1 and 2,
        * this flag will be used to track when the reader is waiting for the second part.
        * Sub-state cannot be used for this at this time since can be changed on a per-message basis.
        */
        private bool m_CompressedFragmentWaitingForSecondMsg;

        /* When processing a compressed fragment sequence within a fragmented message,
         * need to track the fragment Id so that the fragment re-assembly buffer can be found
         * when handling the second part of the sequence:
         * a normal compressed message which does not contain the fragment Id.
         */
        private int m_CompressedFragmentFragId = 0;

        internal int LastReadPosition { get; private set; }

        /* This reference will point to the buffer that contains the "current" data to be read by the application. */
        protected ByteBuffer m_DataBuffer;

        protected ChannelBase m_Channel;

        public ReadBufferStateMachine(ChannelBase channel,IProtocolFunctions protocolFunctions, RipcVersions ripcVersion, Socket socket, int packet_size, Object user_state) 
            : base(socket, packet_size, user_state)
        {
            State = BufferState.NO_DATA;
            SubState = BufferSubState.NORMAL;
            CurrentMsgStartPos = 0;
            CurrentMsgRipcLen = UNKNOWN_LENGTH;
            m_ProtocolFunctions = protocolFunctions;
            m_ProtocolFunctions.SetReadBufferStateMachine(this);
            m_Channel = channel;
            RipcVersion(ripcVersion);

            m_DataBuffer = new ByteBuffer(null, true);
        }

        internal void DisposeInternalBuffers()
        {
            if (m_DataBuffer != null) 
                m_DataBuffer.Dispose();
            if (m_CompressedFragmentAssemblyBuffer != null)
                m_CompressedFragmentAssemblyBuffer.Dispose();   
            if (m_DecompressBuffer != null)
                m_DecompressBuffer.Dispose();
            if (Buffer != null)
                Buffer.Dispose();
            foreach (var frag in m_FragmentedMessagesDict.Values) 
                frag.Dispose();
        }

        private void RipcVersion(RipcVersions ripcVersion)
        {
            m_RipcVersion = ripcVersion;

            if (m_RipcVersion >= RipcVersions.VERSION13)
            {
                m_FlagIdLength = RipcOffsets.FRAGMENTED_ID_LENGTH_RIPC13;
            }
            else
            {
                m_FlagIdLength = RipcOffsets.FRAGMENTED_ID_LENGTH_RIPC12;
            }
        }

        /// <summary>
        /// Advances the state machine to the next state.
        /// Invoke this method after the user has processed a message.
        /// </summary>
        /// <param name="readArgs">The read argument</param>
        /// <param name="error">The error if any</param>
        /// <returns>The current state</returns>
        public BufferState AdvanceOnApplicationRead(ReadArgs readArgs, out Error error)
        {
            error = null;

            Debug.Assert(State == BufferState.KNOWN_COMPLETE);

            int calculatedEndPos = CurrentMsgStartPos + m_ProtocolFunctions.MessageLength();

            Debug.Assert(calculatedEndPos <= Buffer.WritePosition);

            switch(SubState)
            {
                case BufferSubState.NORMAL:
                case BufferSubState.PROCESSING_FRAGMENTED_MESSAGE:
                case BufferSubState.PROCESSING_FRAGMENTED_COMPRESSED_MESSAGE:
                    break;
                case BufferSubState.PROCESSING_PACKED_MESSAGE:
                    if (m_DataPosition != calculatedEndPos)
                    {
                        AdvanceToNextPackedMessage(calculatedEndPos, readArgs);
                        if (m_DataPosition != calculatedEndPos)
                        {
                            return State; // we still have more data to process
                        }
                    }
                    break;
                case BufferSubState.PROCESSING_PACKED_COMPRESSED_MESSAGE:

                    if (m_DataPosition != m_DecompressBuffer.Limit)
                    {
                        AdvanceToNextPackedMessage(m_DecompressBuffer.Limit, readArgs);
                        if (m_DataPosition != m_DecompressBuffer.Limit)
                        {
                            return State; // we still have more data to process
                        }
                    }

                    break;
                case BufferSubState.PROCESSING_COMPLETE_FRAGMENTED_MESSAGE:

                    var frag = m_FragmentedMessagesDict[m_LastReassembledFragmentId];
                    if (frag != null) frag.Dispose();
                    if ( m_FragmentedMessagesDict.Remove(m_LastReassembledFragmentId) )
                    {
                        m_LastReassembledFragmentId = 0;
                    }

                    break;
                default:
                    break; // should not be in this state
            }

            // Checks whether we reached the end of the data in the buffer
            if(calculatedEndPos == Buffer.WritePosition)
            {
                CurrentMsgStartPos = 0;
                m_ProtocolFunctions.UnsetMessageLength();
                State = BufferState.NO_DATA;
            }
            else
            {
                CurrentMsgStartPos += m_ProtocolFunctions.MessageLength();
                m_ProtocolFunctions.UnsetMessageLength();
                UpdateStateCurrentLenUnknown(readArgs, out error);
            }

            return State;
        }

        /// <summary>
        /// Advances the data position to the next packed message (or to the end of the entire RIPC message)
        /// ASSUMPTION: the current state is KNOWN_COMPLETE
        /// ASSUMPTION: the current sub-state is PROCESSING_PACKED_MESSAGE or PROCESSING_PACKED_COMPRESSED_MESSAGE
        /// ASSUMPTION: we are not already at the end of the RIPC message
        /// </summary>
        /// <param name="calculatedEndPos">calculatedEndPos is the calculated end of the entire RIPC message</param>
        /// <param name="readArgs"><see cref="ReadArgs"/> instance</param>
        protected void AdvanceToNextPackedMessage(int calculatedEndPos, ReadArgs readArgs)
        {
            Debug.Assert((State == BufferState.KNOWN_COMPLETE)
                    && (SubState == BufferSubState.PROCESSING_PACKED_MESSAGE || SubState == BufferSubState.PROCESSING_PACKED_COMPRESSED_MESSAGE)
                    && (m_DataPosition != calculatedEndPos));

            // advance to the next packed message (or the end of the entire RIPC message)
            m_DataPosition += m_DataLength;

            // advance if we have not reached the end of the entire (RIPC) message
            if (m_DataPosition != calculatedEndPos)
            {

                if (SubState == BufferSubState.PROCESSING_PACKED_MESSAGE)
                {
                    m_DataLength = Buffer.ReadUShortAt(m_DataPosition);
                }
                else if (SubState == BufferSubState.PROCESSING_PACKED_COMPRESSED_MESSAGE)
                {
                    m_DataLength = m_DecompressBuffer.ReadUShortAt(m_DataPosition);
                }

                if (m_DataLength > 0)
                {
                    m_DataPosition += RipcOffsets.PACKED_MSG_DATA;
                }
                else
                {
                   m_DataPosition = calculatedEndPos;
                }
            }
        }

        private void UpdateStateCurrentLenUnknown(ReadArgs readArgs, out Error error)
        {
            error = null;

            Debug.Assert(CurrentMsgRipcLen == UNKNOWN_LENGTH);

            if (m_ProtocolFunctions.IsRWFProtocol())
            {
                int messageFlagStartPos = CurrentMsgStartPos + m_ProtocolFunctions.AdditionalHeaderLength() + RipcOffsets.MSG_FLAG;

                if (messageFlagStartPos < Buffer.WritePosition)
                {
                    CurrentMsgRipcLen = Buffer.ReadUShortAt(CurrentMsgStartPos + m_ProtocolFunctions.AdditionalHeaderLength());
                    CurrentMsgRipcFlags = (RipcFlags)Buffer.ReadByteAt(messageFlagStartPos);

                    UpdateStateCurrentLenKnown(readArgs, out error);
                }
                else if (messageFlagStartPos < Buffer.Limit)
                {
                    State = BufferState.UNKNOWN_INCOMPLETE;
                }
                else
                {
                    State = BufferState.UNKNOWN_INSUFFICIENT;
                }
            }
        }

        /// <summary>
        /// Transitions from the current state to the next one after the SocketChannel.Receive() call
        /// </summary>
        /// <param name="readReturnCode">This is return code from SocketChannel.Receive()</param>
        /// <param name="readArgs">The read argument</param>
        /// <param name="error">The error argument</param>
        /// <returns>The new buffer state</returns>
        public BufferState AdvanceOnSocketChannelRead(int readReturnCode, ReadArgs readArgs, out Error error)
        {
            error = null;
            bool hasAddtionalData = false;

            if (readReturnCode == ReadReturnCodes.NO_DATA_READ)
            {
                // Checks whether there is additional data from the last read into the buffer.
                if (LastReadPosition < Buffer.WritePosition)
                {
                    hasAddtionalData = true;
                }
            }

            LastReadPosition = Buffer.WritePosition;
           
            if(readReturnCode != ReadReturnCodes.END_OF_STREAM)
            {
                readArgs.BytesRead = readReturnCode;

                switch(State)
                {
                    case BufferState.KNOWN_INCOMPLETE:
                        if(readReturnCode != ReadReturnCodes.NO_DATA_READ)
                        {
                            UpdateStateCurrentLenKnown(readArgs, out error);
                        }
                        break;
                    case BufferState.UNKNOWN_INCOMPLETE:
                    case BufferState.NO_DATA:
                        if(readReturnCode != ReadReturnCodes.NO_DATA_READ || hasAddtionalData)
                        {
                            UpdateStateCurrentLenUnknown(readArgs, out error);
                        }
                        break;
                    default:
                        Debug.Assert(false); // This should not happend
                        break;
                }
            }
            else
            {
                State = BufferState.END_OF_STREAM;
            }

            return State;
        }

        private void UpdateStateCurrentLenKnown(ReadArgs readArgs, out Error error)
        {
            error = null;

            // Calculate the end of the RIPC message
            int calculatedEndPos = CurrentMsgStartPos + m_ProtocolFunctions.MessageLength();

            if(calculatedEndPos <= Buffer.WritePosition)
            {
                State = BufferState.KNOWN_COMPLETE;

                if(m_ProtocolFunctions.IsRWFProtocol())
                {
                    if(CurrentMsgRipcLen != RipcDataMessage.HeaderSize)
                    {
                        // this is NOT a ping message
                        if( (CurrentMsgRipcFlags & RipcFlags.PACKING) > 0)
                        {
                            if ((CurrentMsgRipcFlags & RipcFlags.COMPRESSION) == 0)
                            {
                                readArgs.UncompressedBytesRead = CurrentMsgRipcLen;
                                SubState = BufferSubState.PROCESSING_PACKED_MESSAGE;
                            }
                            else
                            {
                                SubState = BufferSubState.PROCESSING_PACKED_COMPRESSED_MESSAGE;
                            }
                            ProcessPackedMessage(calculatedEndPos, readArgs);
                        }
                        else if ( (CurrentMsgRipcFlags & RipcFlags.HAS_OPTIONAL_FLAGS) > 0 )
                        {
                            // Only fragmented messages are using these flags.
                            if((CurrentMsgRipcFlags & RipcFlags.COMPRESSION) == 0)
                            {
                                SubState = BufferSubState.PROCESSING_FRAGMENTED_MESSAGE;
                            }
                            else
                            {
                                SubState = BufferSubState.PROCESSING_FRAGMENTED_COMPRESSED_MESSAGE;
                            }

                            ProcessExtendedMessage(calculatedEndPos, readArgs);
                        }
                        else if ( (CurrentMsgRipcFlags & RipcFlags.COMPRESSION) > 0 )
                        {
                            if ((CurrentMsgRipcFlags & RipcFlags.COMP_FRAGMENT) == 0 && !m_CompressedFragmentWaitingForSecondMsg)
                            {
                                // normal compressed message
                                ProcessCompressedMessage(calculatedEndPos);
                            }
                            else
                            {
                                if(SubState == BufferSubState.PROCESSING_FRAGMENTED_COMPRESSED_MESSAGE)
                                {
                                    // This is not a fragment, but part of the fragment group.
                                    // The second part of the CompFragment sequence that we are waiting for has arrived.
                                    ProcessExtendedMessageNonFragment(calculatedEndPos, readArgs);
                                }
                                else if (SubState == BufferSubState.PROCESSING_PACKED_COMPRESSED_MESSAGE)
                                {
                                    ProcessPackedMessage(calculatedEndPos, readArgs);
                                }
                                else
                                {
                                    // Non-fragmented handling of CompFragment sequence:
                                    // This will handle first and last message in the sequence (CompFragment x8 followed by CompressedData x4)
                                    ProcessCompFragmentSequence(calculatedEndPos, readArgs);
                                }
                            }
                        }
                        else
                        {
                            SubState = BufferSubState.NORMAL;
                            m_DataLength = m_ProtocolFunctions.MessageLength() - m_ProtocolFunctions.EntireHeaderLength();
                            m_DataPosition = CurrentMsgStartPos + m_ProtocolFunctions.EntireHeaderLength();
                            m_DataBuffer = Buffer;

                            readArgs.UncompressedBytesRead = m_ProtocolFunctions.MessageLength();
                        }
                    }
                    else
                    {
                        // This is a ping message
                        m_DataLength = 0;
                        m_DataPosition = calculatedEndPos;
                        SubState = BufferSubState.NORMAL;
                    }
                }
            }
            else if(calculatedEndPos <= Buffer.Limit)
            {
                State = BufferState.KNOWN_INCOMPLETE;
            }
            else
            {
                State = BufferState.KNOWN_INSUFFICENT;
            }
        }

        private void ProcessPackedMessage(int calculatedEndPos, ReadArgs readArgs)
        {
            // read the length of the first packed message
            if (SubState == BufferSubState.PROCESSING_PACKED_MESSAGE)
            {
                m_DataLength = Buffer.ReadUShortAt(CurrentMsgStartPos + m_ProtocolFunctions.AdditionalHeaderLength() + RipcLengths.HEADER);
                if (m_DataLength > 0)
                {
                    m_DataPosition = CurrentMsgStartPos + m_ProtocolFunctions.AdditionalHeaderLength() + RipcLengths.HEADER + RipcOffsets.PACKED_MSG_DATA;
                }
                else
                {
                    m_DataPosition = calculatedEndPos;
                }
                m_DataBuffer = Buffer;
            }
            else if (SubState == BufferSubState.PROCESSING_PACKED_COMPRESSED_MESSAGE)
            {
                if (m_DecompressBuffer == null)
                {
                    m_DecompressBuffer = new ByteBuffer(m_Channel.InternalFragmentSize);
                }

                int startOfDataPos = CurrentMsgStartPos + m_ProtocolFunctions.AdditionalHeaderLength() + RipcLengths.HEADER;
                int ripcFlags = Buffer.ReadByteAt(CurrentMsgStartPos + m_ProtocolFunctions.AdditionalHeaderLength() + RipcOffsets.MSG_FLAG);
                if ((ripcFlags & (int)RipcFlags.COMP_FRAGMENT) > 0)
                {
                    CompFragmentAssembly(calculatedEndPos, startOfDataPos);

                    // the packed message is not yet complete, update m_DataPosition with respect to m_DecompressBuffer
                    m_DataLength = 0;
                    m_DataPosition = m_DecompressBuffer.Limit; // advance to the end of the message
                    m_DataBuffer = Buffer;

                    // Since this is the first part of a split compressed message, we only know the bytesRead.
                    // Just add this part's RIPC Header to uncompressedBytesRead.
                    readArgs.UncompressedBytesRead = m_ProtocolFunctions.AdditionalHeaderLength() + RipcLengths.HEADER;
                }
                else if (m_CompressedFragmentWaitingForSecondMsg && (ripcFlags & (int)RipcFlags.COMPRESSION) > 0)
                {
                    if (CompFragmentAssembly(calculatedEndPos, startOfDataPos))
                    {
                        // Now have the re-assembled and de-compressed split packed message:
                        // data has been decompressed into _decompressBuffer.
                        // Proceeding with unpacking the message.
                        m_DataLength = m_DecompressBuffer.ReadUShortAt(0);
                        if (m_DataLength > 0)
                        {
                            m_DataPosition = RipcOffsets.PACKED_MSG_DATA;
                        }
                        else
                        {
                            m_DataPosition = m_DecompressBuffer.Limit;
                        }

                        m_DataBuffer = m_DecompressBuffer;

                        // Add the entire compressed message here to bytesRead,
                        // AdvancedToNextPackedMessage() will only update the UncompressedBytesRead,
                        // as it processes each packed message.
                        int headerLength = m_ProtocolFunctions.AdditionalHeaderLength() + RipcLengths.HEADER;
                        readArgs.UncompressedBytesRead = headerLength + m_DecompressBuffer.Position;
                    }
                }
                else
                {
                    int uncompressedLength = m_Channel.m_Compressor.Decompress(Buffer, m_DecompressBuffer, CurrentMsgStartPos
                            + m_ProtocolFunctions.AdditionalHeaderLength() + RipcLengths.HEADER, calculatedEndPos -
                            (CurrentMsgStartPos + m_ProtocolFunctions.AdditionalHeaderLength() + RipcLengths.HEADER));
                    m_DataLength = m_DecompressBuffer.ReadUShortAt(0);
                    if (m_DataLength > 0)
                    {
                        m_DataPosition = RipcOffsets.PACKED_MSG_DATA;
                    }
                    else
                    {
                        m_DataPosition = m_DecompressBuffer.Limit;
                    }

                    m_DataBuffer = m_DecompressBuffer;

                    // Add the entire compressed message here,
                    // AdvancedToNextPackedMessage() will only update the
                    // UncompressedBytesRead, as it processes each packed message.
                    readArgs.UncompressedBytesRead = m_ProtocolFunctions.AdditionalHeaderLength() + RipcLengths.HEADER + uncompressedLength;
                }
            }
        }

        /// <summary>
        /// Processes the extended header in the current RIPC message.
        /// An extended header is present in all fragmented messages.
        /// </summary>
        /// <param name="calculatedEndPos">The calculated end position of the current message in the Read buffer</param>
        /// <param name="readArgs"></param>
        private void ProcessExtendedMessage(int calculatedEndPos, ReadArgs readArgs)
        {
            Debug.Assert(State == BufferState.KNOWN_COMPLETE);

            ByteBuffer fragmentBuffer = null;

            int currentStartPos = CurrentMsgStartPos + m_ProtocolFunctions.AdditionalHeaderLength();
            RipcOpCode ripcExtendedFlags = (RipcOpCode)Buffer.ReadByteAt(currentStartPos + RipcOffsets.EXTENDED_FLAGS);
            int ripcFlags = Buffer.ReadByteAt(currentStartPos + RipcOffsets.MSG_FLAG);

            int fragmentId = 0;

            if((ripcExtendedFlags & RipcOpCode.FRAGMENT_HEADER) > 0)
            {
                long totalMessageLength = Buffer.ReadUIntAt(currentStartPos + RipcOffsets.FRAGMENTED_MSG_LENGTH);

                if(m_RipcVersion >= RipcVersions.VERSION13)
                {
                    fragmentId = Buffer.ReadUShortAt(currentStartPos + RipcOffsets.FRAGMENT_HEADER_FRAGMENT_ID);
                }
                else
                {
                    fragmentId = Buffer.ReadByteAt(currentStartPos + RipcOffsets.FRAGMENT_HEADER_FRAGMENT_ID);
                }

                fragmentBuffer = AcquireFragmentBuffer(fragmentId, (int)totalMessageLength);

                int startOfDataPos = (currentStartPos + RipcOffsets.FRAGMENT_HEADER_FRAGMENT_ID + m_FlagIdLength);
                m_FragmentationHeaderLength = RipcLengths.FIRST_FRAGMENT_WITHOUT_FRAGID + m_FlagIdLength;
                
                if(SubState == BufferSubState.PROCESSING_FRAGMENTED_MESSAGE)
                {
                    CopyMessageData(fragmentBuffer, Buffer, startOfDataPos, calculatedEndPos - startOfDataPos);

                    readArgs.UncompressedBytesRead = m_FragmentationHeaderLength + (calculatedEndPos - startOfDataPos);
                }
                else if (SubState == BufferSubState.PROCESSING_FRAGMENTED_COMPRESSED_MESSAGE)
                {
                    if ((ripcFlags & (int)RipcFlags.COMP_FRAGMENT) > 0)
                    {
                        m_CompressedFragmentFragId = fragmentId;
                        CompFragmentAssembly(calculatedEndPos, startOfDataPos);
                    }
                    else
                    {
                        if (m_DecompressBuffer == null)
                        {
                            m_DecompressBuffer = new ByteBuffer(m_Channel.InternalFragmentSize);
                        }

                        m_Channel.m_Compressor.Decompress(Buffer, m_DecompressBuffer,
                                startOfDataPos, calculatedEndPos - startOfDataPos);

                        CopyMessageData(fragmentBuffer, m_DecompressBuffer, m_DecompressBuffer.ReadPosition, m_DecompressBuffer.Limit);

                        readArgs.UncompressedBytesRead = m_FragmentationHeaderLength
                                                          + (m_DecompressBuffer.Limit - m_DecompressBuffer.ReadPosition);
                    }
                }

            }
            else if((ripcExtendedFlags & RipcOpCode.FRAGMENT) > 0)
            {
                // this message is a continuation of a fragmented message
                if(m_RipcVersion >= RipcVersions.VERSION13)
                {
                    fragmentId = Buffer.ReadUShortAt(currentStartPos + RipcOffsets.FRAGMENT_ID);
                }
                else
                {
                    fragmentId = Buffer.ReadByteAt(currentStartPos + RipcOffsets.FRAGMENT_ID);
                }

                m_FragmentedMessagesDict.TryGetValue(fragmentId, out fragmentBuffer);
                int startOfDataPos = currentStartPos + RipcOffsets.FRAGMENT_ID + m_FlagIdLength;
                m_FragmentationHeaderLength = RipcLengths.ADDITIONAL_FRAGMENT_WITHOUT_FRAGID + m_FlagIdLength;

                if (SubState == BufferSubState.PROCESSING_FRAGMENTED_MESSAGE)
                {
                    CopyMessageData(fragmentBuffer, Buffer, startOfDataPos, calculatedEndPos - startOfDataPos);

                    readArgs.UncompressedBytesRead = m_FragmentationHeaderLength + (calculatedEndPos - startOfDataPos);
                }
                else if (SubState == BufferSubState.PROCESSING_FRAGMENTED_COMPRESSED_MESSAGE)
                {
                    if ((ripcFlags & (int)RipcFlags.COMP_FRAGMENT) > 0)
                    {
                        m_CompressedFragmentFragId = fragmentId;
                        CompFragmentAssembly(calculatedEndPos, startOfDataPos);
                    }
                    else
                    {
                        if (m_DecompressBuffer == null)
                        {
                            m_DecompressBuffer = new ByteBuffer(m_Channel.InternalFragmentSize);
                        }

                        m_Channel.m_Compressor.Decompress(Buffer, m_DecompressBuffer,
                                startOfDataPos, calculatedEndPos - startOfDataPos);

                        CopyMessageData(fragmentBuffer, m_DecompressBuffer, m_DecompressBuffer.ReadPosition, m_DecompressBuffer.Limit);

                        readArgs.UncompressedBytesRead = m_FragmentationHeaderLength +
                                                          (m_DecompressBuffer.Limit - m_DecompressBuffer.ReadPosition);
                    }
                }
            }

            UpdateFragmentHandler(calculatedEndPos, fragmentBuffer, fragmentId);
        }

        /// <summary>
        /// Returns a buffer used to store the data in a fragmented message,
        /// or null if a buffer could not be acquired.
        /// </summary>
        /// <param name="fragmentId">The ID of the fragment</param>
        /// <param name="totalMessageLength">The total data length of fragmented, after all fragments are received.</param>
        /// <returns>A buffer used to store the data in a fragmented message</returns>
        private ByteBuffer AcquireFragmentBuffer(int fragmentId, int totalMessageLength)
        {
            ByteBuffer messageData = null;

            if(totalMessageLength >= 0 && totalMessageLength <= int.MaxValue)
            {
                messageData = new ByteBuffer(totalMessageLength);

                if(m_FragmentedMessagesDict.ContainsKey(fragmentId))
                {
                    // We never receive all parts of the previous message with this fragment ID
                    ByteBuffer incompleteMsg = m_FragmentedMessagesDict[fragmentId];
                    m_FragmentedMessagesDict.Remove(fragmentId);
                    incompleteMsg.Dispose();
                }

                m_FragmentedMessagesDict[fragmentId] = messageData;
            }
            else
            {
                messageData = null;
            }

            return messageData;
        }

        private void CopyMessageData(ByteBuffer destBuffer, ByteBuffer srcBuffer, int srcOffSet, int srcLength)
        {
            if (destBuffer != null)
            {
                destBuffer.Put(srcBuffer.Contents, srcOffSet, srcLength);
            }
        }

        private void UpdateFragmentHandler(int calculatedEndPos, ByteBuffer data, int fragmentId)
        {
            if(data != null)
            {
                if(data.Position == data.Limit)
                {
                    m_LastReassembledFragmentId = fragmentId; // we reassembled the entire message
                }
                else
                {
                    data = null; // still need more fragments
                }
            }

            m_DataBuffer = data;

            if(m_DataBuffer != null)
            {
                m_DataLength = data.Limit;
                m_DataPosition = 0;
                SubState = BufferSubState.PROCESSING_COMPLETE_FRAGMENTED_MESSAGE;
            }
            else
            {
                // We don't have a complete fragmented message
                m_DataLength = 0;
                m_DataPosition = calculatedEndPos;
                m_DataBuffer = Buffer;

                if(SubState != BufferSubState.PROCESSING_FRAGMENTED_COMPRESSED_MESSAGE && 
                    SubState != BufferSubState.PROCESSING_FRAGMENTED_MESSAGE)
                {
                    SubState = BufferSubState.NORMAL;
                }
            }
        }

        /// <summary>
        /// Determines whether the current buffer has remaining packed data
        /// ASSUMPTION: The current state is BufferState.KNOWN_COMPLETE
        /// </summary>
        /// <returns>true if there is remaining packed data, false otherwise</returns>
        internal Boolean HasRemainingPackedData()
        {
            Boolean retVal = false;

            Debug.Assert(State == BufferState.KNOWN_COMPLETE);

            if (SubState == BufferSubState.PROCESSING_PACKED_MESSAGE)
            {
                retVal = (m_DataPosition + m_DataLength) != (CurrentMsgStartPos + m_ProtocolFunctions.AdditionalHeaderLength() + CurrentMsgRipcLen);
            }
            else if (SubState == BufferSubState.PROCESSING_PACKED_COMPRESSED_MESSAGE)
            {
                retVal = (m_DataPosition + m_DataLength) != m_DecompressBuffer.Limit;
            }

            return retVal;
        }

        /// <summary>
        /// Advances the state machine because the read buffer has been compacted.
        /// </summary>
        /// <returns>The new buffer state as the buffer has been compacted</returns>
        internal BufferState AdvanceOnCompact()
        {
            LastReadPosition = Buffer.WritePosition;
            CurrentMsgStartPos = 0;

            switch(State)
            {
                case BufferState.KNOWN_INSUFFICENT:
                    State = BufferState.KNOWN_INCOMPLETE;
                    break;
                case BufferState.UNKNOWN_INSUFFICIENT:
                    State = BufferState.UNKNOWN_INCOMPLETE;
                    break;
                default:
                    Debug.Assert(false); // Invalid state
                    break;
            }

            return State;
        }

        /// <summary>
        /// Returns the length of the current RIPC message(length includes the RIPC header)
        /// </summary>
        /// <returns>The entire message length</returns>
        internal int CurrentMessageLength()
        {
            Debug.Assert(State == BufferState.KNOWN_COMPLETE);

            return m_ProtocolFunctions.MessageLength();
        }

        /// <summary>
        /// Returns a reference to the buffer containing the current data
        /// </summary>
        /// <returns>The buffer containing the current data</returns>
        internal ByteBuffer DataBuffer()
        {
            Debug.Assert(State == BufferState.KNOWN_COMPLETE);

            return m_DataBuffer;
        }

        /// <summary>
        /// Returns the position of the current data
        /// </summary>
        /// <returns>The position of the current data</returns>
        internal int DataPosition()
        {
            Debug.Assert(State == BufferState.KNOWN_COMPLETE);

            return m_DataPosition;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        internal int DataLength()
        {
            Debug.Assert(State == BufferState.KNOWN_COMPLETE);

            return m_DataLength;
        }

        protected void ProcessCompressedMessage(int calculatedEndPos)
        {
            SubState = BufferSubState.PROCESSING_COMPRESSED_MESSAGE;
            m_DataLength = CurrentMsgRipcLen - RipcLengths.HEADER;
            m_DataPosition = CurrentMsgStartPos + m_ProtocolFunctions.AdditionalHeaderLength() + RipcLengths.HEADER;
            m_DataBuffer = Buffer;
        }

        /* Re-assembles fragmented compression.
        * The first message will contain CompFrag flag (x8).
        * The second part will be a normal CompressedData flag (x4).
        * A fragmented message can occur with normal messages, or fragmented messages
        * (in which case the CompFrag flag will be combined with the fragmentation flag (x1)).
        *
        * Returns true when the second part of the fragmented compression is received and re-assembled with the first.
        * Returns false after processing the first part of the fragmented compression (compFrag flag).
        */
        private bool CompFragmentAssembly(int calculatedEndPos, int startOfDataPos)
        {
            if (!m_CompressedFragmentWaitingForSecondMsg)
            {
                // first compressed fragment
                m_CompressedFragmentWaitingForSecondMsg = true;

                // copy into assembly buffer and wait for next compressed fragment
                // to decompress
                if (m_CompressedFragmentAssemblyBuffer != null)
                {
                    m_CompressedFragmentAssemblyBuffer.Clear();
                }
                else
                {
                    m_CompressedFragmentAssemblyBuffer = new ByteBuffer(m_Channel.InternalFragmentSize + 100);
                }
                
                CopyMessageData(m_CompressedFragmentAssemblyBuffer, Buffer, startOfDataPos, calculatedEndPos - startOfDataPos);

                return false;
            }
            else
            {
                // last compressed fragment
                m_CompressedFragmentWaitingForSecondMsg = false;

                // copy into assembly buffer and decompress entire assembled buffer
                CopyMessageData(m_CompressedFragmentAssemblyBuffer, Buffer, startOfDataPos, calculatedEndPos - startOfDataPos);

                if (m_DecompressBuffer == null)
                {
                    m_DecompressBuffer = new ByteBuffer(m_Channel.InternalFragmentSize);
                }

                m_DataLength = m_Channel.m_Compressor.Decompress(m_CompressedFragmentAssemblyBuffer, m_DecompressBuffer, 0,
                        m_CompressedFragmentAssemblyBuffer.Position);

                // we now have a complete compressed fragmented message
                return true;
            }
        }

        /* Process compFragment message in the normal case: no fragmentation */
        private void ProcessCompFragmentSequence(int calculatedEndPos, ReadArgs readArgs)
        {
            if (!CompFragmentAssembly(calculatedEndPos, CurrentMsgStartPos + m_ProtocolFunctions.AdditionalHeaderLength() + RipcLengths.HEADER))
            {
                // waiting for second part of the fragmented compression
                m_DataBuffer = Buffer;
                m_DataLength = 0;
                m_DataPosition = calculatedEndPos; // advance to end of message

                readArgs.UncompressedBytesRead = RipcLengths.HEADER;
            }
            else
            {
                // fragmented compression is re-assembled
                m_DataBuffer = m_DecompressBuffer;
                m_DataLength = m_DataBuffer.Limit;
                m_DataPosition = 0;
                SubState = BufferSubState.NORMAL;

                readArgs.UncompressedBytesRead = RipcLengths.HEADER + m_DataLength;
            }
        }

        /* Process the final (second) part of a CompFragment sequence within a fragmented message.
        * This part will be added to the fragment assembly buffer identified by the _compressedFragmentFragId.
        */
        private void ProcessExtendedMessageNonFragment(int calculatedEndPos, ReadArgs readArgs)
        {
            ByteBuffer data;

            Debug.Assert(m_CompressedFragmentWaitingForSecondMsg == true);

            CompFragmentAssembly(calculatedEndPos, CurrentMsgStartPos + m_ProtocolFunctions.AdditionalHeaderLength() + RipcLengths.HEADER);

            data = m_FragmentedMessagesDict[m_CompressedFragmentFragId];

            // add _decompressBuffer fragment assembly
            CopyMessageData(data, m_DecompressBuffer, 0, m_DecompressBuffer.Limit);

            readArgs.UncompressedBytesRead = RipcLengths.HEADER + m_FragmentationHeaderLength + m_DataLength;

            UpdateFragmentHandler(calculatedEndPos, data, m_CompressedFragmentFragId);
        }
    }
}
