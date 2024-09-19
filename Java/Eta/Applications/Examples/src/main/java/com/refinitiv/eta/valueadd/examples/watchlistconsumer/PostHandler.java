/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.watchlistconsumer;

import java.net.InetAddress;
import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DictionaryEntry;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.FieldEntry;
import com.refinitiv.eta.codec.FieldList;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.PostMsg;
import com.refinitiv.eta.codec.PostMsgFlags;
import com.refinitiv.eta.codec.PostUserRights;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.Real;
import com.refinitiv.eta.codec.RealHints;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StatusMsgFlags;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceItem;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceRefresh;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceUpdate;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.InstrumentNameTypes;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorSubmitOptions;

/*
 * This is the post handler for the ETA Value Add consumer application.
 * It provides methods for sending on stream and off stream post messages.
 */
class PostHandler
{
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

    private ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

    PostHandler()
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

    /* Enables offstream posting mode */
    void enableOffstreamPost(boolean shouldOffstreamPost)
    {
        this.shouldOffstreamPost = shouldOffstreamPost;
    }

    /* Enables onstream posting mode */
    void enableOnstreamPost(boolean shouldOnstreamPost)
    {
        this.shouldOnstreamPost = shouldOnstreamPost;
    }

    /*
     * Returns whether or not offstream posting mode is enabled.
     */
    boolean enableOffstreamPost()
    {
        return shouldOffstreamPost;
    }

    /*
     * Returns whether or not onstream posting mode is enabled.
     */
    boolean enableOnstreamPost()
    {
        return this.shouldOnstreamPost;
    }

    /* 
     * Set user provided publisher id and publisher address
     */
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

    /*
     * Initializes timer for Post messages to be sent This method simply gets
     * the current time and sets up the first time that a post message should be
     * sent based off of that and the post interval.
     */
    void initPostHandler()
    {
        nextPostTime = System.currentTimeMillis() + POST_MESSAGE_FREQUENCY * 1000;
    }

