///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.util.concurrent.locks.ReentrantLock;
import com.refinitiv.eta.valueadd.common.VaPool;

class ServerPool
{
	private static boolean _intialized = false;
	private static VaPool _clientSessionPool = new VaPool(true);
	private static VaPool _itemInfoPool = new VaPool(true);
	private static long CLIENT_HANDLE = 0;
	private static long ITEM_HANDLE = 0;
	
	static ReentrantLock _clientHandleLock = new java.util.concurrent.locks.ReentrantLock();
	static ReentrantLock _itemLock = new java.util.concurrent.locks.ReentrantLock();
	
	static void initialize(OmmServerBaseImpl ommServerBaseImpl, int clientSession, int itemInfo)
	{
		if ( _intialized )
			return;
		
		for (int index = 0; index < clientSession ; ++index)
		{
			_clientSessionPool.add(new ClientSession(ommServerBaseImpl));
		}
		
		int numberOfItemInfo = clientSession * itemInfo;
		
		for (int index = 0; index < numberOfItemInfo ; ++index)
		{
			_itemInfoPool.add(new ItemInfo());
		}
		
		_intialized = true;
	}
	
	static ClientSession getClientSession(OmmServerBaseImpl ommServerBaseImpl)
	{
		_clientHandleLock.lock();
		
		ClientSession clientSession = (ClientSession)_clientSessionPool.poll();
		if( clientSession == null )
		{
			clientSession = new ClientSession(ommServerBaseImpl);
			_clientSessionPool.updatePool(clientSession);
			_clientHandleLock.unlock();
			
			return clientSession;
		}
		else
		{
			clientSession.clear();
			_clientHandleLock.unlock();
			
			return clientSession;
		}
	}
	
	static ItemInfo getItemInfo()
	{	
		_itemLock.lock();
		ItemInfo itemInfo = (ItemInfo)_itemInfoPool.poll();
		
		if( itemInfo == null )
		{
			itemInfo = new ItemInfo();
			_itemInfoPool.updatePool(itemInfo);
			_itemLock.unlock();
			
			return itemInfo;
		}
		else
		{
			itemInfo.clear();
			_itemLock.unlock();
			
			return itemInfo;
		}
	}
	
	static long getClientHandle()
	{
		_clientHandleLock.lock();
		++CLIENT_HANDLE;
		_clientHandleLock.unlock();
		
		return CLIENT_HANDLE;
	}
	
	static long getItemHandle()
	{
		_itemLock.lock();
		++ITEM_HANDLE;
		_itemLock.unlock();
		
		return ITEM_HANDLE;
	}
}
