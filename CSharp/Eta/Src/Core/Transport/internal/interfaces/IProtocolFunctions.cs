/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;
using LSEG.Eta.Common;

namespace LSEG.Eta.Internal.Interfaces
{
    internal interface IProtocolFunctions
    {
        /* This function doesn't include the packed header length if any. */
        int EstimateHeaderLength();

        /* This method includes header and message length */
        int MessageLength();

        /* Gets a large buffer for handling message fragmentation. */
        ITransportBuffer GetBigBuffer(int size, out Error error);

        /* Checks whether to write this buffer as a fragmented message. */
        bool WriteAsFragmentedMessage(TransportBuffer buffer);

        /* Writes transport header into the buffer */
        int PrependTransportHdr(TransportBuffer buffer, short ripcFlags, bool addHeaderLength = true);

        /* Gets remaining bytes in the big buffer after pausing */
        int RemaingBytesAfterPausing(TransportBuffer bigBuffer);

        /* Populates fragmented messages without compression */
        int PopulateFragment(TransportBuffer bigBuffer, bool firstFragment, TransportBuffer buffer, byte flags, WriteArgs writeArgs);

        /* Populates fragmented messages with compression */
        int WriteFragmentCompressed(TransportBuffer bigBuffer, TransportBuffer buffer, WriteArgs writeArgs, bool firstFragment, out Error error);

        /* Checks whether to compare the specified message size with the low compression threshold */
        bool CheckCompressionFragmentedMsg(int messageSize);

        /* Set an instance of ReadBufferStateMachine for parsing messages */
        void SetReadBufferStateMachine(ReadBufferStateMachine readBufferStateMachine);

        /* Unset the current message length */
        void UnsetMessageLength();

        /* Returns true if the protocol function is used for RWF protocol */
        bool IsRWFProtocol();

        /* This is used to get addtional header length for a transport type*/
        int AdditionalHeaderLength();

        /* Returns the entire header length of a message */
        int EntireHeaderLength();

        /* Checks whether the current message is a ping message*/
        bool IsPingMessage();

        /* Packs current buffer */
        int PackBuffer(TransportBuffer packedBuffer, bool reserveNextPackedHdr, IChannel chnl, out Error error);

        /* This is used to prepend the websocket header. */
        int PrependInitChnlHdr(ByteBuffer sourceData, ByteBuffer destinationData);

        int InitChnlReadFromChannel(ByteBuffer sourceData, out Error error);

        int InitChnlReadFromChannelProvider(ByteBuffer dst, out Error error);

        /* Checks whether the compression is needed for sending the message. */
        bool CompressedData(ChannelBase channelBase, int messageLength, WriteArgs writeArgs)
        {
            if(channelBase.m_SessionOutCompression > 0 && (writeArgs.Flags & WriteFlags.DO_NOT_COMPRESS) == 0)
            {
                // only compress if within low threshold
                if(messageLength >= channelBase.m_SessionCompLowThreshold)
                {
                    // set m_CompressPriority if necessary
                    if (channelBase.m_CompressPriority == 99)
                    {
                        channelBase.m_CompressPriority = (int)writeArgs.Priority;
                    }

                    // only compress with initial message priority
                    if (writeArgs.Priority == (WritePriorities)channelBase.m_CompressPriority)
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        /* Compress the buffer and write according to the connection type. */
        int WriteCompressed(TransportBuffer buffer, WriteArgs writeArgs, out Error error);
    }
}
