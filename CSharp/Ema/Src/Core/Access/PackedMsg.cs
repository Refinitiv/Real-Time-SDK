/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.ValueAdd.Reactor;
using System.Text;
using LSEG.Eta.Codec;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// PackedMsg contains a list of messaged packed into a transport buffer to be sent across the wire together.
    /// </summary>
    public sealed class PackedMsg
    {
        const int MSG_PACKING_BUFFER_SIZE = 6000;

        /// <summary>
        /// Creates a PackedMsg with an instance of <see cref="OmmProvider"/>
        /// </summary>
        /// <param name="provider">OmmProvider instance</param>
        public PackedMsg(OmmProvider provider)
        {
            m_ProviderImpl = provider.m_OmmProviderImpl;
            m_EncodeIterator = new EncodeIterator();
        }

        /// <summary>
        /// For Non-Interactive Provider applications, initialize a new packed buffer.
        /// Also sets the maximum size of the new packed messages write buffer.
        /// </summary>
        /// <param name="maxSize">maximum size of the packed message buffer. Defaults to 6000 bytes.</param>
        /// <returns>Reference to current <see cref="PackedMsg"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">
        /// if the user tries to call this method with Interactive Provider or the connection is not established or
        /// it is not possible to allocate buffer for packed messages.
        /// </exception>
        public PackedMsg InitBuffer(int maxSize = MSG_PACKING_BUFFER_SIZE)
        {
            Clear();

            if(m_ProviderImpl.ProviderRole == OmmProviderConfig.ProviderRoleEnum.INTERACTIVE)
            {
                throw new OmmInvalidUsageException("This method is used for Non-Interactive provider only." +
                    " Setting a client handle is required when using Interactive Provider.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            m_OmmNiProviderImpl = (OmmNiProviderImpl)m_ProviderImpl;
            ReactorChannel = m_OmmNiProviderImpl.LoginCallbackClient!.ActiveChannelInfo()?.ReactorChannel;

            if(ReactorChannel == null || ReactorChannel.Channel == null)
            {
                throw new OmmInvalidUsageException("InitBuffer() failed because connection is not established.", OmmInvalidUsageException.ErrorCodes.NO_ACTIVE_CHANNEL);
            }

            if ((PackedBuffer = ReactorChannel.GetBuffer(maxSize, true, out var errorInfo)) == null)
            {
                StringBuilder strBuilder = new(512);
                strBuilder.AppendLine("Failed to get packed buffer in InitBuffer().")
                    .AppendLine($"Channel { ReactorChannel.Channel}").AppendLine($"Error Id: {errorInfo?.Error.ErrorId}")
                    .AppendLine($"Internal SysError: {errorInfo?.Error.SysError}")
                    .AppendLine($"Error Text: {errorInfo?.Error.Text}");

                throw new OmmInvalidUsageException(strBuilder.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }


            m_RemainingSize = maxSize;

            return this;
        }

        /// <summary>
        /// For Interactive Provider applications, initialize a new write buffer and sets the client handle for this
        /// PackedMsg to submit messages to. Also sets the maximum size of the new packed messages write buffer.
        /// </summary>
        /// <param name="clientHandle">unique client identifier associated by EMA with a connected client</param>
        /// <param name="maxSize">maximum size of the packed message buffer. Defaults to 6000 bytes.</param>
        /// <returns>Reference to current <see cref="PackedMsg"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">
        /// if the user tries to call this method with Interactive Provider or the connection is not established or
        /// it is not possible to allocate buffer for packed messages.
        /// </exception>
        /// <remarks>The handle is required to be set before adding messages or submitting the packedMsg.</remarks>
        public PackedMsg InitBuffer(long clientHandle, int maxSize = MSG_PACKING_BUFFER_SIZE)
        {
            Clear();

            ClientHandle = clientHandle;

            if (m_ProviderImpl.ProviderRole == OmmProviderConfig.ProviderRoleEnum.INTERACTIVE)
            {
                m_OmmIProviderImpl = (OmmIProviderImpl)m_ProviderImpl;
                ClientSession? clientSession = m_OmmIProviderImpl.ServerChannelHandler.GetClientSession(clientHandle);

                if (clientSession == null)
                {
                    throw new OmmInvalidUsageException("Client handle is not valid.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }

                ReactorChannel = clientSession.Channel();
            }
            else
            {
                throw new OmmInvalidUsageException("This method is used for Interactive provider only." +
                    " Setting a client handle is not required when using Non-Interactive Provider.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            if (ReactorChannel == null || ReactorChannel.Channel == null)
            {
                throw new OmmInvalidUsageException("InitBuffer() failed because connection is not established.", OmmInvalidUsageException.ErrorCodes.NO_ACTIVE_CHANNEL);
            }

            if ((PackedBuffer = ReactorChannel.GetBuffer(maxSize, true, out var errorInfo)) == null)
            {
                StringBuilder strBuilder = new(512);
                strBuilder.AppendLine("Failed to get packed buffer in InitBuffer().")
                    .AppendLine($"Channel { ReactorChannel.Channel}").AppendLine($"Error Id: {errorInfo?.Error.ErrorId}")
                    .AppendLine($"Internal SysError: {errorInfo?.Error.SysError}")
                    .AppendLine($"Error Text: {errorInfo?.Error.Text}");

                throw new OmmInvalidUsageException(strBuilder.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            m_RemainingSize = maxSize;

            return this;
        }

        /// <summary>
        /// Adds a Msg to the packed message buffer if there is enough space in the buffer to add the Msg.
        /// </summary>
        /// <param name="msg">message to add to this packed message.</param>
        /// <param name="itemHandle">item handle which is used for adding this msg</param>
        /// <returns>Reference to current <see cref="PackedMsg"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">
        /// if the connection is not established or lost or the message is empty of it fails to encode the message or
        /// is it not possible to add the message to the packed buffer.
        /// </exception>
        public PackedMsg AddMsg(Msg msg, long itemHandle)
        {
            if (ReactorChannel == null || ReactorChannel.Channel == null)
            {
                throw new OmmInvalidUsageException("AddMsg() failed because connection is not established.", OmmInvalidUsageException.ErrorCodes.NO_ACTIVE_CHANNEL);
            }
            else if(PackedBuffer == null)
            {
                throw new OmmInvalidUsageException("AddMsg() failed to add Msg with non init transport buffer.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else
            {
                bool niProvHandleAdded = false;
                int streamId = 0;
                Eta.Codec.IMsg? etaMsg = null;

                msg.EncodeComplete();

                if(msg.Encoder is null)
                {
                    ReleaseBuffer();

                    throw new OmmInvalidUsageException("Incoming message to pack is empty.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }

                if(m_OmmIProviderImpl != null)
                {
                    ItemInfo? itemInfo = m_OmmIProviderImpl.GetItemInfo(itemHandle);

                    if(itemInfo == null)
                    {
                        ReleaseBuffer();

                        throw new OmmInvalidUsageException("Incorrect item handle for incoming message.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                    }

                    etaMsg = msg.m_msgEncoder.m_rsslMsg;
                    etaMsg.StreamId = itemInfo.StreamId;
                    
                    if(msg.HasServiceId)
                    {
                        if (m_OmmIProviderImpl.GetDirectoryServiceStore().GetServiceNameById(msg.ServiceId(), out _) == false)
                        {
                            var errorMessage = new StringBuilder(512).Append("Attempt to add ")
                               .Append(DataType.AsString(Utilities.ToEmaMsgClass[etaMsg.MsgClass]))
                               .Append(" with service Id of ").Append(msg.ServiceId())
                               .Append(" that was not included in the SourceDirectory. Dropping this ")
                               .Append($"{DataType.AsString(Utilities.ToEmaMsgClass[etaMsg.MsgClass])}.").ToString();

                            throw new OmmInvalidUsageException(errorMessage, OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        }
                    }
                    else if (msg.HasServiceName)
                    {
                        if (m_OmmIProviderImpl.GetDirectoryServiceStore().GetServiceIdByName(msg.ServiceName(), out int serviceId))
                        {
                            if (etaMsg.MsgKey != null)
                            {
                                etaMsg.MsgKey.Flags |= MsgKeyFlags.HAS_SERVICE_ID;
                                etaMsg.MsgKey.ServiceId = serviceId;
                            }
                        }
                        else
                        {
                            var errorMessage = new StringBuilder(512).Append("Attempt to add ")
                                .Append(DataType.AsString(Utilities.ToEmaMsgClass[etaMsg.MsgClass]))
                                .Append(" with service name of ").Append(msg.ServiceName())
                                .Append(" that was not included in the SourceDirectory. Dropping this ")
                                .Append($"{DataType.AsString(Utilities.ToEmaMsgClass[etaMsg.MsgClass])}.").ToString();

                            throw new OmmInvalidUsageException(errorMessage, OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        }
                    }
                }
                else if(m_OmmNiProviderImpl != null)
                {
                    m_OmmNiProviderImpl.UserLock.Enter();

                    try
                    {
                        StreamInfo? streamInfo = m_OmmNiProviderImpl.GetStreamInfo(itemHandle);
                        etaMsg = msg.m_msgEncoder.m_rsslMsg;

                        if (msg.HasServiceId)
                        {
                            if (m_OmmNiProviderImpl.GetDirectoryServiceStore().GetServiceNameById(msg.ServiceId(), out _) == false)
                            {
                                var errorMessage = new StringBuilder(512).Append("Attempt to add ")
                                   .Append(DataType.AsString(Utilities.ToEmaMsgClass[etaMsg.MsgClass]))
                                   .Append(" with service Id of ").Append(msg.ServiceId())
                                   .Append(" that was not included in the SourceDirectory. Dropping this ")
                                   .Append($"{DataType.AsString(Utilities.ToEmaMsgClass[etaMsg.MsgClass])}.").ToString();

                                throw new OmmInvalidUsageException(errorMessage, OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                            }
                        }
                        else if (msg.HasServiceName)
                        {
                            if(m_OmmNiProviderImpl.GetDirectoryServiceStore().GetServiceIdByName(msg.ServiceName(), out int serviceId))
                            {
                                if(etaMsg.MsgKey != null)
                                {
                                    etaMsg.MsgKey.Flags |= MsgKeyFlags.HAS_SERVICE_ID;
                                    etaMsg.MsgKey.ServiceId = serviceId;
                                }
                            }
                            else
                            {
                                var errorMessage = new StringBuilder(512).Append("Attempt to add ")
                                .Append(DataType.AsString(Utilities.ToEmaMsgClass[etaMsg.MsgClass]))
                                .Append(" with service name of ").Append(msg.ServiceName())
                                .Append(" that was not included in the SourceDirectory. Dropping this ")
                                .Append($"{DataType.AsString(Utilities.ToEmaMsgClass[etaMsg.MsgClass])}.").ToString();

                                throw new OmmInvalidUsageException(errorMessage, OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                            }
                        }

                        if (streamInfo != null)
                        {
                            etaMsg.StreamId = streamInfo.StreamId;
                        }
                        else
                        {
                            streamId = m_OmmNiProviderImpl.NextProviderStreamId();
                            streamInfo = new StreamInfo(StreamInfo.StreamTypeEnum.PROVIDING, streamId);
                            streamInfo.Handle = itemHandle;
                            m_OmmNiProviderImpl.AddStreamInfo(streamInfo);
                            etaMsg.StreamId = streamInfo.StreamId;
                            niProvHandleAdded = true;
                        }
                    }
                    finally
                    {
                        m_OmmNiProviderImpl.UserLock.Exit();
                    }
                }

                m_EncodeIterator.Clear();

                CodecReturnCode ret;

                if((ret = m_EncodeIterator.SetBufferAndRWFVersion(PackedBuffer, ReactorChannel.MajorVersion, ReactorChannel.MinorVersion)) 
                    < CodecReturnCode.SUCCESS)
                {
                    ReleaseBuffer();

                    if (niProvHandleAdded)
                    {
                        m_OmmNiProviderImpl!.UserLock.Enter();
                        m_OmmNiProviderImpl.RemoveStreamInfo(itemHandle);
                        m_OmmNiProviderImpl.ReturnProviderStreamId(streamId);
                        m_OmmNiProviderImpl.UserLock.Exit();
                    }

                    throw new OmmInvalidUsageException($"Failed EncodeIterator.SetBufferAndRWFVersion() with error: {ret.GetAsInfo()}.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }

                if((ret = etaMsg!.Encode(m_EncodeIterator)) < CodecReturnCode.SUCCESS)
                {
                    if (ret != CodecReturnCode.BUFFER_TOO_SMALL)
                    {
                        ReleaseBuffer();
                    }

                    if (niProvHandleAdded)
                    {
                        m_OmmNiProviderImpl!.UserLock.Enter();
                        m_OmmNiProviderImpl.RemoveStreamInfo(itemHandle);
                        m_OmmNiProviderImpl.ReturnProviderStreamId(streamId);
                        m_OmmNiProviderImpl.UserLock.Exit();
                    }

                    throw new OmmInvalidUsageException($"Failed IMsg.Encode() with error: {ret.GetAsInfo()}", 
                        ret == CodecReturnCode.BUFFER_TOO_SMALL ? 
                        OmmInvalidUsageException.ErrorCodes.BUFFER_TOO_SMALL:
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }

                ReactorReturnCode retCode = ReactorChannel.PackBuffer(PackedBuffer!, out var errorInfo);
                
                if(retCode < ReactorReturnCode.SUCCESS)
                {
                    if(niProvHandleAdded)
                    {
                        m_OmmNiProviderImpl!.UserLock.Enter();
                        m_OmmNiProviderImpl.RemoveStreamInfo(itemHandle);
                        m_OmmNiProviderImpl.ReturnProviderStreamId(streamId);
                        m_OmmNiProviderImpl.UserLock.Exit();
                    }

                    ReleaseBuffer();

                    StringBuilder strBuilder = new(2048);
                    int errorCode;
                    switch(retCode)
                    {
                        case ReactorReturnCode.SHUTDOWN:
                            {
                                errorCode = OmmInvalidUsageException.ErrorCodes.NO_ACTIVE_CHANNEL;
                                break;
                            }
                        default:
                            {
                                errorCode = OmmInvalidUsageException.ErrorCodes.FAILURE;
                                break;
                            }
                    }

                    strBuilder.Append("Failed to pack buffer during AddMsg().")
                        .AppendLine($"Msg: {msg.ToString()}")
                        .AppendLine($"Channel {ReactorChannel.Channel}").AppendLine($"Error Id: {errorInfo?.Error.ErrorId}")
                        .AppendLine($"Internal SysError: {errorInfo?.Error.SysError}")
                        .AppendLine($"Error Text: {errorInfo?.Error.Text}");

                    throw new OmmInvalidUsageException(strBuilder.ToString(), errorCode);
                }

                m_RemainingSize = (int)retCode; // Set the number of bytes left in the packed buffer
                m_PackedMsgCount++;
            }

            return this;
        }

        /// <summary>
        /// Clears the entries in the PackedMessage.
        /// </summary>
        /// <returns>Reference to current <see cref="PackedMsg"/> object.</returns>
        public PackedMsg Clear()
        {
            ReleaseBuffer();
            m_RemainingSize = 0;
            m_PackedMsgCount = 0;
            ClientHandle = 0;
            ReactorChannel = null;
            return this;
        }

        /// <summary>
        /// Returns the remaining size in the buffer available for message packing.
        /// </summary>
        /// <returns>remaining size of packed buffer.</returns>
        public int RemainingSize()
        {
            return m_RemainingSize;
        }

        /// <summary>
        /// Returns the amount of currently packed messages in this PackedMsg object.
        /// </summary>
        /// <returns>number of packed messages.</returns>
        public int PackedMsgCount()
        {
            return m_PackedMsgCount;
        }

        void ReleaseBuffer()
        {
            if(PackedBuffer != null && ReactorChannel != null)
            {
                ReactorReturnCode retCode = ReactorChannel.ReleaseBuffer(PackedBuffer, out var reactorErrorInfo);

                if (retCode < ReactorReturnCode.SUCCESS)
                {
                    throw new OmmInvalidUsageException($"Failed to release packed buffer in PackedMsg. " +
                        $"Error:{reactorErrorInfo?.Error.Text}", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }
            }

            PackedBuffer = null;
        }

        #region internal member variables
        IOmmProviderImpl m_ProviderImpl;
        OmmIProviderImpl? m_OmmIProviderImpl;
        OmmNiProviderImpl? m_OmmNiProviderImpl;
        internal long ClientHandle { get; private set; }
        internal ITransportBuffer? PackedBuffer { get; set; }
        internal ReactorChannel? ReactorChannel { get; private set; }
        int m_RemainingSize;
        int m_PackedMsgCount;
        EncodeIterator m_EncodeIterator;
        #endregion
    }
}
