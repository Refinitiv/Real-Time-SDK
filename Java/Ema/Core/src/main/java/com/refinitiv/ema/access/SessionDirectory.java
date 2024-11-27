///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|              Copyright (C) 2024 LSEG. All rights reserved.                --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CopyMsgFlags;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service.ServiceState;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;

class SessionDirectory<T>
{
	private List<SessionChannelInfo<T>>		_sessionChannelInfoList;
	private Service			_service;
	private String 			_serviceName;
	private boolean			_isUpdated; // This is used to indicate whether there is an update to fan-out.
	
	// This is used to restore the original service flag after generating a source directory response from request filter.
	private int             _originalFlag;
	private Map<String, HashSet<SingleItem<T>>> _itemNameMap; // This is active item map for requesting the same item name and service name to the same channel.
	
	// This is recovery item map to wait for the service to recover for a channel. This map is used for normal channel only.
	private Map<SessionChannelInfo<T>, LinkedHashMap<String, HashSet<SingleItem<T>>>> _recoveringItemMapBySessionChannel; 
	private ArrayDeque<SingleItem<T>> _pendingItemQueue; // This is used to recover items when the concrete service is available.
	private ConsumerSession<T> _consumerSession;
	private WatchlistResult _watchlistResult;
	private WatchlistResult tempWatchlistResult;
	
	SessionDirectory(ConsumerSession<T> consumerSession, String serviceName)
	{
		_serviceName = serviceName;
		
		_sessionChannelInfoList = new ArrayList<SessionChannelInfo<T>>();
		
		_itemNameMap = new LinkedHashMap<String, HashSet<SingleItem<T>>>();
		
		_recoveringItemMapBySessionChannel = new HashMap<SessionChannelInfo<T>,LinkedHashMap<String, HashSet<SingleItem<T>>>>();
		
		_pendingItemQueue = new ArrayDeque<SingleItem<T>>();
		
		_consumerSession = consumerSession;
		
		_watchlistResult = new WatchlistResult();
		
		tempWatchlistResult = new WatchlistResult();
	}
	
	void restoreServiceFlags()
	{
		_service.flags(_originalFlag);
	}
	
	void resetUpdateFlags()
	{
		_isUpdated = false;
	}
	
	List<SessionChannelInfo<T>> sessionChannelList()
	{
		return _sessionChannelInfoList;
	}
	
	void sessionChannelList(List<SessionChannelInfo<T>> sessionChannelList)
	{
		_sessionChannelInfoList = sessionChannelList;
	}
	
	Service service()
	{
		return _service;
	}
	
	WatchlistResult watchlistResult()
	{
		return _watchlistResult;
	}
	
	Directory<T> directory(com.refinitiv.eta.codec.RequestMsg rsslRequestMsg)
	{
		if(_sessionChannelInfoList.size() != 0)
		{
			/* Gets the Directory from the first available ReactorChannel*/
			Directory<T> directory = null;
			for(SessionChannelInfo<T> session : _sessionChannelInfoList)
			{
				directory = session.getDirectoryByName(_serviceName);
				Service service = directory.service();
				
				if(_consumerSession.watchlist().checkMatchingService(rsslRequestMsg, service, _watchlistResult))
				{
					session.updateSessionDirectory(directory, this);
					break;
				}
				else
				{
					directory = null;
				}
			}
			
			return directory;
		}
		else
		{
			_watchlistResult.resultText = "No matching service present.";
		}
		
		return null;
	}
	
	Directory<T> directory()
	{
		if(_sessionChannelInfoList.size() != 0)
		{
			/* Gets the Directory from the first available ReactorChannel*/
			Directory<T> directory = null;
			for(SessionChannelInfo<T> session : _sessionChannelInfoList)
			{
				directory = session.getDirectoryByName(_serviceName);
				Service service = directory.service();
				
				if(!service.checkHasState() || service.state().serviceState() == 0 || service.state().acceptingRequests() == 0)
				{
					_watchlistResult.resultText = "Service not up";
					continue;
				}
				else
				{
					return directory;
				}
			}
			
			return directory;
		}
		else
		{
			_watchlistResult.resultText = "No matching service present.";
		}
		
		return null;
	}
	
	Directory<T> matchingWithExistingDirectory(HashSet<SingleItem<T>> itemHashSet, com.refinitiv.eta.codec.RequestMsg rsslRequestMsg)
	{
		if(itemHashSet != null)
		{
			Iterator<SingleItem<T>> it = itemHashSet.iterator();
			
			SingleItem<T>	singleItem;
			while(it.hasNext())
			{
				singleItem = it.next();
				Directory<T> directory = singleItem.directory();
				
				if(_consumerSession.watchlist().checkMatchingService(rsslRequestMsg, directory.service(), tempWatchlistResult))
				{
					return directory;
				}
			}
		}
		
		
		return null;
	}
	
