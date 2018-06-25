package com.thomsonreuters.upa.examples.consumer;

import java.net.InetAddress;
import java.nio.ByteBuffer;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DictionaryEntry;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.FieldEntry;
import com.thomsonreuters.upa.codec.FieldList;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.PostMsg;
import com.thomsonreuters.upa.codec.PostMsgFlags;
import com.thomsonreuters.upa.codec.PostUserRights;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.RealHints;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StatusMsgFlags;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.examples.common.ChannelSession;
import com.thomsonreuters.upa.shared.rdm.marketprice.MarketPriceItem;
import com.thomsonreuters.upa.shared.rdm.marketprice.MarketPriceRefresh;
import com.thomsonreuters.upa.shared.rdm.marketprice.MarketPriceUpdate;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.InstrumentNameTypes;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportReturnCodes;

/**
 * This is the post handler for the UPA consumer application. It provides
 * methods for sending on stream and off stream post messages.
 */
public class PostHandler
{
    public static final int TRANSPORT_BUFFER_SIZE_MSG_POST = ChannelSession.MAX_MSG_SIZE;
    public static final int TRANSPORT_BUFFER_SIZE_DATA_POST = ChannelSession.MAX_MSG_SIZE;

    private int nextPostId;
    private int nextSeqNum;
    private double itemData;

    private final int POST_MESSAGE_FREQUENCY = 5; // in seconds
    private long nextPostTime;
    private boolean postWithMsg;
    private boolean shouldOffstreamPost, shouldOnstreamPost, hasInputPrincipalIdentitity;
    private String publisherId;
    private String publisherAddress;
    private boolean postRefresh;
    private Buffer postNestedMsgPayLoad;
    private MarketPriceRefresh marketPriceRefresh = new MarketPriceRefresh();
    private MarketPriceUpdate marketPriceUpdate = new MarketPriceUpdate();
    private int streamId;
    private Buffer postItemName;
    private int serviceId;
    private DataDictionary dictionary;

    private PostMsg postMsg = (PostMsg)CodecFactory.createMsg();
    private FieldList fList = CodecFactory.createFieldList();
    private FieldEntry fEntry = CodecFactory.createFieldEntry();
    private Real tempReal = CodecFactory.createReal();
    private EncodeIterator encIter = CodecFactory.createEncodeIterator();
    private EncodeIterator postMsgEncIter = CodecFactory.createEncodeIterator();
    private StatusMsg statusMsg = (StatusMsg)CodecFactory.createMsg();
    private boolean offstreamPostSent = false;

    /**
     * Instantiates a new post handler.
     */
    public PostHandler()
    {
        postWithMsg = true;
        shouldOffstreamPost = false;
        shouldOnstreamPost = false;
        hasInputPrincipalIdentitity = false;
        nextPostId = 1;
        nextSeqNum = 1;
        itemData = 12.00;
        postItemName = CodecFactory.createBuffer();
        postRefresh = true;
    }

    /** enables offstream posting mode */
    void enableOffstreamPost(boolean shouldOffstreamPost)
    {
        this.shouldOffstreamPost = shouldOffstreamPost;
    }

    /** enables onstream posting mode */
    void enableOnstreamPost(boolean shouldOnstreamPost)
    {
        this.shouldOnstreamPost = shouldOnstreamPost;
    }

    /**
     * Enables offstream posting mode.
     *
     * @return true, if successful
     */
    public boolean enableOffstreamPost()
    {
        return shouldOffstreamPost;
    }

    /**
     * Enables onstream posting mode.
     *
     * @return true, if successful
     */
    public boolean enableOnstreamPost()
    {
        return this.shouldOnstreamPost;
    }

    
    void setPublisherInfo(String publisherId, String publisherAddress)
    {
    	this.hasInputPrincipalIdentitity = true;
    	this.publisherId = publisherId;
    	this.publisherAddress = publisherAddress;
    }

        
    /* increases the value of the data being posted */
    private double nextItemData()
    {
        itemData += 0.01;
        return itemData;
    }

