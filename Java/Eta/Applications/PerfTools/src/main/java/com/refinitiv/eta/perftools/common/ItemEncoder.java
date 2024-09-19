/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.common;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.GenericMsg;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.PostMsg;
import com.refinitiv.eta.codec.PostUserInfo;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.RefreshMsgFlags;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.codec.UpdateMsg;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;

/**
 * Provides encoding of messages containing item data, in the forms of
 * refreshes, updates, posts, and generic messages.
 */
public class ItemEncoder
{
    private EncodeIterator              _eIter;                 // encode iterator
    private PostMsg                     _postMsg;               // post message
    private GenericMsg                  _genMsg;                // generic message
    private MarketPriceEncoder          _marketPriceEncoder;    // market price encoder
    private XmlMsgData                  _xmlMsgData;            // XML file message data
    private Msg                         _tmpMsg;    

	/**
	 * Instantiates a new item encoder.
	 *
	 * @param msgData the msg data
	 */
	public ItemEncoder(XmlMsgData msgData)
	{
		_xmlMsgData = msgData;
    	_marketPriceEncoder = new MarketPriceEncoder(_xmlMsgData);
    	
    	_eIter = CodecFactory.createEncodeIterator();
        _postMsg = (PostMsg)CodecFactory.createMsg();
        _genMsg = (GenericMsg)CodecFactory.createMsg();
        _tmpMsg = CodecFactory.createMsg();
	}
	
	/**
	 * Encodes a Post message for an item.
	 *
	 * @param channel - channel to encode post message for.
	 * @param itemInfo - market data item to encode post message for.
	 * @param msgBuf  - TransportBuffer to encode post message into.
	 * @param postUserInfo the post user info
	 * @param encodeStartTime - if &gt;0, this is a latency timestamp to be included in the post message.
	 * @return &lt;0 if encoding fails, 0 otherwise.
	 */
	public int encodeItemPost(Channel channel, ItemInfo itemInfo, TransportBuffer msgBuf, PostUserInfo postUserInfo, long encodeStartTime)
	{
	    _eIter.clear();
		int ret = _eIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
		if (ret != CodecReturnCodes.SUCCESS)
		{
			System.out.printf("setBufferAndRWFVersion() failed: %d\n", ret);
			return ret;
		}

		// Prepare post message.
		_postMsg.msgClass(MsgClasses.POST);
		_postMsg.streamId(itemInfo.streamId());
		_postMsg.applyPostComplete();
		_postMsg.postUserInfo().userAddr(postUserInfo.userAddr());
		_postMsg.postUserInfo().userId(postUserInfo.userId());
		_postMsg.domainType(itemInfo.attributes().domainType());
		_postMsg.containerType(DataTypes.MSG);

		// Prepare update message. 
		UpdateMsg updateMsg = (UpdateMsg)_tmpMsg;
		updateMsg.msgClass(MsgClasses.UPDATE);
		updateMsg.applyHasPostUserInfo();
		updateMsg.postUserInfo().userAddr(postUserInfo.userAddr());
		updateMsg.postUserInfo().userId(postUserInfo.userId());
		updateMsg.domainType(itemInfo.attributes().domainType());

		// Encode post.
		ret = _postMsg.encodeInit(_eIter, 0);
		if (ret < CodecReturnCodes.SUCCESS)
			return ret;

		// Encode update.
		switch(itemInfo.attributes().domainType())
		{
			case DomainTypes.MARKET_PRICE:
			    updateMsg.containerType(DataTypes.FIELD_LIST);
				if ((ret = updateMsg.encodeInit(_eIter, 0)) < CodecReturnCodes.SUCCESS)
					return ret;

				if ((ret = _marketPriceEncoder.encodeDataBody(_eIter, _marketPriceEncoder.nextPost((MarketPriceItem)itemInfo.itemData()),
								MsgClasses.POST, encodeStartTime)) < CodecReturnCodes.SUCCESS)
					return ret;
				break;
			default:
				return CodecReturnCodes.FAILURE;
		}

		ret = updateMsg.encodeComplete(_eIter, true);
		if (ret < CodecReturnCodes.SUCCESS)
			return ret;

		ret = _postMsg.encodeComplete(_eIter, true);
		if (ret < CodecReturnCodes.SUCCESS)
			return ret;

		return CodecReturnCodes.SUCCESS;
	}
	
