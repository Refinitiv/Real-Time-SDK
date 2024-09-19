/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using System;

namespace LSEG.Eta.PerfTools.Common
{
    /// <summary>
    /// This is the dictionary handler for the ETA ConsPerf application. It provides 
    /// methods for loading the field/enumType dictionaries from a file and sending 
    /// requests for those dictionaries to a provider. Methods for processing the 
    /// dictionary response and closing a dictionary stream are also provided.
    /// </summary>
    public class DictionaryHandler
    {
        public const int FIELD_DICTIONARY_TYPE = 1;
        public const int ENUM_DICTIONARY_TYPE = 2;

        public const int FIELD_DICTIONARY_STREAM_ID = 3;
        public const int ENUM_TYPE_DICTIONARY_STREAM_ID = 4;

        public const int VERBOSITY = 7;

        public const int MAX_MSG_SIZE = 1024;
        public const int TRANSPORT_BUFFER_SIZE_REQUEST = MAX_MSG_SIZE;
        public const int TRANSPORT_BUFFER_SIZE_CLOSE = MAX_MSG_SIZE;

        private const string FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
        protected const string FIELD_DICTIONARY_DOWNLOAD_NAME = "RWFFld";
        private const string ENUM_TABLE_FILE_NAME = "enumtype.def";
        protected const string ENUM_TABLE_DOWNLOAD_NAME = "RWFEnum";

        /// <summary>
        /// Dictionary used by the handler
        /// </summary>
        public DataDictionary DataDictionary { get; private set; } = new DataDictionary();

        /// <summary>
        /// Indicates whether FieldDictionary has been loaded
        /// </summary>
        public bool FieldDictionaryLoaded { get; private set; } = false;

        /// <summary>
        /// Indicates whether FieldDictionary has been loaded from file
        /// </summary>
        public bool FieldDictionaryLoadedFromFile { get; private set; } = false;

        /// <summary>
        /// Indicates whether EnumType Dictionary has been loaded
        /// </summary>
        public bool EnumTypeDictionaryLoaded { get; private set; } = false;

        /// <summary>
        /// Indicates whether EnumType Dictionary has been loaded from file
        /// </summary>
        public bool EnumTypeDictionaryLoadedFromFile { get; private set; } = false;

        protected State[] states; // first - field dictionary state, second - enum dictionary state

        /// <summary>
        /// Current Service Id to request dictionary from
        /// </summary>
        public int ServiceId { get; set; }

        protected int fieldDictionaryStreamId = -1;
        protected int enumDictionaryStreamId = -1;

        private DictionaryRequest dictionaryRequest;
        protected DictionaryClose dictionaryClose;
        protected DictionaryRefresh dictionaryRefresh;
        protected EncodeIterator encIter;

        /// <summary>
        /// Instantiates DictionaryHandler
        /// </summary>
        public DictionaryHandler()
        {
            states = new State[2];
            states[0] = new State();
            states[1] = new State();
            dictionaryRequest = new DictionaryRequest();
            dictionaryClose = new DictionaryClose();
            dictionaryRefresh = new DictionaryRefresh();
            encIter = new EncodeIterator();
        }

        /// <summary>
        ///  Load dictionary from file if possible. Otherwise an attempt to download it will be made.
        /// </summary>
        /// <param name="error"><see cref="CodecError"/> instance that will hold error information in case loading fails.</param>
        public void LoadDictionary(out CodecError error)
        {
            DataDictionary.Clear();
            if (DataDictionary.LoadFieldDictionary(FIELD_DICTIONARY_FILE_NAME, out error) < 0)
            {
                Console.WriteLine($"Unable to load field dictionary.  Will attempt to download from provider.\n\tText: {error?.Text}");
            }
            else
            {
                FieldDictionaryLoaded = true;
                FieldDictionaryLoadedFromFile = true;
            }

            if (DataDictionary.LoadEnumTypeDictionary(ENUM_TABLE_FILE_NAME, out error) < 0)
            {
                Console.WriteLine($"Unable to load enum dictionary.  Will attempt to download from provider.\n\tText: {error?.Text}");
            }
            else
            {
                EnumTypeDictionaryLoaded = true;
                EnumTypeDictionaryLoadedFromFile = true;
            }
        }

