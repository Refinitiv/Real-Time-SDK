/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared.provider;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;

/**
 * Item request information.
 */
public class ItemRequestInfo
{
    public int streamId;
    public Buffer itemName;
    public ItemInfo itemInfo;
    public boolean isStreamingRequest;
    public boolean isPrivateStreamRequest;
    public boolean includeKeyInUpdates;
    public MsgKey msgKey;
    public Channel channel;
    public ReactorChannel reactorChannel;
    public boolean isInUse;
    public int domainType;

    /**
     * Instantiates a new item request info.
     */
    public ItemRequestInfo()
    {
        streamId = 0;
        itemName = CodecFactory.createBuffer();
        itemInfo = null;
        isStreamingRequest = false;
        isPrivateStreamRequest = false;
        includeKeyInUpdates = false;
        msgKey = CodecFactory.createMsgKey();
        channel = null;
        reactorChannel = null;
        isInUse = false;
        domainType = 0;
    }

    /**
     * Clear.
     */
    public void clear()
    {
        streamId = 0;
        itemName.clear();
        itemInfo = null;
        isStreamingRequest = false;
        isPrivateStreamRequest = false;
        includeKeyInUpdates = false;
        msgKey.clear();
        channel = null;
        reactorChannel = null;
        isInUse = false;
        domainType = 0;
    }
    
    /**
     * Stream id.
     *
     * @return the int
     */
    public int streamId()
    {
    	return streamId;
    }
    
    /**
     * Stream id.
     *
     * @param streamId the stream id
     */
    public void streamId(int streamId)
    {
    	this.streamId = streamId;
    }
    
    /**
     * Item name.
     *
     * @return the buffer
     */
    public Buffer itemName()
    {
    	return itemName;
    }
    
    /**
     * Item info.
     *
     * @return the item info
     */
    public ItemInfo itemInfo()
    {
    	return itemInfo;
    }
    
    /**
     * Item info.
     *
     * @param itemInfo the item info
     */
    public void itemInfo(ItemInfo itemInfo)
    {
    	this.itemInfo = itemInfo;
    }
    
    /**
     * Checks if is streaming request.
     *
     * @return true, if is streaming request
     */
    public boolean isStreamingRequest()
    {
    	return isStreamingRequest;
    }
    
    /**
     * Checks if is streaming request.
     *
     * @param isStreamingRequest the is streaming request
     */
    public void isStreamingRequest(boolean isStreamingRequest)
    {
    	this.isStreamingRequest = isStreamingRequest;
    }
    
    /**
     * Checks if is private stream request.
     *
     * @return true, if is private stream request
     */
    public boolean isPrivateStreamRequest()
    {
    	return isPrivateStreamRequest;
    }
    
    /**
     * Checks if is private stream request.
     *
     * @param isPrivateStreamRequest the is private stream request
     */
    public void isPrivateStreamRequest(boolean isPrivateStreamRequest)
    {
    	this.isPrivateStreamRequest = isPrivateStreamRequest;
    }
    
    /**
     * Include key in updates.
     *
     * @return true, if successful
     */
    public boolean includeKeyInUpdates()
    {
    	return includeKeyInUpdates;
    }
    
    /**
     * Include key in updates.
     *
     * @param includeKeyInUpdates the include key in updates
     */
    public void includeKeyInUpdates(boolean includeKeyInUpdates)
    {
    	this.includeKeyInUpdates = includeKeyInUpdates;
    }
    
    /**
     * Msg key.
     *
     * @return the msg key
     */
    public MsgKey msgKey()
    {
    	return msgKey;
    }
    
    /**
     * Channel.
     *
     * @return the channel
     */
    public Channel channel()
    {
    	return channel;
    }

    /**
     * Channel.
     *
     * @param channel the channel
     */
    public void channel(Channel channel)
    {
    	this.channel = channel;
    }

    /**
     * ReactorChannel.
     *
     * @return the reactor channel.
     */
    public ReactorChannel reactorChannel() {
        return reactorChannel;
    }

    /**
     * ReactorChannel.
     *
     * @param reactorChannel the reactor channel.
     */
    public void reactorChannel(ReactorChannel reactorChannel) {
        this.reactorChannel = reactorChannel;
    }

    /**
     * Checks if is in use.
     *
     * @return true, if is in use
     */
    public boolean isInUse()
    {
    	return isInUse;
    }
    
    /**
     * Checks if is in use.
     *
     * @param isInUse the is in use
     */
    public void isInUse(boolean isInUse)
    {
    	this.isInUse = isInUse;
    }
    
    /**
     * Domain type.
     *
     * @return the int
     */
    public int domainType()
    {
    	return domainType;
    }

    /**
     * Domain type.
     *
     * @param domainType the domain type
     */
    public void domainType(int domainType)
    {
    	this.domainType = domainType;
    }
}