	boolean checkServiceAreDownForWSBChannel(SessionChannelInfo<T> currentSession)
	{
		/* Checks whether this directory still provides a source directory for this service*/
		if(_sessionChannelInfoList.contains(currentSession) == false)
			return true;
		
		Directory<T> directory = currentSession.getDirectoryByName(_serviceName);
		if(directory.service().checkHasState())
		{
			ServiceState serviceState = directory.service().state();
			
			if(serviceState.serviceState() == 0 || serviceState.acceptingRequests() == 0)
			{
				return true;
			}
		}
		else
		{
			return true;
		}
	
		return false;
	}
	
	Directory<T> updateSessionChannelInfo(int domainType, ReactorChannel current, boolean retryWithCurrentDir, HashSet<Directory<T>> itemClosedDirHash)
	{
		if(_sessionChannelInfoList.size() != 0)
		{
			/* Gets the Directory from the first available ReactorChannel*/
			Directory<T> directory = null;
			SessionChannelInfo<T> currentSession = null;
			Directory<T> currentDirectory = null;
			
			for(SessionChannelInfo<T> session : _sessionChannelInfoList)
			{
				directory = session.getDirectoryByName(_serviceName);
				
				if(directory != null)
				{
					if(itemClosedDirHash != null)
					{	/* Moves to next directory if the directory is requested */
						if(itemClosedDirHash.contains(directory))
							continue;
					}
					
					/* Ensure that the service provides the requested capability */
					if(!SessionWatchlist.isCapabilitySupported(domainType, directory.service()))
					{
						continue;
					}
					
					if(directory.channelInfo().rsslReactorChannel() == current)
					{
						if(retryWithCurrentDir)
						{
							currentSession = session;
							currentDirectory = directory;
						}
						continue;
					}
					
					if(directory.service().checkHasState())
					{
						ServiceState serviceState = directory.service().state();
						/* Check to ensure that the service is up and accepting request */
						if(serviceState.serviceState() != 0 && (!serviceState.checkHasAcceptingRequests() || serviceState.acceptingRequests() != 0))
						{
							session.updateSessionDirectory(directory, this);/* Update ReactorChannel with a new one */
							ReactorChannel reactorChannel = directory.channelInfo().rsslReactorChannel();
							
							if(reactorChannel != null && 
									(reactorChannel.state() == ReactorChannel.State.UP || reactorChannel.state() == ReactorChannel.State.READY))
							{
								return directory;
							}
						}
					}
				}
			}
			
			/* Try to check with the current session */
			if(currentSession != null)
			{
				if(currentDirectory.service().checkHasState())
				{
					/* Check to ensure that the service is up and accepting request */
					if(currentDirectory.service().state().serviceState() != 0 && currentDirectory.service().state().acceptingRequests() != 0)
					{
						ReactorChannel reactorChannel = currentDirectory.channelInfo().rsslReactorChannel();
						if(reactorChannel != null && 
								(reactorChannel.state() == ReactorChannel.State.UP || reactorChannel.state() == ReactorChannel.State.READY))
						{
							currentSession.updateSessionDirectory(currentDirectory, this); /* Update with the current ReactorChannel */
							return currentDirectory;
						}
					}
				}
			}
		}
		
		return null;
	}

	SessionDirectory<T> service(Service service)
	{
		_service = service;
		_originalFlag = service.flags();
		return this;
	}
	
	SessionDirectory<T> serviceName(String serviceName)
	{
		_serviceName = serviceName;
		return this;
	}
	
	String serviceName()
	{
		return _serviceName;
	}
	
	void originalFlags(int flags)
	{
		_originalFlag = flags;
	}
	
	boolean isUpdated()
	{
		return _isUpdated;
	}
	
	void isUpdated(boolean updated)
	{
		_isUpdated = updated;
	}
	
	void putDirectoryByItemName(String itemName, SingleItem<T> _item)
	{
		HashSet<SingleItem<T>> itemSet = _itemNameMap.get(itemName);
		if(itemSet == null)
		{
			itemSet = new HashSet<SingleItem<T>>();
		}
		
		itemSet.add(_item);
		
		_itemNameMap.put(itemName, itemSet);
	}
	
	void putDirectoryByHashSet(HashSet<SingleItem<T>> itemSet, SingleItem<T> _item)
	{	
		itemSet.add(_item);	
	}
	
