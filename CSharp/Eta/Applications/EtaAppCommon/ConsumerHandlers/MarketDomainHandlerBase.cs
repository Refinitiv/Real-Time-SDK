/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using System;
using System.Collections.Generic;
using System.Text;
using Buffer = LSEG.Eta.Codec.Buffer;
using DateTime = LSEG.Eta.Codec.DateTime;
using Double = LSEG.Eta.Codec.Double;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Eta.Example.Common
{
    /// <summary>
    /// Base class for MarketPrice, MarketByPrice and MarketByOrder domain Handlers.
    /// </summary>
    public abstract class MarketDomainHandlerBase
    {
        public static int TRANSPORT_BUFFER_SIZE_REQUEST = ChannelSession.MAX_MSG_SIZE;

        public static int TRANSPORT_BUFFER_SIZE_CLOSE = ChannelSession.MAX_MSG_SIZE;

        private int domainType;

        protected ChannelSession? redirectChnl;

        private LoginRefresh? redirectLoginInfo;

        private Service? redirectSrcDirInfo;

        private MarketRequest marketRequest;

        private MarketPriceClose closeMessage;

        protected StreamIdWatchList? watchList;

        private bool viewRequested = false;
        private bool snapshotRequested = false;
        private List<string> viewFieldList;

        protected FieldList fieldList = new FieldList();
        protected FieldEntry fieldEntry = new FieldEntry();
        private UInt fidUIntValue = new UInt();
        private Int fidIntValue = new Int();
        private Real fidRealValue = new Real();
        private Enum fidEnumValue = new Enum();
        private Date fidDateValue = new Date();
        private Time fidTimeValue = new Time();
        private DateTime fidDateTimeValue = new DateTime();
        private Float fidFloatValue = new Float();
        private Double fidDoubleValue = new Double();
        private Qos fidQosValue = new Qos();
        private State fidStateValue = new State();
        private EncodeIterator encIter = new EncodeIterator();

        public MarketDomainHandlerBase(StreamIdWatchList watchList) : this((int)DomainType.MARKET_PRICE, watchList) { }

        protected MarketDomainHandlerBase(int domainType, StreamIdWatchList watchList)
        {
            this.watchList = watchList;
            this.domainType = domainType;
            marketRequest = CreateRequest();
            closeMessage = new MarketPriceClose();
            viewFieldList = new List<string>();
            viewFieldList.Add("6");
            viewFieldList.Add("22");
            viewFieldList.Add("25");
            viewFieldList.Add("32");
        }

        protected abstract MarketRequest CreateRequest();

        protected abstract CodecReturnCode Decode(Msg msg, DecodeIterator dIter, DataDictionary dictionary);

        public void ViewRequest(bool doViewRequest)
        {
            viewRequested = doViewRequest;
        }

        public void SnapshotRequest(bool snapshotRequested)
        {
            this.snapshotRequested = snapshotRequested;
        }

        protected void RemoveMarketPriceItemEntry(int streamId)
        {
            watchList!.Remove(streamId);
        }

        protected TransportReturnCode CloseStream(ChannelSession chnl, int streamId, out Error? error)
        {
            ITransportBuffer? msgBuf = chnl.GetTransportBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, out error);
            if (msgBuf == null)
                return TransportReturnCode.FAILURE;

            closeMessage.Clear();
            closeMessage.StreamId = streamId;
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(msgBuf, chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);

            CodecReturnCode ret = closeMessage.Encode(encIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("Encode MarketPriceClose Failed: code " + ret + ".\n");
                error = new Error()
                {
                    Text = "Encode MarketPriceClose Failed: code " + ret + ".\n",
                    ErrorId = TransportReturnCode.FAILURE
                };
                return TransportReturnCode.FAILURE;
            }
            return chnl.Write(msgBuf, out error);
        }

        public int GetFirstItem(Buffer mpItemName)
        {
            return watchList!.GetFirstItem(mpItemName);
        }

        private bool HasMarketPriceCapability(List<long> capabilities)
        {
            foreach (long capability in capabilities)
            {
                if (capability.Equals((long)marketRequest.DomainType))
                    return true;
            }
            return false;
        }

        /// <summary>
        /// Encodes and sends item requests for three market price domains. 
        /// </summary>
        /// <param name="chnl">The channel to send a source directory request to</param>
        /// <param name="itemNames">List of item names</param>
        /// <param name="isPrivateStream">flag indicating if requested items are private stream or not.</param>
        /// <param name="loginInfo">RDM login information</param>
        /// <param name="serviceInfo">RDM directory response information</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode SendItemRequests(ChannelSession chnl, List<string> itemNames, bool isPrivateStream, LoginRefresh loginInfo, Service serviceInfo, out Error? error)
        {
            error = null;
            if (itemNames == null || itemNames.Count == 0)
                return TransportReturnCode.SUCCESS;

            // check to see if the provider supports the market price domain
            if (!HasMarketPriceCapability(serviceInfo.Info.CapabilitiesList))
            {
                error = new Error()
                {
                    Text = "'" + marketRequest.DomainType + "' not supported by the indicated provider"
                };
                return TransportReturnCode.FAILURE;
            }

            // set redirect channel for private stream redirect
            redirectChnl = chnl;

            // set login information for private stream redirect
            redirectLoginInfo = loginInfo;

            // set source directory information for private stream redirect
            redirectSrcDirInfo = serviceInfo;

            GenerateRequest(marketRequest, isPrivateStream, serviceInfo, loginInfo);

            // If there is only one item in the itemList, it is a waste of bandwidth
            // to send a batch request
            if (itemNames.Count == 1)
            {
                return SendRequest(chnl, itemNames, out error);
            }

            if (!(loginInfo.HasFeatures &&
                  loginInfo.SupportedFeatures.HasSupportBatchRequests &&
                  loginInfo.SupportedFeatures.SupportBatchRequests == 1))
            {
                Console.WriteLine("Connected Provider does not support Batch Requests. Sending Market Price requests as individual request messages.");
                return SendRequest(chnl, itemNames, out error);
            }

            // batch
            return SendBatchRequest(chnl, itemNames, out error);
        }

        private void GenerateRequest(MarketRequest marketPriceRequest, bool isPrivateStream, Service srcDirInfo, LoginRefresh loginInfo)
        {
            marketPriceRequest.Clear();

            if (!snapshotRequested)
                marketPriceRequest.Streaming = true;
            marketPriceRequest.HasServiceId = true;
            marketPriceRequest.ServiceId = srcDirInfo.ServiceId;
            marketPriceRequest.HasPriority = true;
            marketPriceRequest.PriorityClass = 1;
            marketPriceRequest.PriorityCount = 1;
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

            if (loginInfo.HasFeatures &&
                loginInfo.SupportedFeatures.HasSupportViewRequests &&
                loginInfo.SupportedFeatures.SupportViewRequests == 1 &&
                viewRequested)
            {
                marketPriceRequest.HasView = true;
                marketPriceRequest.ViewFields.AddRange(viewFieldList);
            }
        }

        private TransportReturnCode SendBatchRequest(ChannelSession chnl, List<String> itemNames, out Error? error)
        {
            int batchStreamId = watchList!.Add(domainType, "BATCH_" + new Date(), marketRequest.PrivateStream);
            marketRequest.StreamId = batchStreamId;
            int totalBytes = 0;
            foreach (string itemName in itemNames)
            {
                watchList.Add(domainType, itemName, marketRequest.PrivateStream);
                marketRequest.ItemNames.Add(itemName);
                try
                {
                    totalBytes += Encoding.UTF8.GetBytes(itemName).Length;
                }
                catch (Exception e)
                {
                    error = new Error()
                    {
                        Text = "Failed to get itemname length: " + e.Message
                    };
                    return TransportReturnCode.FAILURE;
                }
            }

            return EncodeAndSendRequest(chnl, marketRequest, totalBytes, out error);
        }

        private TransportReturnCode SendRequest(ChannelSession chnl, List<String> itemNames, out Error? error)
        {
            TransportReturnCode ret;
            foreach (string itemName in itemNames)
            {
                int streamId = watchList!.Add(domainType, itemName, marketRequest.PrivateStream);

                marketRequest.ItemNames.Clear();
                marketRequest.ItemNames.Add(itemName);

                marketRequest.StreamId = streamId;
                ret = EncodeAndSendRequest(chnl, marketRequest, 0, out error);
                if (ret < TransportReturnCode.SUCCESS)
                    return ret;
            }

            error = null;
            return TransportReturnCode.SUCCESS;
        }

        private TransportReturnCode EncodeAndSendRequest(ChannelSession chnl, MarketRequest marketRequest, int totalBytes, out Error? error)
        {
            int totalBytesToAllocate = Math.Min(totalBytes * 2, ChannelSession.MAX_MSG_SIZE * 4);

            //get a buffer for the item request
            ITransportBuffer? msgBuf = chnl.GetTransportBuffer(Math.Max(TRANSPORT_BUFFER_SIZE_REQUEST, totalBytesToAllocate), false, out error);

            if (msgBuf == null)
            {
                return TransportReturnCode.FAILURE;

            }
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(msgBuf, chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);

            CodecReturnCode ret = marketRequest.Encode(encIter);
            if (ret < CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = "MarketRequest.Encode() failed"
                };
                return TransportReturnCode.FAILURE;
            }

            Console.WriteLine(marketRequest.ToString());
            return chnl.Write(msgBuf, out error);
        }

        /// <summary>
        /// Processes a market domain response. This consists of extracting the key, printing out the item name contained in the key, 
        /// decoding the field list and field entry, and calling decodeFieldEntry() to decode the field entry data.
        /// </summary>
        /// <param name="msg">The partially decoded message</param>
        /// <param name="dIter">The decode iterator</param>
        /// <param name="dictionary">The dictionary</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode ProcessResponse(Msg msg, DecodeIterator dIter, DataDictionary dictionary, out Error? error)
        {
            error = null;
            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH:
                    return HandleRefresh(msg, dIter, dictionary, out error);
                case MsgClasses.UPDATE:
                    return HandleUpdate(msg, dIter, dictionary);
                case MsgClasses.STATUS:
                    return HandleStatus(msg, out error);
                case MsgClasses.ACK:
                    return HandleAck(msg);
                default:
                    Console.WriteLine("Received Unhandled Item Msg Class: " + msg.MsgClass);
                    break;
            }

            return TransportReturnCode.SUCCESS;
        }

        protected TransportReturnCode HandleAck(Msg msg)
        {
            Console.WriteLine("Received AckMsg for stream " + msg.StreamId);

            StringBuilder fieldValue = new StringBuilder();
            GetItemName(msg, fieldValue);

            IAckMsg ackMsg = msg;

            fieldValue.Append("\tackId=" + ackMsg.AckId);
            if (ackMsg.CheckHasSeqNum())
                fieldValue.Append("\tseqNum=" + ackMsg.SeqNum);
            if (ackMsg.CheckHasNakCode())
                fieldValue.Append("\tnakCode=" + ackMsg.NakCode);
            if (ackMsg.CheckHasText())
                fieldValue.Append("\ttext=" + ackMsg.Text.ToString());

            Console.WriteLine(fieldValue.ToString());
            return TransportReturnCode.SUCCESS;
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
                    IRefreshMsg refreshMsg = msg;
                    fieldValue.Append(key.Name.ToString() + (refreshMsg.CheckPrivateStream() ? " (PRIVATE STREAM)" : "") + "\nDOMAIN: " +
                            DomainTypes.ToString(msg.DomainType) + "\n");
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
            // cached item name
            {
                WatchListEntry wle = watchList!.Get(msg.StreamId);

                if (wle != null)
                {
                    fieldValue.Append(wle.ItemName + (wle.IsPrivateStream ? " (PRIVATE STREAM)" : " ") + "\nDOMAIN: " + DomainTypes.ToString(msg.DomainType) + "\n");
                    if (msg.MsgClass == MsgClasses.UPDATE)
                    {
                        fieldValue.Append("UPDATE TYPE: " + UpdateEventTypes.ToString(((IUpdateMsg)msg).UpdateType) + "\n");
                    }
                }
                else
                {
                    // check if this is login stream for offstream posting
                    if (msg.StreamId == LoginHandler.LOGIN_STREAM_ID)
                    {
                        fieldValue.Append("OFFPOST " + "\nDOMAIN: " + DomainTypes.ToString(msg.DomainType) + "\n");
                    }
                }
            }
        }

        protected TransportReturnCode HandleStatus(Msg msg, out Error? error)
        {
            IStatusMsg statusMsg = msg;
            error = null;
            Console.WriteLine("Received Item StatusMsg for stream " + msg.StreamId);
            if (!statusMsg.CheckHasState())
                return TransportReturnCode.SUCCESS;

            // get state information
            State state = statusMsg.State;
            Console.WriteLine("	" + state);

            WatchListEntry wle = watchList!.Get(msg.StreamId);
            if (wle != null)
            {
                // update our state table with the new state
                if (!statusMsg.CheckPrivateStream()) // non-private stream 
                {
                    // check if this response should be on private stream but is not
                    if (!statusMsg.CheckPrivateStream()) // non-private stream 
                    {
                        // check if this response should be on private stream but is not
                        // batch responses for private stream may be sent on non-private
                        // stream
                        // if this is the case, close the stream
                        if (wle.IsPrivateStream && !wle.ItemName!.Contains("BATCH_"))
                        {
                            Console.WriteLine("Received non-private response for stream " + msg.StreamId + " that should be private - closing stream");
                            // close stream
                            if (CloseStream(redirectChnl!, msg.StreamId, out var closeError) != TransportReturnCode.SUCCESS)
                                Console.WriteLine($"Failure while closing the stream: {closeError?.Text}");

                            // remove private stream entry from list
                            RemoveMarketPriceItemEntry(msg.StreamId);
                            error = new Error()
                            {
                                Text = "Received non-private response for stream " + msg.StreamId + " that should be private - closing stream",
                                ErrorId = TransportReturnCode.FAILURE
                            };
                            return TransportReturnCode.FAILURE;
                        }
                    }
                }
                wle.ItemState!.DataState(statusMsg.State.DataState());
                wle.ItemState.StreamState(statusMsg.State.StreamState());
            }

            // redirect to private stream if indicated
            if (statusMsg.State.StreamState() == StreamStates.REDIRECTED && (statusMsg.CheckPrivateStream()))
            {
                TransportReturnCode ret = RedirectToPrivateStream(msg.StreamId, out error);
                if (ret != TransportReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            return TransportReturnCode.SUCCESS;
        }

        protected TransportReturnCode HandleUpdate(Msg msg, DecodeIterator dIter, DataDictionary dictionary)
        {
            IUpdateMsg updateMsg = msg;
            PostUserInfo pu = updateMsg.PostUserInfo;
            if (pu != null)
            {
                Console.WriteLine(" Received UpdateMsg for stream " + updateMsg.StreamId + " from publisher with user ID: " + pu.UserId + " at user address: " + pu.UserAddrToString(pu.UserAddr));
            }
            if (Decode(msg, dIter, dictionary) == CodecReturnCode.SUCCESS)
            {
                return TransportReturnCode.SUCCESS;
            }
            else
            {
                return TransportReturnCode.FAILURE;
            }
        }

        protected TransportReturnCode HandleRefresh(Msg msg, DecodeIterator dIter, DataDictionary dictionary, out Error? error)
        {
            IRefreshMsg refreshMsg = msg;
            PostUserInfo pu = refreshMsg.PostUserInfo;
            if (pu != null)
            {
                Console.WriteLine(" Received RefreshMsg for stream " + refreshMsg.StreamId + " from publisher with user ID: " + pu.UserId + " at user address: " + pu.UserAddrToString(pu.UserAddr));
            }

            WatchListEntry wle = watchList!.Get(msg.StreamId);

            // check if this response should be on private stream but is not
            // if this is the case, close the stream 
            if (!refreshMsg.CheckPrivateStream() && wle.IsPrivateStream)
            {
                Console.WriteLine("Received non-private response for stream " + msg.StreamId +
                        " that should be private - closing stream");
                // close stream
                if (CloseStream(redirectChnl!, msg.StreamId, out var closeError) != TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine($"Error closing stream: {closeError?.Text}");
                }

                // remove private stream entry from list
                RemoveMarketPriceItemEntry(msg.StreamId);
                error = new Error()
                {
                    Text = "Received non-private response for stream " + msg.StreamId + " that should be private - closing stream"
                };
                return TransportReturnCode.FAILURE;
            }
            // update our item state list if its a refresh, then process just like update
            wle.ItemState!.DataState(refreshMsg.State.DataState());
            wle.ItemState.StreamState(refreshMsg.State.StreamState());

            error = null;
            if (Decode(msg, dIter, dictionary) == CodecReturnCode.SUCCESS)
            {
                return TransportReturnCode.SUCCESS;
            }
            else
            {
                return TransportReturnCode.FAILURE;
            }
        }

        /// <summary>
        /// Close all item streams.
        /// </summary>
        /// <param name="chnl">The channel to send a item stream close to</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode CloseStreams(ChannelSession chnl, out Error? error)
        {
            TransportReturnCode ret = 0;
            List<KeyValuePair<StreamIdKey, WatchListEntry>> itemsToRemove = new List<KeyValuePair<StreamIdKey, WatchListEntry>>();

            var iter = watchList!.GetEnumerator();
            while (iter.MoveNext())
            {
                KeyValuePair<StreamIdKey, WatchListEntry> entry = (KeyValuePair<StreamIdKey, WatchListEntry>)iter.Current;

                // we only want to close a stream if it was not already closed (e.g.
                // rejected by provider, closed via refresh or status, or redirected)
                if (entry.Value.ItemState!.IsFinal())
                    continue;
                if (entry.Value.DomainType == domainType)
                {
                    ret = CloseStream(chnl, entry.Key.StreamId, out error);
                    if (ret != TransportReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                    itemsToRemove.Add(entry);
                }
            }

            foreach (var item in itemsToRemove)
            {
                watchList.Remove(item.Key.StreamId);
            }

            error = null;
            return TransportReturnCode.SUCCESS;
        }
        
        private TransportReturnCode RedirectToPrivateStream(int streamId, out Error? error)
        {
            WatchListEntry wle = watchList!.Get(streamId);

            RemoveMarketPriceItemEntry(streamId);
            int psStreamId = watchList.Add(domainType, wle.ItemName!, true);

            GenerateRequest(marketRequest, true, redirectSrcDirInfo!, redirectLoginInfo!);
            marketRequest.ItemNames.Add(wle.ItemName!);
            marketRequest.StreamId = psStreamId;
            return EncodeAndSendRequest(redirectChnl!, marketRequest, 0, out error);
        }

        protected CodecReturnCode DecodeFieldEntry(FieldEntry fEntry, DecodeIterator dIter,
                DataDictionary dictionary, StringBuilder fieldValue)
        {
            // get dictionary entry
            IDictionaryEntry dictionaryEntry = dictionary.Entry(fEntry.FieldId);

            // return if no entry found
            if (dictionaryEntry == null)
            {
                fieldValue.Append("\tFid " + fEntry.FieldId + " not found in dictionary");
                return CodecReturnCode.SUCCESS;
            }

            // print out fid name
            fieldValue.Append("\t" + fEntry.FieldId + "/" + dictionaryEntry.GetAcronym().ToString() + ": ");

            // Decode and print out fid value
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
                        Console.WriteLine("DecodeUInt() failed: <" + ret + ">\n");
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
                        Console.WriteLine("DecodeInt() failed: <" + ret + ">");

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
                        Console.WriteLine("DecodeFloat() failed: <" + ret + ">\n");

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
                        Console.WriteLine("DecodeDouble() failed: <" + ret + ">\n");

                        return ret;
                    }
                    break;
                case DataTypes.REAL:
                    ret = fidRealValue.Decode(dIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        fieldValue.Append(fidRealValue.ToString());
                    }
                    else if (ret != CodecReturnCode.BLANK_DATA)
                    {
                        Console.WriteLine("DecodeReal() failed: <" + ret + ">\n");

                        return ret;
                    }
                    break;
                case DataTypes.ENUM:
                    ret = fidEnumValue.Decode(dIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        IEnumType enumType = dictionary.EntryEnumType(dictionaryEntry, fidEnumValue);

                        if (enumType == null)
                        {
                            fieldValue.Append(fidEnumValue.ToInt());
                        }
                        else
                        {
                            fieldValue.Append(enumType.Display.ToString() + "(" +
                                    fidEnumValue.ToInt() + ")");
                        }
                    }
                    else if (ret != CodecReturnCode.BLANK_DATA)
                    {
                        Console.WriteLine("DecodeEnum() failed: <" + ret + ">\n");

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
                        Console.WriteLine("DecodeDate() failed: <" + ret + ">\n");

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
                        Console.WriteLine("DecodeTime() failed: <" + ret + ">\n");

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
                        Console.WriteLine("DecodeDateTime() failed: <" + ret + ">\n");
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
                        Console.WriteLine("DecodeQos() failed: <" + ret + ">\n");

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
                        Console.WriteLine("DecodeState() failed: <" + ret + ">\n");

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
    }
}
