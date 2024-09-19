/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.PerfTools.Common;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

using static LSEG.Eta.Rdm.Dictionary;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.PerfTools.ProvPerf
{
    /// <summary>
    /// Provides sending of the provperf's dictionary, if requested
    /// </summary>
    public class DictionaryProvider
    {
        private const int MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE = 128000;
        private const int MAX_DICTIONARY_STATUS_MSG_SIZE = 1024;

        private const string FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
        private const string ENUM_TABLE_FILE_NAME = "enumtype.def";

        /// <summary>
        /// Field dictionary name
        /// </summary>
        public Buffer FieldDictionaryDownloadName { get; set; } = new Buffer();
        /// <summary>
        /// Enumtype dictionary name
        /// </summary>
        public Buffer EnumTypeDictionaryDownloadName { get; set; } = new Buffer();
        /// <summary>
        /// DataDictionary instance
        /// </summary>
        public DataDictionary Dictionary { get; set; }
        /// <summary>
        /// Dictionary request
        /// </summary>
        public DictionaryRequest DictionaryRequest { get; set; }

        private EncodeIterator m_EncodeIter;
        private ChannelInfo m_ChnlInfo;
        private DictionaryRefresh m_DictionaryRefresh;
        private DictionaryStatus m_DictionaryStatus;

        // Use the VA Reactor instead of the ETA Channel for sending and receiving
        private ReactorSubmitOptions m_ReactorSubmitOptions;

        // Use the VA Reactor instead of the ETA Channel for sending and receiving
        private ReactorChannelInfo m_ReactorChnlInfo;


        public DictionaryProvider()
        {
            m_EncodeIter = new EncodeIterator();
            m_ChnlInfo = new ChannelInfo();

            Dictionary = new DataDictionary();
            DictionaryRequest = new DictionaryRequest();
            m_DictionaryRefresh = new DictionaryRefresh();
            m_DictionaryStatus = new DictionaryStatus();

            FieldDictionaryDownloadName.Data("RWFFld");
            EnumTypeDictionaryDownloadName.Data("RWFEnum");

            m_ReactorSubmitOptions = new ReactorSubmitOptions();
            m_ReactorSubmitOptions.Clear();
            m_ReactorSubmitOptions.WriteArgs.Priority = WritePriorities.HIGH;
            m_ReactorChnlInfo = new ReactorChannelInfo();

        }

        /// <summary>
        /// Loads Field Dictionary and enumtype dectionary
        /// </summary>
        /// <param name="error"><see cref="Error"/> instance that contains failure details</param>
        /// <returns>true if the dictionaries were loaded successfully, false otherwise</returns>
        public bool LoadDictionary(out Error? error)
        {
            Dictionary.Clear();
            CodecError codecError;

            if (Dictionary.LoadFieldDictionary(FIELD_DICTIONARY_FILE_NAME, out codecError) < 0)
            {
                error = new Error()
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = codecError.Text
                };
                return false;
            }

            if (Dictionary.LoadEnumTypeDictionary(ENUM_TABLE_FILE_NAME, out codecError) < 0)
            {
                error = new Error()
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = codecError.Text
                };
                return false;
            }

            error = null;
            return true;
        }

        /// <summary>
        /// Processes a dictionary request
        /// </summary>
        /// <param name="channelHandler">Channel handler for the directory messages</param>
        /// <param name="clientChannelInfo">Information about client connected</param>
        /// <param name="msg">The partially decoded message</param>
        /// <param name="dIter">The decode iterator</param>
        /// <param name="error"><see cref="Error"/> instance that contains failure details</param>
        /// <returns><see cref="TransportReturnCode"/> value indicating the status of the operation</returns>
        public TransportReturnCode ProcessMsg(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, Msg msg, DecodeIterator dIter, out Error? error)
        {
            switch (msg.MsgClass)
            {
                case MsgClasses.REQUEST:
                    CodecReturnCode codecReturnCode = DictionaryRequest.Decode(dIter, msg);
                    if (codecReturnCode != CodecReturnCode.SUCCESS)
                    {
                        error = new Error()
                        {
                            Text = $"DictionaryRequest.Decode() failed with return code: {codecReturnCode.GetAsString()}"
                        };
                        return TransportReturnCode.FAILURE;
                    }

                    Console.WriteLine($"Received Dictionary Request for DictionaryName: {DictionaryRequest.DictionaryName}");
                    if (FieldDictionaryDownloadName.Equals(DictionaryRequest.DictionaryName))
                    {
                        return SendFieldDictionaryResponse(channelHandler, clientChannelInfo, out error);
                    }
                    else if (EnumTypeDictionaryDownloadName.Equals(DictionaryRequest.DictionaryName))
                    {
                        return SendEnumTypeDictionaryResponse(channelHandler, clientChannelInfo, out error);
                    }
                    else
                    {
                        return SendRequestReject(channelHandler, clientChannelInfo, msg.StreamId, DictionaryRejectReason.UNKNOWN_DICTIONARY_NAME, out error);
                    }
                case MsgClasses.CLOSE:
                    Console.WriteLine($"Received Dictionary Close for streamId {msg.StreamId}");
                    break;
                default:
                    error = new Error()
                    {
                        Text = $"Received unhandled Source Directory msg type: {msg.MsgClass}"
                    };
                    return TransportReturnCode.FAILURE;
            }

            error = null;
            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Sends the dictionary request reject status message for a channel
        /// </summary>
        /// <param name="channelHandler">The channel to send request reject status message to</param>
        /// <param name="clientChannelInfo"></param>
        /// <param name="streamId">The stream id of the request</param>
        /// <param name="reason">The reason for the reject</param>
        /// <param name="error"><see cref="Error"/> instance that carries information about failure</param>
        /// <returns><see cref="TransportReturnCode"/> value indicating the status of the operation</returns>
        private TransportReturnCode SendRequestReject(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, int streamId, DictionaryRejectReason reason, out Error error)
        {
            IChannel channel = clientChannelInfo.Channel!;

            // get a buffer for the dictionary request reject status
            ITransportBuffer msgBuf = channel.GetBuffer(MAX_DICTIONARY_STATUS_MSG_SIZE, false, out error);
            if (msgBuf == null)
                return TransportReturnCode.FAILURE;

            // encode dictionary request reject status
            TransportReturnCode transportReturnCode = EncodeDictionaryRequestReject(channel, streamId, reason, msgBuf, out error!);
            if (transportReturnCode != TransportReturnCode.SUCCESS)
                return TransportReturnCode.FAILURE;

            // send request reject status
            return channelHandler.WriteChannel(clientChannelInfo, msgBuf, 0, out error);
        }

        /// <summary>
        /// Sends a field dictionary response to a channel.
        /// </summary>
        ///
        /// This consists of getting a message buffer, encoding the dictionary response,
        /// and sending the dictionary response to the server.  Returns success if send
        /// dictionary response succeeds or failure if send response fails.
        ///
        /// <param name="channelHandler">the client channel handler</param>
        /// <param name="clientChannelInfo">client channel information to send dictionary response to.</param>
        /// <param name="error"><see cref="Error"/> instance that contains failure details</param>
        /// <returns><see cref="TransportReturnCode"/> value indicating the status of the operation</returns>
        private TransportReturnCode SendFieldDictionaryResponse(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, out Error error)
        {
            // set-up message
            m_DictionaryRefresh.Clear();
            m_DictionaryRefresh.StreamId = DictionaryRequest.StreamId;
            m_DictionaryRefresh.DictionaryType = Types.FIELD_DEFINITIONS;
            m_DictionaryRefresh.DataDictionary = Dictionary;
            m_DictionaryRefresh.State.StreamState(StreamStates.OPEN);
            m_DictionaryRefresh.State.DataState(DataStates.OK);
            m_DictionaryRefresh.State.Code(StateCodes.NONE);
            m_DictionaryRefresh.Verbosity = DictionaryRequest.Verbosity;
            m_DictionaryRefresh.ServiceId = DictionaryRequest.ServiceId;
            m_DictionaryRefresh.DictionaryName.Data(DictionaryRequest.DictionaryName.Data(), DictionaryRequest.DictionaryName.Position, DictionaryRequest.DictionaryName.Length);
            m_DictionaryRefresh.Solicited = true;

            IChannel channel = clientChannelInfo.Channel!;
            TransportReturnCode writeRet;
            while (true)
            {
                TransportReturnCode transportReturnCode = channel.Info(m_ChnlInfo, out error);
                if (transportReturnCode != TransportReturnCode.SUCCESS)
                {
                    return TransportReturnCode.FAILURE;
                }

                // get a buffer for the dictionary response
                ITransportBuffer msgBuf = channel.GetBuffer(m_ChnlInfo.MaxFragmentSize, false, out error);
                if (msgBuf == null)
                {
                    return TransportReturnCode.FAILURE;
                }

                m_EncodeIter.Clear();

                CodecReturnCode codecReturnCode = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
                if (codecReturnCode != CodecReturnCode.SUCCESS)
                {
                    error = new Error()
                    {
                        Text = $"EncodeIter.SetBufferAndRWFVersion() failed with return code: {codecReturnCode.GetAsString()}"
                    };
                    return TransportReturnCode.FAILURE;
                }

                m_DictionaryRefresh.State.Text().Data($"Field Dictionary Refresh (starting fid {m_DictionaryRefresh.StartFid})");
                codecReturnCode = m_DictionaryRefresh.Encode(m_EncodeIter);
                if (codecReturnCode < CodecReturnCode.SUCCESS)
                {
                    error = new Error()
                    {
                        Text = $"DictionaryRefresh.Encode() failed with code: {codecReturnCode.GetAsString()}"
                    };
                    return TransportReturnCode.FAILURE;
                }

                // send dictionary response
                if ((writeRet = channelHandler.WriteChannel(clientChannelInfo, msgBuf, 0, out error)) < TransportReturnCode.SUCCESS)
                {
                    return TransportReturnCode.FAILURE;
                }

                // break out of loop when all dictionary responses sent
                if (codecReturnCode == CodecReturnCode.SUCCESS)
                {
                    break;
                }
            }

            return writeRet;
        }

        /// <summary>
        /// Sends a enum dictionary response to a channel. This consists of getting a message buffer,
        /// encoding the dictionary response, and sending the dictionary response to the server.
        /// Returns success if send dictionary response succeeds or failure if send response fails.
        /// </summary>
        /// <param name="channelHandler">the client channel handler</param>
        /// <param name="clientChannelInfo">client channel information to send dictionary response toparam>
        /// <param name="error"><see cref="Error"/> instance that contains failure details</param>
        /// <returns><see cref="TransportReturnCode"/> value indicating the status of the operation</returns>
        private TransportReturnCode SendEnumTypeDictionaryResponse(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, out Error error)
        {
            IChannel channel = clientChannelInfo.Channel!;

            // get a buffer for the dictionary response
            ITransportBuffer msgBuf = channel.GetBuffer(MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE, false, out error);
            if (msgBuf == null)
            {
                return TransportReturnCode.FAILURE;
            }

            //encode dictionary refresh - enum type
            m_DictionaryRefresh.Clear();
            m_DictionaryRefresh.StreamId = DictionaryRequest.StreamId;
            m_DictionaryRefresh.Solicited = true;
            m_DictionaryRefresh.DictionaryType = Types.ENUM_TABLES;
            m_DictionaryRefresh.DataDictionary = Dictionary;
            m_DictionaryRefresh.State.StreamState(StreamStates.OPEN);
            m_DictionaryRefresh.State.DataState(DataStates.OK);
            m_DictionaryRefresh.State.Code(StateCodes.NONE);
            m_DictionaryRefresh.Verbosity = DictionaryRequest.Verbosity;
            m_DictionaryRefresh.DictionaryName.Data(DictionaryRequest.DictionaryName.Data(), DictionaryRequest.DictionaryName.Position, DictionaryRequest.DictionaryName.Length);
            m_DictionaryRefresh.RefreshComplete = true;

            m_EncodeIter.Clear();
            CodecReturnCode codecReturnCode = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = $"EncodeIter.SetBufferAndRWFVersion() failed with return code: {codecReturnCode.GetAsString()}"
                };
                return TransportReturnCode.FAILURE;
            }

            // encode message
            codecReturnCode = m_DictionaryRefresh.Encode(m_EncodeIter);
            if (codecReturnCode < CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = $"DictionaryRefresh.Encode() failed with code: {codecReturnCode.GetAsString()}"
                };
                return TransportReturnCode.FAILURE;
            }

            // send dictionary response
            return channelHandler.WriteChannel(clientChannelInfo, msgBuf, 0, out error);
        }

        /// <summary>
        /// Encodes the dictionary request reject status. Returns success if encoding succeeds or failure if encoding fails.
        /// </summary>
        /// <param name="chnl">The channel to send request reject status message to</param>
        /// <param name="streamId">The stream id of the request</param>
        /// <param name="reason">The reason for the reject</param>
        /// <param name="msgBuf">The message buffer to encode the dictionary request reject into</param>
        /// <param name="error"><see cref="Error"/> instance that contains error details in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value indicating the status of the operation</returns>
        private TransportReturnCode EncodeDictionaryRequestReject(IChannel chnl, int streamId, DictionaryRejectReason reason, ITransportBuffer msgBuf, out Error? error)
        {
            // set-up message
            m_DictionaryStatus.Clear();
            m_DictionaryStatus.StreamId = streamId;
            m_DictionaryStatus.HasState = true;
            m_DictionaryStatus.State.DataState(DataStates.SUSPECT);

            switch (reason)
            {
                case DictionaryRejectReason.UNKNOWN_DICTIONARY_NAME:
                    m_DictionaryStatus.State.Code(StateCodes.NOT_FOUND);
                    m_DictionaryStatus.State.StreamState(StreamStates.CLOSED);
                    m_DictionaryStatus.State.Text().Data($"Dictionary request rejected for stream id {streamId} - dictionary name unknown");
                    break;
                case DictionaryRejectReason.MAX_DICTIONARY_REQUESTS_REACHED:
                    m_DictionaryStatus.State.Code(StateCodes.TOO_MANY_ITEMS);
                    m_DictionaryStatus.State.StreamState(StreamStates.CLOSED_RECOVER);
                    m_DictionaryStatus.State.Text().Data($"Dictionary request rejected for stream id {streamId} max request count reached");
                    break;
                default:
                    break;
            }

            // clear encode iterator
            m_EncodeIter.Clear();

            // encode message
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret < CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = $"EncodeIter.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                };
                return TransportReturnCode.FAILURE;
            }
            ret = m_DictionaryStatus.Encode(m_EncodeIter);

            if (ret < CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = $"DictionaryStatus.Encode() failed with return code: {ret.GetAsString()}"
                };
                return TransportReturnCode.FAILURE;
            }
            error = null;
            return TransportReturnCode.SUCCESS;
        }

        #region Reactor Responses


        /// <summary>
        /// Sends a field dictionary response to a reactor channel.
        /// </summary>
        ///
        /// This consists of getting a message buffer, encoding the dictionary response,
        /// and sending the dictionary response to the server. Returns success if send
        /// dictionary response succeeds or failure if send response fails.
        ///
        /// <param name="clientChannelInfo">Client channel information to send dictionary response to.</param>
        ///
        internal ReactorReturnCode SendFieldDictionaryResponseReactor(ClientChannelInfo? clientChannelInfo, out ReactorErrorInfo? errorInfo)
        {
            // set-up message
            m_DictionaryRefresh.Clear();
            m_DictionaryRefresh.StreamId = DictionaryRequest.StreamId;
            m_DictionaryRefresh.DictionaryType = Types.FIELD_DEFINITIONS;
            m_DictionaryRefresh.DataDictionary = Dictionary;
            m_DictionaryRefresh.State.StreamState(StreamStates.OPEN);
            m_DictionaryRefresh.State.DataState(DataStates.OK);
            m_DictionaryRefresh.State.Code(StateCodes.NONE);
            m_DictionaryRefresh.Verbosity = DictionaryRequest.Verbosity;
            m_DictionaryRefresh.ServiceId = DictionaryRequest.ServiceId;
            m_DictionaryRefresh.DictionaryName.Data(DictionaryRequest.DictionaryName.Data(), DictionaryRequest.DictionaryName.Position, DictionaryRequest.DictionaryName.Length);
            m_DictionaryRefresh.Solicited = true;

            ReactorChannel reactorChannel = clientChannelInfo!.ReactorChannel!;

            while (true)
            {
                if (reactorChannel.Info(m_ReactorChnlInfo, out errorInfo) != ReactorReturnCode.SUCCESS)
                {
                    return ReactorReturnCode.FAILURE;
                }

                // get a buffer for the dictionary response
                ITransportBuffer? msgBuf = reactorChannel.GetBuffer(m_ReactorChnlInfo.ChannelInfo.MaxFragmentSize, false, out errorInfo);
                if (msgBuf == null)
                {
                    return ReactorReturnCode.FAILURE;
                }

                m_EncodeIter.Clear();
                CodecReturnCode codeRet = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, reactorChannel.MajorVersion, reactorChannel.MinorVersion);
                if (codeRet != CodecReturnCode.SUCCESS)
                {
                    errorInfo = new ReactorErrorInfo();
                    errorInfo.Error.Text = "EncodeIter.setBufferAndRWFVersion() failed with return code: " + codeRet;
                    errorInfo.Error.ErrorId = TransportReturnCode.FAILURE;
                    return ReactorReturnCode.FAILURE;
                }

                // next message chunk is encoded (if dictionary exceeds buffer size)
                m_DictionaryRefresh.State.Text().Data($"Field Dictionary Refresh (starting fid {m_DictionaryRefresh.StartFid})");
                codeRet = m_DictionaryRefresh.Encode(m_EncodeIter);
                if (codeRet < CodecReturnCode.SUCCESS)
                {
                    errorInfo = new ReactorErrorInfo();
                    errorInfo.Error.Text = "DictionaryRefresh.encode() failed: " + codeRet;
                    errorInfo.Error.ErrorId = TransportReturnCode.FAILURE;
                    return ReactorReturnCode.FAILURE;
                }

                // send dictionary response
                if (reactorChannel.Submit(msgBuf, m_ReactorSubmitOptions, out errorInfo) < ReactorReturnCode.SUCCESS)
                {
                    return ReactorReturnCode.FAILURE;
                }

                // break out of loop when all dictionary responses sent
                if (codeRet == CodecReturnCode.SUCCESS)
                {
                    break;
                }
            }

            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Sends a enum dictionary response to a reactor channel.
        /// </summary>
        ///
        /// This consists of getting a message buffer, encoding the dictionary response,
        /// and sending the dictionary response to the server. Returns success if send
        /// dictionary response succeeds or failure if send response fails.
        ///
        /// <param name="clientChannelInfo">Client channel information to send dictionary response to.</param>
        ///
        internal ReactorReturnCode SendEnumTypeDictionaryResponseReactor(ClientChannelInfo? clientChannelInfo, out ReactorErrorInfo? errorInfo)
        {
            ReactorChannel reactorChannel = clientChannelInfo!.ReactorChannel!;

            // get a buffer for the dictionary response
            ITransportBuffer? msgBuf = reactorChannel.GetBuffer(MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE, false, out errorInfo);
            if (msgBuf == null)
                return ReactorReturnCode.FAILURE;

            //encode dictionary refresh - enum type
            m_DictionaryRefresh.Clear();
            m_DictionaryRefresh.StreamId = DictionaryRequest.StreamId;
            m_DictionaryRefresh.Solicited = true;
            m_DictionaryRefresh.DictionaryType = Types.ENUM_TABLES;
            m_DictionaryRefresh.DataDictionary = Dictionary;
            m_DictionaryRefresh.State.StreamState(StreamStates.OPEN);
            m_DictionaryRefresh.State.DataState(DataStates.OK);
            m_DictionaryRefresh.State.Code(StateCodes.NONE);
            m_DictionaryRefresh.Verbosity = DictionaryRequest.Verbosity;
            m_DictionaryRefresh.DictionaryName.Data(DictionaryRequest.DictionaryName.Data(),
                DictionaryRequest.DictionaryName.Position, DictionaryRequest.DictionaryName.Length);
            m_DictionaryRefresh.RefreshComplete = true;

            m_EncodeIter.Clear();
            CodecReturnCode codeRet = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, reactorChannel.MajorVersion, reactorChannel.MinorVersion);
            if (codeRet != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = "EncodeIter.SetBufferAndRWFVersion() failed with return code: " + codeRet;
                errorInfo.Error.ErrorId = TransportReturnCode.FAILURE;
                return ReactorReturnCode.FAILURE;
            }

            // encode message
            codeRet = m_DictionaryRefresh.Encode(m_EncodeIter);
            if (codeRet < CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = "DictionaryRefresh.Encode() failed";
                errorInfo.Error.ErrorId = TransportReturnCode.FAILURE;
                return ReactorReturnCode.FAILURE;
            }

            // send dictionary response
            ReactorReturnCode ret = reactorChannel.Submit(msgBuf, m_ReactorSubmitOptions, out errorInfo);
            if (ret < ReactorReturnCode.SUCCESS)
                return ReactorReturnCode.FAILURE;

            return ReactorReturnCode.SUCCESS;
        }


        /// <summary>
        /// Sends the dictionary request reject status message for a reactor channel.
        /// </summary>
        ///
        /// <param name="chnl">The channel to send request reject status message to</param>
        /// <param name="streamId">The stream id of the request</param>
        /// <param name="reason">The reason for the reject</param>
        ///
        internal ReactorReturnCode SendRequestRejectReactor(ClientChannelInfo? clientChannelInfo, int streamId,
            DictionaryRejectReason reason, out ReactorErrorInfo? errorInfo)
        {
            ReactorChannel reactorChannel = clientChannelInfo!.ReactorChannel!;

            // get a buffer for the dictionary request reject status
            ITransportBuffer? msgBuf = reactorChannel.GetBuffer(MAX_DICTIONARY_STATUS_MSG_SIZE, false, out errorInfo);
            if (msgBuf == null)
                return ReactorReturnCode.FAILURE;

            // encode dictionary request reject status
            TransportReturnCode transRet = EncodeDictionaryRequestReject(reactorChannel.Channel!, streamId, reason, msgBuf, out var encodeError);
            if (transRet != TransportReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = encodeError?.Text;
                errorInfo.Error.ErrorId = encodeError?.ErrorId ?? TransportReturnCode.FAILURE;
                return ReactorReturnCode.FAILURE;
            }

            // send request reject status
            return reactorChannel.Submit(msgBuf, m_ReactorSubmitOptions, out errorInfo);
        }

        #endregion
    }
}
