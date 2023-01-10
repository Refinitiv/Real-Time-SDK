/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Threading;
using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.Example.Common
{
    /// <summary>
    /// This is the dictionary handler for both the ETA - C# Interactive Provider
    /// application. Only two dictionary streams (field and enum type) per channel
    /// are allowed by this simple provider.
    /// <para>
    /// It provides methods for processing dictionary requests from consumers and
    /// sending back the responses.
    /// </para>
    /// <para>
    /// Methods for sending dictionary request reject/close status messages,
    /// initializing the dictionary provider, loading dictionaries from files,
    /// getting the loaded dictionary, and closing dictionary streams are also
    /// provided.
    /// </para>
    /// </summary>
    public class ProviderDictionaryHandler
    {
        private static readonly int MAX_MSG_SIZE = 1024;
        private static readonly int MAX_FIELD_DICTIONARY_MSG_SIZE = 8192;
        private static readonly int MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE = 12800;
        private static readonly int MAX_DICTIONARY_STATUS_MSG_SIZE = 1024;

        private static readonly string FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
        private static readonly string ENUM_TYPE_FILE_NAME = "enumtype.def";

        private static readonly Buffer FIELD_DICTIONARY_DOWNLOAD_NAME = new Buffer();

        private static readonly Buffer ENUM_TYPE_DOWNLOAD_NAME = new Buffer();

        private static readonly int FIELD_DICTIONARY_STREAM_ID = -1;
        private static readonly int ENUM_TYPE_DICTIONARY_STREAM_ID = -2;

        bool m_FieldDictionaryLoadedFromFile = false;
        bool m_EnumDictionaryLoadedFromFile = false;

        bool m_FieldDictionaryDownloaded = false;
        bool m_EnumDictionaryDownloaded = false;

        private ProviderSession? m_ProviderSession = null;
        public DataDictionary Dictionary { get; private set; } = new DataDictionary();
        private DictionaryRequestInfoList m_DictionaryRequestInfoList;

        private EncodeIterator m_EncodeIter = new EncodeIterator();
        private DictionaryRefresh m_DictionaryRefresh = new DictionaryRefresh();
        private DictionaryStatus m_DictionaryStatus = new DictionaryStatus();
        private DictionaryRequest m_DictionaryRequest = new DictionaryRequest();
        private DictionaryClose? m_DictionaryClose;

        private State[] m_States = new State[2]; // first=field dictionary state
                                                 // second=enum dictionary state

        static ProviderDictionaryHandler()
        {
            FIELD_DICTIONARY_DOWNLOAD_NAME.Data("RWFFld");
            ENUM_TYPE_DOWNLOAD_NAME.Data("RWFEnum");
        }

        /// <summary>
        /// Instantiates a new provider dictionary handler.
        /// </summary>
        /// <param name="providerSesssion">the provider sesssion</param>
        public ProviderDictionaryHandler(ProviderSession providerSesssion)
        {
            m_ProviderSession = providerSesssion;
            m_DictionaryRequestInfoList = new DictionaryRequestInfoList();

            m_States[0] = new State();
            m_States[1] = new State();
        }

        /// <summary>
        /// Initializes dictionary information fields.
        /// </summary>
        public void Init()
        {
            m_DictionaryRequestInfoList.Init();
        }

        /// <summary>
        /// Calls the method to free the dictionary
        /// </summary>
        public void FreeDictionary()
        {
            Dictionary.Clear();
        }

        /// <summary>
        /// Loads dictionary files.
        /// </summary>
        /// <param name="error">the error</param>
        /// <returns><c>true</c> if successful otherwise <c>false</c></returns>
        public bool LoadDictionary(out CodecError? error)
        {
            Dictionary.Clear();
            if (Dictionary.LoadFieldDictionary(FIELD_DICTIONARY_FILE_NAME, out error) < 0)
            {
                Console.WriteLine($"Unable to load field dictionary: {FIELD_DICTIONARY_FILE_NAME}.Error Text: {error.Text}");
                return false;
            }
            else
            {
                m_FieldDictionaryLoadedFromFile = true;
            }

            if (Dictionary.LoadEnumTypeDictionary(ENUM_TYPE_FILE_NAME, out error) < 0)
            {
                Console.WriteLine($"Unable to load enum dictionary: {ENUM_TYPE_FILE_NAME}.Error Text: {error.Text}");
                return false;
            }
            else
            {
                m_EnumDictionaryLoadedFromFile = true;
            }

            return true;
        }

        /// <summary>
        /// Indicates if the dictionary has been loaded.
        /// </summary>
        public bool IsDictionaryReady
        {
            get
            {
                return (m_FieldDictionaryLoadedFromFile && m_EnumDictionaryLoadedFromFile);
            }
        }

        /// <summary>
        /// Send dictionary requests.
        /// </summary>
        /// <param name="chnl">the channel</param>
        /// <param name="error">the error</param>
        /// <param name="serviceId">the service id</param>
        /// <returns><see cref="TransportReturnCode"/></returns>
        public CodecReturnCode SendDictionaryRequests(IChannel chnl, out Error? error, int serviceId)
        {
            CodecReturnCode sendStatus = RequestDictionary(chnl, out error, serviceId, FIELD_DICTIONARY_STREAM_ID, FIELD_DICTIONARY_DOWNLOAD_NAME.ToString());
            if (sendStatus == CodecReturnCode.SUCCESS)
            {
                sendStatus = RequestDictionary(chnl, out error, serviceId, ENUM_TYPE_DICTIONARY_STREAM_ID, ENUM_TYPE_DOWNLOAD_NAME.ToString());
            }

            return sendStatus;
        }

        CodecReturnCode RequestDictionary(IChannel chnl, out Error? error, int serviceId, int streamId, string dictName)
        {
            /* get a buffer for the dictionary request */
            ITransportBuffer msgBuf = chnl.GetBuffer(MAX_MSG_SIZE, false, out error);
            if (msgBuf == null)
            {
                return CodecReturnCode.FAILURE;
            }

            /* encode dictionary request */
            m_DictionaryRequest.Clear();

            m_DictionaryRequest.DictionaryName.Data(dictName);
            m_DictionaryRequest.StreamId = streamId;
            m_DictionaryRequest.ServiceId = serviceId;
            m_DictionaryRequest.Verbosity = Rdm.Dictionary.VerbosityValues.NORMAL;

            m_EncodeIter.Clear();

            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                };

                return ret;
            }

            ret = m_DictionaryRequest.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"EncodeDictionaryRequest(): Failed <code: {ret.GetAsString()}"
                };

                return ret;
            }

            Console.WriteLine(m_DictionaryRequest);

            //send request
            return (CodecReturnCode)m_ProviderSession!.Write(chnl, msgBuf, out error);
        }

        /// <summary>
        /// Processes a dictionary request. This consists of calling
        /// <c>DecodeDictionaryRequest()</c> to decode the request and calling
        /// <c>sendDictionaryResonse()</c> to send the dictionary response. Returns success
        /// if dictionary request processing succeeds or failure if processing fails.
        /// </summary>
        /// <param name="chnl">The channel of the request msg</param>
        /// <param name="msg">the msg</param>
        /// <param name="dIter">the decode iterator</param>
        /// <param name="error">the error</param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode ProcessRequest(IChannel chnl, Msg msg, DecodeIterator dIter, out Error? error)
        {
            // decode dictionary request
            m_DictionaryRequest.Clear();
            CodecReturnCode ret = m_DictionaryRequest.Decode(dIter, msg);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"DecodeDictionaryRequest() failed with return code: {ret.GetAsString()}"
                };

                return ret;
            }

            DictionaryRequestInfo? dictionaryRequestInfo = m_DictionaryRequestInfoList.Get(chnl, m_DictionaryRequest);
            if (dictionaryRequestInfo == null)
            {
                return SendRequestReject(chnl, msg.StreamId, DictionaryRejectReason.MAX_DICTIONARY_REQUESTS_REACHED, out error);
            }

            Console.WriteLine($"Received Dictionary Request for DictionaryName: {dictionaryRequestInfo.DictionaryRequest.DictionaryName}");
            if (FIELD_DICTIONARY_DOWNLOAD_NAME.Equals(dictionaryRequestInfo.DictionaryRequest.DictionaryName))
            {
                // send field dictionary refresh message 
                ret = SendFieldDictionaryResponse(chnl, dictionaryRequestInfo, out error);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }
            else if (ENUM_TYPE_DOWNLOAD_NAME.Equals(dictionaryRequestInfo.DictionaryRequest.DictionaryName))
            {
                // send enum dictionary refresh message
                ret = SendEnumTypeDictionaryResponse(chnl, dictionaryRequestInfo, out error);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }
            else
            {
                ret = SendRequestReject(chnl, msg.StreamId, DictionaryRejectReason.UNKNOWN_DICTIONARY_NAME, out error);
                return ret;
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Process message.
        /// </summary>
        /// <param name="chnl">the channel</param>
        /// <param name="msg">the message</param>
        /// <param name="dIter">the decode iterator</param>
        /// <param name="error">the error</param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode ProcessMessage(IChannel chnl, Msg msg, DecodeIterator dIter, out Error? error)
        {
            error = null;
            switch (msg.MsgClass)
            {
                case MsgClasses.REQUEST:
                    return ProcessRequest(chnl, msg, dIter, out error);

                case MsgClasses.REFRESH:
                    return ProcessRefresh(msg, dIter, out error);

                case MsgClasses.CLOSE:
                    Console.WriteLine($"Received Dictionary Close for StreamId {msg.StreamId}");

                    // close dictionary stream
                    CloseStream(msg.StreamId);
                    return CodecReturnCode.SUCCESS;

                case MsgClasses.STATUS:
                    Console.WriteLine("Received StatusMsg for dictionary");
                    IStatusMsg statusMsg = msg;
                    if (statusMsg.CheckHasState())
                    {
                        Console.WriteLine($"   {statusMsg.State}");
                        State state = statusMsg.State;
                        if (msg.StreamId == FIELD_DICTIONARY_STREAM_ID)
                        {
                            m_States[0].DataState(state.DataState());
                            m_States[0].StreamState(state.StreamState());
                        }
                        else if (msg.StreamId == ENUM_TYPE_DICTIONARY_STREAM_ID)
                        {
                            m_States[1].DataState(state.DataState());
                            m_States[1].StreamState(state.StreamState());
                        }
                    }
                    return CodecReturnCode.SUCCESS;

                default:
                    Console.WriteLine($"Received Unhandled Dictionary MsgClass: {msg.MsgClass}");
                    return CodecReturnCode.FAILURE;
            }
        }

        CodecReturnCode ProcessRefresh(Msg msg, DecodeIterator dIter, out Error? error)
        {
            error = null;
            CodecReturnCode ret = m_DictionaryRefresh.Decode(dIter, msg);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"Error decoding dictionary refresh: <{ret.GetAsString()}>"
                };

                return ret;
            }

            if (m_DictionaryRefresh.HasInfo)
            {
                switch (m_DictionaryRefresh.DictionaryType)
                {
                    case Rdm.Dictionary.Types.FIELD_DEFINITIONS:
                        break;
                    case Rdm.Dictionary.Types.ENUM_TABLES:
                        break;
                    default:

                        error = new Error()
                        {
                            ErrorId = TransportReturnCode.FAILURE,
                            Text = $"Received unexpected dictionary message on stream {msg.StreamId}"
                        };

                        return CodecReturnCode.FAILURE;
                }
            }

            if (m_DictionaryRefresh.StreamId == FIELD_DICTIONARY_STREAM_ID)
            {
                Console.WriteLine("Received Dictionary Refresh for field dictionary");

                IRefreshMsg refreshMsg = msg;

                m_States[0].DataState(refreshMsg.State.DataState());
                m_States[0].StreamState(refreshMsg.State.StreamState());

                ret = Dictionary.DecodeFieldDictionary(dIter, Rdm.Dictionary.VerbosityValues.VERBOSE, out CodecError codecError);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    error = new Error()
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = codecError.Text
                    };

                    return ret;
                }

                if (m_DictionaryRefresh.RefreshComplete)
                {
                    m_FieldDictionaryDownloaded = true;
                    if (!m_EnumDictionaryDownloaded)
                        Console.WriteLine("Field Dictionary complete, waiting for Enum Table...");
                }
            }
            else if (m_DictionaryRefresh.StreamId == ENUM_TYPE_DICTIONARY_STREAM_ID)
            {
                Console.WriteLine("Received Dictionary Refresh for enum type");

                IRefreshMsg refreshMsg = msg;

                m_States[1].DataState(refreshMsg.State.DataState());
                m_States[1].StreamState(refreshMsg.State.StreamState());

                ret = Dictionary.DecodeEnumTypeDictionary(dIter, Rdm.Dictionary.VerbosityValues.VERBOSE, out CodecError codecError);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }

                if (m_DictionaryRefresh.RefreshComplete)
                {
                    m_EnumDictionaryDownloaded = true;
                    if (!m_FieldDictionaryDownloaded)
                        Console.WriteLine("Enumerated Types Dictionary complete, waiting for Field Dictionary...");
                }
            }
            else
            {
                error = new Error()
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"Received unexpected dictionary message on stream {msg.StreamId}"
                };

                return CodecReturnCode.FAILURE;
            }

            if (m_FieldDictionaryDownloaded && m_EnumDictionaryDownloaded)
                Console.WriteLine("Dictionary Download complete for both field and enum type dictionaries");

            return CodecReturnCode.SUCCESS;
        }

        private void CloseStream(int streamId)
        {
            // find original request information associated with streamId
            foreach (DictionaryRequestInfo dictionraryReqInfo in m_DictionaryRequestInfoList)
            {
                if (dictionraryReqInfo.DictionaryRequest.StreamId == streamId && dictionraryReqInfo.IsInUse)
                {
                    // clear original request information
                    Console.WriteLine($"Closing dictionary stream id {dictionraryReqInfo.DictionaryRequest.StreamId} with dictionary name: " + dictionraryReqInfo.DictionaryRequest.DictionaryName);
                    dictionraryReqInfo.Clear();
                    break;
                }
            }
        }

        /// <summary>
        /// Closes all open dictionary streams for a channel.
        /// </summary>
        /// <param name="chnl">The channel to close the dictionary streams for</param>
        public void CloseRequests(IChannel chnl)
        {
            // find original request information associated with chnl
            foreach (DictionaryRequestInfo dictionraryReqInfo in m_DictionaryRequestInfoList)
            {
                if (dictionraryReqInfo.Channel == chnl && dictionraryReqInfo.IsInUse)
                {
                    // clear original request information
                    Console.WriteLine($"Closing dictionary stream id {dictionraryReqInfo.DictionaryRequest.StreamId} with dictionary name: {dictionraryReqInfo.DictionaryRequest.DictionaryName}");
                    dictionraryReqInfo.Clear();
                }
            }
        }

        /// <summary>
        /// Sends the dictionary close status message(s) for a channel. This consists
        /// of finding all request information for this channel and sending the close
        /// status messages to the channel.
        /// </summary>
        /// <param name="chnl">the channel to send close status message(s) to</param>
        /// <param name="error">the error</param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode SendCloseStatusMsgs(IChannel chnl, out Error? error)
        {
            error = null;
            foreach (DictionaryRequestInfo dictionraryReqInfo in m_DictionaryRequestInfoList)
            {
                if (dictionraryReqInfo.IsInUse && dictionraryReqInfo.Channel == chnl)
                {
                    CodecReturnCode ret = SendCloseStatusMsg(chnl, dictionraryReqInfo.DictionaryRequest.StreamId, out error);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                }
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Sends the dictionary close status message for a channel. Returns success
        /// if send dictionary close status succeeds or failure if it fails.
        /// </summary>
        /// <param name="chnl">The channel to send close status message to</param>
        /// <param name="streamId">The stream id of the close status</param>
        /// <param name="error">The error</param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode SendCloseStatusMsg(IChannel chnl, int streamId, out Error? error)
        {
            error = null;
            // get a buffer for the dictionary close status 
            ITransportBuffer msgBuf = chnl.GetBuffer(MAX_DICTIONARY_STATUS_MSG_SIZE, false, out error);

            if (msgBuf != null)
            {
                // encode directory close
                m_EncodeIter.Clear();
                CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    error = new Error()
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = $"EncodeIterator.setBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                    };

                    return ret;
                }

                // encode dictionary close status
                m_DictionaryStatus.Clear();
                m_DictionaryStatus.StreamId = streamId;
                m_DictionaryStatus.HasState = true;
                m_DictionaryStatus.State.StreamState(StreamStates.CLOSED);
                m_DictionaryStatus.State.DataState(DataStates.SUSPECT);
                m_DictionaryStatus.State.Text().Data("Dictionary stream closed");
                ret = m_DictionaryStatus.Encode(m_EncodeIter);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine($"EncodeDictionaryCloseStatus() failed with return code: {ret.GetAsString()}");
                    return ret;
                }

                // send close status
                return (CodecReturnCode)m_ProviderSession!.Write(chnl, msgBuf, out error);
            }
            else
            {
                return CodecReturnCode.FAILURE;
            }
        }

        private CodecReturnCode SendRequestReject(IChannel chnl, int streamId, DictionaryRejectReason reason, out Error? error)
        {
            // get a buffer for the dictionary request reject status
            ITransportBuffer msgBuf = chnl.GetBuffer(MAX_DICTIONARY_STATUS_MSG_SIZE, false, out error);

            if (msgBuf != null)
            {
                // encode dictionary request reject status 
                CodecReturnCode ret = EncodeDictionaryRequestReject(chnl, streamId, reason, msgBuf, out error);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }

                // send request reject status 
                return (CodecReturnCode)m_ProviderSession!.Write(chnl, msgBuf, out error);
            }
            else
            {
                return CodecReturnCode.FAILURE;
            }
        }

        private CodecReturnCode EncodeDictionaryRequestReject(IChannel chnl, int streamId, DictionaryRejectReason reason, ITransportBuffer msgBuf, out Error? error)
        {
            error = null;
            // clear encode iterator
            m_EncodeIter.Clear();

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
                    m_DictionaryStatus.State.Text().Data("Dictionary request rejected for stream id " + streamId + " - dictionary name unknown");
                    break;
                case DictionaryRejectReason.MAX_DICTIONARY_REQUESTS_REACHED:
                    m_DictionaryStatus.State.Code(StateCodes.TOO_MANY_ITEMS);
                    m_DictionaryStatus.State.StreamState(StreamStates.CLOSED_RECOVER);
                    m_DictionaryStatus.State.Text().Data("Dictionary request rejected for stream id " + streamId + " -  max request count reached");
                    break;
                default:
                    break;
            }

            // encode message
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeIterator.setBufferAndRWFVersion() failed with return code: {ret.GetAsString()}");
                return ret;
            }

            ret = m_DictionaryStatus.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("DictionaryStatus.Encode() failed");
                return ret;
            }

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode SendFieldDictionaryResponse(IChannel chnl, DictionaryRequestInfo dictionaryReqInfo, out Error? error)
        {
            m_DictionaryRefresh.Clear();

            m_DictionaryRefresh.StreamId = dictionaryReqInfo.DictionaryRequest.StreamId;
            m_DictionaryRefresh.DictionaryType = Rdm.Dictionary.Types.FIELD_DEFINITIONS;
            m_DictionaryRefresh.DataDictionary = Dictionary;
            m_DictionaryRefresh.State.StreamState(StreamStates.OPEN);
            m_DictionaryRefresh.State.DataState(DataStates.OK);
            m_DictionaryRefresh.State.Code(StateCodes.NONE);
            m_DictionaryRefresh.Verbosity = dictionaryReqInfo.DictionaryRequest.Verbosity;
            m_DictionaryRefresh.ServiceId = dictionaryReqInfo.DictionaryRequest.ServiceId;
            m_DictionaryRefresh.DictionaryName.Data(dictionaryReqInfo.DictionaryRequest.DictionaryName.Data());
            m_DictionaryRefresh.Solicited = true;

            bool firstMultiPart = true;

            while (true)
            {
                // get a buffer for the dictionary response
                ITransportBuffer msgBuf = chnl.GetBuffer(MAX_FIELD_DICTIONARY_MSG_SIZE, false, out error);
                if (msgBuf == null)
                    return CodecReturnCode.FAILURE;

                m_DictionaryRefresh.State.Text().Data($"Field Dictionary Refresh (starting fid {m_DictionaryRefresh.StartFid})");

                msgBuf.Data.Limit = MAX_FIELD_DICTIONARY_MSG_SIZE;

                // clear encode iterator
                m_EncodeIter.Clear();
                CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    error = new Error()
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = $"EncodeIterator.setBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                    };

                    return ret;
                }

                if (firstMultiPart)
                {
                    m_DictionaryRefresh.ClearCache = true;
                    firstMultiPart = false;
                }
                else
                    m_DictionaryRefresh.Flags = DictionaryRefreshFlags.SOLICITED;

                // encode message
                ret = m_DictionaryRefresh.Encode(m_EncodeIter);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    error = new Error()
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = "DictionaryRefresh.Encode() failed"
                    };

                    return ret;
                }

                // send dictionary response
                if (m_ProviderSession!.Write(chnl, msgBuf, out error) != TransportReturnCode.SUCCESS)
                    return CodecReturnCode.FAILURE;

                // break out of loop when all dictionary responses sent
                if (ret == CodecReturnCode.SUCCESS)
                {
                    break;
                }

                // sleep between dictionary responses
                Thread.Sleep(1);

            }

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode SendEnumTypeDictionaryResponse(IChannel chnl, DictionaryRequestInfo dictionaryReqInfo, out Error? error)
        {
            m_DictionaryRefresh.Clear();

            m_DictionaryRefresh.StreamId = dictionaryReqInfo.DictionaryRequest.StreamId;
            m_DictionaryRefresh.DictionaryType = Rdm.Dictionary.Types.ENUM_TABLES;
            m_DictionaryRefresh.DataDictionary = Dictionary;
            m_DictionaryRefresh.ServiceId = dictionaryReqInfo.DictionaryRequest.ServiceId;
            m_DictionaryRefresh.Verbosity = dictionaryReqInfo.DictionaryRequest.Verbosity;
            m_DictionaryRefresh.Solicited = true;

            m_DictionaryRefresh.State.StreamState(StreamStates.OPEN);
            m_DictionaryRefresh.State.DataState(DataStates.OK);
            m_DictionaryRefresh.State.Code(StateCodes.NONE);
            m_DictionaryRefresh.State.Text().Data("Enum Type Dictionary Refresh");

            bool firstMultiPart = true;

            // dictionaryName
            m_DictionaryRefresh.DictionaryName.Data(dictionaryReqInfo.DictionaryRequest.DictionaryName.Data());

            while (true)
            {
                // get a buffer for the dictionary response
                ITransportBuffer msgBuf = chnl.GetBuffer(MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE, false, out error);
                if (msgBuf == null)
                    return CodecReturnCode.FAILURE;

                m_DictionaryRefresh.State.Text().Data($"Enum Type Dictionary Refresh (starting enum {m_DictionaryRefresh.StartEnumTableCount}");

                msgBuf.Data.Limit = MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE;

                // clear encode iterator
                m_EncodeIter.Clear();
                CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    error = new Error()
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = $"EncodeIterator.setBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                    };

                    return ret;
                }

                if (firstMultiPart)
                {
                    m_DictionaryRefresh.ClearCache = true;
                    firstMultiPart = false;
                }
                else
                    m_DictionaryRefresh.Flags = DictionaryRefreshFlags.SOLICITED;

                // encode message
                ret = m_DictionaryRefresh.Encode(m_EncodeIter);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    error = new Error()
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = "DictionaryRefresh.Encode() failed"
                    };

                    return ret;
                }

                // send dictionary response
                if (m_ProviderSession!.Write(chnl, msgBuf, out error) != TransportReturnCode.SUCCESS)
                    return CodecReturnCode.FAILURE;

                // break out of loop when all dictionary responses sent
                if (ret == CodecReturnCode.SUCCESS)
                {
                    break;
                }

                // sleep between dictionary responses
                Thread.Sleep(1);
            }

            return CodecReturnCode.SUCCESS;
        }

        private bool CloseDictionary(IChannel chnl, int streamId, out Error? error)
        {
            if (m_DictionaryClose is null)
            {
                m_DictionaryClose = new DictionaryClose();
            }

            /* get a buffer for the dictionary close */
            ITransportBuffer msgBuf = chnl.GetBuffer(MAX_MSG_SIZE, false, out error);
            if (msgBuf == null)
            {
                return false;
            }

            m_EncodeIter.Clear();
            m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);

            /* encode dictionary close */
            m_DictionaryClose.Clear();
            m_DictionaryClose.StreamId = streamId;

            CodecReturnCode ret = m_DictionaryClose.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"encodeDictionaryClose(): Failed <code: {ret.GetAsString()}>");
                return false;
            }

            /* send close */
            if (m_ProviderSession!.Write(chnl, msgBuf, out error) != TransportReturnCode.SUCCESS)
            {
                return false;
            }

            Dictionary.Clear();

            return true;
        }

        /// <summary>
        /// Closes stream.
        /// </summary>
        /// <param name="clientChannel">Close stream.</param>
        /// <param name="error">the error</param>
        public void CloseStream(IChannel clientChannel, out Error? error)
        {
            error = null;
            /* close dictionary stream */
            if (m_FieldDictionaryDownloaded)
            {
                /*
                 * we only want to close a stream if it was not already closed (e.g.
                 * rejected by provider, closed via refresh or status)
                 */
                if (m_States[0].DataState() == DataStates.OK && m_States[0].StreamState() == StreamStates.NON_STREAMING)
                {
                    if (CloseDictionary(clientChannel, FIELD_DICTIONARY_STREAM_ID, out error))
                    {
                        m_FieldDictionaryDownloaded = false;
                        m_States[0].Clear();
                    }
                }
            }

            if (m_EnumDictionaryDownloaded)
            {
                /*
                 * we only want to close a stream if it was not already closed (e.g.
                 * rejected by provider, closed via refresh or status)
                 */
                if (m_States[1].DataState() == DataStates.OK && m_States[1].StreamState() == StreamStates.NON_STREAMING)
                {
                    if (CloseDictionary(clientChannel, ENUM_TYPE_DICTIONARY_STREAM_ID, out error))
                    {
                        m_EnumDictionaryDownloaded = false;
                        m_States[1].Clear();
                    }
                }
            }
        }
    }
}