    /**
     * Creates a Post message for an item.
     *
     * @param channel - channel to encode post message for.
     * @param itemInfo - market data item to encode post message for.
     * @param postMsg - PostMsg to create post message into.
     * @param postBuffer - buffer for UpdateMsg in encoded data body.
     * @param postUserInfo the post user info
     * @param encodeStartTime - if &gt;0, this is a latency timestamp to be included in the post message.
     * @return &lt;0 if encoding fails, 0 otherwise.
     */
    public int createItemPost(Channel channel, ItemInfo itemInfo, PostMsg postMsg, Buffer postBuffer, PostUserInfo postUserInfo, long encodeStartTime)
    {
        _eIter.clear();
        int ret = _eIter.setBufferAndRWFVersion(postBuffer, channel.majorVersion(), channel.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.printf("setBufferAndRWFVersion() failed: %d\n", ret);
            return ret;
        }

        // Prepare post message.
        postMsg.msgClass(MsgClasses.POST);
        postMsg.streamId(itemInfo.streamId());
        postMsg.applyPostComplete();
        postMsg.postUserInfo().userAddr(postUserInfo.userAddr());
        postMsg.postUserInfo().userId(postUserInfo.userId());
        postMsg.domainType(itemInfo.attributes().domainType());
        postMsg.containerType(DataTypes.MSG);

        // Prepare update message. 
        UpdateMsg updateMsg = (UpdateMsg)_tmpMsg;
        updateMsg.msgClass(MsgClasses.UPDATE);
        updateMsg.applyHasPostUserInfo();
        updateMsg.postUserInfo().userAddr(postUserInfo.userAddr());
        updateMsg.postUserInfo().userId(postUserInfo.userId());
        updateMsg.domainType(itemInfo.attributes().domainType());

        // Encode update.
        switch(itemInfo.attributes().domainType())
        {
            case DomainTypes.MARKET_PRICE:
                updateMsg.containerType(DataTypes.FIELD_LIST);
                if ((ret = updateMsg.encodeInit(_eIter, 0)) < CodecReturnCodes.SUCCESS)
                    return ret;

                if ((ret = _marketPriceEncoder.encodeDataBody(_eIter, _marketPriceEncoder.nextPost((MarketPriceItem)itemInfo.itemData()),
                                MsgClasses.POST, encodeStartTime)) < CodecReturnCodes.SUCCESS)
                    return ret;
                break;
            default:
                return CodecReturnCodes.FAILURE;
        }

        ret = updateMsg.encodeComplete(_eIter, true);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;
        
        // set encodedDataBody on PostMsg
        postMsg.encodedDataBody(postBuffer);

        return CodecReturnCodes.SUCCESS;
    }
    
	/**
	 * Encodes a Generic message for an item.
	 * 
	 * @param channel - channel to encode generic message for.
	 * @param itemInfo - market data item to encode generic message for.
	 * @param msgBuf  - TransportBuffer to encode generic message into.
	 * @param encodeStartTime - if &gt;0, this is a latency timestamp to be included in the generic message.
	 * @return &lt;0 if encoding fails, 0 otherwise.
	 */
	public int encodeItemGenMsg(Channel channel, ItemInfo itemInfo, TransportBuffer msgBuf, long encodeStartTime)
	{
	    _eIter.clear();
		int ret = _eIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
		if (ret != CodecReturnCodes.SUCCESS)
		{
			System.out.printf("setBufferAndRWFVersion() failed: %d\n", ret);
			return ret;
		}

		// Prepare generic message.
		_genMsg.msgClass(MsgClasses.GENERIC);
		_genMsg.streamId(itemInfo.streamId());
		_genMsg.domainType(itemInfo.attributes().domainType());

		// Encode generic msg.
		switch(itemInfo.attributes().domainType())
		{
			case DomainTypes.MARKET_PRICE:
			    _genMsg.containerType(DataTypes.FIELD_LIST);
		        ret = _genMsg.encodeInit(_eIter, 0);
		        if (ret < CodecReturnCodes.SUCCESS)
		            return ret;

				if ((ret = _marketPriceEncoder.encodeDataBody(_eIter, _marketPriceEncoder.nextGenMsg((MarketPriceItem)itemInfo.itemData()),
								MsgClasses.GENERIC, encodeStartTime)) < CodecReturnCodes.SUCCESS)
					return ret;
				break;
			default:
				return CodecReturnCodes.FAILURE;
		}

		assert( _genMsg.containerType() == DataTypes.FIELD_LIST);
		
		ret = _genMsg.encodeComplete(_eIter, true);
		if (ret < CodecReturnCodes.SUCCESS)
			return ret;

		return CodecReturnCodes.SUCCESS;
	}
   