	@SuppressWarnings("unchecked")
	void removeDirectoryByItemName(String itemName, SingleItem<T> singleItem)
	{
		HashSet<SingleItem<T>> itemSet = _itemNameMap.get(itemName);
		boolean itemRemoved = false;
		if(itemSet != null)
		{
			itemRemoved = itemSet.remove(singleItem);
		}
		
		if(!itemRemoved)
		{
			SessionChannelInfo<T> sessionChannelInfo = (SessionChannelInfo<T>) singleItem.directory().channelInfo().sessionChannelInfo();
			
			LinkedHashMap<String, HashSet<SingleItem<T>>> recoveringItemMap = _recoveringItemMapBySessionChannel.get(sessionChannelInfo);
			
			if(recoveringItemMap != null)
			{
				itemSet = recoveringItemMap.get(itemName);
				if(itemSet != null)
				{
					itemSet.remove(singleItem);
				}
			}
		}
	}
	
	HashSet<SingleItem<T>> getDirectoryByItemName(String itemName)
	{
		return _itemNameMap.get(itemName);
	}

	public void addPendingRequest(SingleItem<T> singleItem, ReqMsg reqMsg) 
	{
		singleItem._requestMsg = (RequestMsg)CodecFactory.createMsg();
		
		singleItem._requestMsg.msgClass(MsgClasses.REQUEST);
		((ReqMsgImpl)reqMsg).rsslMsg().copy(singleItem._requestMsg, CopyMsgFlags.ALL_FLAGS);
		
		_pendingItemQueue.addLast(singleItem);
	}

	public void handlePendingRequests(SessionChannelInfo<T> sessionChannelInfo, Service service) {
		
		int count = _pendingItemQueue.size();
		SingleItem<T> singleItem = _pendingItemQueue.poll();
		
		Directory<T> directory;
		
		while(singleItem != null)
		{
			directory = directory(singleItem._requestMsg);
			
			if(directory != null)
			{
				singleItem._directory = directory;
				singleItem._serviceName = _serviceName;
		
				/* The item state is changed to normal item stream */
				singleItem.state(SingleItem.ItemStates.NORMAL);
				singleItem.rsslSubmit(singleItem._requestMsg, false);
				
				if(!singleItem._itemName.isEmpty())
					putDirectoryByItemName(singleItem._itemName, singleItem);
			}
			else
			{
				_pendingItemQueue.add(singleItem);
			}
			
			if ( (--count) == 0)
			{
				break;
			}
			
			singleItem = _pendingItemQueue.poll();
		}
		
		HandleRecoveringRequests(sessionChannelInfo, true);
	}

	@SuppressWarnings("unchecked")
	public void addRecoveringQueue(SingleItem<T> singleItem) 
	{
		HashSet<SingleItem<T>> hashSet = _itemNameMap.get(singleItem._itemName);
		
		if(hashSet != null)
		{
			SessionChannelInfo<T> sessionChannelInfo = (SessionChannelInfo<T>) singleItem.directory().channelInfo().sessionChannelInfo();
			
			LinkedHashMap<String, HashSet<SingleItem<T>>> recoveringItemMap = _recoveringItemMapBySessionChannel.get(sessionChannelInfo);
			
			if(recoveringItemMap == null)
			{
				recoveringItemMap = new LinkedHashMap<String, HashSet<SingleItem<T>>>();
				_recoveringItemMapBySessionChannel.put(sessionChannelInfo, recoveringItemMap);
			}
			
			HashSet<SingleItem<T>> recoveryHashSet = recoveringItemMap.get(singleItem._itemName);
			if(recoveryHashSet == null)
			{
				HashSet<SingleItem<T>> itemSet = new HashSet<SingleItem<T>>();
				
				itemSet.add(singleItem);
				
				recoveringItemMap.put(singleItem._itemName, itemSet);
			}
			else
			{
				recoveryHashSet.add(singleItem);
			}

			hashSet.remove(singleItem);
		}		
	}
	
