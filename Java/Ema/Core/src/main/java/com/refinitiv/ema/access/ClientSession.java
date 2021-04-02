///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.valueadd.common.VaNode;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;

class ClientSession extends VaNode
{
    private LongObject _clientHandle;
    private ReactorChannel _rsslReactorChannel;
    private HashMap<LongObject, ItemInfo>	_itemInfoByStreamIdMap;
    private HashMap<LongObject, HashMap<Buffer, ArrayList<ItemInfo>>>_serviceGroupIdToItemInfoMap;
    private HashSet<ItemInfo>  _itemInfoByItemInfoSet = null;
    private boolean _isLogin;
    private OmmServerBaseImpl _ommServerBaseImpl;
    private boolean _removingInCloseAll;
    private boolean _isADHSession = false;
    private LongObject _loginHandle;
    protected int _majorVersion;
    protected int _minorVersion;

    ClientSession(OmmServerBaseImpl ommServerBaseImpl)
    {
    	_ommServerBaseImpl = ommServerBaseImpl;
    	
    	_itemInfoByStreamIdMap = new HashMap<LongObject, ItemInfo>(_ommServerBaseImpl.activeConfig().itemCountHint);
    	
    	_serviceGroupIdToItemInfoMap = new HashMap<>();
    	
    	if(!_ommServerBaseImpl.activeConfig().acceptMessageSameKeyButDiffStream)
    	{
    		_itemInfoByItemInfoSet = new HashSet<ItemInfo>(_ommServerBaseImpl.activeConfig().itemCountHint);
    	}
    	
    	_clientHandle = new LongObject();
        _clientHandle.value(ServerPool.getClientHandle());
		_loginHandle = new LongObject();
        _rsslReactorChannel = null;
        _isLogin = false;
        _removingInCloseAll = false;
    }

    LongObject clientHandle()
    {
        return _clientHandle;
    }

    void channel(ReactorChannel rsslReactorChannel)
    {
        _rsslReactorChannel = rsslReactorChannel;

        _majorVersion = rsslReactorChannel.majorVersion();
        _minorVersion = rsslReactorChannel.minorVersion();
    }

    ReactorChannel channel()
    {
        return _rsslReactorChannel;
    }

    void addItemInfo(ItemInfo itemInfo)
    {
    	_itemInfoByStreamIdMap.put(itemInfo.streamId(), itemInfo);
    }

    void removeItemInfo(ItemInfo itemInfo)
    {
    	if( _removingInCloseAll == false )
    	{
    		_itemInfoByStreamIdMap.remove(itemInfo.streamId());
    		
    		if ( _itemInfoByItemInfoSet != null )
    		{
    			if(itemInfo.domainType() > EmaRdm.MMT_DICTIONARY)
    			{
    				_itemInfoByItemInfoSet.remove(itemInfo);
    			}
    		}
    	}
    }
    
    ItemInfo getItemInfo(LongObject streamId)
    {
    	_ommServerBaseImpl.userLock().lock();
    	
    	ItemInfo itemInfo = _itemInfoByStreamIdMap.get(streamId);
    	
    	_ommServerBaseImpl.userLock().unlock();
    	
    	return itemInfo;
    }
    
    boolean checkingExistingReq(ItemInfo itemInfo)
    {
    	if ( _itemInfoByItemInfoSet == null )
    	{
    		return false;
    	}
    	
    	boolean found = false;
    	
    	_ommServerBaseImpl.userLock().lock();
    	
    	found = _itemInfoByItemInfoSet.contains(itemInfo);
    		
    	if(!found)
    	{
    		_itemInfoByItemInfoSet.add(itemInfo);
    	}
    	
    	_ommServerBaseImpl.userLock().unlock();
    	
    	return found;
    }
    
    Collection<ItemInfo> itemInfoList()
    {
    	return _itemInfoByStreamIdMap.values();
    }
    
    HashMap<LongObject, HashMap<Buffer, ArrayList<ItemInfo>>> serviceGroupIdToItemInfoMap()
    {
    	return _serviceGroupIdToItemInfoMap;
    }

    void closeAllItemInfo()
    {
        if (_ommServerBaseImpl != null)
        {	
        	_removingInCloseAll = true;
        	
        	Iterator<ItemInfo> iter = _itemInfoByStreamIdMap.values().iterator();
        	
        	ItemInfo itemInfo;
        	
        	while(iter.hasNext())
        	{
        		itemInfo = iter.next();
        		
        		switch(itemInfo.domainType())
        		{
        		case EmaRdm.MMT_DICTIONARY:
        			_ommServerBaseImpl._dictionaryHandler.getItemInfoList().remove(itemInfo);
        			break;
        		case EmaRdm.MMT_DIRECTORY:
        			_ommServerBaseImpl._directoryHandler.getItemInfoList().remove(itemInfo);
        			break;
        		case EmaRdm.MMT_LOGIN:
        			_ommServerBaseImpl._loginHandler.getItemInfoList().remove(itemInfo);
					_loginHandle.clear();
        			break;
        		default:
        			break;
        		}
        		
        		_ommServerBaseImpl.removeItemInfo(itemInfo, false);
        	}
        	
        	_itemInfoByStreamIdMap.clear();
        	
        	Iterator<HashMap<Buffer, ArrayList<ItemInfo>>> groupIdToItemInfoIt =  _serviceGroupIdToItemInfoMap.values().iterator();
        	
        	while(groupIdToItemInfoIt.hasNext())
        	{
        		HashMap<Buffer, ArrayList<ItemInfo>> groupIdToItemInfo = groupIdToItemInfoIt.next();
        		
        		Iterator<ArrayList<ItemInfo>> itemInfoListIt = groupIdToItemInfo.values().iterator();
        		
        		while(itemInfoListIt.hasNext())
        		{
        			itemInfoListIt.next().clear();
        		}
        		
        		groupIdToItemInfo.clear();
        	}
        	
        	_serviceGroupIdToItemInfoMap.clear();
        	
        	if ( _itemInfoByItemInfoSet != null )
        	{
        		_itemInfoByItemInfoSet.clear();
        	}
        	
        	_removingInCloseAll = false;
        }
    }

    public String toString()
    {
        String returnString = "\tClient handle = " + _clientHandle;

        return returnString;
    }

    void isLogin(boolean isLogin)
    {
        _isLogin = isLogin;
    }

    boolean isADHSession()
    {
        return _isADHSession;
    }
    
    void isADHSession(boolean isADHSession)
    {
    	_isADHSession = isADHSession;
    }

    boolean isLogin()
    {
        return _isLogin;
    }

	public long getLoginHandle() {
		return _loginHandle.value();
	}

	public void setLoginHandle(long loginHandle) {
		this._loginHandle.value(loginHandle);
	}

	public void resetLoginHandle() {
    	this._loginHandle.clear();
	}

	void clear()
    {
		_loginHandle.clear();
    	_itemInfoByStreamIdMap.clear();
    	_serviceGroupIdToItemInfoMap.clear();
    	
    	if ( _itemInfoByItemInfoSet != null )
    	{
    		_itemInfoByItemInfoSet.clear();
    	}
    	
        _rsslReactorChannel = null;
        _isLogin = false;
        _removingInCloseAll = false;
    }
	
	@Override
    public void returnToPool()
    {
		_rsslReactorChannel = null;
    	
    	super.returnToPool();
    }
}