        /// <summary>
        /// Sends both field and enumType dictionary requests to a channel. 
        /// This consists of getting a message buffer, encoding the dictionary request, 
        /// and sending the dictionary request to the server.        
        /// </summary>
        /// <param name="chnl">The channel to send a dictionary requests to</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public ITransportBuffer? GetRequest(IChannel chnl, int dictionaryType, out Error? error)
        {
            // initialize state management array
            // these will be updated as refresh and status messages are received
            states[0].DataState(DataStates.NO_CHANGE);
            states[0].StreamState(StreamStates.UNSPECIFIED);

            states[1].DataState(DataStates.NO_CHANGE);
            states[1].StreamState(StreamStates.UNSPECIFIED);

            // get a buffer for the dictionary request
            ITransportBuffer msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, out error);

            if (msgBuf != null)
            {
                // encode dictionary request
                dictionaryRequest.Clear();
                if (dictionaryType == FIELD_DICTIONARY_TYPE)
                {
                    dictionaryRequest.DictionaryName.Data(FIELD_DICTIONARY_DOWNLOAD_NAME);
                    dictionaryRequest.StreamId = FIELD_DICTIONARY_STREAM_ID;
                }
                else
                {
                    dictionaryRequest.DictionaryName.Data(ENUM_TABLE_DOWNLOAD_NAME);
                    dictionaryRequest.StreamId = ENUM_TYPE_DICTIONARY_STREAM_ID;
                }
                dictionaryRequest.ServiceId = ServiceId;
                dictionaryRequest.Verbosity = VERBOSITY;

                encIter.Clear();
                encIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);

