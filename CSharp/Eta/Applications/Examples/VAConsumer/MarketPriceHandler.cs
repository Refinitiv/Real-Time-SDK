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
    /// This is the market price handler for the ETA Value Add consumer application.
    /// </summary>
    ///
    /// It provides methods for sending the market price request(s) to a provider
    /// and processing the response(s). Methods for decoding a field entry from a
    /// response, and closing market price streams are also provided.
    internal class MarketPriceHandler
    {
        public int LoginStreamId { get; internal set; }
        public bool SnapshotRequest { get; internal set; }
        public bool ViewRequest { get; internal set; }

        private const int TRANSPORT_BUFFER_SIZE_REQUEST = 1000;
        private const int TRANSPORT_BUFFER_SIZE_CLOSE = 1000;

        private int m_DomainType;

        /// <summary>
        /// Channel to use for private stream redirect
        /// </summary>
        protected ReactorChannel? m_RedirectChannel;

        /// <summary>
        /// Login information to use for private stream redirect
        /// </summary>
        private LoginRefresh? m_RedirectLoginInfo;

        /// <summary>
        /// Source directory information to use for private stream redirect
        /// </summary>
        private Service? m_RedirectSrcDirInfo;

        private MarketPriceRequest m_MarketPriceRequest;

        private MarketPriceClose m_CloseMessage;

        protected StreamIdWatchList m_WatchList;

        private EncodeIterator m_EncodeIterator = new();

        private List<string> m_ViewFieldList;

        private ReactorSubmitOptions m_SubmitOptions = new();

        protected FieldList fieldList = new();
        protected FieldEntry fieldEntry = new();
        private UInt fidUIntValue = new();
        private Int fidIntValue = new();
        private Real fidRealValue = new();
        private Codec.Enum fidEnumValue = new();
        private Codec.Date fidDateValue = new();
        private Time fidTimeValue = new();
        private Codec.DateTime fidDateTimeValue = new();
        private Codec.Float fidFloatValue = new();
        private Codec.Double fidDoubleValue = new();
        private Qos fidQosValue = new();
        private State fidStateValue = new();


        public MarketPriceHandler(StreamIdWatchList watchList) : this(DomainType.MARKET_PRICE, watchList)
        { }

        protected MarketPriceHandler(DomainType domainType, StreamIdWatchList watchList)
        {
            m_WatchList = watchList;
            m_DomainType = (int)domainType;
            m_MarketPriceRequest = CreateMarketPriceRequest();
            m_CloseMessage = new MarketPriceClose();
            m_ViewFieldList = new() { "6", "22", "25", "32" };
        }

        protected virtual MarketPriceRequest CreateMarketPriceRequest()
        {
            return new MarketPriceRequest();
        }

        public ReactorReturnCode SendItemRequests(ReactorChannel chnl, List<string> itemNames, bool isPrivateStream,
            LoginRefresh loginInfo, Service serviceInfo, out ReactorErrorInfo? errorInfo)
        {
            if (itemNames == null || itemNames.Count == 0)
            {
                errorInfo = null;
                return ReactorReturnCode.SUCCESS;
            }

            /* check to see if the provider supports the market price domain */
            if (!HasMarketPriceCapability(serviceInfo.Info.CapabilitiesList))
            {
                errorInfo = new();
                errorInfo.Error.Text = $"'{(DomainType)m_MarketPriceRequest.DomainType}' not supported by the indicated provider";
                return ReactorReturnCode.FAILURE;
            }

            /* set redirect channel for private stream redirect */
            m_RedirectChannel = chnl;

            /* set login information for private stream redirect */
            m_RedirectLoginInfo = loginInfo;

            /* set source directory information for private stream redirect */
            m_RedirectSrcDirInfo = serviceInfo;

            GenerateRequest(m_MarketPriceRequest, isPrivateStream, serviceInfo, loginInfo);

            // If there is only one item in the itemList, it is a waste of bandwidth
            // to send a batch request
            if (itemNames.Count == 1)
            {
                return SendRequest(chnl, itemNames, out errorInfo);
            }
            else
            {
                if (!(loginInfo.HasFeatures
                    && loginInfo.SupportedFeatures.HasSupportBatchRequests
                    && loginInfo.SupportedFeatures.SupportBatchRequests == 1))
                {
                    Console.WriteLine("Connected Provider does not support Batch Requests. Sending Market Price requests as individual request messages.");
                    return SendRequest(chnl, itemNames, out errorInfo);
                }
                else
                {
                    // batch
                    return SendBatchRequest(chnl, itemNames, out errorInfo);
                }
            }
        }


        /// <summary>
        /// This method is used while posting to query the first requested market
        /// price item, if any.
        /// </summary>
        ///
        /// It will populate the passed in buffer with the name and length
        /// information and return the streamId associated with the stream. If
        /// mpItemName->length is 0 and streamId is returned as 0, this indicates
        /// that there is no valid name available.
        ///
        public int GetFirstItem(Codec.Buffer mpItemName)
        {
            return m_WatchList.GetFirstItem(mpItemName);
        }


        private bool HasMarketPriceCapability(List<long> capabilities)
        {
            return capabilities.Any((capability) => capability == m_MarketPriceRequest.DomainType);
        }

        private void GenerateRequest(MarketPriceRequest marketPriceRequest, bool isPrivateStream, Service srcDirInfo, LoginRefresh loginInfo)
        {
            marketPriceRequest.Clear();

            if (!SnapshotRequest)
                marketPriceRequest.Streaming = true;

            marketPriceRequest.HasServiceId = true;
            marketPriceRequest.ServiceId = srcDirInfo.ServiceId;
            marketPriceRequest.HasPriority = true;
            marketPriceRequest.Priority = (1, 1);

            if (srcDirInfo.Info.QosList.Count > 0)
            {
                marketPriceRequest.HasQos = true;
                marketPriceRequest.Qos.IsDynamic = false;
                marketPriceRequest.Qos.TimeInfo(srcDirInfo.Info.QosList[0].TimeInfo());
                marketPriceRequest.Qos.Timeliness(srcDirInfo.Info.QosList[0].Timeliness());
                marketPriceRequest.Qos.RateInfo(srcDirInfo.Info.QosList[0].RateInfo());
                marketPriceRequest.Qos.Rate(srcDirInfo.Info.QosList[0].Rate());
            }
            if (isPrivateStream)
                marketPriceRequest.PrivateStream = true;

            if (loginInfo.HasFeatures
                && loginInfo.SupportedFeatures.HasSupportViewRequests
                && loginInfo.SupportedFeatures.SupportViewRequests == 1
                && ViewRequest)
            {
                marketPriceRequest.HasView = true;
                marketPriceRequest.ViewFields.AddRange(m_ViewFieldList);
            }
        }

        private ReactorReturnCode SendRequest(ReactorChannel chnl, List<string> itemNames, out ReactorErrorInfo? errorInfo)
        {
            foreach (string itemName in itemNames)
            {
                int streamId = m_WatchList.Add(m_DomainType, itemName, m_MarketPriceRequest.PrivateStream);

                m_MarketPriceRequest.ItemNames.Clear();
                m_MarketPriceRequest.ItemNames.Add(itemName);

                m_MarketPriceRequest.StreamId = streamId;
                ReactorReturnCode ret = EncodeAndSendRequest(chnl, m_MarketPriceRequest, out errorInfo);
                if (ret < ReactorReturnCode.SUCCESS)
                    return ret;
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        private ReactorReturnCode SendBatchRequest(ReactorChannel chnl, List<string> itemNames, out ReactorErrorInfo? errorInfo)
        {
            int batchStreamId = m_WatchList.Add(m_DomainType, "BATCH_" + System.DateTime.Now.Ticks, m_MarketPriceRequest.PrivateStream);
            m_MarketPriceRequest.StreamId = batchStreamId;

            foreach (string itemName in itemNames)
            {
                m_WatchList.Add(m_DomainType, itemName, m_MarketPriceRequest.PrivateStream);
                m_MarketPriceRequest.ItemNames.Add(itemName);
            }

            return EncodeAndSendRequest(chnl, m_MarketPriceRequest, out errorInfo);
        }

        private ReactorReturnCode EncodeAndSendRequest(ReactorChannel chnl, MarketPriceRequest marketPriceRequest, out ReactorErrorInfo? errorInfo)
        {
            //get a buffer for the item request
            ITransportBuffer? msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, out errorInfo);

            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
            }

            m_EncodeIterator.Clear();
            m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);

            CodecReturnCode ret = marketPriceRequest.Encode(m_EncodeIterator);
            if (ret < CodecReturnCode.SUCCESS)
            {
                if(errorInfo is null)
                {
                    errorInfo = new ReactorErrorInfo();
                }
                errorInfo.Error.Text = "MarketPriceRequest.encode() failed";
                errorInfo.Error.ErrorId = (TransportReturnCode)ret;
                return (ReactorReturnCode)ret;
            }

            Console.WriteLine(marketPriceRequest.ToString());
            return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }


        /// <summary>
        /// Processes a market price response.
        /// </summary>
        ///
        /// This consists of extracting the key, printing out the item name contained
        /// in the key, decoding the field list and field entry, and calling
        /// DecodeFieldEntry() to decode the field entry data.
        ///
        public ReactorReturnCode ProcessResponse(Msg msg, DecodeIterator dIter, DataDictionary dictionary, out ReactorErrorInfo? errorInfo)
        {
            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH:
                    return HandleRefresh(msg, dIter, dictionary, out errorInfo);
                case MsgClasses.UPDATE:
                    errorInfo = null;
                    return HandleUpdate(msg, dIter, dictionary);
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

            StringBuilder fieldValue = new();
            GetItemName(msg, fieldValue);

            IAckMsg ackMsg = (IAckMsg)msg;

            fieldValue.Append("\tackId=" + ackMsg.AckId + "\n");
            if (ackMsg.CheckHasSeqNum())
                fieldValue.Append("\tseqNum=" + ackMsg.SeqNum + "\n");
            if (ackMsg.CheckHasNakCode())
                fieldValue.Append("\tnakCode=" + ackMsg.NakCode + "\n");
            if (ackMsg.CheckHasText())
                fieldValue.Append("\ttext=" + ackMsg.Text.ToString());

            Console.WriteLine(fieldValue.ToString());
            return ReactorReturnCode.SUCCESS;
        }

        protected void GetItemName(Msg msg, StringBuilder fieldValue)
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
                        + (refreshMsg.CheckPrivateStream()
                           ? " (PRIVATE STREAM)"
                           : "")
                        + "\nDOMAIN: " + DomainTypes.ToString(msg.DomainType) + "\n");
                }
                else
                {
                    fieldValue.Append(key.Name.ToString() + "\nDOMAIN: " +
                            DomainTypes.ToString(msg.DomainType) + "\n");
                    if (msg.MsgClass == MsgClasses.UPDATE)
                    {
                        fieldValue.Append("UPDATE TYPE: " + UpdateEventTypes.ToString(((IUpdateMsg)msg).UpdateType) + "\n");
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
                        + (wle.IsPrivateStream
                           ? " (PRIVATE STREAM)" : " ")
                        + "\nDOMAIN: " + DomainTypes.ToString(msg.DomainType) + "\n");
                    if (msg.MsgClass == MsgClasses.UPDATE)
                    {
                        fieldValue.Append("UPDATE TYPE: " + UpdateEventTypes.ToString(((IUpdateMsg)msg).UpdateType) + "\n");
                    }
                }
                else
                {
                    // check if this is login stream for offstream posting
                    if (msg.StreamId == LoginStreamId)
                    {
                        fieldValue.Append("OFFPOST " + "\nDOMAIN: " + DomainTypes.ToString(msg.DomainType) + "\n");
                    }
                }
            }
        }

        private ReactorReturnCode HandleStatus(Msg msg, out ReactorErrorInfo? errorInfo)
        {
            IStatusMsg statusMsg = (IStatusMsg)msg;
            Console.WriteLine("Received Item StatusMsg for stream " + msg.StreamId);
            if (!statusMsg.CheckHasState())
            {
                errorInfo = null;
                return ReactorReturnCode.SUCCESS;
            }

            // get state information
            State state = statusMsg.State;
            Console.WriteLine("\t" + state);

            var wle = m_WatchList.Get(msg.StreamId);
            if (wle != null)
            {
                /* update our state table with the new state
                 * check if this response should be on private stream but is not
                 */
                if (!statusMsg.CheckPrivateStream()) /* non-private stream */
                {
                    /*
                     * check if this response should be on private stream but is not
                     * batch responses for private stream may be sent on non-private
                     * stream
                     */
                    /* if this is the case, close the stream */
                    if (wle.IsPrivateStream && !wle.ItemName.Contains("BATCH_"))
                    {
                        Console.WriteLine("Received non-private response for stream " +
                                          msg.StreamId + " that should be private - closing stream");
                        // close stream
                        CloseStream(m_RedirectChannel!, msg.StreamId, out errorInfo);
                        // remove private stream entry from list
                        RemoveMarketPriceItemEntry(msg.StreamId);
                        return ReactorReturnCode.FAILURE;
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
                if (ret != ReactorReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

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
            RemoveMarketPriceItemEntry(streamId);

            /* add item name to private stream list */
            var psStreamId = m_WatchList.Add(m_DomainType, wle.ItemName, true);

            GenerateRequest(m_MarketPriceRequest, true, m_RedirectSrcDirInfo!, m_RedirectLoginInfo!);
            m_MarketPriceRequest.ItemNames.Add(wle.ItemName);
            m_MarketPriceRequest.StreamId = psStreamId;
            return EncodeAndSendRequest(m_RedirectChannel!, m_MarketPriceRequest, out errorInfo);
        }

        private ReactorReturnCode HandleUpdate(Msg msg, DecodeIterator dIter, DataDictionary dictionary)
        {
            IUpdateMsg updateMsg = (IUpdateMsg)msg;
            PostUserInfo pu = updateMsg.PostUserInfo;
            if (pu != null)
            {
                Console.WriteLine(" Received UpdateMsg for stream "
                    + updateMsg.StreamId + " from publisher with user ID: " + pu.UserId
                    + " at user address: " + pu.UserAddrToString(pu.UserAddr));
            }

            return (ReactorReturnCode)Decode(msg, dIter, dictionary);
        }

        private ReactorReturnCode HandleRefresh(Msg msg, DecodeIterator dIter, DataDictionary dictionary, out ReactorErrorInfo? errorInfo)
        {
            IRefreshMsg refreshMsg = (IRefreshMsg)msg;
            PostUserInfo pu = refreshMsg.PostUserInfo;
            if (pu != null)
            {
                Console.WriteLine(" Received RefreshMsg for stream " + refreshMsg.StreamId
                    + " from publisher with user ID: " + pu.UserId
                    + " at user address: " + pu.UserAddrToString(pu.UserAddr));
            }

            var wle = m_WatchList.Get(msg.StreamId);
            if (wle == null)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = "No such entry in watchlist";
                return ReactorReturnCode.FAILURE;
            }

            /* check if this response should be on private stream but is not */
            /* if this is the case, close the stream */
            if (!refreshMsg.CheckPrivateStream() && wle.IsPrivateStream)
            {
                Console.WriteLine("Received non-private response for stream " + msg.StreamId +
                        " that should be private - closing stream");
                // close stream
                CloseStream(m_RedirectChannel!, msg.StreamId, out _);

                // remove private stream entry from list
                RemoveMarketPriceItemEntry(msg.StreamId);

                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"Received non-private response for stream {msg.StreamId} that should be private - closing stream";

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

        private void RemoveMarketPriceItemEntry(int streamId)
        {
            m_WatchList.Remove(streamId);
        }

        protected virtual CodecReturnCode Decode(Msg msg, DecodeIterator dIter, DataDictionary dictionary)
        {
            StringBuilder fieldValue = new StringBuilder();
            GetItemName(msg, fieldValue);
            if (msg.MsgClass == MsgClasses.REFRESH)
                fieldValue.Append((((IRefreshMsg)msg).State).ToString() + "\n");

            return DecodePayload(dIter, dictionary, fieldValue);
        }

        public virtual CodecReturnCode DecodePayload(DecodeIterator dIter, DataDictionary dictionary, StringBuilder fieldValue)
        {
            CodecReturnCode ret = fieldList.Decode(dIter, null);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("DecodeFieldList() failed with return code: " + ret);
                return ret;
            }

            // decode each field entry in list
            while ((ret = fieldEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("DecodeFieldEntry() failed with return code: " + ret);
                    return ret;
                }

                ret = DecodeFieldEntry(fieldEntry, dIter, dictionary, fieldValue);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("DecodeFieldEntry() failed");
                    return ret;
                }
                fieldValue.Append('\n');
            }
            Console.WriteLine(fieldValue.ToString());

            return CodecReturnCode.SUCCESS;
        }

        /*
     * This is used by all market price domain handlers to output field lists.
     *
     * Decodes the field entry data and prints out the field entry data with
     * help of the dictionary. Returns success if decoding succeeds or failure
     * if decoding fails.
     */
        protected CodecReturnCode DecodeFieldEntry(FieldEntry fEntry, DecodeIterator dIter,
                DataDictionary dictionary, StringBuilder fieldValue)
        {
            // get dictionary entry
            var dictionaryEntry = dictionary.Entry(fEntry.FieldId);

            // return if no entry found
            if (dictionaryEntry == null)
            {
                fieldValue.Append("\tFid " + fEntry.FieldId + " not found in dictionary");
                return CodecReturnCode.SUCCESS;
            }

            // print out fid name
            fieldValue.Append("\t" + fEntry.FieldId + "/" + dictionaryEntry.GetAcronym().ToString() + ": ");

            // decode and print out fid value
            int dataType = dictionaryEntry.GetRwfType();
            CodecReturnCode ret = 0;
            switch (dataType)
            {
                case DataTypes.UINT:
                    ret = fidUIntValue.Decode(dIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        fieldValue.Append(fidUIntValue.ToLong());
                    }
                    else if (ret != CodecReturnCode.BLANK_DATA)
                    {
                        Console.WriteLine($"DecodeUInt() failed: <{ret}>");
                        return ret;
                    }
                    break;
                case DataTypes.INT:
                    ret = fidIntValue.Decode(dIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        fieldValue.Append(fidIntValue.ToLong());
                    }
                    else if (ret != CodecReturnCode.BLANK_DATA)
                    {
                        Console.WriteLine($"DecodeInt() failed: <{ret}>");

                        return ret;
                    }
                    break;
                case DataTypes.FLOAT:
                    ret = fidFloatValue.Decode(dIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        fieldValue.Append(fidFloatValue.ToFloat());
                    }
                    else if (ret != CodecReturnCode.BLANK_DATA)
                    {
                        Console.WriteLine($"DecodeFloat() failed: <{ret}>");

                        return ret;
                    }
                    break;
                case DataTypes.DOUBLE:
                    ret = fidDoubleValue.Decode(dIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        fieldValue.Append(fidDoubleValue.ToDouble());
                    }
                    else if (ret != CodecReturnCode.BLANK_DATA)
                    {
                        Console.WriteLine($"DecodeDouble() failed: <{ret}>");

                        return ret;
                    }
                    break;
                case DataTypes.REAL:
                    ret = fidRealValue.Decode(dIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        fieldValue.Append(fidRealValue.ToDouble());
                    }
                    else if (ret != CodecReturnCode.BLANK_DATA)
                    {
                        Console.WriteLine($"DecodeReal() failed: <{ret}>");

                        return ret;
                    }
                    break;
                case DataTypes.ENUM:
                    ret = fidEnumValue.Decode(dIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        var enumType = dictionary.EntryEnumType(dictionaryEntry, fidEnumValue);

                        if (enumType == null)
                        {
                            fieldValue.Append(fidEnumValue.ToInt());
                        }
                        else
                        {
                            fieldValue.Append(enumType.Display.ToString() + "(" + fidEnumValue.ToInt() + ")");
                        }
                    }
                    else if (ret != CodecReturnCode.BLANK_DATA)
                    {
                        Console.WriteLine($"DecodeEnum() failed: <{ret}>");

                        return ret;
                    }
                    break;
                case DataTypes.DATE:
                    ret = fidDateValue.Decode(dIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        fieldValue.Append(fidDateValue.ToString());

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
                        fieldValue.Append(fidTimeValue.ToString());
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
                        fieldValue.Append(fidDateTimeValue.ToString());
                    }
                    else if (ret != CodecReturnCode.BLANK_DATA)
                    {
                        Console.WriteLine($"DecodeDateTime() failed: <{ret}>");
                        return ret;
                    }
                    break;
                case DataTypes.QOS:
                    ret = fidQosValue.Decode(dIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        fieldValue.Append(fidQosValue.ToString());
                    }
                    else if (ret != CodecReturnCode.BLANK_DATA)
                    {
                        Console.WriteLine($"DecodeQos() failed: <{ret}>");

                        return ret;
                    }
                    break;
                case DataTypes.STATE:
                    ret = fidStateValue.Decode(dIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        fieldValue.Append(fidStateValue.ToString());
                    }
                    else if (ret != CodecReturnCode.BLANK_DATA)
                    {
                        Console.WriteLine($"DecodeState() failed: <{ret}>");

                        return ret;
                    }
                    break;
                // For an example of array decoding, see
                // FieldListCodec.exampleDecode()
                case DataTypes.ARRAY:
                    break;
                case DataTypes.BUFFER:
                case DataTypes.ASCII_STRING:
                case DataTypes.UTF8_STRING:
                case DataTypes.RMTES_STRING:
                    if (fEntry.EncodedData.Length > 0)
                    {
                        fieldValue.Append(fEntry.EncodedData.ToString());
                    }
                    else
                    {
                        ret = CodecReturnCode.BLANK_DATA;
                    }
                    break;
                default:
                    fieldValue.Append("Unsupported data type (" + DataTypes.ToString(dataType) + ")");
                    break;
            }
            if (ret == CodecReturnCode.BLANK_DATA)
            {
                fieldValue.Append("<blank data>");
            }

            return CodecReturnCode.SUCCESS;
        }


        /// <summary>
        /// Close all item streams.
        /// </summary>
        /// <param name="chnl"></param>
        /// <param name="errorInfo"></param>
        /// <returns></returns>
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
                    if (ret != ReactorReturnCode.SUCCESS)
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


        protected ReactorReturnCode CloseStream(ReactorChannel chnl, int streamId, out ReactorErrorInfo? errorInfo)
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
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"Error: EncodeMarketPriceClose(): Failed <{ret}>");
            }

            return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }
    }
}
