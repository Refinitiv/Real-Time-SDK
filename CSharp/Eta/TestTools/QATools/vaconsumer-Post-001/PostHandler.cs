/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;
using System.Net;

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Reactor;


namespace LSEG.Eta.ValueAdd.Consumer
{
    /// <summary>
    /// This is the post handler for the ETA Value Add consumer application.
    /// </summary>
    ///
    /// It provides methods for sending on stream and off stream post messages.
    /// This is the post handler for the ETA Value Add consumer application.
    /// It provides methods for sending on stream and off stream post messages.
    ///
    internal class PostHandler
    {
        public bool EnableOnstreamPost { get; internal set; }

        public bool EnableOffstreamPost { get; internal set; }

        /// <summary>
        /// Stream Id for posting data.
        /// </summary>
        public int StreamId { get; internal set; }

        /// <summary>
        /// Service id for posting message.
        /// </summary>
        public int ServiceId { get; internal set; }

        /// <summary>
        /// Data dictionary used for encoding posting data.
        /// </summary>
        internal DataDictionary Dictionary { get; set; } = new();

        /// <summary>
        /// Item name for posting message.
        /// </summary>
        public Codec.Buffer PostItemName { get; internal set; } = new Codec.Buffer();

        private const int TRANSPORT_BUFFER_SIZE_MSG_POST = 5000;
        private const int TRANSPORT_BUFFER_SIZE_DATA_POST = 5000;

        private int m_NextPostId;
        private int m_NextSeqNum;
        private double m_ItemData;

        private readonly TimeSpan POST_MESSAGE_FREQUENCY = TimeSpan.FromSeconds(5);

        private System.DateTime m_NextPostTime = System.DateTime.MinValue;
        private bool m_PostWithMsg;
        private bool m_HasInputPrincipalIdentitity;
        private string m_PublisherId = String.Empty;
        private string m_PublisherAddress = String.Empty;
        private bool m_PostRefresh;
        private MarketPriceRefresh m_MarketPriceRefresh = new();
        private MarketPriceUpdate m_MarketPriceUpdate = new();

        private IPostMsg m_PostMsg = (IPostMsg)new Msg();
        private FieldList fList = new();
        private FieldEntry fEntry = new();
        private Codec.Real tempReal = new();
        private EncodeIterator m_EncodeIterator = new();
        private EncodeIterator m_PostMessageEncodeIterator = new();
        private IStatusMsg m_StatusMsg = (IStatusMsg)new Msg();
        private bool m_OffstreamPostSent = false;

        private ReactorSubmitOptions m_SubmitOptions = new();

        public PostHandler()
        {
            m_PostWithMsg = true;
            m_HasInputPrincipalIdentitity = false;
            m_NextPostId = 1;
            m_NextSeqNum = 1;
            m_ItemData = 12.00;
            m_PostRefresh = true;
        }

        /// <summary>
        /// Set user provided publisher id and publisher address
        /// </summary>
        /// <param name="publisherId"></param>
        /// <param name="publisherAddress"></param>
        public void SetPublisherInfo(string publisherId, string publisherAddress)
        {
            this.m_HasInputPrincipalIdentitity = true;
            this.m_PublisherId = publisherId;
            this.m_PublisherAddress = publisherAddress;
        }


        /// <summary>
        /// increases the value of the data being posted
        /// </summary>
        /// <returns></returns>
        private double NextItemData()
        {
            m_ItemData += 0.01;
            return m_ItemData;
        }


        /// <summary>
        /// Initializes timer for Post messages to be sent This method simply gets
        /// the current time and sets up the first time that a post message should be
        /// </summary>
        public void InitPostHandler()
        {
            m_NextPostTime = System.DateTime.Now + POST_MESSAGE_FREQUENCY;
        }


