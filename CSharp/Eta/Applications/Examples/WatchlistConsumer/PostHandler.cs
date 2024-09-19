/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.WatchlistConsumer;

using System;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.Net.Sockets;

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Reactor;

/// <summary>
/// This is the post handler for the ETA Value Add consumer application.
/// </summary>
/// <remarks>It provides methods for sending on stream and off stream post messages.</remarks>
public class PostHandler
{
    private int m_NextPostId;
    private int m_NextSeqNum;
    private double m_ItemData;
    private static readonly TimeSpan m_PostMessageFrequency = TimeSpan.FromSeconds(5);
    private System.DateTime m_NextPostTime;
    private bool m_PostWithMsg;
    private string? m_PublisherId;
    private string? m_PublisherAddress;
    private bool m_PostRefresh;
    private Eta.Codec.Buffer m_PostNestedMsgPayLoad = new();
    private readonly MarketPriceRefresh m_MarketPriceRefresh = new();
    private readonly MarketPriceUpdate m_MarketPriceUpdate = new();

    private readonly Msg m_PostMsg = new();
    private readonly FieldList m_FList = new();
    private readonly FieldEntry m_FEntry = new();
    private readonly Real m_TempReal = new();
    private readonly EncodeIterator m_EncIter = new();
    private readonly EncodeIterator m_PostMsgEncIter = new();
    private readonly Msg m_StatusMsg = new();
    private bool m_OffstreamPostSent = false;

    private readonly ReactorSubmitOptions m_SubmitOptions = new();

    public PostHandler()
    {
        m_PostWithMsg = true;
        m_NextPostId = 1;
        m_NextSeqNum = 1;
        m_ItemData = 12.00;
        m_PostRefresh = true;
    }

    public Eta.Codec.Buffer PostItemName { get; }  = new();

    /// <summary>
    /// Enables offstream posting mode
    /// </summary>
    public bool EnableOffStreamPost { get; set; }

    /// <summary>
    /// Enables onstream posting mode
    /// </summary>
    public bool EnableOnStreamPost { get; set; }

    public bool HasInputPrincipalIdentitity { get; set; }

    public int StreamId { get; set; }

    public int ServiceId { get; set; }
    public DataDictionary? Dictionary { get; set; }

    /// <summary>
    /// Sets user provided publisher id and publisher address
    /// </summary>
    /// <param name="publisherId">Publisher id.</param>
    /// <param name="publisherAddress">Publisher address.</param>
    public void SetPublisherInfo(string publisherId, string publisherAddress)
    {
        HasInputPrincipalIdentitity = true;
        m_PublisherId = publisherId;
        m_PublisherAddress = publisherAddress;
    }

    /// <summary>
    /// Increases the value of the data being posted.
    /// </summary>
    /// <returns></returns>
    private double NextItemData()
    {
        m_ItemData += 0.01;
        return m_ItemData;
    }

    /// <summary>
    /// Initializes timer for Post messages to be sent.
    /// This method simply gets the current time and sets up the first time that a post message should be sent based off of that and the post interval.
    /// </summary>
    public void InitPostHandler()
    {
        m_NextPostTime = System.DateTime.Now + m_PostMessageFrequency;
    }

    /// <summary>
    /// Uses the current time and the nextPostTime to determine whether a postMsg
    /// should be sent.If a post message should be sent, the time is calculated
    /// for the next post after this one.Additionally, the postWithMsg flag is
    /// toggled so that posting alternates between posting with a full message as
    /// payload or data as payload.
    /// </summary>
    internal CodecReturnCode HandlePosts(ReactorChannel chnl, out ReactorErrorInfo? errorInfo)
    {
        var currentTime = System.DateTime.Now;

        if (currentTime >= m_NextPostTime)
        {
            if (EnableOnStreamPost)
            {
                CodecReturnCode ret = SendOnstreamPostMsg(chnl, m_PostWithMsg, out errorInfo);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }else
            {
                errorInfo = null;
            }
            if (EnableOffStreamPost)
            {
                CodecReturnCode ret = SendOffstreamPostMsg(chnl, m_PostWithMsg, out errorInfo);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }
            else if(EnableOnStreamPost)
            {
                errorInfo = null;
            }

            m_NextPostTime = currentTime + m_PostMessageFrequency;

            /* iterate between post with msg and post with data */
            if (m_PostWithMsg == true)
            {
                m_PostWithMsg = false;
            }
            else
            {
                m_PostWithMsg = true;
            }
        }else
        {
            errorInfo = null;
        }
        return CodecReturnCode.SUCCESS;
    }

