package com.thomsonreuters.upa.valueadd.examples.provider;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.transport.Channel;

/**
 * Item request information.
 */
public class ItemRequestInfo
{
    int streamId;
    Buffer itemName;
    ItemInfo itemInfo;
    boolean isStreamingRequest;
    boolean isPrivateStreamRequest;
    boolean includeKeyInUpdates;
    MsgKey msgKey;
    Channel channel;
    boolean isInUse;
    int domainType;

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
        isInUse = false;
        domainType = 0;
    }

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
        isInUse = false;
        domainType = 0;
    }
    
    public int streamId()
    {
    	return streamId;
    }
    
    public void streamId(int streamId)
    {
    	this.streamId = streamId;
    }
    
    public Buffer itemName()
    {
    	return itemName;
    }
    
    public ItemInfo itemInfo()
    {
    	return itemInfo;
    }
    
    public void itemInfo(ItemInfo itemInfo)
    {
    	this.itemInfo = itemInfo;
    }
    
    public boolean isStreamingRequest()
    {
    	return isStreamingRequest;
    }
    
    public void isStreamingRequest(boolean isStreamingRequest)
    {
    	this.isStreamingRequest = isStreamingRequest;
    }
    
    public boolean isPrivateStreamRequest()
    {
    	return isPrivateStreamRequest;
    }
    
    public void isPrivateStreamRequest(boolean isPrivateStreamRequest)
    {
    	this.isPrivateStreamRequest = isPrivateStreamRequest;
    }
    
    public boolean includeKeyInUpdates()
    {
    	return includeKeyInUpdates;
    }
    
    public void includeKeyInUpdates(boolean includeKeyInUpdates)
    {
    	this.includeKeyInUpdates = includeKeyInUpdates;
    }
    
    public MsgKey msgKey()
    {
    	return msgKey;
    }
    
    public Channel channel()
    {
    	return channel;
    }

    public void channel(Channel channel)
    {
    	this.channel = channel;
    }

    public boolean isInUse()
    {
    	return isInUse;
    }
    
    public void isInUse(boolean isInUse)
    {
    	this.isInUse = isInUse;
    }
    
    public int domainType()
    {
    	return domainType;
    }

    public void domainType(int domainType)
    {
    	this.domainType = domainType;
    }
}