        /// <summary>
        /// Uses the current time and the nextPostTime to determine whether a postMsg
        /// should be sent.
        /// </summary>
        ///
        /// If a post message should be sent, the time is calculated for the next post
        /// after this one. Additionally, the postWithMsg flag is toggled so that posting
        /// alternates between posting with a full message as payload or data as payload.
        public ReactorReturnCode HandlePosts(ReactorChannel chnl, out ReactorErrorInfo? errorInfo)
        {
            System.DateTime currentTime = System.DateTime.Now;

            if (currentTime >= m_NextPostTime)
            {
                if (EnableOnstreamPost)
                {
                    ReactorReturnCode ret = SendOnstreamPostMsg(chnl, m_PostWithMsg, out errorInfo);
                    if (ret != ReactorReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
                if (EnableOffstreamPost)
                {
                    ReactorReturnCode ret = SendOffstreamPostMsg(chnl, out errorInfo);
                    if (ret != ReactorReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }

                m_NextPostTime = currentTime + POST_MESSAGE_FREQUENCY;

                /* iterate between post with msg and post with data */
                if (m_PostWithMsg == true)
                {
                    m_PostWithMsg = false;
                }
                else
                {
                    m_PostWithMsg = true;
                }
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }


        /// <summary>
        /// Encodes and sends an on-stream post message.
        /// </summary>
        ///
        /// It will either send a post that contains a full message or a post that
        /// contains only data payload, based on the postWithMsg parameter. If true,
        /// post will contain a message.
        ///
        /// This method only sends one post message, however it is called
        /// periodically over a time increment by the handlePosts method
        private ReactorReturnCode SendOnstreamPostMsg(ReactorChannel chnl, bool postWithMsg, out ReactorErrorInfo? errorInfo)
        {
            if (StreamId == 0)
            {
                // no items available to post on
                Console.WriteLine("Currently no available market price streams to on-stream post to.  Will retry shortly.");
                errorInfo = null;
                return ReactorReturnCode.SUCCESS;
            }

            // get a buffer for the item request
            ITransportBuffer? msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_MSG_POST, false, out errorInfo);
            if (msgBuf == null)
                return ReactorReturnCode.FAILURE;

            CodecReturnCode ret;
            if (postWithMsg == true)
            {

                if ((ret = EncodePostWithMsg(chnl, msgBuf)) != CodecReturnCode.SUCCESS)
                {
                    return (ReactorReturnCode)ret;
                }
            }
            else
            {
                if ((ret = EncodePostWithData(chnl, msgBuf)) != CodecReturnCode.SUCCESS)
                {
                    return (ReactorReturnCode)ret;
                }
            }

            // send post message
            return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }


        /// Encodes and sends an off-stream post message.
        ///
        /// This method only sends one post message, however it is called
        /// periodically over a time increment by the handlePosts method
        private ReactorReturnCode SendOffstreamPostMsg(ReactorChannel chnl, out ReactorErrorInfo? errorInfo)
        {
            // get a buffer for the item request
            ITransportBuffer? msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_DATA_POST, false, out errorInfo);
            if (msgBuf == null)
                return ReactorReturnCode.FAILURE;

            CodecReturnCode ret = EncodePostWithMsg(chnl, msgBuf);
            if (ret != CodecReturnCode.SUCCESS)
                return ReactorReturnCode.FAILURE;

            // send post message
            var submitRet = chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
            if (submitRet != ReactorReturnCode.SUCCESS)
                return ReactorReturnCode.FAILURE;

            m_OffstreamPostSent = true;
            return ReactorReturnCode.SUCCESS;
        }



        /// <summary>
        /// It encodes a PostMsg, populating the postUserInfo with the IPAddress and
        /// process ID of the machine running the application.
        /// </summary>
        ///
        /// The payload of the PostMsg is an UpdateMsg. The UpdateMsg contains a FieldList
        /// containing several fields. The same message encoding functionality used by the
        /// provider application is leveraged here to encode the contents.
        ///
        /// This method is internally used by sendPostMsg
        ///
        private CodecReturnCode EncodePostWithMsg(ReactorChannel chnl, ITransportBuffer msgBuf)
        {
            // First encode message for payload
            m_PostMsg.Clear();

            // set-up message
            m_PostMsg.MsgClass = MsgClasses.POST;
            m_PostMsg.StreamId = StreamId;
            m_PostMsg.DomainType = (int)DomainType.MARKET_PRICE;
            m_PostMsg.ContainerType = DataTypes.MSG;

            // Note: post message key not required for on-stream post
            m_PostMsg.ApplyPostComplete();
            m_PostMsg.ApplyAck();
            m_PostMsg.ApplyHasPostId();
            m_PostMsg.ApplyHasSeqNum();
            m_PostMsg.ApplyHasMsgKey();
            m_PostMsg.ApplyHasPostUserRights();
            m_PostMsg.PostId = m_NextPostId++;
            m_PostMsg.SeqNum = m_NextSeqNum++;

            // populate post user info
            if (m_HasInputPrincipalIdentitity)
            {
                m_PostMsg.PostUserInfo.UserAddrFromString(m_PublisherAddress);
                m_PostMsg.PostUserInfo.UserId = Int32.Parse(m_PublisherId);
            }
            else
            {
                try
                {
                    m_PostMsg.PostUserInfo.UserAddr = BitConverter.ToUInt32(Dns.GetHostAddresses(Dns.GetHostName())
                        .Where(ip => ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)!.FirstOrDefault()?.GetAddressBytes());
                }
                catch (Exception e)
                {
                    Console.WriteLine("Populating postUserInfo failed. Dns.GetHostAddresses(Dns.GetHostName()) exception: " + e.Message);
                    return CodecReturnCode.FAILURE;
                }
                m_PostMsg.PostUserInfo.UserId = Process.GetCurrentProcess().Id;
            }

            m_PostMsg.PostUserRights = PostUserRights.CREATE | PostUserRights.DELETE;

            m_PostMsg.MsgKey.ApplyHasNameType();
            m_PostMsg.MsgKey.ApplyHasName();
            m_PostMsg.MsgKey.ApplyHasServiceId();
            m_PostMsg.MsgKey.Name.Data(PostItemName.Data(), PostItemName.Position, PostItemName.Length);

            m_PostMsg.MsgKey.NameType = InstrumentNameTypes.RIC;
            m_PostMsg.MsgKey.ServiceId = ServiceId;

            //APIQA
            /* Encode Field List for putting it as the attib container */
            {
                FieldList fList = new FieldList();
                FieldEntry fEntry = new FieldEntry();
                IDictionaryEntry dictionaryEntry;
                UInt uintValue = new UInt();
                Codec.Buffer keyBuffer = new Codec.Buffer();
                keyBuffer.Data(new ByteBuffer(1024));
                Codec.Buffer inputValue = new Codec.Buffer();
                m_EncodeIterator.Clear();
                CodecReturnCode ret1 = m_EncodeIterator.SetBufferAndRWFVersion(keyBuffer, chnl.MajorVersion, chnl.MinorVersion);
                if (ret1 != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("m_EncodeIterator SetBufferAndRWFVersion() failed:  <{ret1}>");
                    return ret1;
                }
                fList.Flags = FieldListFlags.HAS_STANDARD_DATA;

                if ((ret1 = fList.EncodeInit(m_EncodeIterator, null, 0)) < CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("fEntry Encode() failed: <" + ret1.GetAsString() + ">");
                    return ret1;
                }

                /* CD_VER */
                dictionaryEntry = Dictionary.Entry(32745);
                if (dictionaryEntry != null)
                {
                    fEntry.FieldId = 32745;
                    fEntry.DataType = dictionaryEntry.RwfType;
                    uintValue.Value(1);
                    if ((ret1 = fEntry.Encode(m_EncodeIterator, uintValue)) < CodecReturnCode.SUCCESS)
                    {
                        Console.WriteLine("fEntry Encode() failed: <" + ret1.GetAsString() + ">");
                        return ret1;
                    }
                }
                /* CD_TOKEN */
                fEntry.Clear();
                dictionaryEntry = Dictionary.Entry(32746);
                if (dictionaryEntry != null)
                {
                    fEntry.FieldId = 32746;
                    fEntry.DataType = dictionaryEntry.RwfType;
                    inputValue.Data("QVFJQzV3TTJMWTRTZmN5WElqRNE9USTFNems1QUFKVE1RQUNNREk9Iw==");
                    if ((ret1 = fEntry.Encode(m_EncodeIterator, inputValue)) < CodecReturnCode.SUCCESS)
                    {
                        Console.WriteLine("fEntry Encode() failed: <" + ret1.GetAsString() + ">");
                        return ret1;
                    }
                }
                /* CD_APPTYPE */
                fEntry.Clear();
                dictionaryEntry = Dictionary.Entry(32747);
                if (dictionaryEntry != null)
                {
                    fEntry.FieldId = 32747;
                    fEntry.DataType = dictionaryEntry.RwfType;
                    inputValue.Data("Insertlink");
                    if ((ret1 = fEntry.Encode(m_EncodeIterator, inputValue)) < CodecReturnCode.SUCCESS)
                    {
                        Console.WriteLine("fEntry Encode() failed: <" + ret1.GetAsString() + ">");
                        return ret1;
                    }
                }
                /* CD_APPNAME */
                fEntry.Clear();
                dictionaryEntry = Dictionary.Entry(32748);
                if (dictionaryEntry != null)
                {
                    fEntry.FieldId = 32748;
                    fEntry.DataType = dictionaryEntry.RwfType;
                    inputValue.Data("upalibtest");
                    if ((ret1 = fEntry.Encode(m_EncodeIterator, inputValue)) < CodecReturnCode.SUCCESS)
                    {
                        Console.WriteLine("fEntry Encode() failed: <" + ret1.GetAsString() + ">");
                        return ret1;
                    }
                }
                /* CD_APPVER */
                fEntry.Clear();
                dictionaryEntry = Dictionary.Entry(32749);
                if (dictionaryEntry != null)
                {
                    fEntry.FieldId = 32749;
                    fEntry.DataType = dictionaryEntry.RwfType;
                    inputValue.Data("1.0");
                    if ((ret1 = fEntry.Encode(m_EncodeIterator, inputValue)) < CodecReturnCode.SUCCESS)
                    {
                        Console.WriteLine("fEntry Encode() failed: <" + ret1.GetAsString() + ">");
                        return ret1;
                    }
                }
                /* CD_RSRVD1 */
                fEntry.Clear();
                dictionaryEntry = Dictionary.Entry(32750);
                if (dictionaryEntry != null)
                {
                    fEntry.FieldId = 32750;
                    fEntry.DataType = dictionaryEntry.RwfType;
                    inputValue.Data("MmJtMjU3ag==");
                    if ((ret1 = fEntry.Encode(m_EncodeIterator, inputValue)) < CodecReturnCode.SUCCESS)
                    {
                        Console.WriteLine("fEntry Encode() failed: <" + ret1.GetAsString() + ">");
                        return ret1;
                    }
                }

                /* complete encode field list */
                if((ret1 = fList.EncodeComplete(m_EncodeIterator, true))!= CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("fList EncodeComplete() failed: <" + ret1.GetAsString() + ">");
                    return ret1;
                }

                m_PostMsg.DomainType = (int)DomainType.CONTRIBUTION;
                m_PostMsg.MsgKey.AttribContainerType = DataTypes.FIELD_LIST;
                m_PostMsg.MsgKey.EncodedAttrib = keyBuffer;
                m_PostMsg.MsgKey.Flags |= MsgKeyFlags.HAS_ATTRIB;
            }
            //END APIQA

            // encode post message
            m_EncodeIterator.Clear();
            CodecReturnCode ret = m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"Encoder.SetBufferAndRWFVersion() failed:  <{ret}>");
                return ret;
            }

            ret = m_PostMsg.EncodeInit(m_EncodeIterator, 0);
            if (ret != CodecReturnCode.ENCODE_CONTAINER)
            {
                Console.WriteLine($"EncodeMsgInit() failed:  <{ret}>");
                return ret;
            }

            // encode market price response into a buffer

            MarketPriceItem mpItemInfo = new MarketPriceItem();
            mpItemInfo.InitFields();
            mpItemInfo.TRDPRC_1 = NextItemData();
            mpItemInfo.BID = NextItemData();
            mpItemInfo.ASK = NextItemData();

            // get a buffer for nested market price refresh
            Codec.Buffer postNestedMsgPayLoad = new();
            postNestedMsgPayLoad.Data(new ByteBuffer(1024));

            // Although we are encoding RWF message, this code
            // encodes nested message into a separate buffer.
            // this is because MarketPrice.encode message is shared by all
            // applications, and it expects to encode the message into a stand alone
            // buffer.
            ret = m_EncodeIterator.EncodeNonRWFInit(postNestedMsgPayLoad);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeNonRWFDataTypeInit() failed:  <{ret}>");
                return CodecReturnCode.FAILURE;
            }

