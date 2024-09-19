/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.Example.Common
{
    /// <summary>
    /// This is the post handler for the ETA consumer application. 
    /// It provides methods for sending on stream and off stream post messages.
    /// </summary>
    public class PostHandler
    {
        public const int TRANSPORT_BUFFER_SIZE_MSG_POST = ChannelSession.MAX_MSG_SIZE;
        public const int TRANSPORT_BUFFER_SIZE_DATA_POST = ChannelSession.MAX_MSG_SIZE;
        private const int POST_MESSAGE_FREQUENCY = 5; // in seconds

        private int nextPostId;
        private int nextSeqNum;
        private double itemData;

        
        private long nextPostTime;
        private bool postWithMsg;
       
        private bool hasInputPrincipalIdentitity;
        private string? publisherId;
        private string? publisherAddress;
        private bool postRefresh;
        private Buffer postNestedMsgPayLoad = new Buffer();
        private MarketPriceRefresh marketPriceRefresh = new MarketPriceRefresh();
        private MarketPriceUpdate marketPriceUpdate = new MarketPriceUpdate();        
        private Buffer postItemName;        
        private DataDictionary? dictionary;

        private IPostMsg postMsg = new Msg();
        private FieldList fList = new FieldList();
        private FieldEntry fEntry = new FieldEntry();
        private Real tempReal = new Real();
        private EncodeIterator encIter = new EncodeIterator();
        private EncodeIterator postMsgEncIter = new EncodeIterator();
        private IStatusMsg statusMsg = new Msg();

        public bool EnableOffstreamPost { get; set; }
        public bool EnableOnstreamPost { get; set; }
        public int ServiceId { get; set; }
        public int StreamId { get; set; }
        public Buffer PostItemName { get => postItemName; set { postItemName.Data(value.Data(), value.Position, value.Length); postItemName.Data().Flip(); } }
        public DataDictionary? Dictionary { get => dictionary; set { dictionary = value; } }

        private bool offstreamPostSent = false;

        /// <summary>
        /// Instantiates a new post handler.
        /// </summary>
        public PostHandler()
        {
            postWithMsg = true;
            EnableOffstreamPost = false;
            EnableOnstreamPost = false;
            hasInputPrincipalIdentitity = false;
            nextPostId = 1;
            nextSeqNum = 1;
            itemData = 12.00;
            postItemName = new Buffer();
            postRefresh = true;
        }

        /// <summary>
        /// Initializes timer for Post messages to be sent. 
        /// This method simply gets the current time and sets up the first time 
        /// that a post message should be sent based off of that and the post interval.
        /// </summary>
        public void InitPostHandler()
        {
            nextPostTime = DateTimeOffset.Now.ToUnixTimeMilliseconds() + POST_MESSAGE_FREQUENCY * 1000;
        }

        public void SetPublisherInfo(string id, string address)
        {
            hasInputPrincipalIdentitity = true;
            publisherId = id;
            publisherAddress = address;
        }

        private double NextItemData()
        {
            itemData += 0.01;
            return itemData;
        }

        /// <summary>
        /// Uses the current time and the nextPostTime to determine whether a postMsg should be sent. 
        /// If a post message should be sent, the time is calculated for the next post after this one. 
        /// Additionally, the postWithMsg flag is toggled so that posting alternates 
        /// between posting with a full message as payload or data as payload.
        /// </summary>
        /// <param name="chnl">The channel for which posting is handled</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode HandlePosts(ChannelSession chnl, out Error? error)
        {
            long currentTime = DateTimeOffset.Now.ToUnixTimeMilliseconds();

            if (currentTime >= nextPostTime)
            {
                if (EnableOnstreamPost)
                {
                    TransportReturnCode ret = SendOnstreamPostMsg(chnl, postWithMsg, out error);
                    if (ret != TransportReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
                if (EnableOffstreamPost)
                {
                    TransportReturnCode ret = SendOffstreamPostMsg(chnl, postWithMsg, out error);
                    if (ret != TransportReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }

                nextPostTime = currentTime + POST_MESSAGE_FREQUENCY * 1000;

                /* iterate between post with msg and post with data */
                if (postWithMsg == true)
                {
                    postWithMsg = false;
                }
                else
                {
                    postWithMsg = true;
                }
            }

            error = null;
            return TransportReturnCode.SUCCESS;
        }

        private TransportReturnCode SendOnstreamPostMsg(ChannelSession chnlSession, bool postWithMsg, out Error? error)
        {
            error = null;
            CodecReturnCode ret;
            if (StreamId == 0)
            {
                Console.WriteLine("Currently no available market price streams to on-stream post to.  Will retry shortly.");
                return TransportReturnCode.SUCCESS;
            }

            ITransportBuffer? msgBuf = chnlSession.GetTransportBuffer(TRANSPORT_BUFFER_SIZE_MSG_POST, false, out error);
            if (msgBuf == null)
                return TransportReturnCode.FAILURE;

            if (postWithMsg == true)
            {
                if ((ret = EncodePostWithMsg(chnlSession, msgBuf)) != CodecReturnCode.SUCCESS)
                {
                    error = new Error()
                    {
                        Text = $"Failed encoding post with message, return code {ret.GetAsString()}",
                        ErrorId = TransportReturnCode.FAILURE
                    };
                    return TransportReturnCode.FAILURE;
                }
            }
            else
            {
                if ((ret = EncodePostWithData(chnlSession, msgBuf)) != CodecReturnCode.SUCCESS)
                {
                    error = new Error()
                    {
                        Text = $"Failed encoding post with data, return code {ret.GetAsString()}",
                        ErrorId = TransportReturnCode.FAILURE
                    };
                    return TransportReturnCode.FAILURE;
                }
            }

            return chnlSession.Write(msgBuf, out error);
        }

        private TransportReturnCode SendOffstreamPostMsg(ChannelSession chnlSession, bool postWithMsg, out Error? error)
        {
            ITransportBuffer? msgBuf = chnlSession.GetTransportBuffer(TRANSPORT_BUFFER_SIZE_DATA_POST, false, out error);
            if (msgBuf == null)
                return TransportReturnCode.FAILURE;

            CodecReturnCode ret = EncodePostWithMsg(chnlSession, msgBuf);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = $"Failed encoding post with message, return code {ret.GetAsString()}",
                    ErrorId = TransportReturnCode.FAILURE
                };
                return TransportReturnCode.FAILURE;
            }

            TransportReturnCode wRet = chnlSession.Write(msgBuf, out error);
            if (wRet != TransportReturnCode.SUCCESS)
                return TransportReturnCode.FAILURE;

            offstreamPostSent = true;
            return TransportReturnCode.SUCCESS;
        }

        private CodecReturnCode EncodePostWithMsg(ChannelSession chnlSession, ITransportBuffer msgBuf)
        {
            postMsg.Clear();

            postMsg.MsgClass = MsgClasses.POST;
            postMsg.StreamId = StreamId;
            postMsg.DomainType = (int)DomainType.MARKET_PRICE;
            postMsg.ContainerType = DataTypes.MSG;

            postMsg.ApplyPostComplete();
            postMsg.ApplyAck();
            postMsg.ApplyHasPostId();
            postMsg.ApplyHasSeqNum();
            postMsg.ApplyHasMsgKey();
            postMsg.ApplyHasPostUserRights();
            postMsg.PostId = nextPostId++;
            postMsg.SeqNum = nextSeqNum++;

            if (hasInputPrincipalIdentitity)
            {
                postMsg.PostUserInfo.UserAddrFromString(publisherAddress);
                postMsg.PostUserInfo.UserId = int.Parse(publisherId!);
            }
            else
            {
                try
                {
                    postMsg.PostUserInfo.UserAddrFromString(Dns.GetHostAddresses(Dns.GetHostName())
                        .Where(ip => ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)!.FirstOrDefault()?.ToString() ?? "");
                }
                catch (Exception e)
                {
                    Console.WriteLine("Populating postUserInfo failed. Exception: " + e.Message);
                    return CodecReturnCode.FAILURE;
                }
                postMsg.PostUserInfo.UserId = Environment.ProcessId;
            }
            postMsg.PostUserRights = PostUserRights.CREATE | PostUserRights.DELETE;
            postMsg.MsgKey.ApplyHasNameType();
            postMsg.MsgKey.ApplyHasName();
            postMsg.MsgKey.ApplyHasServiceId();
            postMsg.MsgKey.Name.Data(postItemName.Data(), postItemName.Position, postItemName.Length);
            postMsg.MsgKey.NameType = InstrumentNameTypes.RIC;
            postMsg.MsgKey.ServiceId = ServiceId;

            encIter.Clear();
            CodecReturnCode ret = encIter.SetBufferAndRWFVersion(msgBuf, chnlSession.Channel!.MajorVersion, chnlSession.Channel.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("Encoder.setBufferAndRWFVersion() failed:  <" + ret + ">");
                return ret;
            }

            ret = postMsg.EncodeInit(encIter, 0);
            if (ret != CodecReturnCode.ENCODE_CONTAINER)
            {
                Console.WriteLine("EncodeMsgInit() failed:  <" + ret + ">");
                return ret;
            }

            // encode market price response into a buffer

            MarketPriceItem mpItemInfo = new MarketPriceItem();
            mpItemInfo.InitFields();
            mpItemInfo.TRDPRC_1 = NextItemData();
            mpItemInfo.BID = NextItemData();
            mpItemInfo.ASK = NextItemData();

            // get a buffer for nested market price refresh
            postNestedMsgPayLoad = new Buffer();
            postNestedMsgPayLoad.Data(new ByteBuffer(1024));

            // Although we are encoding RWF message, this code
            // encodes nested message into a separate buffer.
            // this is because MarketPrice.encode message is shared by all
            // applications, and it expects to encode the message into a stand alone
            // buffer.
            ret = encIter.EncodeNonRWFInit(postNestedMsgPayLoad);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("EncodeNonRWFDataTypeInit() failed:  <" + ret + ">");
                return ret;
            }

            postMsgEncIter.Clear();
            ret = postMsgEncIter.SetBufferAndRWFVersion(postNestedMsgPayLoad, chnlSession.Channel.MajorVersion, chnlSession.Channel.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("EncodeIter.setBufferAndRWFVersion() failed:  <" + ret + ">");
                return ret;
            }

            if (postRefresh)
            {
                marketPriceRefresh.Clear();
                marketPriceRefresh.RefreshComplete = true;
                marketPriceRefresh.ClearCache = true;
                marketPriceRefresh.StreamId = StreamId;
                marketPriceRefresh.ItemName.Data(postItemName.Data(), postItemName.Position, postItemName.Length);
                marketPriceRefresh.State.StreamState(StreamStates.OPEN);
                marketPriceRefresh.State.DataState(DataStates.OK);
                marketPriceRefresh.State.Code(StateCodes.NONE);
                marketPriceRefresh.State.Text().Data("Item Refresh Completed");
                marketPriceRefresh.ServiceId = ServiceId;
                marketPriceRefresh.HasServiceId = true;
                marketPriceRefresh.MarketPriceItem = mpItemInfo;
                marketPriceRefresh.HasQos = true;
                marketPriceRefresh.Qos.IsDynamic = false;
                marketPriceRefresh.Qos.Rate(QosRates.TICK_BY_TICK);
                marketPriceRefresh.Qos.Timeliness(QosTimeliness.REALTIME);
                marketPriceRefresh.DataDictionary = dictionary;

                ret = marketPriceRefresh.Encode(postMsgEncIter);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("encodeMarketPriceRefresh() failed:  <" + ret + ">");
                    return ret;
                }
                postRefresh = false;
            }
            else
            {
                marketPriceUpdate.Clear();
                marketPriceUpdate.StreamId = StreamId;
                marketPriceUpdate.ItemName.Data(postItemName.Data(), postItemName.Position, postItemName.Length);
                marketPriceUpdate.MarketPriceItem = mpItemInfo;
                marketPriceUpdate.DataDictionary = dictionary;

                ret = marketPriceUpdate.Encode(postMsgEncIter);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("encodeMarketPriceUpdate() failed:  <" + ret + ">");
                    return ret;
                }
            }

            ret = encIter.EncodeNonRWFComplete(postNestedMsgPayLoad, true);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("EncodeNonRWFDataTypeComplete() failed:  <" + ret + ">");
                return ret;
            }

            if ((ret = postMsg.EncodeComplete(encIter, true)) < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("EncodeMsgComplete() failed with return code: " + ret);
                return ret;
            }

            Console.WriteLine("\n\nSENDING POST WITH MESSAGE:\n" + "  streamId = " + postMsg.StreamId + "\n  postId   = " + postMsg.PostId + "\n  seqNum   = " + postMsg.SeqNum);

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode EncodePostWithData(ChannelSession chnlSession, ITransportBuffer msgBuf)
        {
            CodecReturnCode ret = 0;
            IDictionaryEntry? dictionaryEntry = null;
            MarketPriceItem itemInfo = new MarketPriceItem();

            encIter.Clear();
            fList.Clear();
            fEntry.Clear();

            postMsg.MsgClass = MsgClasses.POST;
            postMsg.StreamId = StreamId;
            postMsg.DomainType = (int)DomainType.MARKET_PRICE;
            postMsg.ContainerType = DataTypes.FIELD_LIST;

            postMsg.ApplyPostComplete();
            postMsg.ApplyAck();
            postMsg.ApplyHasPostId();
            postMsg.ApplyHasSeqNum();
            postMsg.PostId = nextPostId++;
            postMsg.SeqNum = nextSeqNum++;
    
            if (hasInputPrincipalIdentitity)
            {
                postMsg.PostUserInfo.UserAddrFromString(publisherAddress);
                postMsg.PostUserInfo.UserId = int.Parse(publisherId!);
            }
            else
            {
                try
                {
                    postMsg.PostUserInfo.UserAddrFromString(Dns.GetHostAddresses(Dns.GetHostName())
                        .Where(ip => ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)!.FirstOrDefault()?.ToString() ?? "");
                }
                catch (Exception e)
                {
                    Console.WriteLine("Populating postUserInfo failed. Exception: " + e.Message);
                    return CodecReturnCode.FAILURE;
                }
                postMsg.PostUserInfo.UserId = Environment.ProcessId;
            }
            encIter.SetBufferAndRWFVersion(msgBuf, chnlSession.Channel!.MajorVersion, chnlSession.Channel.MinorVersion);

            if ((ret = postMsg.EncodeInit(encIter, 0)) < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("EncodeMsgInit() failed: <" + ret.GetAsString() + ">");
                return ret;
            }

            itemInfo.InitFields();
            itemInfo.TRDPRC_1 = NextItemData();
            itemInfo.BID = NextItemData();
            itemInfo.ASK = NextItemData();

            fList.ApplyHasStandardData();
            if ((ret = fList.EncodeInit(encIter, null, 0)) < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("EncodeFieldListInit() failed: <" + ret.GetAsString() + ">");
                return ret;
            }

            // TRDPRC_1
            fEntry.Clear();
            dictionaryEntry = dictionary!.Entry(MarketPriceItem.TRDPRC_1_FID);
            if (dictionaryEntry != null)
            {
                fEntry.FieldId = MarketPriceItem.TRDPRC_1_FID;
                fEntry.DataType = dictionaryEntry.GetRwfType();
                tempReal.Clear();
                tempReal.Value(itemInfo.TRDPRC_1, RealHints.EXPONENT_2);
                if ((ret = fEntry.Encode(encIter, tempReal)) < CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("EncodeFieldEntry() failed: <" + ret.GetAsString() + ">");
                    return ret;
                }
            }
            // BID
            fEntry.Clear();
            dictionaryEntry = dictionary.Entry(MarketPriceItem.BID_FID);
            if (dictionaryEntry != null)
            {
                fEntry.FieldId = MarketPriceItem.BID_FID;
                fEntry.DataType = dictionaryEntry.GetRwfType();
                tempReal.Clear();
                tempReal.Value(itemInfo.BID, RealHints.EXPONENT_2);
                if ((ret = fEntry.Encode(encIter, tempReal)) < CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("EncodeFieldEntry() failed: <" + ret.GetAsString() + ">");

                    return ret;
                }
            }
            // ASK
            fEntry.Clear();
            dictionaryEntry = dictionary.Entry(MarketPriceItem.ASK_FID);
            if (dictionaryEntry != null)
            {
                fEntry.FieldId = MarketPriceItem.ASK_FID;
                fEntry.DataType = dictionaryEntry.GetRwfType();
                tempReal.Clear();
                tempReal.Value(itemInfo.ASK, RealHints.EXPONENT_2);
                if ((ret = fEntry.Encode(encIter, tempReal)) < CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("EncodeFieldEntry() failed: <" + ret.GetAsString() + ">");
                    return ret;
                }
            }

            if ((ret = fList.EncodeComplete(encIter, true)) < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("EncodeFieldListComplete() failed: <" + ret.GetAsString() + ">");

                return ret;
            }

            if ((ret = postMsg.EncodeComplete(encIter, true)) < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("EncodeMsgComplete() failed: <" + ret.GetAsString() + ">");
                return ret;
            }

            Console.WriteLine("\n\nSENDING POST WITH DATA:\n" + "  streamId = " + postMsg.StreamId + "\n  postId   = " + postMsg.PostId + "\n  seqNum   = " + postMsg.SeqNum);

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// This function encodes and sends an off-stream post close status message.
        /// </summary>
        /// <param name="channelSession">The current channel session</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode CloseOffStreamPost(ChannelSession channelSession, Error? error)
        {
            if (!offstreamPostSent)
                return TransportReturnCode.SUCCESS;

            ITransportBuffer? msgBuf = channelSession.GetTransportBuffer(TRANSPORT_BUFFER_SIZE_MSG_POST, false, out error);
            if (msgBuf == null)
                return TransportReturnCode.FAILURE;

            postMsg.Clear();

            postMsg.MsgClass = MsgClasses.POST;
            postMsg.StreamId = StreamId;
            postMsg.DomainType = (int)DomainType.MARKET_PRICE;
            postMsg.ContainerType = DataTypes.MSG;

            // Note: This example sends a status close when it shuts down.
            // So don't ask for an ACK (that we will never get)
            postMsg.Flags = PostMsgFlags.POST_COMPLETE | PostMsgFlags.HAS_POST_ID | PostMsgFlags.HAS_SEQ_NUM | PostMsgFlags.HAS_POST_USER_RIGHTS | PostMsgFlags.HAS_MSG_KEY;

            postMsg.PostId = nextPostId++;
            postMsg.SeqNum = nextSeqNum++;
            postMsg.PostUserRights = PostUserRights.CREATE | PostUserRights.DELETE;

            postMsg.MsgKey.ApplyHasNameType();
            postMsg.MsgKey.ApplyHasName();
            postMsg.MsgKey.ApplyHasServiceId();
            postMsg.MsgKey.Name.Data(postItemName.Data(), postItemName.Position, postItemName.Length);
            postMsg.MsgKey.NameType = InstrumentNameTypes.RIC;
            postMsg.MsgKey.ServiceId = ServiceId;
       
            if (hasInputPrincipalIdentitity)
            {
                postMsg.PostUserInfo.UserAddrFromString(publisherAddress);
                postMsg.PostUserInfo.UserId = int.Parse(publisherId!);
            }
            else
            {
                try
                {
                    postMsg.PostUserInfo.UserAddrFromString(Dns.GetHostAddresses(Dns.GetHostName())
                        .Where(ip => ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)!.FirstOrDefault()?.ToString() ?? "");
                }
                catch (Exception e)
                {
                    Console.WriteLine("Populating postUserInfo failed. Dns.GetHostAddresses(Dns.GetHostName()) exception: " + e.Message);
                    error = new Error()
                    {
                        Text = "Populating postUserInfo failed. Dns.GetHostAddresses(Dns.GetHostName()) exception: " + e.Message,
                        ErrorId = TransportReturnCode.FAILURE
                    };
                    return TransportReturnCode.FAILURE;
                }
                postMsg.PostUserInfo.UserId = Environment.ProcessId;
            }

            statusMsg.Flags = StatusMsgFlags.HAS_STATE;
            statusMsg.MsgClass = MsgClasses.STATUS;
            statusMsg.StreamId = StreamId;
            statusMsg.DomainType = (int)DomainType.MARKET_PRICE;
            statusMsg.ContainerType = DataTypes.NO_DATA;
            statusMsg.State.StreamState(StreamStates.CLOSED);
            statusMsg.State.DataState(DataStates.SUSPECT);

            encIter.Clear();
            CodecReturnCode ret = encIter.SetBufferAndRWFVersion(msgBuf, channelSession.Channel!.MajorVersion, channelSession.Channel.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("Encoder.SetBufferAndRWFVersion() failed:  <" + ret + ">");
                error = new Error()
                {
                    Text = "Encoder.SetBufferAndRWFVersion() failed:  <" + ret + ">",
                    ErrorId = TransportReturnCode.FAILURE
                };
                return TransportReturnCode.FAILURE;
            }

            ret = postMsg.EncodeInit(encIter, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("PostMsg.EncodeInit() failed:  <" + ret + ">");
                error = new Error()
                {
                    Text = "PostMsg.EncodeInit() failed:  <" + ret + ">",
                    ErrorId = TransportReturnCode.FAILURE
                };
                return TransportReturnCode.FAILURE;
            }

            ret = statusMsg.EncodeInit(encIter, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("StatusMsg.EncodeInit() failed:  <" + ret + ">");
                error = new Error()
                {
                    Text = "StatusMsg.EncodeInit() failed:  <" + ret + ">",
                    ErrorId = TransportReturnCode.FAILURE
                };
                return TransportReturnCode.FAILURE;
            }

            ret = statusMsg.EncodeComplete(encIter, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("StatusMsg.EncodeComplete() failed:  <" + ret + ">");
                error = new Error()
                {
                    Text = "StatusMsg.EncodeComplete() failed:  <" + ret + ">",
                    ErrorId = TransportReturnCode.FAILURE
                };
                return TransportReturnCode.FAILURE;
            }

            ret = postMsg.EncodeComplete(encIter, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("PostMsg.EncodeComplete() failed:  <" + ret + ">");
                error = new Error()
                {
                    Text = "PostMsg.EncodeComplete() failed:  <" + ret + ">",
                    ErrorId = TransportReturnCode.FAILURE
                };
                return TransportReturnCode.FAILURE;
            }

            return channelSession.Write(msgBuf, out error);
        }
    }
}