    /*
     * Uses the current time and the nextPostTime to determine whether a postMsg
     * should be sent. If a post message should be sent, the time is calculated
     * for the next post after this one. Additionally, the postWithMsg flag is
     * toggled so that posting alternates between posting with a full message as
     * payload or data as payload.
     */
    int handlePosts(ReactorChannel chnl, ReactorErrorInfo errorInfo)
    {
        long currentTime = System.currentTimeMillis();

        if (currentTime >= nextPostTime)
        {
            if (shouldOnstreamPost)
            {
                int ret = sendOnstreamPostMsg(chnl, postWithMsg, errorInfo);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }
            if (shouldOffstreamPost)
            {
                int ret = sendOffstreamPostMsg(chnl, postWithMsg, errorInfo);
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
            if (errorInfo != null)
            	System.out.println(errorInfo.error().text());
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
    private int sendOnstreamPostMsg(ReactorChannel chnl, boolean postWithMsg, ReactorErrorInfo errorInfo)
    {
        if (streamId == 0)
        {
            // no items available to post on
            System.out.println("Currently no available market price streams to on-stream post to.  Will retry shortly.");
            return CodecReturnCodes.SUCCESS;
        }

        int ret = 0;
        if (postWithMsg == true)
        {

            if ((ret = encodePostWithMsg(chnl, errorInfo)) != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }
        else
        {
            if ((ret = encodePostWithData(chnl, errorInfo)) != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }

        return chnl.submit(postMsg, submitOptions, errorInfo);
    }

    /**
     * Encodes and sends an off-stream post message.
     * 
     * This method only sends one post message, however it is called
     * periodically over a time increment by the handlePosts method
     */
    private int sendOffstreamPostMsg(ReactorChannel chnl, boolean postWithMsg, ReactorErrorInfo errorInfo)
    {
        int ret = 0;
        if (postWithMsg == true)
        {
            if ((ret = encodePostWithMsg(chnl, errorInfo)) != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }
        else
        {
            if ((ret = encodePostWithData(chnl, errorInfo)) != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }        
                                
        // send post message
        ret = chnl.submit(postMsg, submitOptions, errorInfo);
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
    private int encodePostWithMsg(ReactorChannel chnl, ReactorErrorInfo errorInfo)
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
        postMsg.partNum(nextSeqNum);
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

        // encode market price response into a buffer

        MarketPriceItem mpItemInfo = new MarketPriceItem();
        mpItemInfo.initFields();
        mpItemInfo.TRDPRC_1 = nextItemData();
        mpItemInfo.BID = nextItemData();
        mpItemInfo.ASK = nextItemData();

        // get a buffer for nested market price refresh
        postNestedMsgPayLoad = CodecFactory.createBuffer();
        postNestedMsgPayLoad.data(ByteBuffer.allocate(1024));

        postMsgEncIter.clear();
        int ret = postMsgEncIter.setBufferAndRWFVersion(postNestedMsgPayLoad, chnl.majorVersion(), chnl.minorVersion());
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

        postMsg.encodedDataBody(postNestedMsgPayLoad);
       
        errorInfo.error().text("\n\nSENDING POST WITH MESSAGE:\n" + "  streamId = " + postMsg.streamId() + "\n  postId   = " + postMsg.postId() + "\n  seqNum   = " + postMsg.seqNum() + "\n");

        return CodecReturnCodes.SUCCESS;
    }

    /* This method is internally used by sendPostMsg */
    /*
     * It encodes a PostMsg, populating the postUserInfo with the IPAddress and
     * process ID of the machine running the application. The payload of the
     * PostMsg is a FieldList containing several fields.
     */
    private int encodePostWithData(ReactorChannel chnl, ReactorErrorInfo errorInfo)
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
        postMsg.partNum(nextSeqNum);
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
        
        postNestedMsgPayLoad = CodecFactory.createBuffer();
        postNestedMsgPayLoad.data(ByteBuffer.allocate(1024));
        // encode message
        encIter.setBufferAndRWFVersion(postNestedMsgPayLoad, chnl.majorVersion(), Codec.minorVersion());

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
        
        errorInfo.error().text("\n\nSENDING POST WITH DATA:\n" + "  streamId = " + postMsg.streamId() + "\n  postId   = " + postMsg.postId() + "\n  seqNum   = " + postMsg.seqNum() + "\n");

        postMsg.encodedDataBody(postNestedMsgPayLoad);
        
        return CodecReturnCodes.SUCCESS;
    }

    /*
     * This function encodes and sends an off-stream post close status message.
     */
    int closeOffStreamPost(ReactorChannel chnl, ReactorErrorInfo errorInfo)
    {
        // first check if we have sent offstream posts
        if (!offstreamPostSent)
            return CodecReturnCodes.SUCCESS;

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
        postMsg.partNum(nextSeqNum);
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

        postNestedMsgPayLoad = CodecFactory.createBuffer();
        postNestedMsgPayLoad.data(ByteBuffer.allocate(1024));
 
        // encode post message
        encIter.clear();
        int ret = encIter.setBufferAndRWFVersion(postNestedMsgPayLoad, chnl.majorVersion(), chnl.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("Encoder.setBufferAndRWFVersion() failed:  <" + CodecReturnCodes.toString(ret) + ">");
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

        postMsg.encodedDataBody(postNestedMsgPayLoad);
        // send post message
        return chnl.submit(postMsg, submitOptions, errorInfo);
    }

    /*
     * Stream Id for posting data.
     */
    int streamId()
    {
        return streamId;
    }

    /*
     * Stream Id for posting data.
     */
    void streamId(int streamId)
    {
        this.streamId = streamId;
    }

    /*
     * Item name for posting message.
     */
    Buffer postItemName()
    {
        return postItemName;
    }

    /*
     * Service id for posting message.
     */
    int serviceId()
    {
        return serviceId;
    }

    /*
     * Service id for posting message.
     */
    void serviceId(int serviceId)
    {
        this.serviceId = serviceId;
    }

    /*
     * Data dictionary used for encoding posting data.
     */
    DataDictionary dictionary()
    {
        return dictionary;
    }

    /*
     * Data dictionary used for encoding posting data.
     */
    void dictionary(DataDictionary dictionary)
    {
        this.dictionary = dictionary;
    }    
}