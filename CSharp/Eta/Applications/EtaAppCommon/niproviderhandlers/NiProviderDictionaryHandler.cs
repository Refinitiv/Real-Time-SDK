/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Common.Interfaces;
using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Rdm;
using Refinitiv.Eta.Transports;
using Refinitiv.Eta.ValueAdd.Rdm;
using System;

namespace Refinitiv.Eta.Example.Common
{
    /// <summary>
    /// This is the dictionary handler for the ETA consumer application. 
    /// It provides methods for loading the field/enumType dictionaries from a file or requesting from ADH.
    /// </summary>
    public class NiProviderDictionaryHandler
    {
        private const string FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
        private const string ENUM_TYPE_FILE_NAME = "enumtype.def";

        public const string FIELD_DICTIONARY_DOWNLOAD_NAME = "RWFFld";
        public const string ENUM_TYPE_DOWNLOAD_NAME = "RWFEnum";
    
        private const int FIELD_DICTIONARY_STREAM_ID = -1;
        private const int ENUM_TYPE_DICTIONARY_STREAM_ID = -2;

        public DataDictionary DataDictionary { get; set; }

        private bool bfieldDictionaryDownloaded = false;
        private bool bEnumTypeDictionaryLoaded = false;

        private DictionaryRequest dictionaryRequest = new DictionaryRequest();
        private DictionaryRefresh dictionaryRefresh = new DictionaryRefresh();
        private DictionaryClose dictionaryClose = new DictionaryClose();

        private EncodeIterator encodeIter = new EncodeIterator();

        private State[] states;

        /// <summary>
        /// Instantiates a new NIProvider dictionary handler.
        /// </summary>
        public NiProviderDictionaryHandler()
        {
            DataDictionary = new DataDictionary();

            states = new State[2];
            states[0] = new State();
            states[1] = new State();
        }

        /// <summary>
        /// Loads dictionary from file.
        /// </summary>
        /// <param name="error"><see cref="Error"/> instance that carries information about failure 
        /// in case of nusuccessful operation.</param>
        /// <returns>true if the Field and EnumType dictionaries were loaded, otherwise false.</returns>
        public bool LoadDictionary(out CodecError error)
        {
            DataDictionary.Clear();
            if (DataDictionary.LoadFieldDictionary(FIELD_DICTIONARY_FILE_NAME, out error) < 0)
            {
                Console.WriteLine("Unable to load field dictionary: {0}. Error Text: {1}\n", FIELD_DICTIONARY_FILE_NAME, error.Text);
                return false;
            }
            else
            {
                Console.WriteLine("Loaded field dictionary from: {0}\n", FIELD_DICTIONARY_FILE_NAME);
            }

            if (DataDictionary.LoadEnumTypeDictionary(ENUM_TYPE_FILE_NAME, out error) < 0)
            {
                Console.WriteLine("Unable to load enum dictionary: {0}. Error Text: {1}\n", ENUM_TYPE_FILE_NAME, error.Text);
                return false;
            }
            else
            {
                Console.WriteLine("Loaded enumtype dictionary from: {0}\n", ENUM_TYPE_FILE_NAME);
            }

            return true;
        }

        /// <summary>
        /// Send dictionary requests.
        /// </summary>
        /// <param name="chnl">The channel that receives the request.</param>
        /// <param name="error"><see cref="Error"/> instance that carries information about failure 
        /// in case of nusuccessful operation.</param>
        /// <param name="serviceId">the service id.</param>
        /// <returns><see cref="TransportReturnCode"/> value indicating the status of the operation.</returns>
        public TransportReturnCode SendDictionaryRequests(ChannelSession chnl, out Error? error, int serviceId)
        {
            if (dictionaryRequest == null)
            {
                dictionaryRequest = new DictionaryRequest();
            }

            TransportReturnCode requestStatus = RequestDictionary(chnl, serviceId, FIELD_DICTIONARY_STREAM_ID, FIELD_DICTIONARY_DOWNLOAD_NAME, out error);

            if (requestStatus == TransportReturnCode.SUCCESS)
            {
                requestStatus = RequestDictionary(chnl, serviceId, ENUM_TYPE_DICTIONARY_STREAM_ID, ENUM_TYPE_DOWNLOAD_NAME, out error);
            }

            return requestStatus;
        }

