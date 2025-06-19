/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;


namespace LSEG.Eta.ValueAdd.Consumer
{

    /// <summary>
    /// This is the yield curve handler for the ETA Value Add consumer application.
    /// </summary>
    ///
    /// <remarks>
    /// It provides methods for sending the yield curve request(s) to a provider
    /// and processing the response(s).
    ///
    /// Methods for decoding vectors containing field lists and/or arrays, decoding
    /// field entries from a response, closing yield curve streams, and
    /// adding/removing items to/from the item list are also provided.
    ///
    /// Methods for decoding a field entry from a response, and closing market
    /// price streams are also provided.
    /// </remarks>
    internal class YieldCurveHandler
    {
        public int LoginStreamId { get; internal set; }

        /// <summary>
        /// Used to enable snapshot requesting. Send a set of
        /// items as a snapshot request.
        /// </summary>
        public bool SnapshotRequest { get; internal set; }

        private const int TRANSPORT_BUFFER_SIZE_REQUEST = 1000;
        private const int TRANSPORT_BUFFER_SIZE_CLOSE = 1000;

        private int m_DomainType;

        /// <summary>
        /// Channel to use for private stream redirect
        /// </summary>
        private ReactorChannel? m_RedirectChnl;

        /// <summary>
        /// Login information to use for private stream redirect
        /// </summary>
        private LoginRefresh? m_RedirectLoginInfo;

        /// <summary>
        /// Source directory information to use for private stream redirect
        /// </summary>
        private Service? m_RedirectSrcDirInfo;
        private YieldCurveRequest m_YieldCurveRequest;
        private MarketPriceClose m_CloseMessage;
        private readonly StreamIdWatchList m_WatchList;

        // temp. reusable variables used for encoding
        private FieldList fieldList = new();
        private FieldEntry fieldEntry = new();
        private FieldList embeddedFieldList = new();
        private FieldEntry embeddedFieldEntry = new();
        private VectorEntry vectorEntry = new();
        private ArrayEntry arrayEntry = new();
        private LocalFieldSetDefDb m_LocalFieldSetDefDb = new();
        private Int fidIntValue = new();
        private Real fidRealValue = new();
        private Codec.Date fidDateValue = new();
        private Time fidTimeValue = new();
        private Codec.DateTime fidDateTimeValue = new();
        private Vector fidVectorValue = new();
        private Codec.Array fidArrayValue = new();
        private Codec.Buffer fidBufferValue = new();
        private EncodeIterator m_EncodeIterator = new();
        private StringBuilder m_FieldValue = new StringBuilder();

        private int m_IndentCount;
        private readonly string[] INDENTS = { "", "    ", "        ", "            " };

        private ReactorSubmitOptions m_SubmitOptions = new();


        public YieldCurveHandler(StreamIdWatchList watchList) : this(DomainType.YIELD_CURVE, watchList)
        {
        }

        private YieldCurveHandler(DomainType domainType, StreamIdWatchList watchList)
        {
            m_WatchList = watchList;
            m_DomainType = (int)domainType;
            m_YieldCurveRequest = CreateYieldCurveRequest();
            m_CloseMessage = new MarketPriceClose();
        }

        private YieldCurveRequest CreateYieldCurveRequest()
        {
            return new YieldCurveRequest();
        }


        private void RemoveYieldCurveItemEntry(int streamId)
        {
            m_WatchList.Remove(streamId);
        }

        private bool HasYieldCurveCapability(List<long> capabilities)
        {
            return capabilities.Any((capability) => capability == m_YieldCurveRequest.DomainType);
        }