    /// <summary>
    /// Encodes and sends an on-stream post message.
    /// </summary>
    /// <remarks>
    /// It will either send a post that contains a full message or a post that
    /// contains only data payload, based on the postWithMsg parameter. If true,
    /// post will contain a message.
    ///
    /// This method only sends one post message, however it is called
    /// periodically over a time increment by the handlePosts method
    /// </remarks>
    private CodecReturnCode SendOnstreamPostMsg(ReactorChannel chnl, bool postWithMsg, out ReactorErrorInfo? errorInfo)
    {
        if (StreamId == 0)
        {
            // no items available to post on
            Console.WriteLine("Currently no available market price streams to on-stream post to.  Will retry shortly.");
            errorInfo = null;
            return CodecReturnCode.SUCCESS;
        }

        CodecReturnCode ret = 0;
        if (postWithMsg == true)
        {
            if ((ret = EncodePostWithMsg(chnl, out errorInfo)) != CodecReturnCode.SUCCESS)
            {
                return ret;
            }
        }
        else
        {
            if ((ret = EncodePostWithData(chnl, out errorInfo)) != CodecReturnCode.SUCCESS)
            {
                return ret;
            }
        }
        return (CodecReturnCode)chnl.Submit(m_PostMsg, m_SubmitOptions, out _);
    }

    /// <summary>
    /// Encodes and sends an off-stream post message.
    /// </summary>
    /// <remarks>This method only sends one post message, however it is called periodically over a time increment by the handlePosts method.</remarks>
    private CodecReturnCode SendOffstreamPostMsg(ReactorChannel chnl, bool postWithMsg, out ReactorErrorInfo? errorInfo)
    {
        CodecReturnCode ret;
        if (postWithMsg == true)
        {
            if ((ret = EncodePostWithMsg(chnl, out errorInfo)) != CodecReturnCode.SUCCESS)
            {
                return ret;
            }
        }
        else
        {
            if ((ret = EncodePostWithData(chnl, out errorInfo)) != CodecReturnCode.SUCCESS)
            {
                return ret;
            }
        }

        // send post message
        ret = (CodecReturnCode)chnl.Submit(m_PostMsg, m_SubmitOptions, out _);
        if (ret != (int)TransportReturnCode.SUCCESS)
            return CodecReturnCode.FAILURE;

        m_OffstreamPostSent = true;
        return CodecReturnCode.SUCCESS;
    }

    /// <summary>
    /// This method is internally used by sendPostMsg.
    /// </summary>
    /// <remarks>
    /// It encodes a PostMsg, populating the postUserInfo with the IPAddress and
    /// process ID of the machine running the application. The payload of the
    /// PostMsg is an UpdateMsg. The UpdateMsg contains a FieldList containing
    /// several fields. The same message encoding functionality used by the
    /// provider application is leveraged here to encode the contents.
    /// </remarks>
    private CodecReturnCode EncodePostWithMsg(ReactorChannel chnl, out ReactorErrorInfo? errorInfo)
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
        m_PostMsg.PartNum = m_NextSeqNum;
        m_PostMsg.SeqNum = m_NextSeqNum++;