            m_PostMessageEncodeIterator.Clear();
            ret = m_PostMessageEncodeIterator.SetBufferAndRWFVersion(postNestedMsgPayLoad, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeIter.setBufferAndRWFVersion() failed:  <{ret}>");
                return CodecReturnCode.FAILURE;
            }

            if (m_PostRefresh)
            {
                m_MarketPriceRefresh.Clear();
                m_MarketPriceRefresh.RefreshComplete = true;
                m_MarketPriceRefresh.ClearCache = true;
                m_MarketPriceRefresh.StreamId = StreamId;
                m_MarketPriceRefresh.ItemName.Data(PostItemName.Data(), PostItemName.Position, PostItemName.Length);
                m_MarketPriceRefresh.State.StreamState(StreamStates.OPEN);
                m_MarketPriceRefresh.State.DataState(DataStates.OK);
                m_MarketPriceRefresh.State.Code(StateCodes.NONE);
                m_MarketPriceRefresh.State.Text().Data("Item Refresh Completed");
                m_MarketPriceRefresh.ServiceId = ServiceId;
                m_MarketPriceRefresh.HasServiceId = true;
                m_MarketPriceRefresh.MarketPriceItem = mpItemInfo;
                m_MarketPriceRefresh.HasQos = true;
                m_MarketPriceRefresh.Qos.IsDynamic = false;
                m_MarketPriceRefresh.Qos.Rate(QosRates.TICK_BY_TICK);
                m_MarketPriceRefresh.Qos.Timeliness(QosTimeliness.REALTIME);
                m_MarketPriceRefresh.DataDictionary = Dictionary;

                ret = m_MarketPriceRefresh.Encode(m_PostMessageEncodeIterator);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine($"EncodeMarketPriceRefresh() failed:  <{ret}>");
                    return CodecReturnCode.FAILURE;
                }
                m_PostRefresh = false;
            }
            else
            {
                m_MarketPriceUpdate.Clear();
                m_MarketPriceUpdate.StreamId = StreamId;
                m_MarketPriceUpdate.ItemName.Data(PostItemName.Data(), PostItemName.Position, PostItemName.Length);
                m_MarketPriceUpdate.MarketPriceItem = mpItemInfo;
                m_MarketPriceUpdate.DataDictionary = Dictionary;

                ret = m_MarketPriceUpdate.Encode(m_PostMessageEncodeIterator);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine($"EncodeMarketPriceUpdate() failed:  <{ret}>");
                    return CodecReturnCode.FAILURE;
                }
            }

            ret = m_EncodeIterator.EncodeNonRWFComplete(postNestedMsgPayLoad, true);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeNonRWFDataTypeComplete() failed:  <{ret}>");
                return CodecReturnCode.FAILURE;
            }

            // complete encode message
            if ((ret = m_PostMsg.EncodeComplete(m_EncodeIterator, true)) < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeMsgComplete() failed with return code: {ret}");
                return ret;
            }

            Console.WriteLine("\n\nSENDING POST WITH MESSAGE:\n" + "  streamId = " + m_PostMsg.StreamId
                + "\n  postId   = " + m_PostMsg.PostId + "\n  seqNum   = " + m_PostMsg.SeqNum);

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// It encodes a PostMsg, populating the postUserInfo with the IPAddress and
        /// process ID of the machine running the application.
        /// </summary>
        ///
        /// The payload of the PostMsg is a FieldList containing several fields.
        ///
        /// This method is internally used by SendPostMsg
        ///
        private CodecReturnCode EncodePostWithData(ReactorChannel chnl, ITransportBuffer msgBuf)
        {
            CodecReturnCode ret = 0;
            IDictionaryEntry dictionaryEntry;
            MarketPriceItem itemInfo = new MarketPriceItem();

            // clear encode iterator
            m_EncodeIterator.Clear();
            fList.Clear();
            fEntry.Clear();

            // set-up message
            m_PostMsg.MsgClass = MsgClasses.POST;
            m_PostMsg.StreamId = StreamId;
            m_PostMsg.DomainType = (int)DomainType.MARKET_PRICE;
            m_PostMsg.ContainerType = DataTypes.FIELD_LIST;

            // Note: post message key not required for on-stream post
            m_PostMsg.ApplyPostComplete();
            m_PostMsg.ApplyAck(); // request ACK
            m_PostMsg.ApplyHasPostId();
            m_PostMsg.ApplyHasSeqNum();

            m_PostMsg.PostId = m_NextPostId++;
            m_PostMsg.SeqNum = m_NextSeqNum++;

            // populate post user info
            if (m_HasInputPrincipalIdentitity)
            {
                m_PostMsg.PostUserInfo.UserAddrFromString(m_PublisherAddress);
                m_PostMsg.PostUserInfo.UserId = Int32.Parse(m_PublisherId);

            }
            else
            {
                try
                {
                    m_PostMsg.PostUserInfo.UserAddr = BitConverter.ToUInt32(Dns.GetHostAddresses(Dns.GetHostName())
                        .Where(ip => ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)!.FirstOrDefault()?.GetAddressBytes());
                }
                catch (Exception e)
                {
                    Console.WriteLine("Populating postUserInfo failed. Dns.GetHostAddresses(Dns.GetHostName()) exception: " + e.Message);
                    return CodecReturnCode.FAILURE;
                }
                m_PostMsg.PostUserInfo.UserId = Process.GetCurrentProcess().Id;
            }

            // encode message
            m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);

            if ((ret = m_PostMsg.EncodeInit(m_EncodeIterator, 0)) < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeMsgInit() failed: <{ret}>");
                return ret;
            }

            itemInfo.InitFields();
            itemInfo.TRDPRC_1 = NextItemData();
            itemInfo.BID = NextItemData();
            itemInfo.ASK = NextItemData();

            // encode field list
            fList.ApplyHasStandardData();
            if ((ret = fList.EncodeInit(m_EncodeIterator, null, 0)) < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeFieldListInit() failed: <{ret}>");
                return ret;
            }

            // TRDPRC_1
            fEntry.Clear();
            dictionaryEntry = Dictionary.Entry(MarketPriceItem.TRDPRC_1_FID);
            if (dictionaryEntry != null)
            {
                fEntry.FieldId = MarketPriceItem.TRDPRC_1_FID;
                fEntry.DataType = dictionaryEntry.GetRwfType();
                tempReal.Clear();
                tempReal.Value(itemInfo.TRDPRC_1, RealHints.EXPONENT_2);
                if ((ret = fEntry.Encode(m_EncodeIterator, tempReal)) < CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine($"EncodeFieldEntry() failed: <{ret}>");
                    return ret;
                }
            }
            // BID
            fEntry.Clear();
            dictionaryEntry = Dictionary.Entry(MarketPriceItem.BID_FID);
            if (dictionaryEntry != null)
            {
                fEntry.FieldId = MarketPriceItem.BID_FID;
                fEntry.DataType = dictionaryEntry.GetRwfType();
                tempReal.Clear();
                tempReal.Value(itemInfo.BID, RealHints.EXPONENT_2);
                if ((ret = fEntry.Encode(m_EncodeIterator, tempReal)) < CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine($"EncodeFieldEntry() failed: <{ret}>");

                    return ret;
                }
            }
            // ASK
            fEntry.Clear();
            dictionaryEntry = Dictionary.Entry(MarketPriceItem.ASK_FID);
            if (dictionaryEntry != null)
            {
                fEntry.FieldId = MarketPriceItem.ASK_FID;
                fEntry.DataType = dictionaryEntry.GetRwfType();
                tempReal.Clear();
                tempReal.Value(itemInfo.ASK, RealHints.EXPONENT_2);
                if ((ret = fEntry.Encode(m_EncodeIterator, tempReal)) < CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine($"EncodeFieldEntry() failed: <{ret}>");
                    return ret;
                }
            }

            // complete encode field list
            if ((ret = fList.EncodeComplete(m_EncodeIterator, true)) < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeFieldListComplete() failed: <{ret}>");

                return ret;
            }

            // complete encode post message
            if ((ret = m_PostMsg.EncodeComplete(m_EncodeIterator, true)) < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeMsgComplete() failed: <{ret}>");
                return ret;
            }

            Console.WriteLine("\n\nSENDING POST WITH DATA:\n" + "  streamId = " + m_PostMsg.StreamId
                + "\n  postId   = " + m_PostMsg.PostId + "\n  seqNum   = " + m_PostMsg.SeqNum);

            return CodecReturnCode.SUCCESS;
        }


        /// <summary>
        /// Encodes and sends an off-stream post close status message.
        /// </summary>
        public ReactorReturnCode CloseOffStreamPost(ReactorChannel chnl, out ReactorErrorInfo? errorInfo)
        {
            // first check if we have sent offstream posts
            if (!m_OffstreamPostSent)
            {
                errorInfo = null;
                return ReactorReturnCode.SUCCESS;
            }

            // get a buffer for the item request
            ITransportBuffer? msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_MSG_POST, false, out errorInfo);
            if (msgBuf == null)
                return ReactorReturnCode.FAILURE;

            m_PostMsg.Clear();

            // set-up post message
            m_PostMsg.MsgClass = MsgClasses.POST;
            m_PostMsg.StreamId = StreamId;
            m_PostMsg.DomainType = (int)DomainType.MARKET_PRICE;
            m_PostMsg.ContainerType = DataTypes.MSG;

            // Note: This example sends a status close when it shuts down.
            // So don't ask for an ACK (that we will never get)
            m_PostMsg.Flags = PostMsgFlags.POST_COMPLETE
                | PostMsgFlags.HAS_POST_ID | PostMsgFlags.HAS_SEQ_NUM
                | PostMsgFlags.HAS_POST_USER_RIGHTS | PostMsgFlags.HAS_MSG_KEY;

            m_PostMsg.PostId = m_NextPostId++;
            m_PostMsg.SeqNum = m_NextSeqNum++;
            m_PostMsg.PostUserRights = PostUserRights.CREATE | PostUserRights.DELETE;

            // set post item name
            m_PostMsg.MsgKey.ApplyHasNameType();
            m_PostMsg.MsgKey.ApplyHasName();
            m_PostMsg.MsgKey.ApplyHasServiceId();
            m_PostMsg.MsgKey.Name.Data(PostItemName.Data(), PostItemName.Position, PostItemName.Length);
            m_PostMsg.MsgKey.NameType = InstrumentNameTypes.RIC;
            m_PostMsg.MsgKey.ServiceId = ServiceId;

            // populate default post user info
            if (m_HasInputPrincipalIdentitity)
            {
                m_PostMsg.PostUserInfo.UserAddrFromString(m_PublisherAddress);
                m_PostMsg.PostUserInfo.UserId = Int32.Parse(m_PublisherId);
            }
            else
            {
                try
                {
                    m_PostMsg.PostUserInfo.UserAddr = BitConverter.ToUInt32(Dns.GetHostAddresses(Dns.GetHostName())
                        .Where(ip => ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)!.FirstOrDefault()?.GetAddressBytes());
                }
                catch (Exception e)
                {
                    Console.WriteLine("Populating postUserInfo failed. Dns.GetHostAddresses(Dns.GetHostName()) exception: " + e.Message);
                    return ReactorReturnCode.FAILURE;
                }
                m_PostMsg.PostUserInfo.UserId = Process.GetCurrentProcess().Id;
            }

            // set-up status message that will be nested in the post message
            m_StatusMsg.Flags = StatusMsgFlags.HAS_STATE;
            m_StatusMsg.MsgClass = MsgClasses.STATUS;
            m_StatusMsg.StreamId = StreamId;
            m_StatusMsg.DomainType = (int)DomainType.MARKET_PRICE;
            m_StatusMsg.ContainerType = DataTypes.NO_DATA;
            m_StatusMsg.State.StreamState(StreamStates.CLOSED);
            m_StatusMsg.State.DataState(DataStates.SUSPECT);

            // encode post message
            m_EncodeIterator.Clear();
            CodecReturnCode ret = m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"Encoder.SetBufferAndRWFVersion() failed:  <{ret}>");
                return ReactorReturnCode.FAILURE;
            }

            ret = m_PostMsg.EncodeInit(m_EncodeIterator, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"PostMsg.EncodeInit() failed:  <{ret}>");
                return ReactorReturnCode.FAILURE;
            }

            // encode nested status message
            ret = m_StatusMsg.EncodeInit(m_EncodeIterator, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"StatusMsg.EncodeInit() failed:  <{ret}>");
                return ReactorReturnCode.FAILURE;
            }

            // complete encode status message
            ret = m_StatusMsg.EncodeComplete(m_EncodeIterator, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"StatusMsg.EncodeComplete() failed:  <{ret}>");
                return ReactorReturnCode.FAILURE;
            }

            // complete encode post message
            ret = m_PostMsg.EncodeComplete(m_EncodeIterator, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"PostMsg.EncodeComplete() failed:  <{ret}>");
                return ReactorReturnCode.FAILURE;
            }

            // send post message
            return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }
    }
}
