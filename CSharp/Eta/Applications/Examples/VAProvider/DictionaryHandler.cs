/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Threading;

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using static LSEG.Eta.Rdm.Dictionary;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValueAdd.Provider
{
    public class DictionaryHandler
    {
        private const int MAX_FIELD_DICTIONARY_MSG_SIZE = 8192;
        private const int MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE = 12800;
        private const int MAX_DICTIONARY_STATUS_MSG_SIZE = 1024;

        private const string FIELD_DICTIONARY_NAME = "RWFFld";
        private const string ENUM_TYPE_DICTIONARY_NAME = "RWFEnum";

        private const string FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
        private const string ENUM_TABLE_FILE_NAME = "enumtype.def";

        public static Buffer FieldDictionaryDownloadName { get; set; }
        public static Buffer EnumTypeDictionaryDownloadName { get; set; }

        private ReactorSubmitOptions m_SubmitOptions = new ReactorSubmitOptions();       
        private DictionaryRequestInfoList m_DictionaryRequestInfoList;

        private EncodeIterator m_EncodeIter = new EncodeIterator();
        private DictionaryRefresh m_DictionaryRefresh = new DictionaryRefresh();
        private DictionaryStatus m_DictionaryStatus = new DictionaryStatus();

        public DataDictionary Dictionary { get; set; }

        static DictionaryHandler()
        {
            FieldDictionaryDownloadName = new Buffer();
            FieldDictionaryDownloadName.Data(FIELD_DICTIONARY_NAME);
            EnumTypeDictionaryDownloadName = new Buffer();
            EnumTypeDictionaryDownloadName.Data(ENUM_TYPE_DICTIONARY_NAME);
        }

        public DictionaryHandler()
        {
            Dictionary = new DataDictionary();
            m_DictionaryRequestInfoList = new DictionaryRequestInfoList();           
        }

        /// <summary>
        /// Initializes dictionary information fields.
        /// </summary>
        public void Init()
        {
            m_DictionaryRequestInfoList.Init();
        }

        /// <summary>
        /// Loads Field Dictionary and EnumType dicitonary for file names specified
        /// </summary>
        /// <returns>true in case dictionaries were loaded successfully, false if at least one dictionary was not loaded successfully</returns>
        public bool LoadDictionary()
        {
            Dictionary.Clear();
            if (Dictionary.LoadFieldDictionary(FIELD_DICTIONARY_FILE_NAME, out CodecError fdError) < 0)
            {
                Console.WriteLine($"Failed to load FieldDictionary, error: {fdError?.Text}");
                return false;
            }

            if (Dictionary.LoadEnumTypeDictionary(ENUM_TABLE_FILE_NAME, out CodecError edError) < 0)
            {
                Console.WriteLine($"Failed to load EnumType Dictionary, error: {edError?.Text}");
                return false;
            }                

            return true;
        }

        /// <summary>
        /// Closes a dictionary stream
        /// </summary>
        /// <param name="streamId">the id of the stream to be closed</param>
        public void CloseStream(int streamId)
        {
            // find original request information associated with streamId
            foreach (DictionaryRequestInfo dictionraryReqInfo in m_DictionaryRequestInfoList)
            {
                if (dictionraryReqInfo.DictionaryRequest.StreamId == streamId && dictionraryReqInfo.IsInUse)
                {
                    // clear original request information
                    Console.WriteLine($"Closing dictionary stream id {dictionraryReqInfo.DictionaryRequest.StreamId} " +
                        $"with dictionary name: {dictionraryReqInfo.DictionaryRequest.DictionaryName}");
                    dictionraryReqInfo.Clear();
                    break;
                }
            }
        }

        /// <summary>
        /// Closes all open dictionary streams for a channel
        /// </summary>
        /// <param name="chnl">the <see cref="ReactorChannel"/> that corresponds to the current client</param>
        public void CloseStream(ReactorChannel chnl)
        {
            // find original request information associated with chnl
            foreach (DictionaryRequestInfo dictionraryReqInfo in m_DictionaryRequestInfoList)
            {
                if (dictionraryReqInfo.Channel == chnl.Channel && dictionraryReqInfo.IsInUse)
                {
                    // clear original request information
                    Console.WriteLine($"Closing dictionary stream id {dictionraryReqInfo.DictionaryRequest.StreamId} " +
                        $"with dictionary name: {dictionraryReqInfo.DictionaryRequest.DictionaryName}");
                    dictionraryReqInfo.Clear();
                }
            }
        }

        /// <summary>
        /// Sends the dictionary close status message(s) for a channel. 
        /// This consists of finding all request information for this channel 
        /// and sending the close status messages to the channel.
        /// </summary>
        /// <param name="chnl">the reactor channel</param>
        /// <param name="errorInfo">the <see cref="ReactorErrorInfo"/> structure that contains error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value indicating the status of the operation</returns>
        public ReactorReturnCode SendCloseStatusMsgs(ReactorChannel chnl, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret;
            foreach (DictionaryRequestInfo dictionraryReqInfo in m_DictionaryRequestInfoList)
            {
                if (dictionraryReqInfo.IsInUse && dictionraryReqInfo.Channel == chnl.Channel)
                {
                    ret = SendCloseStatusMsg(chnl, dictionraryReqInfo.DictionaryRequest.StreamId, out errorInfo);
                    if (ret != ReactorReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Sends the dictionary close status message for a channel. 
        /// Returns success if send dictionary close status succeeds or failure if it fails.
        /// </summary>
        /// <param name="chnl">the reactor channel</param>
        /// <param name="streamId">the id of the stream</param>
        /// <param name="errorInfo">the <see cref="ReactorErrorInfo"/> structure that contains error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value indicating the status of the operation</returns>
        public ReactorReturnCode SendCloseStatusMsg(ReactorChannel chnl, int streamId, out ReactorErrorInfo? errorInfo)
        {
            // get a buffer for the dictionary close status 
            ITransportBuffer? msgBuf = chnl.GetBuffer(MAX_DICTIONARY_STATUS_MSG_SIZE, false, out errorInfo);

            if (msgBuf != null)
            {
                // encode directory close
                m_EncodeIter.Clear();
                CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    errorInfo = new ReactorErrorInfo();
                    errorInfo.Error.Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}";
                    return ReactorReturnCode.FAILURE;
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
                    errorInfo = new ReactorErrorInfo();
                    errorInfo.Error.Text = $"EncodeDictionaryCloseStatus() failed with return code: {ret.GetAsString()}";
                    return ReactorReturnCode.FAILURE; ;
                }

                // send close status
                return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
            }
            else
            {
                return ReactorReturnCode.FAILURE;
            }
        }

        /// <summary>
        /// Encodes the dictionary request reject status. 
        /// Returns success if encoding succeeds or failure if encoding fails
        /// </summary>
        /// <param name="chnl">the <see cref="ReactorChannel"/> that corresponds to the current client</param>
        /// <param name="streamId">the id of the dictionary stream</param>
        /// <param name="reason">dictionary request reject reason</param>
        /// <param name="msgBuf"><see cref="ITransportBuffer"/> instance to encode dictionary reject to</param>
        /// <param name="errorInfo">the <see cref="ReactorErrorInfo"/> structure that contains error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value indicating the status of the operation</returns>
        private ReactorReturnCode EncodeDictionaryRequestReject(ReactorChannel chnl, int streamId, DictionaryRejectReason reason, ITransportBuffer msgBuf, ReactorErrorInfo errorInfo)
        {
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
                    m_DictionaryStatus.State.Text().Data($"Dictionary request rejected for stream id {streamId} - dictionary name unknown");
                    break;
                case DictionaryRejectReason.MAX_DICTIONARY_REQUESTS_REACHED:
                    m_DictionaryStatus.State.Code(StateCodes.TOO_MANY_ITEMS);
                    m_DictionaryStatus.State.StreamState(StreamStates.CLOSED_RECOVER);
                    m_DictionaryStatus.State.Text().Data($"Dictionary request rejected for stream id {streamId} -  max request count reached");
                    break;
                case DictionaryRejectReason.DICTIONARY_RDM_DECODER_FAILED:
                    m_DictionaryStatus.State.Code(StateCodes.USAGE_ERROR);
                    m_DictionaryStatus.State.Text().Data($"Dictionary request rejected for stream id {streamId} - decoding failure, error: {(errorInfo?.Error?.Text != null ? errorInfo?.Error.Text : "")}");
                    break;
                default:
                    break;
            }

            // encode message
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}");
                return ReactorReturnCode.FAILURE;
            }

            ret = m_DictionaryStatus.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("DictionaryStatus.Encode() failed");
                return ReactorReturnCode.FAILURE;
            }

            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Sends a field dictionary response to a channel. 
        /// This consists of getting a message buffer, encoding the dictionary response, and sending 
        /// the dictionary response to the server. Returns success if send dictionary response 
        /// succeeds or failure if send response fails. 
        /// </summary>
        /// <param name="chnl">the <see cref="ReactorChannel"/> that corresponds to the client</param>
        /// <param name="dictionaryRequest">the diciotnary request received from the client</param>
        /// <param name="errorInfo"><see cref="ReactorErrorInfo"/> instance with error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value indicating the success of the operation</returns>
        public ReactorReturnCode SendFieldDictionaryResponse(ReactorChannel chnl, DictionaryRequest dictionaryRequest, out ReactorErrorInfo? errorInfo)
        {
            m_DictionaryRefresh.Clear();

            m_DictionaryRefresh.StreamId = dictionaryRequest.StreamId;
            m_DictionaryRefresh.DictionaryType = Types.FIELD_DEFINITIONS;
            m_DictionaryRefresh.DataDictionary = Dictionary;
            m_DictionaryRefresh.State.StreamState(StreamStates.OPEN);
            m_DictionaryRefresh.State.DataState(DataStates.OK);
            m_DictionaryRefresh.State.Code(StateCodes.NONE);
            m_DictionaryRefresh.Verbosity = dictionaryRequest.Verbosity;
            m_DictionaryRefresh.ServiceId = dictionaryRequest.ServiceId;
            m_DictionaryRefresh.DictionaryName = dictionaryRequest.DictionaryName;
            m_DictionaryRefresh.Solicited = true;

            bool firstMultiPart = true;

            while (true)
            {
                // get a buffer for the dictionary response
                ITransportBuffer? msgBuf = chnl.GetBuffer(MAX_FIELD_DICTIONARY_MSG_SIZE, false, out errorInfo);
                if (msgBuf == null)
                {
                    errorInfo = new ReactorErrorInfo();
                    return ReactorReturnCode.FAILURE;
                }

                m_DictionaryRefresh.State.Text().Data($"Field Dictionary Refresh (starting fid {m_DictionaryRefresh.StartFid})");

                msgBuf.Data.Limit = MAX_FIELD_DICTIONARY_MSG_SIZE;

                // clear encode iterator
                m_EncodeIter.Clear();
                CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    errorInfo = new ReactorErrorInfo();
                    errorInfo.Error.Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}";
                    return ReactorReturnCode.FAILURE;
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
                    errorInfo = new ReactorErrorInfo();
                    errorInfo.Error.Text = "DictionaryRefresh.Encode() failed";
                    return ReactorReturnCode.FAILURE;
                }

                // send dictionary response
                if (chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo) != ReactorReturnCode.SUCCESS)
                {
                    errorInfo = new ReactorErrorInfo();
                    errorInfo.Error.Text = "ReactorChannel.Submit failed";
                    return ReactorReturnCode.FAILURE;
                }

                // break out of loop when all dictionary responses sent
                if (ret == CodecReturnCode.SUCCESS)
                {
                    break;
                }

                // sleep between dictionary responses
                try
                {
                    Thread.Sleep(1);
                }
                catch (Exception)
                {
                }
            }

            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Sends an enum type dictionary response to a channel. 
        /// This consists of getting a message buffer, encoding the dictionary response, 
        /// and sending the dictionary response to the server. Returns success 
        /// if send dictionary response succeeds or failure if send response fails. 
        /// </summary>
        /// <param name="chnl">the <see cref="ReactorChannel"/> instance associated with the current client</param>
        /// <param name="dictionaryRequest">current dictionary request</param>
        /// <param name="errorInfo"><see cref="ReactorErrorInfo"/> instance with error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value indocating the success of the operation</returns>
        public ReactorReturnCode SendEnumTypeDictionaryResponse(ReactorChannel chnl, DictionaryRequest dictionaryRequest, out ReactorErrorInfo? errorInfo)
        {
            m_DictionaryRefresh.Clear();

            m_DictionaryRefresh.StreamId = dictionaryRequest.StreamId;
            m_DictionaryRefresh.DictionaryType = Types.ENUM_TABLES;
            m_DictionaryRefresh.DataDictionary = Dictionary;
            m_DictionaryRefresh.ServiceId = dictionaryRequest.ServiceId;
            m_DictionaryRefresh.Verbosity = dictionaryRequest.Verbosity;
            m_DictionaryRefresh.Solicited = true;

            m_DictionaryRefresh.State.StreamState(StreamStates.OPEN);
            m_DictionaryRefresh.State.DataState(DataStates.OK);
            m_DictionaryRefresh.State.Code(StateCodes.NONE);

            // dictionaryName
            m_DictionaryRefresh.DictionaryName = dictionaryRequest.DictionaryName;

            bool firstMultiPart = true;

            while (true)
            {
                // get a buffer for the dictionary response
                ITransportBuffer? msgBuf = chnl.GetBuffer(MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE, false, out errorInfo);
                if (msgBuf == null)
                {
                    return ReactorReturnCode.FAILURE;
                }

                m_DictionaryRefresh.State.Text().Data("Enum Type Dictionary Refresh (starting enum " + m_DictionaryRefresh.StartEnumTableCount + ")");

                msgBuf.Data.Limit = MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE;

                // clear encode iterator
                m_EncodeIter.Clear();
                CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    errorInfo = new ReactorErrorInfo();
                    errorInfo.Error.Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}";
                    return ReactorReturnCode.FAILURE;
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
                    errorInfo = new ReactorErrorInfo();
                    errorInfo.Error.Text = "DictionaryRefresh.Encode() failed";
                    return ReactorReturnCode.FAILURE;
                }

                // send dictionary response
                if (chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo) != ReactorReturnCode.SUCCESS)
                {
                    errorInfo = new ReactorErrorInfo();
                    errorInfo.Error.Text = "ReactorChannel.Submit failed";
                    return ReactorReturnCode.FAILURE;
                }

                // break out of loop when all dictionary responses sent
                if (ret == CodecReturnCode.SUCCESS)
                {
                    break;
                }

                // sleep between dictionary responses
                try
                {
                    Thread.Sleep(1);
                }
                catch (Exception)
                {
                }
            }

            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Gets dictionary request information for the channel.
        /// </summary>
        /// <param name="reactorChannel">the reactor channel responsible for the request</param>
        /// <param name="dictionaryRequest">current dictionary request</param>
        /// <returns><see cref="DictionaryRequestInfo"/> instance</returns>
        public DictionaryRequestInfo? GetDictionaryRequestInfo(ReactorChannel reactorChannel, DictionaryRequest dictionaryRequest)
        {
            return m_DictionaryRequestInfoList.Get(reactorChannel.Channel!, dictionaryRequest);
        }

        /// <summary>
        /// Sends the dictionary request reject status message for a channel
        /// </summary>
        /// <param name="chnl">the <see cref="ReactorChannel"/> instance associated with the current client</param>
        /// <param name="streamId">the id of the stream to send the reject to</param>
        /// <param name="reason">the reject reason</param>
        /// <param name="errorInfo"><see cref="ReactorErrorInfo"/> instance with error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value indocating the success of the operation</returns>
        public ReactorReturnCode SendRequestReject(ReactorChannel chnl, int streamId, DictionaryRejectReason reason, out ReactorErrorInfo? errorInfo)
        {
            // get a buffer for the dictionary request reject status
            ITransportBuffer? msgBuf = chnl.GetBuffer(MAX_DICTIONARY_STATUS_MSG_SIZE, false, out errorInfo);

            if (msgBuf != null)
            {
                // encode dictionary request reject status 
                ReactorReturnCode ret = EncodeDictionaryRequestReject(chnl, streamId, reason, msgBuf, errorInfo!);
                if (ret != ReactorReturnCode.SUCCESS)
                {
                    return ret;
                }

                // send request reject status 
                return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
            }
            else
            {
                return ReactorReturnCode.FAILURE;
            }
        }
    }
}