        // populate post user info
        if (HasInputPrincipalIdentitity && m_PublisherAddress is not null && m_PublisherId is not null)
        {
            m_PostMsg.PostUserInfo.UserAddrFromString(m_PublisherAddress);
            m_PostMsg.PostUserInfo.UserId = int.Parse(m_PublisherId);
        }
        else
        {
            try
            {
                m_PostMsg.PostUserInfo.UserAddrFromString(GetHostAddress());
            }
            catch (Exception e)
            {
                errorInfo = new();
                errorInfo.Error.Text = "Populating PostUserInfo failed. Dns.GetHostAddresses(Dns.GetHostName()) exception: " + e.Message;
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

        // encode market price response into a buffer

        MarketPriceItem mpItemInfo = new();
        mpItemInfo.InitFields();
        mpItemInfo.TRDPRC_1 = NextItemData();
        mpItemInfo.BID = NextItemData();
        mpItemInfo.ASK = NextItemData();

        // get a buffer for nested market price refresh
        m_PostNestedMsgPayLoad = new();
        m_PostNestedMsgPayLoad.Data(new ByteBuffer(1024));

        m_PostMsgEncIter.Clear();
        CodecReturnCode ret = m_PostMsgEncIter.SetBufferAndRWFVersion(m_PostNestedMsgPayLoad, chnl.MajorVersion, chnl.MinorVersion);
        if (ret != CodecReturnCode.SUCCESS)
        {
            errorInfo = new();
            errorInfo.Error.Text = "EncodeIter.SetBufferAndRWFVersion() failed:  <" + ret.GetAsString();
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
            Eta.Codec.Buffer text = new();
            text.Data("Item Refresh Completed");
            m_MarketPriceRefresh.State.Text(text);
            m_MarketPriceRefresh.ServiceId = ServiceId;
            m_MarketPriceRefresh.HasServiceId = true;
            m_MarketPriceRefresh.MarketPriceItem = mpItemInfo;
            m_MarketPriceRefresh.HasQos = true;
            m_MarketPriceRefresh.Qos.IsDynamic = false;
            m_MarketPriceRefresh.Qos.Rate(QosRates.TICK_BY_TICK);
            m_MarketPriceRefresh.Qos.Timeliness(QosTimeliness.REALTIME);
            m_MarketPriceRefresh.DataDictionary = Dictionary;

            ret = m_MarketPriceRefresh.Encode(m_PostMsgEncIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new();
                errorInfo.Error.Text = $"MarketPriceRefresh.Encode() failed:  <{CodecReturnCodeExtensions.GetAsString(ret)}>";
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

            ret = m_MarketPriceUpdate.Encode(m_PostMsgEncIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new();
                errorInfo.Error.Text = "MarketPriceUpdate.Encode() failed:  <" + CodecReturnCodeExtensions.GetAsString(ret);
                return CodecReturnCode.FAILURE;
            }
        }

        m_PostMsg.EncodedDataBody = m_PostNestedMsgPayLoad;
        Console.WriteLine($"\n\nSENDING POST WITH MESSAGE:\n  streamId = {m_PostMsg.StreamId}\n  postId   = {m_PostMsg.PostId}\n  seqNum   = {m_PostMsg.SeqNum}\n");
        errorInfo = null;
        return CodecReturnCode.SUCCESS;
    }

    private static string? GetHostAddress() => Dns.GetHostAddresses(Dns.GetHostName())
                        .Where(ip => ip.AddressFamily == AddressFamily.InterNetwork)
                        .Select(a => a.ToString())
                        .FirstOrDefault();

    /// <summary>
    /// This method is internally used by sendPostMsg.
    /// </summary>
    /// <remarks>
    /// It encodes a PostMsg, populating the postUserInfo with the IPAddress and
    /// process ID of the machine running the application. The payload of the
    /// PostMsg is a FieldList containing several fields.
    /// </remarks>
    private CodecReturnCode EncodePostWithData(ReactorChannel chnl, out ReactorErrorInfo? errorInfo)
    {
        CodecReturnCode ret;
        IDictionaryEntry? dictionaryEntry;
        MarketPriceItem itemInfo = new();

        // clear encode iterator
        m_EncIter.Clear();
        m_FList.Clear();
        m_FEntry.Clear();

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
        m_PostMsg.PartNum = m_NextSeqNum;
        m_PostMsg.SeqNum = m_NextSeqNum++;

        // populate post user info
        if (HasInputPrincipalIdentitity && m_PublisherAddress is not null && m_PublisherId is not null)
        {
            m_PostMsg.PostUserInfo.UserAddrFromString(m_PublisherAddress);
            m_PostMsg.PostUserInfo.UserId = int.Parse(m_PublisherId);
        }
        else
        {
            try
            {
                m_PostMsg.PostUserInfo.UserAddrFromString(GetHostAddress());
            }
            catch (Exception e)
            {
                errorInfo = new();
                errorInfo.Error.Text = "Populating postUserInfo failed. Dns.GetHostAddresses(Dns.GetHostName()) exception: " + e.Message;
                return CodecReturnCode.FAILURE;
            }
            m_PostMsg.PostUserInfo.UserId = Environment.ProcessId;
        }

        m_PostNestedMsgPayLoad = new Eta.Codec.Buffer();
        m_PostNestedMsgPayLoad.Data(new ByteBuffer(1024));
        // encode message
        m_EncIter.SetBufferAndRWFVersion(m_PostNestedMsgPayLoad, chnl.MajorVersion, Codec.MinorVersion());

        itemInfo.InitFields();
        itemInfo.TRDPRC_1 = NextItemData();
        itemInfo.BID = NextItemData();
        itemInfo.ASK = NextItemData();

        // encode field list
        m_FList.ApplyHasStandardData();
        if ((ret = m_FList.EncodeInit(m_EncIter, null, 0)) < CodecReturnCode.SUCCESS)
        {
            errorInfo = new();
            errorInfo.Error.Text = "FieldList.EncodeInit() failed: <" + CodecReturnCodeExtensions.GetAsString(ret) + ">";
            return ret;
        }

        // TRDPRC_1
        m_FEntry.Clear();
        if (Dictionary is not null)
        {
            dictionaryEntry = Dictionary.Entry((int)MarketPriceItem.TRDPRC_1_FID);
            if (dictionaryEntry != null)
            {
                m_FEntry.FieldId = MarketPriceItem.TRDPRC_1_FID;
                m_FEntry.DataType = dictionaryEntry.GetRwfType();
                m_TempReal.Clear();
                m_TempReal.Value(itemInfo.TRDPRC_1, RealHints.EXPONENT_2);
                if ((ret = m_FEntry.Encode(m_EncIter, m_TempReal)) < CodecReturnCode.SUCCESS)
                {
                    errorInfo = new();
                    errorInfo.Error.Text = "FieldEntry.Encode() failed: <" + CodecReturnCodeExtensions.GetAsString(ret) + ">";
                    return ret;
                }
            }
        }
        // BID
        m_FEntry.Clear();
        if (Dictionary is not null)
        {
            dictionaryEntry = Dictionary.Entry(MarketPriceItem.BID_FID);
            if (dictionaryEntry != null)
            {
                m_FEntry.FieldId = MarketPriceItem.BID_FID;
                m_FEntry.DataType = dictionaryEntry.GetRwfType();
                m_TempReal.Clear();
                m_TempReal.Value(itemInfo.BID, RealHints.EXPONENT_2);
                if ((ret = m_FEntry.Encode(m_EncIter, m_TempReal)) < CodecReturnCode.SUCCESS)
                {
                    errorInfo = new();
                    errorInfo.Error.Text = $"FieldEntry.Encode() failed: <{CodecReturnCodeExtensions.GetAsString(ret)}>";
                    return ret;
                }
            }
        }
        // ASK
        m_FEntry.Clear();
        dictionaryEntry = Dictionary?.Entry(MarketPriceItem.ASK_FID);
        if (dictionaryEntry != null)
        {
            m_FEntry.FieldId = MarketPriceItem.ASK_FID;
            m_FEntry.DataType = dictionaryEntry.GetRwfType();
            m_TempReal.Clear();
            m_TempReal.Value(itemInfo.ASK, RealHints.EXPONENT_2);
            if ((ret = m_FEntry.Encode(m_EncIter, m_TempReal)) < CodecReturnCode.SUCCESS)
            {
                errorInfo = new();
                errorInfo.Error.Text = $"FieldEntry.Encode() failed: <{CodecReturnCodeExtensions.GetAsString(ret)}>";
                return ret;
            }
        }

        // complete encode field list
        if ((ret = m_FList.EncodeComplete(m_EncIter, true)) < CodecReturnCode.SUCCESS)
        {
            errorInfo = new();
            errorInfo.Error.Text = $"FieldList.EncodeComplete() failed: <{CodecReturnCodeExtensions.GetAsString(ret)}>";
            return ret;
        }

        Console.WriteLine($"\n\nSENDING POST WITH DATA:\n  streamId = {m_PostMsg.StreamId}\n  postId   = {m_PostMsg.PostId}\n  seqNum   = {m_PostMsg.SeqNum}\n");
        errorInfo = null;
        m_PostMsg.EncodedDataBody = m_PostNestedMsgPayLoad;

        return CodecReturnCode.SUCCESS;
    }

    /// <summary>
    /// This function encodes and sends an off-stream post close status message.
    /// </summary>
    public CodecReturnCode CloseOffStreamPost(ReactorChannel chnl, out ReactorErrorInfo? errorInfo)
    {
        // first check if we have sent offstream posts
        if (!m_OffstreamPostSent)
        {
            errorInfo = null;
            return CodecReturnCode.SUCCESS;
        }

        m_PostMsg.Clear();

        // set-up post message
        m_PostMsg.MsgClass = MsgClasses.POST;
        m_PostMsg.StreamId = StreamId;
        m_PostMsg.DomainType = (int)DomainType.MARKET_PRICE;
        m_PostMsg.ContainerType = DataTypes.MSG;

        // Note: This example sends a status close when it shuts down.
        // So don't ask for an ACK (that we will never get)
        m_PostMsg.Flags = PostMsgFlags.POST_COMPLETE | PostMsgFlags.HAS_POST_ID | PostMsgFlags.HAS_SEQ_NUM | PostMsgFlags.HAS_POST_USER_RIGHTS | PostMsgFlags.HAS_MSG_KEY;

        m_PostMsg.PostId = m_NextPostId++;
        m_PostMsg.PartNum = m_NextSeqNum;
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
        if (HasInputPrincipalIdentitity && m_PublisherAddress is not null && m_PublisherId is not null)
        {
            m_PostMsg.PostUserInfo.UserAddrFromString(m_PublisherAddress);
            m_PostMsg.PostUserInfo.UserId = int.Parse(m_PublisherId);
        }
        else
        {
            try
            {
                m_PostMsg.PostUserInfo.UserAddrFromString(GetHostAddress());
            }
            catch (Exception e)
            {
                errorInfo = new();
                errorInfo.Error.Text = "Populating postUserInfo failed. Dns.GetHostAddresses(Dns.GetHostName()) exception: " + e.StackTrace;
                return CodecReturnCode.FAILURE;
            }
            m_PostMsg.PostUserInfo.UserId = Environment.ProcessId;
        }

        // set-up status message that will be nested in the post message
        m_StatusMsg.Flags = StatusMsgFlags.HAS_STATE;
        m_StatusMsg.MsgClass = MsgClasses.STATUS;
        m_StatusMsg.StreamId = StreamId;
        m_StatusMsg.DomainType = (int)DomainType.MARKET_PRICE;
        m_StatusMsg.ContainerType = DataTypes.NO_DATA;
        m_StatusMsg.State.StreamState(StreamStates.CLOSED);
        m_StatusMsg.State.DataState(DataStates.SUSPECT);

        m_PostNestedMsgPayLoad = new Eta.Codec.Buffer();
        m_PostNestedMsgPayLoad.Data(new ByteBuffer(1024));

        // encode post message
        m_EncIter.Clear();
        CodecReturnCode ret = m_EncIter.SetBufferAndRWFVersion(m_PostNestedMsgPayLoad, chnl.MajorVersion, chnl.MinorVersion);
        if (ret != CodecReturnCode.SUCCESS)
        {
            errorInfo = new();
            errorInfo.Error.Text = "EncoderIterator.SetBufferAndRWFVersion() failed:  <" + CodecReturnCodeExtensions.GetAsString(ret) + ">";
            return ret;
        }

        // encode nested status message
        ret = m_StatusMsg.EncodeInit(m_EncIter, 0);
        if (ret < CodecReturnCode.SUCCESS)
        {
            errorInfo = new();
            errorInfo.Error.Text = "StatusMsg.EncodeInit() failed:  <" + CodecReturnCodeExtensions.GetAsString(ret) + ">";
            return ret;
        }

        // complete encode status message
        ret = m_StatusMsg.EncodeComplete(m_EncIter, true);
        if (ret < CodecReturnCode.SUCCESS)
        {
            errorInfo = new();
            errorInfo.Error.Text = "StatusMsg.EncodeComplete() failed:  <" + CodecReturnCodeExtensions.GetAsString(ret) + ">";
            return ret;
        }

        m_PostMsg.EncodedDataBody = m_PostNestedMsgPayLoad;
        // send post message
        return (CodecReturnCode)chnl.Submit(m_PostMsg, m_SubmitOptions, out errorInfo);
    }
}