        TransportReturnCode RequestDictionary(ChannelSession chnl, int serviceId, int streamId, string dictName, out Error? error)
        {
            ITransportBuffer? msgBuf = chnl.GetTransportBuffer(ChannelSession.MAX_MSG_SIZE, false, out error);
            if (msgBuf == null)
            {
                return TransportReturnCode.FAILURE;
            }

            dictionaryRequest.Clear();

            dictionaryRequest.DictionaryName.Data(dictName);
            dictionaryRequest.StreamId = streamId;
            dictionaryRequest.ServiceId = serviceId;
            dictionaryRequest.Verbosity = Dictionary.VerbosityValues.NORMAL;

            encodeIter.Clear();

            CodecReturnCode ret = encodeIter.SetBufferAndRWFVersion(msgBuf, chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = "EncodeIterator.setBufferAndRWFVersion() failed with return code: " + ret
                };
                return TransportReturnCode.FAILURE;
            }

            ret = dictionaryRequest.Encode(encodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = "EncodeDictionaryRequest(): Failed <code: " + ret + ">"
                };
                return TransportReturnCode.FAILURE;
            }

            Console.WriteLine(dictionaryRequest.ToString());

            return chnl.Write(msgBuf, out error);
        }

        /// <summary>
        /// Processes response message.
        /// </summary>
        /// <param name="msg">The partially decoded message.</param>
        /// <param name="dIter">The <see cref="DecodeIterator"/> instance.</param>
        /// <param name="error"><see cref="Error"/> instance that carries information about failure 
        /// in case of nusuccessful operation.</param>
        /// <returns><see cref="TransportReturnCode"/> value indicating the status of the operation.</returns>
        public TransportReturnCode ProcessResponse(Msg msg, DecodeIterator dIter, out Error? error)
        {
            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH:
                    return ProcessRefresh(msg, dIter, out error);
                case MsgClasses.STATUS:
                    Console.WriteLine("Received StatusMsg for dictionary");
                    IStatusMsg statusMsg = msg;
                    if (statusMsg.CheckHasState())
                    {
                        Console.WriteLine("   " + statusMsg.State);
                        State state = statusMsg.State;
                        if (msg.StreamId == FIELD_DICTIONARY_STREAM_ID)
                        {
                            states[0].DataState(state.DataState());
                            states[0].StreamState(state.StreamState());
                        }
                        else if (msg.StreamId == ENUM_TYPE_DICTIONARY_STREAM_ID)
                        {
                            states[1].DataState(state.DataState());
                            states[1].StreamState(state.StreamState());
                        }
                    }
                    break;

                default:
                    Console.WriteLine("Received Unhandled Dictionary MsgClass: " + msg.MsgClass);
                    break;
            }

            error = null;
            return TransportReturnCode.SUCCESS;

        }

        TransportReturnCode ProcessRefresh(Msg msg, DecodeIterator dIter, out Error? error)
        {
            CodecReturnCode ret = dictionaryRefresh.Decode(dIter, msg);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = "Error decoding dictionary refresh: <" + ret + ">"
                };
                return TransportReturnCode.FAILURE;
            }

            if (dictionaryRefresh.HasInfo)
            {
                switch (dictionaryRefresh.DictionaryType)
                {
                    case Dictionary.Types.FIELD_DEFINITIONS:
                        break;
                    case Dictionary.Types.ENUM_TABLES:
                        break;
                    default:
                        error = new Error()
                        {
                            Text = "Received unexpected dictionary message on stream " + msg.StreamId
                        };
                        return TransportReturnCode.FAILURE;
                }
            }
            CodecError codecError = new CodecError();

            if (dictionaryRefresh.StreamId == FIELD_DICTIONARY_STREAM_ID)
            {
                Console.WriteLine("Received Dictionary Refresh for field dictionary");

                IRefreshMsg refreshMsg = msg;

                states[0].DataState(refreshMsg.State.DataState());
                states[0].StreamState(refreshMsg.State.StreamState());

                ret = DataDictionary.DecodeFieldDictionary(dIter, Dictionary.VerbosityValues.VERBOSE, out codecError);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    error = new Error()
                    {
                        Text = "Unable to decode Field Dictionary"
                    };
                    return TransportReturnCode.FAILURE;
                }

                if (dictionaryRefresh.RefreshComplete)
                {
                    bfieldDictionaryDownloaded = true;
                    if (!bEnumTypeDictionaryLoaded)
                    {
                        Console.WriteLine("Field Dictionary complete, waiting for Enum Table...");
                    }                       
                }
            }
            else if (dictionaryRefresh.StreamId == ENUM_TYPE_DICTIONARY_STREAM_ID)
            {
                Console.WriteLine("Received Dictionary Refresh for enum type");

                IRefreshMsg refreshMsg = msg;

                states[1].DataState(refreshMsg.State.DataState());
                states[1].StreamState(refreshMsg.State.StreamState());

                ret = DataDictionary.DecodeEnumTypeDictionary(dIter, Dictionary.VerbosityValues.VERBOSE, out codecError);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    error = new Error()
                    {
                        Text = "Unable to decode Enum Dictionary"
                    };
                    return TransportReturnCode.FAILURE;
                }

                if (dictionaryRefresh.RefreshComplete)
                {
                    bEnumTypeDictionaryLoaded = true;
                    if (!bfieldDictionaryDownloaded)
                        Console.WriteLine("Enumerated Types Dictionary complete, waiting for Field Dictionary...");
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

            if (bfieldDictionaryDownloaded && bEnumTypeDictionaryLoaded)
                Console.WriteLine("Dictionary Download complete for both field and enum type dictionaries");

            error = null;
            return TransportReturnCode.SUCCESS;
        }

        private bool CloseDictionary(ChannelSession chnl, int streamId, out Error? error)
        {
            ITransportBuffer? msgBuf = chnl.GetTransportBuffer(ChannelSession.MAX_MSG_SIZE, false, out error);
            if (msgBuf == null)
            {
                return false;
            }

            encodeIter.Clear();
            encodeIter.SetBufferAndRWFVersion(msgBuf, chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);

            dictionaryClose.Clear();
            dictionaryClose.StreamId = streamId;

            CodecReturnCode ret = dictionaryClose.Encode(encodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = "encodeDictionaryClose(): Failed <code: " + ret + ">"
                };
                return false;
            }

            TransportReturnCode wret = chnl.Write(msgBuf, out error);
            if (wret != TransportReturnCode.SUCCESS)
            {
                return false;
            }

            DataDictionary.Clear();
            return true;
        }

        /// <summary>
        /// Closes the stream.
        /// </summary>
        /// <param name="channel">The channel to which close message is sent.</param>
        /// <param name="error"></param>
        public void CloseStream(ChannelSession channel, out Error? error)
        {
            error = null;
            if (bfieldDictionaryDownloaded)
            {
                if (states[0].DataState() == DataStates.OK && states[0].StreamState() == StreamStates.NON_STREAMING)
                {
                    if (CloseDictionary(channel, FIELD_DICTIONARY_STREAM_ID, out error) == true)
                    {
                        bfieldDictionaryDownloaded = false;
                        states[0].Clear();
                    }
                }
            }
            if (bEnumTypeDictionaryLoaded)
            {
                if (states[1].DataState() == DataStates.OK && states[1].StreamState() == StreamStates.NON_STREAMING)
                {
                    if (CloseDictionary(channel, ENUM_TYPE_DICTIONARY_STREAM_ID, out error) == true)
                    {
                        bEnumTypeDictionaryLoaded = false;
                        states[1].Clear();
                    }
                }
            }
        }

    }
}