    /**
     * Creates a Generic message for an item.
     * 
     * @param channel - channel to encode generic message for.
     * @param itemInfo - market data item to encode generic message for.
     * @param genericMsg - GenericMsg to create post message into.
     * @param genericBuffer - buffer for UpdateMsg in encoded data body.
     * @param encodeStartTime - if &gt;0, this is a latency timestamp to be included in the generic message.
     * @return &lt;0 if encoding fails, 0 otherwise.
     */
    public int createItemGenMsg(Channel channel, ItemInfo itemInfo, GenericMsg genericMsg, Buffer genericBuffer, long encodeStartTime)
    {
        _eIter.clear();
        int ret = _eIter.setBufferAndRWFVersion(genericBuffer, channel.majorVersion(), channel.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.printf("setBufferAndRWFVersion() failed: %d\n", ret);
            return ret;
        }

        // Prepare generic message.
        genericMsg.msgClass(MsgClasses.GENERIC);
        genericMsg.streamId(itemInfo.streamId());
        genericMsg.domainType(itemInfo.attributes().domainType());

        // Encode generic msg.
        switch(itemInfo.attributes().domainType())
        {
            case DomainTypes.MARKET_PRICE:
                genericMsg.containerType(DataTypes.FIELD_LIST);
                if (ret < CodecReturnCodes.SUCCESS)
                    return ret;

                if ((ret = _marketPriceEncoder.encodeDataBody(_eIter, _marketPriceEncoder.nextGenMsg((MarketPriceItem)itemInfo.itemData()),
                                MsgClasses.GENERIC, encodeStartTime)) < CodecReturnCodes.SUCCESS)
                    return ret;
                break;
            default:
                return CodecReturnCodes.FAILURE;
        }

        assert( genericMsg.containerType() == DataTypes.FIELD_LIST);

        // set encodedDataBody on GenericMsg
        genericMsg.encodedDataBody(genericBuffer);

        return CodecReturnCodes.SUCCESS;
    }
   
	/**
	 * Estimates the length of a Post message for an item.
	 * 
	 * @param itemInfo item information to calculate post message length.
	 * @param protocol specifies JSON protocol.
	 * @return buffer size length 
	 */
	public int estimateItemPostBufferLength(ItemInfo itemInfo, int protocol)
	{
		int bufferSize = 64;

		switch(itemInfo.attributes().domainType())
		{
			case DomainTypes.MARKET_PRICE:
				int index = ((MarketPriceItem)itemInfo.itemData()).iMsg();
				bufferSize += _xmlMsgData.marketPricePostMsgs()[index].estimatedContentLength();
				break;
			default:
				bufferSize = 0;
				break;
		}
		
		if(protocol == Codec.JSON_PROTOCOL_TYPE)
		{
			bufferSize *= 2;
		}

		return bufferSize;
	}

	/**
	 * Estimates the length of a Generic message for an item.
	 * 
	 * @param itemInfo item information to calculate generic message length.
	 * @param protocol specifies JSON protocol.
	 * @return buffer size length 
	 */
	public int estimateItemGenMsgBufferLength(ItemInfo itemInfo, int protocol)
	{
		int bufferSize = 64;

		switch(itemInfo.attributes().domainType())
		{
			case DomainTypes.MARKET_PRICE:
				int index = ((MarketPriceItem)itemInfo.itemData()).iMsg();
				bufferSize += _xmlMsgData.marketPriceGenMsgs()[index].estimatedContentLength();
				break;
			default:
				bufferSize = 0;
				break;
		}
		
		if(protocol == Codec.JSON_PROTOCOL_TYPE)
		{
			bufferSize *= 2;
		}

		return bufferSize;
	}
	