    /**
     * Initializes timer for Post messages to be sent This method simply gets
     * the current time and sets up the first time that a post message should be
     * sent based off of that and the post interval.
     */
    public void initPostHandler()
    {
        nextPostTime = System.currentTimeMillis() + POST_MESSAGE_FREQUENCY * 1000;
    }

    /**
     * Uses the current time and the nextPostTime to determine whether a postMsg
     * should be sent. If a post message should be sent, the time is calculated
     * for the next post after this one. Additionally, the postWithMsg flag is
     * toggled so that posting alternates between posting with a full message as
     * payload or data as payload.
     *
     * @param chnl the chnl
     * @param error the error
     * @return the int
     */
    public int handlePosts(ChannelSession chnl, com.thomsonreuters.upa.transport.Error error)
    {
        long currentTime = System.currentTimeMillis();

        if (currentTime >= nextPostTime)
        {
            if (shouldOnstreamPost)
            {
                int ret = sendOnstreamPostMsg(chnl, postWithMsg, error);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }
            if (shouldOffstreamPost)
            {
                int ret = sendOffstreamPostMsg(chnl, postWithMsg, error);
                if (ret != CodecReturnCodes.SUCCESS)
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

        return CodecReturnCodes.SUCCESS;
    }

    /**
     * Encodes and sends an on-stream post message.
     * 
     * It will either send a post that contains a full message or a post that
     * contains only data payload, based on the postWithMsg parameter. If true,
     * post will contain a message.
     * 
     * This method only sends one post message, however it is called
     * periodically over a time increment by the handlePosts method
     */
    private int sendOnstreamPostMsg(ChannelSession chnlSession, boolean postWithMsg, Error error)
    {
        if (streamId == 0)
        {
            // no items available to post on
            System.out.println("Currently no available market price streams to on-stream post to.  Will retry shortly.");
            return CodecReturnCodes.SUCCESS;
        }

        // get a buffer for the item request
        TransportBuffer msgBuf = chnlSession.getTransportBuffer(TRANSPORT_BUFFER_SIZE_MSG_POST, false, error);
        if (msgBuf == null)
            return CodecReturnCodes.FAILURE;

        int ret = 0;
        if (postWithMsg == true)
        {

            if ((ret = encodePostWithMsg(chnlSession, msgBuf)) != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }
        else
        {
            if ((ret = encodePostWithData(chnlSession, msgBuf)) != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }

        // send post message
        return chnlSession.write(msgBuf, error);
    }

    /**
     * Encodes and sends an off-stream post message.
     * 
     * This method only sends one post message, however it is called
     * periodically over a time increment by the handlePosts method
     */
    private int sendOffstreamPostMsg(ChannelSession chnlSession, boolean postWithMsg, Error error)
    {
        // get a buffer for the item request
        TransportBuffer msgBuf = chnlSession.getTransportBuffer(TRANSPORT_BUFFER_SIZE_DATA_POST, false, error);
        if (msgBuf == null)
            return CodecReturnCodes.FAILURE;

        int ret = encodePostWithMsg(chnlSession, msgBuf);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        // send post message
        ret = chnlSession.write(msgBuf, error);
        if (ret != TransportReturnCodes.SUCCESS)
            return CodecReturnCodes.FAILURE;

        offstreamPostSent = true;
        return CodecReturnCodes.SUCCESS;
    }

    /* This method is internally used by sendPostMsg */
    /*
     * It encodes a PostMsg, populating the postUserInfo with the IPAddress and
     * process ID of the machine running the application. The payload of the
     * PostMsg is an UpdateMsg. The UpdateMsg contains a FieldList containing
     * several fields. The same message encoding functionality used by the
     * provider application is leveraged here to encode the contents.
     */
    private int encodePostWithMsg(ChannelSession chnlSession, TransportBuffer msgBuf)
    {
        // First encode message for payload
        postMsg.clear();

        // set-up message
        postMsg.msgClass(MsgClasses.POST);
        postMsg.streamId(streamId);
        postMsg.domainType(DomainTypes.MARKET_PRICE);
        postMsg.containerType(DataTypes.MSG);

        // Note: post message key not required for on-stream post
        postMsg.applyPostComplete();
        postMsg.applyAck();
        postMsg.applyHasPostId();
        postMsg.applyHasSeqNum();
        postMsg.applyHasMsgKey();
        postMsg.applyHasPostUserRights();
        postMsg.postId(nextPostId++);
        postMsg.seqNum(nextSeqNum++);

        // populate post user info        
        if (hasInputPrincipalIdentitity)
        {
        	postMsg.postUserInfo().userAddr(publisherAddress);	
        	postMsg.postUserInfo().userId(Integer.parseInt(publisherId));        	
        }
        else
        {
        	try
        	{
        		postMsg.postUserInfo().userAddr(InetAddress.getLocalHost().getHostAddress());
        	}
        	catch (Exception e)
        	{
        		System.out.println("Populating postUserInfo failed. InetAddress.getLocalHost().getHostAddress exception: " + e.getLocalizedMessage());
        		return CodecReturnCodes.FAILURE;
        	}
        	postMsg.postUserInfo().userId(Integer.parseInt(System.getProperty("pid", "1")));
        }
        postMsg.postUserRights(PostUserRights.CREATE | PostUserRights.DELETE);
        postMsg.msgKey().applyHasNameType();
        postMsg.msgKey().applyHasName();
        postMsg.msgKey().applyHasServiceId();
        postMsg.msgKey().name().data(postItemName.data(), postItemName.position(), postItemName.length());
        postMsg.msgKey().nameType(InstrumentNameTypes.RIC);
        postMsg.msgKey().serviceId(serviceId);

        // encode post message
        encIter.clear();
        int ret = encIter.setBufferAndRWFVersion(msgBuf, chnlSession.channel().majorVersion(), chnlSession.channel().minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("Encoder.setBufferAndRWFVersion() failed:  <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        ret = postMsg.encodeInit(encIter, 0);
        if (ret != CodecReturnCodes.ENCODE_CONTAINER)
        {
            System.out.println("EncodeMsgInit() failed:  <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        // encode market price response into a buffer

        MarketPriceItem mpItemInfo = new MarketPriceItem();
        mpItemInfo.initFields();
        mpItemInfo.TRDPRC_1 = nextItemData();
        mpItemInfo.BID = nextItemData();
        mpItemInfo.ASK = nextItemData();

        // get a buffer for nested market price refresh
        postNestedMsgPayLoad = CodecFactory.createBuffer();
        postNestedMsgPayLoad.data(ByteBuffer.allocate(1024));

        // Although we are encoding RWF message, this code
        // encodes nested message into a separate buffer.
        // this is because MarketPrice.encode message is shared by all
        // applications, and it expects to encode the message into a stand alone
        // buffer.
        ret = encIter.encodeNonRWFInit(postNestedMsgPayLoad);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("EncodeNonRWFDataTypeInit() failed:  <" + CodecReturnCodes.toString(ret));
            return CodecReturnCodes.FAILURE;
        }

        postMsgEncIter.clear();
        ret = postMsgEncIter.setBufferAndRWFVersion(postNestedMsgPayLoad, chnlSession.channel().majorVersion(), chnlSession.channel().minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("EncodeIter.setBufferAndRWFVersion() failed:  <" + CodecReturnCodes.toString(ret));
            return CodecReturnCodes.FAILURE;
        }

        if (postRefresh)
        {
            marketPriceRefresh.clear();
            marketPriceRefresh.applyRefreshComplete();
            marketPriceRefresh.applyClearCache();
            marketPriceRefresh.streamId(streamId);
            marketPriceRefresh.itemName().data(postItemName.data(), postItemName.position(), postItemName.length());
            marketPriceRefresh.state().streamState(StreamStates.OPEN);
            marketPriceRefresh.state().dataState(DataStates.OK);
            marketPriceRefresh.state().code(StateCodes.NONE);
            marketPriceRefresh.state().text().data("Item Refresh Completed");
            marketPriceRefresh.serviceId(serviceId);
            marketPriceRefresh.applyHasServiceId();
            marketPriceRefresh.marketPriceItem(mpItemInfo);
            marketPriceRefresh.applyHasQos();
            marketPriceRefresh.qos().dynamic(false);
            marketPriceRefresh.qos().rate(QosRates.TICK_BY_TICK);
            marketPriceRefresh.qos().timeliness(QosTimeliness.REALTIME);
            marketPriceRefresh.dictionary(dictionary);

            ret = marketPriceRefresh.encode(postMsgEncIter);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("encodeMarketPriceRefresh() failed:  <" + CodecReturnCodes.toString(ret));
                return CodecReturnCodes.FAILURE;
            }
            postRefresh = false;
        }
        else
        {
            marketPriceUpdate.clear();
            marketPriceUpdate.streamId(streamId);
            marketPriceUpdate.itemName().data(postItemName.data(), postItemName.position(), postItemName.length());
            marketPriceUpdate.marketPriceItem(mpItemInfo);
            marketPriceUpdate.dictionary(dictionary);

            ret = marketPriceUpdate.encode(postMsgEncIter);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("encodeMarketPriceUpdate() failed:  <" + CodecReturnCodes.toString(ret));
                return CodecReturnCodes.FAILURE;
            }
        }

        ret = encIter.encodeNonRWFComplete(postNestedMsgPayLoad, true);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("EncodeNonRWFDataTypeComplete() failed:  <" + CodecReturnCodes.toString(ret));
            return CodecReturnCodes.FAILURE;
        }

        // complete encode message
        if ((ret = postMsg.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("EncodeMsgComplete() failed with return code: " + ret);
            return ret;
        }

        System.out.println("\n\nSENDING POST WITH MESSAGE:\n" + "  streamId = " + postMsg.streamId() + "\n  postId   = " + postMsg.postId() + "\n  seqNum   = " + postMsg.seqNum());

        return CodecReturnCodes.SUCCESS;
    }

    /* This method is internally used by sendPostMsg */
    /*
     * It encodes a PostMsg, populating the postUserInfo with the IPAddress and
     * process ID of the machine running the application. The payload of the
     * PostMsg is a FieldList containing several fields.
     */
    private int encodePostWithData(ChannelSession chnlSession, TransportBuffer msgBuf)
    {
        int ret = 0;
        DictionaryEntry dictionaryEntry = null;
        MarketPriceItem itemInfo = new MarketPriceItem();

        // clear encode iterator
        encIter.clear();
        fList.clear();
        fEntry.clear();

        // set-up message
        postMsg.msgClass(MsgClasses.POST);
        postMsg.streamId(streamId);
        postMsg.domainType(DomainTypes.MARKET_PRICE);
        postMsg.containerType(DataTypes.FIELD_LIST);

        // Note: post message key not required for on-stream post
        postMsg.applyPostComplete();
        postMsg.applyAck(); // request ACK
        postMsg.applyHasPostId();
        postMsg.applyHasSeqNum();
        postMsg.postId(nextPostId++);
        postMsg.seqNum(nextSeqNum++);

        // populate post user info        
        if (hasInputPrincipalIdentitity)
        {
        	postMsg.postUserInfo().userAddr(publisherAddress);	
        	postMsg.postUserInfo().userId(Integer.parseInt(publisherId));        	
        }        
        else
        {        
        	try
        	{
        		postMsg.postUserInfo().userAddr(InetAddress.getLocalHost().getHostAddress());
        	}
        	catch (Exception e)
        	{
        		System.out.println("Populating postUserInfo failed. InetAddress.getLocalHost().getHostAddress exception: " + e.getLocalizedMessage());
        		return CodecReturnCodes.FAILURE;
        	}        
        	postMsg.postUserInfo().userId(Integer.parseInt(System.getProperty("pid", "1")));
        }
        // encode message
        encIter.setBufferAndRWFVersion(msgBuf, chnlSession.channel().majorVersion(), Codec.minorVersion());

        if ((ret = postMsg.encodeInit(encIter, 0)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("EncodeMsgInit() failed: <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        itemInfo.initFields();
        itemInfo.TRDPRC_1 = nextItemData();
        itemInfo.BID = nextItemData();
        itemInfo.ASK = nextItemData();

        // encode field list
        fList.applyHasStandardData();
        if ((ret = fList.encodeInit(encIter, null, 0)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("EncodeFieldListInit() failed: <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        // TRDPRC_1
        fEntry.clear();
        dictionaryEntry = dictionary.entry(MarketPriceItem.TRDPRC_1_FID);
        if (dictionaryEntry != null)
        {
            fEntry.fieldId(MarketPriceItem.TRDPRC_1_FID);
            fEntry.dataType(dictionaryEntry.rwfType());
            tempReal.clear();
            tempReal.value(itemInfo.TRDPRC_1, RealHints.EXPONENT_2);
            if ((ret = fEntry.encode(encIter, tempReal)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("EncodeFieldEntry() failed: <" + CodecReturnCodes.toString(ret) + ">");
                return ret;
            }
        }
        // BID
        fEntry.clear();
        dictionaryEntry = dictionary.entry(MarketPriceItem.BID_FID);
        if (dictionaryEntry != null)
        {
            fEntry.fieldId(MarketPriceItem.BID_FID);
            fEntry.dataType(dictionaryEntry.rwfType());
            tempReal.clear();
            tempReal.value(itemInfo.BID, RealHints.EXPONENT_2);
            if ((ret = fEntry.encode(encIter, tempReal)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("EncodeFieldEntry() failed: <" + CodecReturnCodes.toString(ret) + ">");

                return ret;
            }
        }
        // ASK
        fEntry.clear();
        dictionaryEntry = dictionary.entry(MarketPriceItem.ASK_FID);
        if (dictionaryEntry != null)
        {
            fEntry.fieldId(MarketPriceItem.ASK_FID);
            fEntry.dataType(dictionaryEntry.rwfType());
            tempReal.clear();
            tempReal.value(itemInfo.ASK, RealHints.EXPONENT_2);
            if ((ret = fEntry.encode(encIter, tempReal)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("EncodeFieldEntry() failed: <" + CodecReturnCodes.toString(ret) + ">");
                return ret;
            }
        }

        // complete encode field list
        if ((ret = fList.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("EncodeFieldListComplete() failed: <" + CodecReturnCodes.toString(ret) + ">");

            return ret;
        }

        // complete encode post message
        if ((ret = postMsg.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("EncodeMsgComplete() failed: <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        System.out.println("\n\nSENDING POST WITH DATA:\n" + "  streamId = " + postMsg.streamId() + "\n  postId   = " + postMsg.postId() + "\n  seqNum   = " + postMsg.seqNum());

        return CodecReturnCodes.SUCCESS;
    }

    /**
     * This function encodes and sends an off-stream post close status message.
     *
     * @param channelSession the channel session
     * @param error the error
     * @return the int
     */
    public int closeOffStreamPost(ChannelSession channelSession, Error error)
    {
        // first check if we have sent offstream posts
        if (!offstreamPostSent)
            return CodecReturnCodes.SUCCESS;

        // get a buffer for the item request
        TransportBuffer msgBuf = channelSession.getTransportBuffer(TRANSPORT_BUFFER_SIZE_MSG_POST, false, error);
        if (msgBuf == null)
            return CodecReturnCodes.FAILURE;

        postMsg.clear();

        // set-up post message
        postMsg.msgClass(MsgClasses.POST);
        postMsg.streamId(streamId);
        postMsg.domainType(DomainTypes.MARKET_PRICE);
        postMsg.containerType(DataTypes.MSG);

        // Note: This example sends a status close when it shuts down.
        // So don't ask for an ACK (that we will never get)
        postMsg.flags(PostMsgFlags.POST_COMPLETE | PostMsgFlags.HAS_POST_ID | PostMsgFlags.HAS_SEQ_NUM | PostMsgFlags.HAS_POST_USER_RIGHTS | PostMsgFlags.HAS_MSG_KEY);

        postMsg.postId(nextPostId++);
        postMsg.seqNum(nextSeqNum++);
        postMsg.postUserRights(PostUserRights.CREATE | PostUserRights.DELETE);

        // set post item name
        postMsg.msgKey().applyHasNameType();
        postMsg.msgKey().applyHasName();
        postMsg.msgKey().applyHasServiceId();
        postMsg.msgKey().name().data(postItemName.data(), postItemName.position(), postItemName.length());
        postMsg.msgKey().nameType(InstrumentNameTypes.RIC);
        postMsg.msgKey().serviceId(serviceId);

       	// populate default post user info        
        if (hasInputPrincipalIdentitity)
        {
        	postMsg.postUserInfo().userAddr(publisherAddress);	
        	postMsg.postUserInfo().userId(Integer.parseInt(publisherId));        	
        }        
        else
        {
        	try
        	{
        		postMsg.postUserInfo().userAddr(InetAddress.getLocalHost().getHostAddress());
        	}
        	catch (Exception e)
        	{
        		System.out.println("Populating postUserInfo failed. InetAddress.getLocalHost().getHostAddress exception: " + e.getLocalizedMessage());
        		return CodecReturnCodes.FAILURE;
        	}
        	postMsg.postUserInfo().userId(Integer.parseInt(System.getProperty("pid", "1")));
        }
        
        // set-up status message that will be nested in the post message
        statusMsg.flags(StatusMsgFlags.HAS_STATE);
        statusMsg.msgClass(MsgClasses.STATUS);
        statusMsg.streamId(streamId);
        statusMsg.domainType(DomainTypes.MARKET_PRICE);
        statusMsg.containerType(DataTypes.NO_DATA);
        statusMsg.state().streamState(StreamStates.CLOSED);
        statusMsg.state().dataState(DataStates.SUSPECT);

        // encode post message
        encIter.clear();
        int ret = encIter.setBufferAndRWFVersion(msgBuf, channelSession.channel().majorVersion(), channelSession.channel().minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("Encoder.setBufferAndRWFVersion() failed:  <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        ret = postMsg.encodeInit(encIter, 0);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            System.out.println("PostMsg.encodeInit() failed:  <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        // encode nested status message
        ret = statusMsg.encodeInit(encIter, 0);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            System.out.println("StatusMsg.encodeInit() failed:  <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        // complete encode status message
        ret = statusMsg.encodeComplete(encIter, true);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            System.out.println("StatusMsg.encodeComplete() failed:  <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        // complete encode post message
        ret = postMsg.encodeComplete(encIter, true);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            System.out.println("PostMsg.encodeComplete() failed:  <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        // send post message
        return channelSession.write(msgBuf, error);
    }

    /**
     * Stream Id for posting data.
     * 
     * @return streamId
     */
    public int streamId()
    {
        return streamId;
    }

    /**
     * Stream Id for posting data.
     *
     * @param streamId the stream id
     */
    public void streamId(int streamId)
    {
        this.streamId = streamId;
    }

    /**
     * Item name for posting msg.
     * 
     * @return postItemName
     */
    public Buffer postItemName()
    {
        return postItemName;
    }

    /**
     * Service id for posting msg.
     * 
     * @return service id.
     */
    public int serviceId()
    {
        return serviceId;
    }

    /**
     * Service id for posting msg.
     *
     * @param serviceId the service id
     */
    public void serviceId(int serviceId)
    {
        this.serviceId = serviceId;
    }

    /**
     * Data dictionary used for encoding posting data.
     * 
     * @return data dictionary
     */
    public DataDictionary dictionary()
    {
        return dictionary;
    }

    /**
     * Data dictionary used for encoding posting data.
     *
     * @param dictionary the dictionary
     */
    public void dictionary(DataDictionary dictionary)
    {
        this.dictionary = dictionary;
    }
}