        /// <summary>
        /// Encodes and sends item requests for yield curve domains
        /// </summary>
        public ReactorReturnCode SendItemRequests(ReactorChannel chnl, List<string> itemNames,
                bool isPrivateStream, LoginRefresh loginInfo,
                Service serviceInfo, out ReactorErrorInfo? errorInfo)
        {
            if (itemNames == null || itemNames.Count == 0)
            {
                errorInfo = null;
                return ReactorReturnCode.SUCCESS;
            }

            /* check to see if the provider supports the market price domain */
            if (!HasYieldCurveCapability(serviceInfo.Info.CapabilitiesList))
            {
                errorInfo = new();
                errorInfo.Error.Text = "'"
                        + DomainTypes.ToString(m_YieldCurveRequest.DomainType)
                        + "' not supported by the indicated provider";
                return ReactorReturnCode.FAILURE;
            }

            /* set redirect channel for private stream redirect */
            m_RedirectChnl = chnl;

            /* set login information for private stream redirect */
            m_RedirectLoginInfo = loginInfo;

            /* set source directory information for private stream redirect */
            m_RedirectSrcDirInfo = serviceInfo;

            GenerateRequest(m_YieldCurveRequest, isPrivateStream, serviceInfo);

            // If there is only one item in the itemList, it is a waste of bandwidth
            // to send a batch request
            if (itemNames.Count == 1)
            {
                return SendRequest(chnl, itemNames, out errorInfo);
            }

            if (!(loginInfo.HasFeatures
                && loginInfo.SupportedFeatures.HasSupportBatchRequests
                && loginInfo.SupportedFeatures.SupportBatchRequests == 1))
            {
                Console.WriteLine("Connected Provider does not support Batch Requests. Sending Market Price requests as individual request messages.");
                return SendRequest(chnl, itemNames, out errorInfo);
            }

            // batch
            return SendBatchRequest(chnl, itemNames, out errorInfo);
        }

        private void GenerateRequest(YieldCurveRequest yieldCurveRequest,
                bool isPrivateStream, Service srcDirInfo)
        {
            yieldCurveRequest.Clear();

            if (!SnapshotRequest)
                yieldCurveRequest.Streaming = true;
            yieldCurveRequest.HasServiceId = true;
            yieldCurveRequest.ServiceId = srcDirInfo.ServiceId;
            yieldCurveRequest.HasPriority = true;
            yieldCurveRequest.PriorityClass = 1;
            yieldCurveRequest.PriorityCount = 1;
            yieldCurveRequest.HasQos = true;
            yieldCurveRequest.Qos.IsDynamic = false;
            yieldCurveRequest.Qos.TimeInfo(srcDirInfo.Info.QosList[0].TimeInfo());
            yieldCurveRequest.Qos.Timeliness(srcDirInfo.Info.QosList[0].Timeliness());
            yieldCurveRequest.Qos.RateInfo(srcDirInfo.Info.QosList[0].RateInfo());
            yieldCurveRequest.Qos.Rate(srcDirInfo.Info.QosList[0].Rate());
            if (isPrivateStream)
                yieldCurveRequest.IsPrivateStream = true;
        }

        /// <summary>
        /// Sends items as batch request
        /// </summary>
        private ReactorReturnCode SendBatchRequest(ReactorChannel chnl, List<string> itemNames, out ReactorErrorInfo? errorInfo)
        {
            int batchStreamId = m_WatchList.Add(m_DomainType, "BATCH_" + System.DateTime.Now,
                    m_YieldCurveRequest.IsPrivateStream);

            m_YieldCurveRequest.StreamId = batchStreamId;

            foreach (string itemName in itemNames)
            {
                m_WatchList.Add(m_DomainType, itemName, m_YieldCurveRequest.IsPrivateStream);
                m_YieldCurveRequest.ItemNames.Add(itemName);
            }

            return EncodeAndSendRequest(chnl, m_YieldCurveRequest, out errorInfo);
        }

        /// <summary>
        /// Sends one item at a time
        /// </summary>
        private ReactorReturnCode SendRequest(ReactorChannel chnl, List<String> itemNames, out ReactorErrorInfo? errorInfo)
        {
            foreach (string itemName in itemNames)
            {
                int streamId = m_WatchList.Add(m_DomainType, itemName, m_YieldCurveRequest.IsPrivateStream);

                m_YieldCurveRequest.ItemNames.Clear();
                m_YieldCurveRequest.ItemNames.Add(itemName);

                m_YieldCurveRequest.StreamId = streamId;
                ReactorReturnCode ret = EncodeAndSendRequest(chnl, m_YieldCurveRequest, out errorInfo);
                if (ret < ReactorReturnCode.SUCCESS)
                    return ret;
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        private ReactorReturnCode EncodeAndSendRequest(ReactorChannel chnl, YieldCurveRequest yieldCurveRequest, out ReactorErrorInfo? errorInfo)
        {
            // get a buffer for the item request
            ITransportBuffer? msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, out errorInfo);

            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
            }

            m_EncodeIterator.Clear();
            m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);