                CodecReturnCode ret = dictionaryRequest.Encode(encIter);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    error = new Error()
                    {
                        Text = "EncodeDictionaryRequest(): Failed < code: " + ret
                    };
                    return null;
                }
            }

            return msgBuf;
        }

        /// <summary>
        /// Processes a dictionary response. This consists of calling decode() 
        /// on rdm dictionary messages to decode the dictionary and setting 
        /// dictionary download states (fieldDictionaryLoaded and enumTypeDictionaryLoaded) 
        /// after complete dictionaries are received.
        /// </summary>
        /// <param name="chnl">The channel of the response</param>
        /// <param name="msg">The partially decoded message</param>
        /// <param name="decIter">The decode iterator</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode ProcessResponse(IChannel chnl, Msg msg, DecodeIterator decIter, out Error? error)
        {
            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH:
                    return HandleDictionaryRefresh(msg, decIter, out error);
                case MsgClasses.STATUS:
                    Console.WriteLine("Received StatusMsg for dictionary");
                    IStatusMsg statusMsg = msg;
                    if (statusMsg.CheckHasState())
                    {
                        Console.WriteLine($"   {statusMsg.State}");
                        State state = statusMsg.State;
                        if (msg.StreamId == fieldDictionaryStreamId)
                        {
                            states[0].DataState(state.DataState());
                            states[0].StreamState(state.StreamState());
                        }
                        else if (msg.StreamId == enumDictionaryStreamId)
                        {
                            states[1].DataState(state.DataState());
                            states[1].StreamState(state.StreamState());
                        }
                    }
                    break;
                default:
                    Console.WriteLine($"Received Unhandled DataDictionary MsgClass: {msg.MsgClass}");
                    break;
            }

            error = null;
            return TransportReturnCode.SUCCESS;
        }

        private TransportReturnCode HandleDictionaryRefresh(Msg msg, DecodeIterator dIter, out Error? error)
        {
            CodecError codecError;
            CodecReturnCode ret = dictionaryRefresh.Decode(dIter, msg);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = "Error decoding dictionary refresh: " + ret.GetAsString() + ".",
                    ErrorId = TransportReturnCode.FAILURE
                };
                return TransportReturnCode.FAILURE;
            }

            if (dictionaryRefresh.HasInfo)
            {
                switch (dictionaryRefresh.DictionaryType)
                {
                    case Dictionary.Types.FIELD_DEFINITIONS:
                        fieldDictionaryStreamId = dictionaryRefresh.StreamId;
                        break;
                    case Dictionary.Types.ENUM_TABLES:
                        enumDictionaryStreamId = dictionaryRefresh.StreamId;
                        break;
                    default:
                        error = new Error()
                        {
                            Text = "Received unexpected dictionary message on stream " + msg.StreamId,
                            ErrorId = TransportReturnCode.FAILURE
                        };
                        return TransportReturnCode.FAILURE;
                }
            }

            Console.WriteLine("Received DataDictionary Refresh");
            Console.WriteLine(dictionaryRefresh.ToString());

            if (dictionaryRefresh.StreamId == fieldDictionaryStreamId)
            {
                IRefreshMsg refreshMsg = (IRefreshMsg)msg;
                states[0].DataState(refreshMsg.State.DataState());
                states[0].StreamState(refreshMsg.State.StreamState());

                ret = DataDictionary.DecodeFieldDictionary(dIter, Dictionary.VerbosityValues.VERBOSE, out codecError);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    error = new Error()
                    {
                        Text = $"Failed to decode FieldDictionary, code {ret.GetAsString()}, error {codecError?.Text}"
                    };
                    return TransportReturnCode.FAILURE;
                }

                if (dictionaryRefresh.RefreshComplete)
                {
                    FieldDictionaryLoaded = true;
                    if (!EnumTypeDictionaryLoaded)
                    {
                        Console.WriteLine("Field DataDictionary complete, waiting for Enum Table...");
                    }
                }
            }
            else if (dictionaryRefresh.StreamId == enumDictionaryStreamId)
            {
                IRefreshMsg refreshMsg = (IRefreshMsg)msg;
                states[1].DataState(refreshMsg.State.DataState());
                states[1].StreamState(refreshMsg.State.StreamState());

                ret = DataDictionary.DecodeEnumTypeDictionary(dIter, Dictionary.VerbosityValues.VERBOSE, out codecError);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    error = new Error()
                    {
                        Text = $"Failed to decode EnumType Dictionary, code {ret}, error {codecError?.Text}"
                    };
                    return TransportReturnCode.FAILURE;
                }

                if (dictionaryRefresh.RefreshComplete)
                {
                    EnumTypeDictionaryLoaded = true;
                    if (!FieldDictionaryLoaded)
                    {
                        Console.WriteLine("Enumerated Types DataDictionary complete, waiting for Field DataDictionary...");
                    }
                }
            }
            else
            {
                error = new Error()
                {
                    Text = "Received unexpected dictionary message on stream " + msg.StreamId
                };
                return TransportReturnCode.FAILURE;
            }

            error = null;
            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Close the dictionary stream if there is one.
        /// </summary>
        /// <param name="chnl"> The channel to send a dictionary close to.</param>
        /// <param name="streamId">The stream id of the dictionary stream to close.</param>
        /// <param name="error"><see cref="Error"/> instance</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public ITransportBuffer? GetCloseStream(IChannel chnl, int streamId, out Error error)
        {
            // get a buffer for the dictionary close
            ITransportBuffer msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, out error);
            if (msgBuf == null)
            {
                return null;
            }

            // encode dictionary close
            dictionaryClose.Clear();
            dictionaryClose.StreamId = streamId;
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);

            CodecReturnCode ret = dictionaryClose.Encode(encIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = "Encode DictionaryClose failed :  code " + ret.GetAsString() + ".",
                    ErrorId = TransportReturnCode.FAILURE
                };
                return null;
            }
            DataDictionary.Clear();
            if (streamId == FIELD_DICTIONARY_STREAM_ID)
                FieldDictionaryLoaded = false;
            if (streamId == ENUM_TYPE_DICTIONARY_STREAM_ID)
                EnumTypeDictionaryLoaded = false;

            return msgBuf;
        }
    }
}