	/**
     * Estimates the length of a Refresh message for an item.
     * 
     * @param itemInfo item information to calculate refresh message length.
     * @param protocol specifies JSON protocol.
     * 
     * @return buffer size length 
     */
    public int estimateRefreshBufferLength(ItemInfo itemInfo, int protocol)
    {
        int bufferSize = 64;

        if(itemInfo.attributes().msgKey().checkHasName())
            bufferSize += itemInfo.attributes().msgKey().name().length();
        
        if(itemInfo.attributes().msgKey().checkHasName())
            bufferSize +=  itemInfo.attributes().msgKey().encodedAttrib().length();
        
        switch(itemInfo.attributes().domainType())
        {
            case DomainTypes.MARKET_PRICE:
                bufferSize += _xmlMsgData.marketPriceRefreshMsg().estimatedContentLength();
                break;
            default:
                bufferSize = 0;
                break;
        }
        
        if(protocol == Codec.JSON_PROTOCOL_TYPE)
		{
			bufferSize *= 2;
		}

        return bufferSize;
    }
    
    
    /**
     * Estimates the length of a Update message for an item.
     * 
     * @param itemInfo item information to calculate update message length.
     * @param protocol specifies JSON protocol.
     * 
     * @return buffer size length 
     */
    public int estimateUpdateBufferLength(ItemInfo itemInfo, int protocol)
    {
        int bufferSize = 64;

        switch(itemInfo.attributes().domainType())
        {
            case DomainTypes.MARKET_PRICE:
                int index = ((MarketPriceItem)itemInfo.itemData()).iMsg();
                bufferSize += _xmlMsgData.marketPriceUpdateMsgs()[index].estimatedContentLength();
                break;
            default:
                bufferSize = 0;
                break;
        }
        
        if(protocol == Codec.JSON_PROTOCOL_TYPE)
		{
			bufferSize *= 2;
		}

        return bufferSize;
    }
    
    private int encodeUpdate(Channel channel, ItemInfo itemInfo, PostUserInfo postUserInfo,
	        long encodeStartTime, Error error)
	{
    	int ret = CodecReturnCodes.SUCCESS;
    	
	    _tmpMsg.clear();
	    _tmpMsg.msgClass(MsgClasses.UPDATE);

	    UpdateMsg updateMsg = (UpdateMsg)_tmpMsg;

	    updateMsg.domainType(itemInfo.attributes().domainType());
	    updateMsg.streamId(itemInfo.streamId());
	    
	    if(postUserInfo != null)
	    {
	        updateMsg.applyHasPostUserInfo();
	        updateMsg.postUserInfo().userAddr(postUserInfo.userAddr());
	        updateMsg.postUserInfo().userId(postUserInfo.userId());
	    }

	    switch(itemInfo.attributes().domainType())
	    {
	        case DomainTypes.MARKET_PRICE:
	            updateMsg.containerType(DataTypes.FIELD_LIST);
	            ret = updateMsg.encodeInit(_eIter, 0);
	            if (ret < CodecReturnCodes.SUCCESS)
	            {
	                error.text("UpdateMsg.encodeInit() failed with return code: " + CodecReturnCodes.toString(ret));
	                error.errorId(ret);
	                return ret;
	            }
	            MarketPriceItem mpItem = (MarketPriceItem)itemInfo.itemData();
	            int mpItemIndex = mpItem.iMsg();
	            MarketPriceMsg mpMsg = _xmlMsgData.marketPriceUpdateMsgs()[mpItemIndex++];

	            if (mpItemIndex == _xmlMsgData.marketPriceUpdateMsgCount())
	                mpItemIndex = 0;

	            mpItem.iMsg(mpItemIndex);
	            ret = _marketPriceEncoder.encodeDataBody(_eIter, mpMsg, MsgClasses.UPDATE, encodeStartTime);
	            if (ret < CodecReturnCodes.SUCCESS)
	            {
	                error.text("MarketPriceEncoder.encodeDataBody() failed with return code: " + CodecReturnCodes.toString(ret));
	                error.errorId(ret);
	                return ret;
	            }
	            ret = updateMsg.encodeComplete(_eIter, true);
	            if (ret < CodecReturnCodes.SUCCESS)
	            {
	                error.text("UpdateMsg.encodeComplete() failed with return code: " + CodecReturnCodes.toString(ret));
	                error.errorId(ret);
	                return ret;
	            }
	            break;
	        default:
	            return CodecReturnCodes.FAILURE;
	    }

	    return ret;
	}
    
