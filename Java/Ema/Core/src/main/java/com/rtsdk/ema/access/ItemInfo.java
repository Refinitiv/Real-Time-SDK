package com.rtsdk.ema.access;

import java.util.concurrent.ConcurrentHashMap;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.MsgKey;
import com.rtsdk.eta.valueadd.common.VaNode;

class ItemInfo extends VaNode
{
    private int _flags;
    private int _domainType;
    private Buffer _itemGroup;
    private ClientSession _clientSession;
    private LongObject _handle;
    private	LongObject _streamId;
    private MsgKey _msgKey;
    private boolean _sentRefresh;
    private java.util.Map<Long, Integer> _postIdsCount;
    
    class ItemInfoFlags
    {
        final static int NONE = 0;
        final static int STREAMING = 1;
        final static int PRIVATE_STREAM = 2;
        final static int ITEM_GROUP = 4;
    }

    ItemInfo()
    {
        _sentRefresh = false;
        _flags = ItemInfoFlags.NONE;
        _domainType = 0;
        _itemGroup = CodecFactory.createBuffer();
        _clientSession = null;
        _handle = new LongObject();
        _streamId = new LongObject();
        _handle.value(ServerPool.getItemHandle());
        _msgKey = CodecFactory.createMsgKey();
    }

    void setRequestMsg(com.rtsdk.eta.codec.RequestMsg requestMsg)
    {	    	
    	requestMsg.msgKey().copy(_msgKey);
    		
    	_streamId.value(requestMsg.streamId());
        _domainType = requestMsg.domainType();

        if (requestMsg.checkStreaming())
        {
            _flags |= ItemInfoFlags.STREAMING;
        }
        if (requestMsg.checkPrivateStream())
        {
            _flags |= ItemInfoFlags.PRIVATE_STREAM;
        }
    }
    
    MsgKey msgKey()
    {
    	return _msgKey;
    }

    LongObject handle()
    {
        return _handle;
    }

    LongObject streamId()
    {
        return _streamId;
    }
    
    boolean isSentRefresh()
    {
    	return _sentRefresh;
    }

    void streamId(int streamId)
    {
        _streamId.value(streamId);
    }

    int serviceId()
    {
        return _msgKey.serviceId();
    }

    Buffer name()
    {
        return _msgKey.name();
    }

    void name(String name)
    {
    	_msgKey.name().data(name);
    }

    int nameType()
    {
        return _msgKey.nameType();
    }

    void nameType(int nameType)
    {
    	_msgKey.nameType(nameType);
    }
    
    boolean isPrivateStream()
    {
    	return  (_flags & ItemInfoFlags.PRIVATE_STREAM) == ItemInfoFlags.PRIVATE_STREAM ? true : false;
    }
    
    boolean isStreaming()
    {
    	return  (_flags & ItemInfoFlags.STREAMING) == ItemInfoFlags.STREAMING ? true : false;
    }
    
    boolean hasItemGroup()
    {
    	return  (_flags & ItemInfoFlags.ITEM_GROUP) == ItemInfoFlags.ITEM_GROUP ? true : false;
    }

    int flags()
    {
        return _flags;
    }

    void flags(int flags)
    {
        _flags = flags;
    }

    int domainType()
    {
        return _domainType;
    }

    void domainType(int domainType)
    {
        _domainType = domainType;
    }

    Buffer itemGroup()
    {
        return _itemGroup;
    }

    void itemGroup(Buffer itemGroup)
    {
    	_flags |= ItemInfoFlags.ITEM_GROUP;
        _itemGroup = itemGroup;
    }

    ClientSession clientSession()
    {
        return _clientSession;
    }

    void clientSession(ClientSession clientSession)
    {
        _clientSession = clientSession;
    }
    
    void setSentRefresh()
    {
    	_sentRefresh = true;
    }
    
    public void addPostId(long postId){
        if(_postIdsCount == null){
            _postIdsCount = new ConcurrentHashMap<>();
        }
        Integer oldCount = _postIdsCount.get(postId);
        if(oldCount == null){
            _postIdsCount.put(postId, 1);
        }
        else{
            _postIdsCount.put(postId, oldCount + 1);
        }

    }
    
    public boolean removePostId(long id){
        if(_postIdsCount == null){
            return false;
        }
        Integer count = _postIdsCount.get(id);
        if(count == null)
        {
            return false;
        }
        else if(count > 1){
            _postIdsCount.put(id, count-1);
        }
        else
        {
            _postIdsCount.remove(id);
        }
        return true;
    }
    
    @Override
    public int hashCode()
    {
    	if ( _msgKey.checkHasName() )
    	{
    		return _msgKey.name().hashCode();
    	}
    	else
    	{
    		return 0;
    	}
    }
    
    @Override
    public boolean equals(Object other)
    {
        if (other == this)
        {
            return true;
        }
        
        ItemInfo itemInfo;
        
        try
        {
        	itemInfo = (ItemInfo)other;
        }
        catch (ClassCastException e)
        {
            return false;
        }
        
        if ( _domainType != itemInfo._domainType)
        {
        	return false;
        }
        
        if ( isPrivateStream() != itemInfo.isPrivateStream() )
        {
        	return false;
        }
        
        return _msgKey.equals(itemInfo._msgKey);
    }

    void clear()
    {
        _sentRefresh = false;
        _flags = ItemInfoFlags.NONE;
        _itemGroup.clear();
        _clientSession = null;
        _streamId.clear();
        _domainType = 0;
        _msgKey.clear();
        if(_postIdsCount != null) {
            _postIdsCount.clear();
        }
    }

    @Override
    public void returnToPool() {
        if(_postIdsCount != null) {
            _postIdsCount.clear();
        }
        
        super.returnToPool();
    }
}