	public void HandleRecoveringRequests(SessionChannelInfo<T> sessionChannelInfo, boolean sameChannel)
	{
		Map<String, HashSet<SingleItem<T>>> recoveringItemMap = _recoveringItemMapBySessionChannel.get(sessionChannelInfo);
		
		if(recoveringItemMap != null)
		{
			Iterator<Entry<String, HashSet<SingleItem<T>>>> iter =   recoveringItemMap.entrySet().iterator();
			
			while (iter.hasNext())
			{
				Entry<String, HashSet<SingleItem<T>>> entry  = iter.next();
				String itemName = entry.getKey();
				HashSet<SingleItem<T>> hashSet = entry.getValue();
				
				Iterator<SingleItem<T>> singleItemIt = hashSet.iterator();
				
				int count = hashSet.size();
				
				SingleItem<T> singleItem;
				while(singleItemIt.hasNext())
				{
					singleItem = singleItemIt.next();
					
					if(sameChannel)
					{
						if(singleItem.directory().channelInfo().sessionChannelInfo() == sessionChannelInfo)
						{
							boolean isMatched = _consumerSession.watchlist().checkMatchingService(singleItem._requestMsg, singleItem.directory().service(), tempWatchlistResult);
							
							if(isMatched)
							{
								singleItem._serviceName = _serviceName;
						
								/* The item state is changed to normal item stream */
								singleItem.state(SingleItem.ItemStates.NORMAL);
								singleItem.rsslSubmit(singleItem._requestMsg, false);
								
								/* Removes from the recovery queue */
								singleItemIt.remove();
								
								/* Adds to the active queue */
								_itemNameMap.get(itemName).add(singleItem);
							}
							else
							{
								/* Don't remove this item in order to try again later once the service is ready */
							}
						}
					}
					else
					{
						Directory<T> directory;
						directory = directory(singleItem._requestMsg);
						
						if(directory != null)
						{
							singleItem._directory = directory;
							singleItem._serviceName = _serviceName;
					
							/* The item state is changed to normal item stream */
							singleItem.state(SingleItem.ItemStates.NORMAL);
							singleItem.rsslSubmit(singleItem._requestMsg, false);
						}
						else
						{
							singleItem.state(SingleItem.ItemStates.RECOVERING);
							_consumerSession.watchlist().recoverItemQueue().add(singleItem._requestMsg);
							_consumerSession.nextDispatchTime(1000); // Wait for 1 millisecond to recover
						}
						
						/* Removes from the recovery queue */
						singleItemIt.remove();
						
						/* Adds to the active queue */
						_itemNameMap.get(itemName).add(singleItem);
					}
					
					if ( (--count) == 0)
					{
						break;
					}
				}
			}
		}
	}
	
	public void HandleSessionChannelClose(SessionChannelInfo<T> sessionChannelInfo)
	{
		Map<String, HashSet<SingleItem<T>>> recoveringItemMap = _recoveringItemMapBySessionChannel.get(sessionChannelInfo);
		
		if(recoveringItemMap != null)
		{
			Iterator<Entry<String, HashSet<SingleItem<T>>>> iter =   recoveringItemMap.entrySet().iterator();
			
			while (iter.hasNext())
			{
				Entry<String, HashSet<SingleItem<T>>> entry  = iter.next();
				String itemName = entry.getKey();
				HashSet<SingleItem<T>> hashSet = entry.getValue();
				
				Iterator<SingleItem<T>> singleItemIt = hashSet.iterator();
				
				SingleItem<T> singleItem;
				while(singleItemIt.hasNext())
				{
					singleItem = singleItemIt.next();
					
					Directory<T> directory;
					directory = directory(singleItem._requestMsg);
					
					if(directory != null)
					{
						singleItem._directory = directory;
						singleItem._serviceName = _serviceName;
				
						/* The item state is changed to normal item stream */
						singleItem.state(SingleItem.ItemStates.NORMAL);
						if(singleItem.rsslSubmit(singleItem._requestMsg, false) == false)
						{
							singleItem.state(SingleItem.ItemStates.RECOVERING);
							_consumerSession.watchlist().recoverItemQueue().add(singleItem._requestMsg);
							_consumerSession.nextDispatchTime(1000); // Wait for 1 millisecond to recover
						}
					}
					else
					{
						singleItem.state(SingleItem.ItemStates.RECOVERING);
						_consumerSession.watchlist().recoverItemQueue().add(singleItem._requestMsg);
						_consumerSession.nextDispatchTime(1000); // Wait for 1 millisecond to recover
					}
					
					/* Removes from the recovery queue */
					singleItemIt.remove();
					
					/* Adds to the active queue */
					_itemNameMap.get(itemName).add(singleItem);
				}
			}
			
			_recoveringItemMapBySessionChannel.remove(sessionChannelInfo);
		}
	}
	
	/* This is used to fan-out item closed for remaining recovery items */
	public void close()
	{
		/* Send item closed status from pending item queue */
		SingleItem<T> singleItem = _pendingItemQueue.poll();
		
		while(singleItem != null && singleItem._requestMsg != null)
		{
			
			_consumerSession.watchlist().sendItemStatus(singleItem, singleItem._requestMsg, OmmState.StreamState.CLOSED,
					OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, "Consumer session is closed.");
			
			singleItem = _pendingItemQueue.poll();
		}
		
		Iterator<LinkedHashMap<String, HashSet<SingleItem<T>>>> recoveringIt = _recoveringItemMapBySessionChannel.values().iterator();
		while(recoveringIt.hasNext())
		{
			Iterator<HashSet<SingleItem<T>>> hashSetIt = recoveringIt.next().values().iterator();
			
			while(hashSetIt.hasNext())
			{
				for(SingleItem<T> sintleItemEntry : hashSetIt.next())
				{
					_consumerSession.watchlist().sendItemStatus(sintleItemEntry, sintleItemEntry._requestMsg, OmmState.StreamState.CLOSED,
							OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, "Consumer session is closed.");
				}
			}
		}
	}
}