    /**
     * Encodes a Update message for an item.
     *
     * @param channel - channel to encode update message for.
     * @param itemInfo - market data item to encode update message for.
     * @param msgBuf  - TransportBuffer to encode update message into.
     * @param postUserInfo the post user info
     * @param encodeStartTime - if &gt;0, this is a latency timestamp to be included in the update message.
     * @param error - detailed error information in case of encoding failures.
     * @return &lt;0 if encoding fails, 0 otherwise.
     */
	public int encodeUpdate(Channel channel, 
	        ItemInfo itemInfo, TransportBuffer msgBuf, PostUserInfo postUserInfo,
	        long encodeStartTime, Error error)
	{
	    _eIter.clear();
	    int ret = _eIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	        error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
	        error.errorId(ret);
	        return ret;
	    }
	    
	    return encodeUpdate(channel, itemInfo, postUserInfo, encodeStartTime, error);
	}
	
	/**
     * Encodes a Update message for an item.
     *
     * @param channel - channel to encode update message for.
     * @param itemInfo - market data item to encode update message for.
     * @param msgBuf  - Buffer to encode update message into.
     * @param postUserInfo the post user info
     * @param encodeStartTime - if &gt;0, this is a latency timestamp to be included in the update message.
     * @param error - detailed error information in case of encoding failures.
     * @return &lt;0 if encoding fails, 0 otherwise.
     */
	public int encodeUpdate(Channel channel, 
	        ItemInfo itemInfo, Buffer msgBuf, PostUserInfo postUserInfo,
	        long encodeStartTime, Error error)
	{
	    _eIter.clear();
	    int ret = _eIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	        error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
	        error.errorId(ret);
	        return ret;
	    }
	    
	    return encodeUpdate(channel, itemInfo, postUserInfo, encodeStartTime, error);
	}
	
    /**
     * Estimates the length of a Generic message for an item.
     * 
     * @param itemInfo item information to calculate generic message length.
     * 
     * @return buffer size length 
     */
    public int estimateGenMsgBufferLength(ItemInfo itemInfo)
    {
        int bufferSize = 64;

        switch(itemInfo.attributes().domainType())
        {
            case DomainTypes.MARKET_PRICE:
                int index = ((MarketPriceItem)itemInfo.itemData()).iMsg();
                bufferSize += _xmlMsgData.marketPriceGenMsgs()[index].estimatedContentLength();
                break;
            default:
                bufferSize = 0;
                break;
        }

        return bufferSize;
    }
    
    private int encodeRefresh(Channel channel,
            ItemInfo itemInfo, PostUserInfo postUserInfo, long encodeStartTime, Error error)
	{
    	int ret = 0;
	    _tmpMsg.clear();
	    _tmpMsg.msgClass(MsgClasses.REFRESH);

	    RefreshMsg refreshMsg = (RefreshMsg)_tmpMsg;
	    refreshMsg.flags(RefreshMsgFlags.HAS_MSG_KEY | RefreshMsgFlags.REFRESH_COMPLETE | RefreshMsgFlags.HAS_QOS);
	    if((itemInfo.itemFlags() & ItemFlags.IS_SOLICITED) != 0)
	    {
	        refreshMsg.applySolicited();
	        
	        // set clear cache on solicited refreshes
	        refreshMsg.applyClearCache();
	    }

	    itemInfo.attributes().msgKey().copy(refreshMsg.msgKey());
	    
	    refreshMsg.streamId(itemInfo.streamId());

	    // Images for snapshot requests should use the non-streaming state. 
	    refreshMsg.state().streamState((itemInfo.itemFlags() & ItemFlags.IS_STREAMING_REQ) != 0 ? StreamStates.OPEN : StreamStates.NON_STREAMING);
	    refreshMsg.state().dataState(DataStates.OK);
	    refreshMsg.state().code(StateCodes.NONE);

	    refreshMsg.qos().dynamic(false);
	    refreshMsg.qos().rate(QosRates.TICK_BY_TICK);
	    refreshMsg.qos().timeliness(QosTimeliness.REALTIME);

	    refreshMsg.domainType(itemInfo.attributes().domainType());

	    if(postUserInfo != null)
	    {
	        refreshMsg.applyHasPostUserInfo();
	        refreshMsg.postUserInfo().userAddr(postUserInfo.userAddr());
	        refreshMsg.postUserInfo().userId(postUserInfo.userId());
	    }

	    switch(itemInfo.attributes().domainType())
	    {
	        case DomainTypes.MARKET_PRICE:
	            refreshMsg.containerType(DataTypes.FIELD_LIST);
	            ret = refreshMsg.encodeInit(_eIter, 0);
	            if (ret < CodecReturnCodes.SUCCESS)
	            {
	                error.text("RefreshMsg.encodeInit() failed with return code: " + CodecReturnCodes.toString(ret));
	                return ret;
	            }
	            ret = _marketPriceEncoder.encodeDataBody(_eIter, _xmlMsgData.marketPriceRefreshMsg(), MsgClasses.REFRESH, encodeStartTime);
	            if (ret < CodecReturnCodes.SUCCESS)
	            {
	                error.text("MarketPriceEncoder.encodeDataBody() failed with return code: " + CodecReturnCodes.toString(ret));
	                return ret;
	            }
	            ret = refreshMsg.encodeComplete(_eIter, true);
	            if (ret < CodecReturnCodes.SUCCESS)
	            {
	                error.text("RefreshMsg.encodeComplete() failed with return code: " + CodecReturnCodes.toString(ret));
	                return ret;
	            }
	            break;
	        default:
	            return CodecReturnCodes.FAILURE;
	    }

	    return ret;
	}
    

	/**
	 * Encodes a Refresh message for an item.
	 *
	 * @param channel - channel to encode refresh message for.
	 * @param itemInfo - market data item to encode refresh message for.
	 * @param msgBuf  - TransportBuffer to encode refresh message into.
	 * @param postUserInfo the post user info
	 * @param encodeStartTime - if &gt;0, this is a latency timestamp to be included in the refresh message.
	 * @param error - detailed error information in case of encoding failures.
	 * @return &lt;0 if encoding fails, 0 otherwise.
	 */
    public int encodeRefresh(Channel channel,
            ItemInfo itemInfo, TransportBuffer msgBuf, PostUserInfo postUserInfo, long encodeStartTime, Error error)
	{
	    _eIter.clear();
	    int ret = _eIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	        error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
	        return ret;
	    }
	    
	    return encodeRefresh(channel, itemInfo, postUserInfo, encodeStartTime, error);
	}
    
    /**
	 * Encodes a Refresh message for an item.
	 *
	 * @param channel - channel to encode refresh message for.
	 * @param itemInfo - market data item to encode refresh message for.
	 * @param msgBuf  - Buffer to encode refresh message into.
	 * @param postUserInfo the post user info
	 * @param encodeStartTime - if &gt;0, this is a latency timestamp to be included in the refresh message.
	 * @param error - detailed error information in case of encoding failures.
	 * @return &lt;0 if encoding fails, 0 otherwise.
	 */
    public int encodeRefresh(Channel channel,
            ItemInfo itemInfo, Buffer msgBuf, PostUserInfo postUserInfo, long encodeStartTime, Error error)
	{
	    _eIter.clear();
	    int ret = _eIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	        error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
	        return ret;
	    }
	    
	    return encodeRefresh(channel, itemInfo, postUserInfo, encodeStartTime, error);
	}
}