            CodecReturnCode ret = yieldCurveRequest.Encode(m_EncodeIterator);
            if (ret < CodecReturnCode.SUCCESS)
            {
                if(errorInfo is null)
                {
                    errorInfo = new ReactorErrorInfo();
                }
                errorInfo.Error.Text = "YieldCurveRequest.Encode() failed";
                errorInfo.Error.ErrorId = (TransportReturnCode)ret;
                return (ReactorReturnCode)ret;
            }

            Console.WriteLine(yieldCurveRequest.ToString());
            return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }


        /// <summary>
        /// Processes a yield curve response.
        /// </summary>
        ///
        /// <remarks>
        /// This consists of extracting the key, printing out the item name
        /// contained in the key, decoding the field list and field entry, and
        /// calling DecodeFieldEntry() to decode the field entry data.
        /// </remarks>
        public ReactorReturnCode ProcessResponse(Msg msg, DecodeIterator dIter, DataDictionary dictionary, out ReactorErrorInfo? errorInfo)
        {
            m_IndentCount = 0;

            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH:
                    return HandleRefresh(msg, dIter, dictionary, out errorInfo);
                case MsgClasses.UPDATE:
                    return HandleUpdate(msg, dIter, dictionary, out errorInfo);
                case MsgClasses.STATUS:
                    return HandleStatus(msg, out errorInfo);
                case MsgClasses.ACK:
                    errorInfo = null;
                    return HandleAck(msg);
                default:
                    Console.WriteLine("Received Unhandled Item Msg Class: " + msg.MsgClass);
                    break;
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        private ReactorReturnCode HandleAck(Msg msg)
        {
            Console.WriteLine("Received AckMsg for stream " + msg.StreamId);

            m_FieldValue.Length = 0;
            GetItemName(msg, m_FieldValue);

            IAckMsg ackMsg = (IAckMsg)msg;

            m_FieldValue.Append("\tackId=" + ackMsg.AckId);
            if (ackMsg.CheckHasSeqNum())
                m_FieldValue.Append("\tseqNum=" + ackMsg.SeqNum);
            if (ackMsg.CheckHasNakCode())
                m_FieldValue.Append("\tnakCode=" + ackMsg.NakCode);
            if (ackMsg.CheckHasText())
                m_FieldValue.Append("\ttext=" + ackMsg.Text.ToString());

            Console.WriteLine(m_FieldValue.ToString());
            return ReactorReturnCode.SUCCESS;
        }

        private void GetItemName(Msg msg, StringBuilder fieldValue)
        {
            // get key
            IMsgKey key = msg.MsgKey;

            // print out item name from key if it has it
            if (key != null && key.CheckHasName())
            {
                if (msg.MsgClass == MsgClasses.REFRESH)
                {
                    IRefreshMsg refreshMsg = (IRefreshMsg)msg;
                    fieldValue.Append(key.Name.ToString()
                                    + (refreshMsg.CheckPrivateStream() ? " (PRIVATE STREAM)" : "") + "\nDOMAIN: "
                                    + DomainTypes.ToString(msg.DomainType) + "\n");
                }
                else
                {
                    fieldValue.Append(key.Name.ToString() + "\nDOMAIN: "
                            + DomainTypes.ToString(msg.DomainType) + "\n");
                    if (msg.MsgClass == MsgClasses.UPDATE)
                    {
                        fieldValue.Append("UPDATE TYPE: "
                                + UpdateEventTypes.ToString(((IUpdateMsg)msg).UpdateType) + "\n");
                    }
                }
            }
            else
            {
                // cached item name
                var wle = m_WatchList.Get(msg.StreamId);

                if (wle != null)
                {
                    fieldValue.Append(wle.ItemName
                            + (wle.IsPrivateStream ? " (PRIVATE STREAM)" : " ")
                            + "\nDOMAIN: " + DomainTypes.ToString(msg.DomainType)
                            + "\n");
                    if (msg.MsgClass == MsgClasses.UPDATE)
                    {
                        fieldValue.Append("UPDATE TYPE: "
                                + UpdateEventTypes.ToString(((IUpdateMsg)msg).UpdateType) + "\n");
                    }
                }
                else
                {
                    // check if this is login stream for offstream posting
                    if (msg.StreamId == LoginStreamId)
                    {
                        fieldValue.Append("OFFPOST \nDOMAIN: "
                                + DomainTypes.ToString(msg.DomainType) + "\n");
                    }
                }
            }
        }

        private ReactorReturnCode HandleStatus(Msg msg, out ReactorErrorInfo? errorInfo)
        {
            IStatusMsg statusMsg = (IStatusMsg)msg;
            Console.WriteLine($"Received Item StatusMsg for stream {msg.StreamId}");
            if (!statusMsg.CheckHasState())
            {
                errorInfo = null;
                return ReactorReturnCode.SUCCESS;
            }

            // get state information
            State state = statusMsg.State;
            Console.WriteLine($"\t{state}");

            var wle = m_WatchList.Get(msg.StreamId);
            if (wle != null)
            {
                /* update our state table with the new state */
                if (!statusMsg.CheckPrivateStream()) /* non-private stream */
                {
                    /*
                     * check if this response should be on private stream but is not
                     */
                    if (!statusMsg.CheckPrivateStream()) /* non-private stream */
                    {
                        /*
                         * check if this response should be on private stream but is
                         * not batch responses for private stream may be sent on
                         * non-private stream
                         */
                        /* if this is the case, close the stream */
                        if (wle.IsPrivateStream
                            && !wle.ItemName.Contains("BATCH_")
                            && m_RedirectChnl != null)
                        {
                            Console.WriteLine("Received non-private response for stream "
                                            + msg.StreamId
                                            + " that should be private - closing stream");
                            // close stream
                            CloseStream(m_RedirectChnl, msg.StreamId, out errorInfo);
                            // remove private stream entry from list
                            RemoveYieldCurveItemEntry(msg.StreamId);

                            return ReactorReturnCode.FAILURE;
                        }
                    }
                }
                wle.ItemState.DataState(statusMsg.State.DataState());
                wle.ItemState.StreamState(statusMsg.State.StreamState());
            }

            // redirect to private stream if indicated
            if (statusMsg.State.StreamState() == StreamStates.REDIRECTED
                && (statusMsg.CheckPrivateStream()))
            {
                ReactorReturnCode ret = RedirectToPrivateStream(msg.StreamId, out errorInfo);
                if (ret < ReactorReturnCode.SUCCESS)
                    return ret;
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        private ReactorReturnCode HandleUpdate(Msg msg, DecodeIterator dIter, DataDictionary dictionary, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            return (ReactorReturnCode)Decode(msg, dIter, dictionary);
        }

        private ReactorReturnCode HandleRefresh(Msg msg, DecodeIterator dIter, DataDictionary dictionary, out ReactorErrorInfo? errorInfo)
        {
            IRefreshMsg refreshMsg = (IRefreshMsg)msg;
            var wle = m_WatchList.Get(msg.StreamId);
            if (wle == null)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = "No such entry in watchlist";
                return ReactorReturnCode.FAILURE;
            }

            /* check if this response should be on private stream but is not */
            /* if this is the case, close the stream */
            if (!refreshMsg.CheckPrivateStream()
                && wle.IsPrivateStream
                && m_RedirectChnl != null)
            {
                Console.WriteLine("Received non-private response for stream "
                        + msg.StreamId
                        + " that should be private - closing stream");
                // close stream
                CloseStream(m_RedirectChnl, msg.StreamId, out _);

                // remove private stream entry from list
                RemoveYieldCurveItemEntry(msg.StreamId);

                errorInfo = new();
                errorInfo.Error.Text = "Received non-private response for stream "
                        + msg.StreamId
                        + " that should be private - closing stream";
                return ReactorReturnCode.FAILURE;
            }
            /*
             * update our item state list if its a refresh, then process just like
             * update
             */
            wle.ItemState.DataState(refreshMsg.State.DataState());
            wle.ItemState.StreamState(refreshMsg.State.StreamState());

            errorInfo = null;
            return (ReactorReturnCode)Decode(msg, dIter, dictionary);
        }

        private CodecReturnCode Decode(Msg msg, DecodeIterator dIter, DataDictionary dictionary)
        {
            CodecReturnCode ret = 0;

            m_FieldValue.Length = 0;
            GetItemName(msg, m_FieldValue);
            Console.Write(m_FieldValue);

            if (msg.MsgClass == MsgClasses.REFRESH)
                Console.WriteLine((((IRefreshMsg)msg).State).ToString());
            else
                Console.WriteLine();

            ret = DecodeFieldList(dIter, dictionary, fieldList, fieldEntry);

            Console.WriteLine();
            return ret;
        }


        public CodecReturnCode DecodePayload(DecodeIterator dIter, DataDictionary dictionary)
        {
            m_IndentCount = 0;
            return DecodeFieldList(dIter, dictionary, fieldList, fieldEntry);
        }


        private CodecReturnCode DecodeFieldList(DecodeIterator dIter, DataDictionary dictionary, FieldList localFieldList, FieldEntry localFieldEntry)
        {
            localFieldList.Clear();

            CodecReturnCode ret = localFieldList.Decode(dIter, m_LocalFieldSetDefDb);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("DecodeFieldList() failed with return code: " + ret);
                return ret;
            }

            localFieldEntry.Clear();

            m_IndentCount++;

            // decode each field entry in list
            while ((ret = localFieldEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret < CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("DecodeFieldEntry() failed with return code: " + ret);
                    return ret;
                }
                // get dictionary entry
                var dictionaryEntry = dictionary.Entry(localFieldEntry.FieldId);

                // return if no entry found
                if (dictionaryEntry == null)
                {
                    Console.WriteLine("\tFid " + localFieldEntry.FieldId + " not found in dictionary");
                    Console.WriteLine(localFieldEntry.EncodedData.ToHexString());
                    return CodecReturnCode.SUCCESS;
                }

                // print out fid name
                Console.Write(INDENTS[m_IndentCount] + dictionaryEntry.GetAcronym().ToString());
                for (int i = 0; i < 40 - INDENTS[m_IndentCount].Length - dictionaryEntry.GetAcronym().Length; i++)
                {
                    Console.Write(" ");
                }

                // decode and print out fid value
                int dataType = dictionaryEntry.GetRwfType();

                switch (dataType)
                {
                    case DataTypes.VECTOR:
                        ret = DecodeVector(dIter, dictionary);
                        if (ret < CodecReturnCode.SUCCESS)
                        {
                            Console.WriteLine("DecodeVector inside FieldList failed");
                            return ret;
                        }
                        break;
                    case DataTypes.ARRAY:
                        ret = DecodeArray(dIter);
                        if (ret < CodecReturnCode.SUCCESS)
                        {
                            Console.WriteLine("DecodeArray inside FieldList failed");
                            return ret;
                        }
                        break;
                    default:
                        ret = DecodePrimitive(dIter, dataType, false);
                        if (ret < CodecReturnCode.SUCCESS)
                        {
                            Console.WriteLine("DecodePrimitive inside FieldList failed");
                            return ret;
                        }
                        break;
                }
            }
            m_IndentCount--;
            return CodecReturnCode.SUCCESS;
        }


        private CodecReturnCode DecodeVector(DecodeIterator dIter, DataDictionary dictionary)
        {
            CodecReturnCode ret = fidVectorValue.Decode(dIter);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"DecodeReal() failed: <{ret}>");
                return ret;
            }
            if (ret == CodecReturnCode.NO_DATA)
            {
                Console.WriteLine("<no data>");
                return CodecReturnCode.SUCCESS;
            }
            if (fidVectorValue.CheckHasSummaryData())
            {
                Console.WriteLine();
                // fieldList inside summaryData within vector
                ret = DecodeFieldList(dIter, dictionary, embeddedFieldList, embeddedFieldEntry);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine($"DecodeSummaryData failed: <{ret}>");
                    return ret;
                }
            }
            // If the vector flags indicate that set definition content is present,
            // decode the set def db
            if (fidVectorValue.CheckHasSetDefs())
            {
                if (fidVectorValue.ContainerType == DataTypes.FIELD_LIST)
                {
                    m_LocalFieldSetDefDb.Clear();
                    ret = m_LocalFieldSetDefDb.Decode(dIter);
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        Console.WriteLine($"DecodeSetDefDb() failed: <{ret}>");
                        return ret;
                    }
                }
            }

            m_IndentCount++;
            Console.WriteLine();

            vectorEntry.Clear();
            while ((ret = vectorEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret < CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("Error {0} ({1}) encountered with DecodeVectorEntry.  Error Text: {2}",
                                      ret, (int)ret, ret);
                    return ret;
                }

                Console.WriteLine(INDENTS[m_IndentCount] + "INDEX: " + vectorEntry.Index);
                Console.Write(INDENTS[m_IndentCount] + "ACTION: ");
                switch (vectorEntry.Action)
                {
                    case VectorEntryActions.UPDATE:
                        Console.WriteLine("UPDATE_ENTRY");
                        break;
                    case VectorEntryActions.SET:
                        Console.WriteLine("SET_ENTRY");
                        break;
                    case VectorEntryActions.CLEAR:
                        Console.WriteLine("CLEAR_ENTRY");
                        break;
                    case VectorEntryActions.INSERT:
                        Console.WriteLine("INSERT_ENTRY");
                        break;
                    case VectorEntryActions.DELETE:
                        Console.WriteLine("DELETE_ENTRY");
                        break;
                    default:
                        Console.WriteLine("UNKNOWN");
                        break;
                }

                /* Continue decoding vector entries. */
                switch (fidVectorValue.ContainerType)
                {
                    case DataTypes.FIELD_LIST:
                        // fieldList inside vectorEntry within vector
                        ret = DecodeFieldList(dIter, dictionary, embeddedFieldList, embeddedFieldEntry);
                        if (ret < CodecReturnCode.SUCCESS)
                        {
                            Console.WriteLine("Error {0} ({1}) encountered with decoding FieldList within Vector: {2}",
                                            ret, (int)ret, ret.GetAsInfo());
                            return ret;
                        }
                        break;
                    case DataTypes.ARRAY:
                        ret = DecodeArray(dIter);
                        if (ret < CodecReturnCode.SUCCESS)
                        {
                            Console.WriteLine("Error {0} ({1}) encountered with decoding ARRAY within Vector: {2}",
                                            ret, (int)ret, ret.GetAsInfo());
                            return ret;
                        }
                        break;
                    default:
                        Console.WriteLine("Error: Vector contained unhandled containerType " + fidVectorValue.ContainerType);
                        break;
                }

            }
            m_IndentCount--;
            return CodecReturnCode.SUCCESS;
        }


        private CodecReturnCode DecodePrimitive(DecodeIterator dIter, int dataType, bool isArray)
        {
            CodecReturnCode ret = 0;

            switch (dataType)
            {
                case DataTypes.INT:
                    ret = fidIntValue.Decode(dIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        Console.Write(fidIntValue.ToLong());
                    }
                    else if (ret != CodecReturnCode.BLANK_DATA)
                    {
                        Console.WriteLine($"DecodeInt() failed: <{ret}>");
                        return ret;
                    }
                    break;
                case DataTypes.REAL:
                    ret = fidRealValue.Decode(dIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        Console.Write(fidRealValue.ToDouble());
                    }
                    else if (ret != CodecReturnCode.BLANK_DATA)
                    {
                        Console.WriteLine($"DecodeReal() failed: <{ret}>");
                        return ret;
                    }
                    break;
                case DataTypes.DATE:
                    ret = fidDateValue.Decode(dIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        Console.Write(fidDateValue.ToString());
                    }
                    else if (ret != CodecReturnCode.BLANK_DATA)
                    {
                        Console.WriteLine($"DecodeDate() failed: <{ret}>");
                        return ret;
                    }
                    break;
                case DataTypes.TIME:
                    ret = fidTimeValue.Decode(dIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        Console.Write(fidTimeValue.ToString());
                    }
                    else if (ret != CodecReturnCode.BLANK_DATA)
                    {
                        Console.WriteLine($"DecodeTime() failed: <{ret}>");
                        return ret;
                    }
                    break;
                case DataTypes.DATETIME:
                    ret = fidDateTimeValue.Decode(dIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        Console.Write(fidDateTimeValue.ToString());
                    }
                    else if (ret != CodecReturnCode.BLANK_DATA)
                    {
                        Console.WriteLine($"DecodeDateTime() failed: <{ret}>");
                        return ret;
                    }
                    break;
                case DataTypes.ARRAY:
                    ret = DecodeArray(dIter);
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        Console.WriteLine("Error {0} ({1}) encountered with decoding ARRAY was primitive: {2}",
                                                ret, (int)ret, ret.GetAsInfo());
                        return ret;
                    }
                    break;
                case DataTypes.BUFFER:
                case DataTypes.ASCII_STRING:
                case DataTypes.UTF8_STRING:
                case DataTypes.RMTES_STRING:
                    ret = fidBufferValue.Decode(dIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        if (isArray)
                            Console.Write("\"");
                        Console.Write(fidBufferValue.ToString());
                        if (isArray)
                            Console.Write("\"");
                    }
                    else if (ret != CodecReturnCode.BLANK_DATA)
                    {
                        Console.WriteLine($"DecodeString() failed: <{ret}>");
                        return ret;
                    }
                    break;
                default:
                    Console.Write("Unsupported data type (" + DataTypes.ToString(dataType) + ")");
                    break;
            }
            if (ret == CodecReturnCode.BLANK_DATA)
            {
                Console.Write("<blank data>");
            }

            if (!isArray)
                Console.Write("\n");

            return CodecReturnCode.SUCCESS;
        }


        private CodecReturnCode DecodeArray(DecodeIterator dIter)
        {
            bool firstArrayEntry = true;

            Console.Write("{ ");

            fidArrayValue.Clear();
            CodecReturnCode ret = fidArrayValue.Decode(dIter);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"DecodeArray() failed: <{ret}>");
                return ret;
            }

            int dataType = fidArrayValue.PrimitiveType;

            arrayEntry.Clear();
            while ((ret = arrayEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret < CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("Error {0} ({1}) encountered with DecodeArrayEntry.  Error Text: {2}",
                                    ret, (int)ret, ret.GetAsInfo());
                    return ret;
                }

                if (firstArrayEntry)
                    firstArrayEntry = false;
                else
                    Console.Write(", ");
                ret = DecodePrimitive(dIter, dataType, true);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("Error {0} ({1}) encountered with DecodeArrayEntryPrimitives.  Error Text: {2}",
                                    ret, (int)ret, ret.GetAsInfo());
                    return ret;
                }
            }

            Console.Write(" }\n");

            return CodecReturnCode.SUCCESS;
        }


        private ReactorReturnCode CloseStream(ReactorChannel chnl, int streamId, out ReactorErrorInfo? errorInfo)
        {
            /* get a buffer for the item close */
            ITransportBuffer? msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, out errorInfo);
            if (msgBuf == null)
                return ReactorReturnCode.FAILURE;

            /* encode item close */
            m_CloseMessage.Clear();
            m_CloseMessage.StreamId = streamId;
            m_CloseMessage.DomainType = m_DomainType;
            m_EncodeIterator.Clear();
            m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);

            CodecReturnCode ret = m_CloseMessage.Encode(m_EncodeIterator);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeYieldCurveClose(): Failed <{ret}>");
            }
            return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }


        /// <summary>
        /// Close all item streams.
        /// </summary>
        public ReactorReturnCode CloseStreams(ReactorChannel chnl, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret;

            foreach (var entry in m_WatchList)
            {
                /*
                 * we only want to close a stream if it was not already closed (e.g.
                 * rejected by provider, closed via refresh or status, or
                 * redirected)
                 */
                if (entry.Value.ItemState.IsFinal())
                    continue;

                if (entry.Value.DomainType == m_DomainType)
                {
                    ret = CloseStream(chnl, entry.Key, out errorInfo);
                    if (ret < ReactorReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                    // .NET Core versions 3.0+ removes entries from Dictionary
                    // without invalidating active enumerators
                    m_WatchList.Remove(entry.Key);
                }
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }


        /// <summary>
        /// Redirect a request to a private stream. streamId is the stream id to be
        /// redirected to private stream.
        /// </summary>
        private ReactorReturnCode RedirectToPrivateStream(int streamId, out ReactorErrorInfo? errorInfo)
        {
            var wle = m_WatchList.Get(streamId);
            if (wle == null)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = "No such entry in watchlist";
                return ReactorReturnCode.FAILURE;
            }

            /* remove non-private stream entry from list */
            RemoveYieldCurveItemEntry(streamId);

            /* add item name to private stream list */
            int psStreamId = m_WatchList.Add(m_DomainType, wle.ItemName, true);

            GenerateRequest(m_YieldCurveRequest, true, m_RedirectSrcDirInfo!);
            m_YieldCurveRequest.ItemNames.Add(wle.ItemName);
            m_YieldCurveRequest.StreamId = psStreamId;
            return EncodeAndSendRequest(m_RedirectChnl!, m_YieldCurveRequest, out errorInfo);
        }
    }
}
