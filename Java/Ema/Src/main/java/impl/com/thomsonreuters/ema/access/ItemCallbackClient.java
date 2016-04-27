///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;
import com.thomsonreuters.upa.codec.AckMsgFlags;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CloseMsg;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.MsgKeyFlags;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.RefreshMsgFlags;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.RequestMsgFlags;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StatusMsgFlags;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.InstrumentNameTypes;
import com.thomsonreuters.upa.valueadd.common.VaNode;
import com.thomsonreuters.upa.valueadd.reactor.DefaultMsgCallback;
import com.thomsonreuters.upa.valueadd.reactor.ReactorCallbackReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorSubmitOptions;

class ConsumerCallbackClient
{
	protected RefreshMsgImpl			_refreshMsg;
	protected UpdateMsgImpl				_updateMsg;
	protected StatusMsgImpl				_statusMsg;
	protected GenericMsgImpl			_genericMsg;
	protected AckMsgImpl				_ackMsg;
	protected OmmConsumerEventImpl		_event;
	protected OmmConsumerImpl			_consumer;
	protected OmmConsumerClientImpl 	_consumerClient;

	ConsumerCallbackClient(OmmConsumerImpl consumer, String clientName)
	{
		_consumer = consumer;
		_event = new OmmConsumerEventImpl();
		_refreshMsg = new RefreshMsgImpl(true);
		_consumerClient = new OmmConsumerClientImpl();
		
		if (_consumer.loggerClient().isTraceEnabled())
		{
			String temp = "Created " + clientName;
			_consumer.loggerClient().trace(_consumer.formatLogMessage(clientName,
											temp, Severity.TRACE).toString());
		}
	}
	
	StatusMsg rsslStatusMsg() {return null;}
}

class ItemCallbackClient extends ConsumerCallbackClient implements DefaultMsgCallback
{
	private static final String CLIENT_NAME = "ItemCallbackClient";
	
	private HashMap<Long, Item>						_itemMap;
	private StatusMsg								_rsslStatusMsg;
	private com.thomsonreuters.upa.codec.CloseMsg	_rsslCloseMsg;

	ItemCallbackClient(OmmConsumerImpl consumer)
	{
		super(consumer, CLIENT_NAME);
		
		_itemMap = new HashMap<Long, Item>(_consumer.activeConfig().itemCountHint == 0 ? 1024 : _consumer.activeConfig().itemCountHint);
		
		_updateMsg = new UpdateMsgImpl(true);
	}

	void initialize() {}

	public int defaultMsgCallback(ReactorMsgEvent event)
	{
		Msg msg = event.msg();
		ChannelInfo channelInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
        if (msg == null)
        {
        	com.thomsonreuters.upa.transport.Error error = event.errorInfo().error();
        	
        	if (_consumer.loggerClient().isErrorEnabled())
        	{
	        	StringBuilder temp = _consumer.consumerStrBuilder();
	        	temp.append("Received an item event without RsslMsg message")
	        		.append(OmmLoggerClient.CR)
	    			.append("Consumer Name ").append(_consumer.consumerName())
	    			.append(OmmLoggerClient.CR)
	    			.append("RsslReactor ").append(Integer.toHexString(channelInfo.rsslReactor().hashCode()))
	    			.append(OmmLoggerClient.CR)
	    			.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(event.errorInfo().location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
        		_consumer.loggerClient().error(_consumer.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
        	}
        	
    		return ReactorCallbackReturnCodes.SUCCESS;
        }
        
        _event._item = (Item)(event.streamInfo() != null ? event.streamInfo().userSpecObject() : null);
        if (_event._item == null && msg.streamId() != 1)
        {
        	if (_consumer.loggerClient().isErrorEnabled())
        	{
	        	StringBuilder temp = _consumer.consumerStrBuilder();
	        	temp.append("Received an item event without user specified pointer or stream info")
	        		.append(OmmLoggerClient.CR)
	        		.append("Consumer Name ").append(_consumer.consumerName())
	        		.append(OmmLoggerClient.CR)
	        		.append("RsslReactor ").append(Integer.toHexString(channelInfo.rsslReactor().hashCode()))
	        		.append(OmmLoggerClient.CR);
	        	if (event.reactorChannel() != null && event.reactorChannel().selectableChannel() != null)
        			temp.append("RsslReactorChannel ").append(Integer.toHexString(event.reactorChannel().hashCode()))
        			.append(OmmLoggerClient.CR)
        			.append("RsslSelectableChannel ").append(Integer.toHexString(event.reactorChannel().selectableChannel().hashCode()));
	        	else
	        		temp.append("RsslReactorChannel is null").append(OmmLoggerClient.CR);
	        	
	        	_consumer.loggerClient().error(_consumer.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
        	}

        	return ReactorCallbackReturnCodes.SUCCESS;
        }

    	switch (msg.msgClass())
    	{
	    	case MsgClasses.ACK :
	    		if (msg.streamId() == 1)
	    			return _consumer.loginCallbackClient().processAckMsg(msg, channelInfo);
	    		else
	    			return _consumer.itemCallbackClient().processAckMsg(msg, channelInfo);
	    	case MsgClasses.GENERIC :
	    		if (msg.streamId() == 1)
	    			return _consumer.loginCallbackClient().processGenericMsg(msg, channelInfo);
	    		else
	    			return _consumer.itemCallbackClient().processGenericMsg(msg, channelInfo);
	    	case MsgClasses.REFRESH :
	    		return _consumer.itemCallbackClient().processRefreshMsg(msg, channelInfo);
	    	case MsgClasses.STATUS :
	    		return _consumer.itemCallbackClient().processStatusMsg(msg, channelInfo);
	    	case MsgClasses.UPDATE :
	    		return _consumer.itemCallbackClient().processUpdateMsg(msg, channelInfo);
	    	default :
	    		if (_consumer.loggerClient().isErrorEnabled())
	        	{
		        	StringBuilder temp = _consumer.consumerStrBuilder();
		        	temp.append("Received an item event with message containing unhandled message class")
		        		.append(OmmLoggerClient.CR)
		        		.append("Consumer Name ").append(_consumer.consumerName())
		        		.append(OmmLoggerClient.CR)
		        		.append("RsslReactor ").append(Integer.toHexString(channelInfo.rsslReactor().hashCode()))
		        		.append(OmmLoggerClient.CR);
			        	if (event.reactorChannel() != null && event.reactorChannel().selectableChannel() != null)
		        			temp.append("RsslReactorChannel ").append(Integer.toHexString(event.reactorChannel().hashCode()))
		        			.append(OmmLoggerClient.CR)
		        			.append("RsslSelectableChannel ").append(Integer.toHexString(event.reactorChannel().selectableChannel().hashCode()));
			        	else
			        		temp.append("RsslReactorChannel is null").append(OmmLoggerClient.CR);
		        	
		        	_consumer.loggerClient().error(_consumer.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	}
	    		break;
    	}
        
		return ReactorCallbackReturnCodes.SUCCESS;
	}

	int processRefreshMsg(Msg rsslMsg, ChannelInfo channelInfo)
	{
		_refreshMsg.decode(rsslMsg, channelInfo._majorVersion, channelInfo._minorVersion, channelInfo._rsslDictionary);
	
		if (_event._item.type() == Item.ItemType.BATCH_ITEM)
			_event._item = ((BatchItem)_event._item).singleItem(rsslMsg.streamId());
		
		_refreshMsg.service(_event._item.directory().serviceName());

		_event._item.client().onAllMsg(_refreshMsg, _event);
		_event._item.client().onRefreshMsg(_refreshMsg, _event);
		
		int rsslStreamState = ((com.thomsonreuters.upa.codec.RefreshMsg)rsslMsg).state().streamState();
		if (rsslStreamState == StreamStates.NON_STREAMING)
		{
			if (((com.thomsonreuters.upa.codec.RefreshMsg)rsslMsg).checkRefreshComplete())
				_event._item.remove();
		}
		else if (rsslStreamState != StreamStates.OPEN)
		{
			_event._item.remove();
		}

		return ReactorCallbackReturnCodes.SUCCESS;
	}

	int processUpdateMsg(Msg rsslMsg,  ChannelInfo channelInfo)
	{
		_updateMsg.decode(rsslMsg, channelInfo._majorVersion, channelInfo._minorVersion, channelInfo._rsslDictionary);
		
		if (_event._item.type() == Item.ItemType.BATCH_ITEM)
			_event._item = ((BatchItem)_event._item).singleItem(rsslMsg.streamId());
	
		_updateMsg.service(_event._item.directory().serviceName());

		_event._item.client().onAllMsg(_updateMsg, _event);
		_event._item.client().onUpdateMsg(_updateMsg, _event);

		return ReactorCallbackReturnCodes.SUCCESS;
	}

	int processStatusMsg(Msg rsslMsg,  ChannelInfo channelInfo)
	{
		if (_statusMsg == null)
			_statusMsg = new StatusMsgImpl(true);
		
		_statusMsg.decode(rsslMsg, channelInfo._majorVersion, channelInfo._minorVersion, channelInfo._rsslDictionary);
		
		if (_event._item.type() == Item.ItemType.BATCH_ITEM)
			_event._item = ((BatchItem)_event._item).singleItem(rsslMsg.streamId());
		
		_statusMsg.service(_event._item.directory().serviceName());

		_event._item.client().onAllMsg(_statusMsg, _event);
		_event._item.client().onStatusMsg(_statusMsg, _event);

		if (((com.thomsonreuters.upa.codec.StatusMsg)rsslMsg).checkHasState() &&  
				((com.thomsonreuters.upa.codec.StatusMsg)rsslMsg).state().streamState() != StreamStates.OPEN) 
			_event._item.remove();

		return ReactorCallbackReturnCodes.SUCCESS;
	}

	int processGenericMsg(Msg rsslMsg,  ChannelInfo channelInfo)
	{
		if (_genericMsg == null)
			_genericMsg = new GenericMsgImpl(true);

		_genericMsg.decode(rsslMsg, channelInfo._majorVersion, channelInfo._minorVersion, channelInfo._rsslDictionary);
		
		if (_event._item.type() == Item.ItemType.BATCH_ITEM)
			_event._item = ((BatchItem)_event._item).singleItem(rsslMsg.streamId());
		
		_event._item.client().onAllMsg(_genericMsg, _event);
		_event._item.client().onGenericMsg(_genericMsg, _event);

		return ReactorCallbackReturnCodes.SUCCESS;
	}

	int processAckMsg(Msg rsslMsg,  ChannelInfo channelInfo)
	{
		if (_ackMsg == null)
			_ackMsg = new AckMsgImpl(true);
		
		_ackMsg.decode(rsslMsg, channelInfo._majorVersion, channelInfo._minorVersion, channelInfo._rsslDictionary);

		if (_event._item.type() == Item.ItemType.BATCH_ITEM)
			_event._item = ((BatchItem)_event._item).singleItem(rsslMsg.streamId());
				
		_ackMsg.service(_event._item.directory().serviceName());

		_event._item.client().onAllMsg(_ackMsg, _event);
		_event._item.client().onAckMsg(_ackMsg, _event);

		return ReactorCallbackReturnCodes.SUCCESS;
	}
	
	com.thomsonreuters.upa.codec.StatusMsg rsslStatusMsg()
	{
		if (_rsslStatusMsg == null)
			_rsslStatusMsg = (StatusMsg)CodecFactory.createMsg();
		else
			_rsslStatusMsg.clear();
		
		return _rsslStatusMsg;
	}

	long registerClient(ReqMsg reqMsg, OmmConsumerClient consumerClient, Object closure , long parentHandle)
	{
		if (consumerClient == null)
			consumerClient = _consumerClient;
		
		if (parentHandle == 0)
		{
			com.thomsonreuters.upa.codec.RequestMsg requestMsg = ((ReqMsgImpl)reqMsg).rsslMsg();

			switch (requestMsg.domainType())
			{
				case DomainTypes.LOGIN :
				{
					SingleItem item = _consumer.loginCallbackClient().loginItem(reqMsg, consumerClient, closure);

					return addToMap(LongIdGenerator.nextLongId(), item);
				}
				case DomainTypes.DICTIONARY :
				{
					int nameType = requestMsg.msgKey().nameType();
					if ((nameType != InstrumentNameTypes.UNSPECIFIED) && (nameType != InstrumentNameTypes.RIC))
					{
						StringBuilder temp = _consumer.consumerStrBuilder();
						
			        	temp.append("Invalid ReqMsg's name type : ")
			        		.append(nameType)
			        		.append(". OmmConsumer name='").append(_consumer .consumerName()).append("'.");
		
			        	if (_consumer.loggerClient().isErrorEnabled())
			        	{
			        		_consumer.loggerClient().error(_consumer.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			        	}

						if (_consumer.hasConsumerErrorClient())
							_consumer.consumerErrorClient().onInvalidUsage(temp.toString());
						else
							throw (_consumer.ommIUExcept().message(temp.toString()));

						return 0;
					}

					if (requestMsg.msgKey().checkHasName())
					{
						String name = requestMsg.msgKey().name().toString();

						if (!(name.equals(DictionaryCallbackClient.DICTIONARY_RWFFID)) && !(name.equals(DictionaryCallbackClient.DICTIONARY_RWFENUM)))
						{
							StringBuilder temp = _consumer.consumerStrBuilder();
							
				        	temp.append("Invalid ReqMsg's name : ")
				        		.append(name)
				        		.append("\nReqMsg's name must be \"RWFFld\" or \"RWFEnum\" for MMT_DICTIONARY domain type. ")
								.append("OmmConsumer name='").append(_consumer .consumerName()).append("'.");

				        	if (_consumer.loggerClient().isErrorEnabled())
				        	{
				        		_consumer.loggerClient().error(_consumer.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
				        	}

							if (_consumer.hasConsumerErrorClient())
								_consumer.consumerErrorClient().onInvalidUsage(temp.toString());
							else
								throw (_consumer.ommIUExcept().message(temp.toString()));

							return 0;
						}
					}
					else
					{
						StringBuilder temp = _consumer.consumerStrBuilder();
						
			        	temp.append("ReqMsg's name is not defined. ")
							.append("OmmConsumer name='").append(_consumer .consumerName()).append("'.");

			        	if (_consumer.loggerClient().isErrorEnabled())
			        	{
			        		_consumer.loggerClient().error(_consumer.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			        	}

						if (_consumer.hasConsumerErrorClient())
							_consumer.consumerErrorClient().onInvalidUsage(temp.toString());
						else
							throw (_consumer.ommIUExcept().message(temp.toString()));

						return 0;
					}

					DictionaryItem item;
					if ((item = (DictionaryItem)GlobalPool._dictionaryItemPool.poll()) == null)
					{
						item = new DictionaryItem(_consumer, consumerClient, closure);
						GlobalPool._dictionaryItemPool.updatePool(item);
					}
					else
						item.reset(_consumer, consumerClient, closure, null);
					
					if (!item.open(reqMsg))
					{
						item.returnToPool();
						return 0;
					}
					else
					{
						return addToMap(LongIdGenerator.nextLongId(), item);
					}
				}
				case DomainTypes.SOURCE :
				{
					List<ChannelInfo> channels = _consumer.channelCallbackClient().channelList();
					for(ChannelInfo eachChannel : channels)
					{
						DirectoryItem item;
						if ((item = (DirectoryItem)GlobalPool._directoryItemPool.poll()) == null)
						{
							item = new DirectoryItem(_consumer, consumerClient, closure);
							GlobalPool._directoryItemPool.updatePool(item);
						}
						else
							item.reset(_consumer, consumerClient, closure, null);
						item.channelInfo(eachChannel);
						
						if (!item.open(reqMsg))
						{
							item.returnStreamId();
							item.returnToPool();
							return 0;
						}
						else
						{
							return addToMap(LongIdGenerator.nextLongId(), item);
						}
					}
	
					return 0;
				}
				default :
				{
					if (requestMsg.checkHasBatch())
					{
						BatchItem batchItem;
						if ((batchItem = (BatchItem)GlobalPool._batchItemPool.poll()) == null)
						{
							batchItem = new BatchItem(_consumer, consumerClient, closure);
							GlobalPool._batchItemPool.updatePool(batchItem);
						}
						else
							batchItem.reset(_consumer, consumerClient, closure, null);
						
							batchItem.addBatchItems( ((ReqMsgImpl)reqMsg).batchItemList().size() );
							List<SingleItem> items = batchItem.singleItemList();
							int numOfItem = items.size();
							if ( !batchItem.open( reqMsg ) )
							{
								SingleItem item;
								for ( int i = 1 ; i < numOfItem ; i++ )
								{
									item = items.get(i);
									item.returnStreamId();
									item.returnToPool();
								}
								
								batchItem.returnStreamId();
								batchItem.returnToPool();
								
								return 0;
							}
							else
							{
								batchItem.itemId(LongIdGenerator.nextLongId());
								addToMap( batchItem );

								for ( int i = 1 ; i < numOfItem ; i++ )
									addToMap(LongIdGenerator.nextLongId(), items.get(i));
								
								return batchItem.itemId();
							}
					}
					else
					{
						SingleItem item;
						if ((item = (SingleItem)GlobalPool._singleItemPool.poll()) == null)
						{
							item = new SingleItem(_consumer, consumerClient, closure, null);
							GlobalPool._singleItemPool.updatePool(item);
						}
						else
							item.reset(_consumer, consumerClient, closure, null);
						
						if (!item.open(reqMsg))
						{
							item.returnStreamId();
							item.returnToPool();
							return 0;
						}
						else
						{
							return addToMap(LongIdGenerator.nextLongId(), item);
						}
					}
				}
			}
		}
		else 
		{
			//TODO ParentHandle
		}
		
		return 0;
	}
	
	long registerClient(TunnelStreamRequest tunnelStreamReq, OmmConsumerClient consumerClient, Object closure)
	{
		return 0;
	}
	
	void reissue(ReqMsg reqMsg, long handle)
	{
		Item item = _itemMap.get(handle);
		if (item == null)
		{
			StringBuilder temp = _consumer.consumerStrBuilder();
			temp.append("Attempt to use invalid Handle on reissue(). ").append("OmmConsumer name='")
					.append(_consumer.consumerName()).append("'.");

			if (_consumer.loggerClient().isErrorEnabled())
				_consumer.loggerClient().error(
						_consumer.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));

			if (_consumer.hasConsumerErrorClient())
				_consumer.consumerErrorClient().onInvalidHandle(handle, temp.toString());
			else
				throw (_consumer.ommIHExcept().message(temp.toString(), handle));

			return;
		}

		item.modify(reqMsg);
	}

	void unregister(long handle)
	{
		Item item = _itemMap.get(handle);
		if (item != null)
			item.close();
	}

	void submit(PostMsg postMsg, long handle)
	{
		Item found = _itemMap.get((Long) handle);
		if ( found == null )
		{
			StringBuilder temp = _consumer.consumerStrBuilder();
			temp.append("Attempt to use invalid Handle on submit(PostMsg). ").append("OmmConsumer name='")
					.append(_consumer.consumerName()).append("'.");

			if (_consumer.loggerClient().isErrorEnabled())
				_consumer.loggerClient().error(
						_consumer.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));

			if (_consumer.hasConsumerErrorClient())
				_consumer.consumerErrorClient().onInvalidHandle(handle, temp.toString());
			else
				throw (_consumer.ommIHExcept().message(temp.toString(), handle));
			
			return;
		}

		found.submit( postMsg );
	}

	void submit(GenericMsg genericMsg, long handle)
	{
		Item found = _itemMap.get((Long) handle);
		if ( found == null )
		{
			StringBuilder temp = _consumer.consumerStrBuilder();
			temp.append("Attempt to use invalid Handle on submit(GenericMsg). ").append("OmmConsumer name='")
					.append(_consumer.consumerName()).append("'.");

			if (_consumer.loggerClient().isErrorEnabled())
				_consumer.loggerClient().error(
						_consumer.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));

			if (_consumer.hasConsumerErrorClient())
				_consumer.consumerErrorClient().onInvalidHandle(handle, temp.toString());
			else
				throw (_consumer.ommIHExcept().message(temp.toString(), handle));
			
			return;
		}

		found.submit( genericMsg );
	}

	
	//TODO TunnelStream
	//	int processCallback(TunnelStream , TunnelStreamStatusEvent)
	//	int processCallback(TunnelStream , TunnelStreamMsgEvent)
	//	int processCallback(TunnelStream , TunnelStreamQueueMsgEvent)

	long addToMap(long itemId, Item item)
	{
		item.itemId(itemId);
		_itemMap.put(itemId, item);
		
		if (_consumer.loggerClient().isTraceEnabled())
		{
			StringBuilder temp = _consumer.consumerStrBuilder();
			temp.append("Added Item ").append(itemId).append(" to item map" ).append( OmmLoggerClient.CR )
			.append( "OmmConsumer name " ).append( _consumer .consumerName() );
			
			_consumer.loggerClient().trace(_consumer.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
		}
		
		return itemId;
	}
	
	void addToMap(Item item)
	{
		_itemMap.put(item.itemId(), item);
	}
	
	void removeFromMap(Item item)
	{
		if (_consumer.loggerClient().isTraceEnabled())
		{
			StringBuilder temp = _consumer.consumerStrBuilder();
			temp.append("Removed Item ").append(item._itemId).append(" from item map" ).append( OmmLoggerClient.CR )
			.append( "OmmConsumer name " ).append( _consumer .consumerName() );
			
			_consumer.loggerClient().trace(_consumer.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
		}
		
		_itemMap.remove(item.itemId());
	}
	
	com.thomsonreuters.upa.codec.CloseMsg rsslCloseMsg()
	{
		if (_rsslCloseMsg == null)
			_rsslCloseMsg = (CloseMsg)CodecFactory.createMsg();
		else
			_rsslCloseMsg.clear();
		
		return _rsslCloseMsg;
	}
}

abstract class Item extends VaNode
{
	static final class ItemType
	{
		final static int SINGLE_ITEM = 0;
		final static int BATCH_ITEM  = 1;
	}
	
	int						_domainType;
	int						_streamId;
	Object					_closure;
	Item					_parent;
	OmmConsumerClient		_consumerClient;
	OmmConsumerImpl			_consumer;
	long 					_itemId;

	Item() {}
	
	Item(OmmConsumerImpl consumer, OmmConsumerClient consumerClient, Object closure, Item parent)
	{
		_domainType = 0;
		_streamId = 0;
		_closure = closure;
		_parent = parent;
		_consumerClient = consumerClient;
		_consumer = consumer;
	}

	OmmConsumerClient client()
	{
		return _consumerClient;
	}
	
	Object closure()
	{
		return _closure;
	}
	
	Item parent()
	{
		return _parent;
	}
	
	OmmConsumerImpl consumer()
	{
		return _consumer;
	}
	
	void itemId(long itemId)
	{
		_itemId = itemId;
	}
	
	long itemId()
	{
		return _itemId;
	}
	
	void reset(OmmConsumerImpl consumer, OmmConsumerClient consumerClient, Object closure, Item parent)
	{
		_domainType = 0;
		_streamId = 0;
		_closure = closure;
		_parent = parent;
		_consumerClient = consumerClient;
		_consumer = consumer;
	}
	
	int streamId()
	{
		return _streamId;
	}
	
	
	
	abstract boolean open(ReqMsg reqMsg);
	abstract boolean modify(ReqMsg reqMsg);
	abstract boolean submit(PostMsg postMsg);
	abstract boolean submit(GenericMsg genericMsg);
	abstract boolean close();
	abstract void remove();
	abstract int type();
	abstract Directory directory();
}

class SingleItem extends Item
{
	private static final String 	CLIENT_NAME = "SingleItem";
	
	protected Directory	_directory;
	protected ClosedStatusClient	_closedStatusClient;
	

	SingleItem() {}
	
	SingleItem(OmmConsumerImpl consumer, OmmConsumerClient consumerClient, Object closure , Item batchItem)
	{
		super(consumer, consumerClient, closure, batchItem);
	}
	
	@Override
	void reset(OmmConsumerImpl consumer, OmmConsumerClient consumerClient, Object closure , Item batchItem)
	{
		super.reset(consumer, consumerClient, closure, batchItem);
		
		_directory = null;
	}
	
	@Override
	Directory directory()
	{
		return _directory;
	}
	
	@Override
	int type()
	{
		return ItemType.SINGLE_ITEM;
	
	}

	@Override
	boolean open(ReqMsg reqMsg)
	{
		Directory directory = null;

		if (reqMsg.hasServiceName())
		{
			directory = _consumer.directoryCallbackClient().directory(reqMsg.serviceName());
			if (directory == null)
			{
				StringBuilder temp = _consumer.consumerStrBuilder();
	        	temp.append("Service name of '")
	        		.append(reqMsg.serviceName())
	        		.append("' is not found.");

	        	scheduleItemClosedStatus(_consumer.itemCallbackClient(),
											this, ((ReqMsgImpl)reqMsg).rsslMsg(), 
											temp.toString(), reqMsg.serviceName());
	        	
	        	return true;
			}
		}
		else
		{
			if (reqMsg.hasServiceId())
				directory = _consumer.directoryCallbackClient().directory(reqMsg.serviceId());
			else
			{
				scheduleItemClosedStatus(_consumer.itemCallbackClient(),
						this, ((ReqMsgImpl)reqMsg).rsslMsg(), 
						"Passed in request message does not identify any service.",
						null);
	        	
				return true;
			}

			if (directory == null)
			{
				StringBuilder temp = _consumer.consumerStrBuilder();
				
	        	temp.append("Service id of '")
	        		.append(reqMsg.serviceName())
	        		.append("' is not found.");
	        	
	        	scheduleItemClosedStatus(_consumer.itemCallbackClient(),
						this, ((ReqMsgImpl)reqMsg).rsslMsg(), 
						temp.toString(), null);
	        	
	        	return true;
			}
		}

		_directory = directory;

		return submit(((ReqMsgImpl)reqMsg).rsslMsg());
	}
	
	@Override
	boolean modify(ReqMsg reqMsg)
	{
		return submit(((ReqMsgImpl) reqMsg).rsslMsg());
	}

	@Override
	boolean submit(PostMsg postMsg)
	{
		return submit(((PostMsgImpl) postMsg).rsslMsg());
	}

	@Override
	boolean submit(GenericMsg genericMsg)
	{
		return submit(((GenericMsgImpl) genericMsg).rsslMsg());
	}
	
	@Override
	boolean close()
	{
		CloseMsg rsslCloseMsg = _consumer.itemCallbackClient().rsslCloseMsg();
		rsslCloseMsg.msgClass(MsgClasses.CLOSE);
		rsslCloseMsg.containerType(DataTypes.NO_DATA);
		rsslCloseMsg.domainType(_domainType);

		boolean retCode = submit(rsslCloseMsg);

		remove();
		return retCode;
	}
	
	@Override
	void remove()
	{
		if (type() != ItemType.BATCH_ITEM)
		{
			if (_parent != null)
			{
				if (_parent.type() == ItemType.BATCH_ITEM)
					((BatchItem)_parent).decreaseItemCount();
			}
			
			_consumer.itemCallbackClient().removeFromMap(this);
			this.returnStreamId();
			this.returnToPool();
		}
	}

	void returnStreamId()
	{
		if (_directory != null && _streamId != 0)
			_directory.channelInfo().returnStreamId(_streamId);
	}
	
	boolean submit(RequestMsg rsslRequestMsg)
	{
		ReactorSubmitOptions rsslSubmitOptions = _consumer.rsslSubmitOptions();
		rsslSubmitOptions.clear();
		
		int rsslFlags = rsslRequestMsg.msgKey().flags();
		rsslRequestMsg.msgKey().flags(rsslFlags & ~MsgKeyFlags.HAS_SERVICE_ID);

		if (!rsslRequestMsg.checkHasQos())
		{
			rsslRequestMsg.applyHasQos();
			rsslRequestMsg.applyHasWorstQos();
			rsslRequestMsg.qos().dynamic(false);
			rsslRequestMsg.qos().timeliness(QosTimeliness.REALTIME);
			rsslRequestMsg.qos().rate(QosRates.TICK_BY_TICK);
			rsslRequestMsg.worstQos().rate(QosRates.TIME_CONFLATED);
			rsslRequestMsg.worstQos().timeliness(QosTimeliness.DELAYED_UNKNOWN);
			rsslRequestMsg.worstQos().rateInfo(65535);
		}	
		
		if (_consumer.activeConfig().channelConfig.msgKeyInUpdates)
			rsslRequestMsg.applyMsgKeyInUpdates();
		
		if (_directory != null)
			rsslSubmitOptions.serviceName(_directory.serviceName());
		
		rsslSubmitOptions.requestMsgOptions().userSpecObj(this);
		
		int domainType =  rsslRequestMsg.domainType();
		
		if (_streamId == 0)
		{
			if (rsslRequestMsg.checkHasBatch())
			{
				List<SingleItem> items = ((BatchItem)this).singleItemList();
				int numOfItem = items.size();

				rsslRequestMsg.streamId(_directory.channelInfo().nextStreamId(numOfItem));
				_streamId = rsslRequestMsg.streamId();
				
				SingleItem item;
				int itemStreamIdStart = _streamId;
				for ( int index = 0; index < numOfItem; index++)
				{
					item = items.get(index);
					item._directory = _directory;
					item._streamId = ++itemStreamIdStart;
					item._domainType = domainType;
				}
			}
			else
			{
				rsslRequestMsg.streamId(_directory.channelInfo().nextStreamId(0));
				_streamId = rsslRequestMsg.streamId();
			}
		}
		else
			rsslRequestMsg.streamId(_streamId);

		if (_domainType == 0)
			_domainType = domainType;
		else
			rsslRequestMsg.domainType(_domainType);
		
	    ReactorErrorInfo rsslErrorInfo = _consumer.rsslErrorInfo();
		rsslErrorInfo.clear();
		ReactorChannel rsslChannel = _directory.channelInfo().rsslReactorChannel();
		int ret;
		if (ReactorReturnCodes.SUCCESS > (ret = rsslChannel.submit(rsslRequestMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			StringBuilder temp = _consumer.consumerStrBuilder();
			if (_consumer.loggerClient().isErrorEnabled())
        	{
				com.thomsonreuters.upa.transport.Error error = rsslErrorInfo.error();
				
	        	temp.append("Internal error: rsslChannel.submit() failed in SingleItem.submit(RequestMsg)")
	        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	_consumer.loggerClient().error(_consumer.formatLogMessage(SingleItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
        	}
			
			temp.append("Failed to open or modify item request. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(rsslErrorInfo.error().text());
				
			if (_consumer.hasConsumerErrorClient())
				_consumer.consumerErrorClient().onInvalidUsage(temp.toString());
			else
				throw (_consumer.ommIUExcept().message(temp.toString()));

			return false;
	    }
        
		return true;
	}

	boolean submit(CloseMsg rsslCloseMsg)
	{	
		ReactorSubmitOptions rsslSubmitOptions = _consumer.rsslSubmitOptions();
		rsslSubmitOptions.clear();

		rsslSubmitOptions.requestMsgOptions().userSpecObj(this);
	
		if (_streamId == 0)
		{
			if (_consumer.loggerClient().isErrorEnabled())
	        	_consumer.loggerClient().error(_consumer.formatLogMessage(SingleItem.CLIENT_NAME,
	        									"Invalid streamId for this item in in SingleItem.submit(CloseMsg)",
	        									Severity.ERROR));
		}
		else
			rsslCloseMsg.streamId(_streamId);
	
		ReactorErrorInfo rsslErrorInfo = _consumer.rsslErrorInfo();
		rsslErrorInfo.clear();
		ReactorChannel rsslChannel = _directory.channelInfo().rsslReactorChannel();
		int ret;
		if (ReactorReturnCodes.SUCCESS > (ret = rsslChannel.submit(rsslCloseMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			StringBuilder temp = _consumer.consumerStrBuilder();
			
			if (_consumer.loggerClient().isErrorEnabled())
	    	{
				com.thomsonreuters.upa.transport.Error error = rsslErrorInfo.error();
				
	        	temp.append("Internal error: ReactorChannel.submit() failed in SingleItem.submit(CloseMsg)")
	        	.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	_consumer.loggerClient().error(_consumer.formatLogMessage(SingleItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
	    	}
			
			temp.append("Failed to close item request. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(rsslErrorInfo.error().text());
				
			if (_consumer.hasConsumerErrorClient())
				_consumer.consumerErrorClient().onInvalidUsage(temp.toString());
			else
				throw (_consumer.ommIUExcept().message(temp.toString()));
	
			return false;
	    }
	
		return true;
	}

	boolean submit(com.thomsonreuters.upa.codec.PostMsg rsslPostMsg)
	{
		ReactorSubmitOptions rsslSubmitOptions = _consumer.rsslSubmitOptions();
		rsslSubmitOptions.clear();
		
		rsslPostMsg.streamId(_streamId);
		rsslPostMsg.domainType(_domainType);
		
	    ReactorErrorInfo rsslErrorInfo = _consumer.rsslErrorInfo();
		rsslErrorInfo.clear();
		ReactorChannel rsslChannel = _directory.channelInfo().rsslReactorChannel();
		int ret;
		if (ReactorReturnCodes.SUCCESS > (ret = rsslChannel.submit(rsslPostMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			StringBuilder temp = _consumer.consumerStrBuilder();
			if (_consumer.loggerClient().isErrorEnabled())
        	{
				com.thomsonreuters.upa.transport.Error error = rsslErrorInfo.error();
				
	        	temp.append("Internal error: rsslChannel.submit() failed in SingleItem.submit(PostMsg)")
	        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	_consumer.loggerClient().error(_consumer.formatLogMessage(SingleItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
        	}
			
			temp.append("Failed to submit PostMsg on item stream. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(rsslErrorInfo.error().text());
				
			if (_consumer.hasConsumerErrorClient())
				_consumer.consumerErrorClient().onInvalidUsage(temp.toString());
			else
				throw (_consumer.ommIUExcept().message(temp.toString()));

			return false;
	    }
        
		return true;
	}
	
	boolean submit(com.thomsonreuters.upa.codec.GenericMsg rsslGenericMsg)
	{
		ReactorSubmitOptions rsslSubmitOptions = _consumer.rsslSubmitOptions();
		rsslSubmitOptions.clear();
		
		rsslGenericMsg.streamId(_streamId);
		rsslGenericMsg.domainType(_domainType);
		
	    ReactorErrorInfo rsslErrorInfo = _consumer.rsslErrorInfo();
		rsslErrorInfo.clear();
		ReactorChannel rsslChannel = _directory.channelInfo().rsslReactorChannel();
		int ret;
		if (ReactorReturnCodes.SUCCESS > (ret = rsslChannel.submit(rsslGenericMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			StringBuilder temp = _consumer.consumerStrBuilder();
			if (_consumer.loggerClient().isErrorEnabled())
        	{
				com.thomsonreuters.upa.transport.Error error = rsslErrorInfo.error();
				
	        	temp.append("Internal error: rsslChannel.submit() failed in SingleItem.submit(GenericMsg)")
	        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	_consumer.loggerClient().error(_consumer.formatLogMessage(SingleItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
        	}
			
			temp.append("Failed to submit GenericMsg on item stream. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(rsslErrorInfo.error().text());
				
			if (_consumer.hasConsumerErrorClient())
				_consumer.consumerErrorClient().onInvalidUsage(temp.toString());
			else
				throw (_consumer.ommIUExcept().message(temp.toString()));

			return false;
	    }
        
		return true;
	}
		
	ClosedStatusClient closedStatusClient(ConsumerCallbackClient client, Item item, Msg rsslMsg, String statusText, String serviceName)
	{
		if (_closedStatusClient == null)
			_closedStatusClient = new ClosedStatusClient(client, item, rsslMsg, statusText, serviceName);
		else
			_closedStatusClient.reset(client, item, rsslMsg, statusText, serviceName);
		
		return _closedStatusClient;
	}
	
	void scheduleItemClosedStatus(ConsumerCallbackClient client, Item item, Msg rsslMsg, String statusText, String serviceName)
	{
		if (_closedStatusClient != null) return;
    	
		_closedStatusClient = new ClosedStatusClient(client, item, rsslMsg, statusText, serviceName);
    	_consumer.addTimeoutEvent(1000, _closedStatusClient);
	}
}

class BatchItem extends SingleItem
{
	private static final String 	CLIENT_NAME = "BatchItem";
	
	private List<SingleItem>		_singleItemList = new ArrayList<SingleItem>();
	private  int	 _itemCount;
	
	BatchItem() {}
			
	BatchItem(OmmConsumerImpl consumer, OmmConsumerClient consumerClient, Object closure)
	{
		super(consumer, consumerClient, closure, null);
		
		_singleItemList = new ArrayList<SingleItem>();
		_itemCount = 1;
	}
	
	@Override
	void reset(OmmConsumerImpl consumer, OmmConsumerClient consumerClient, Object closure, Item item)
	{
		super.reset(consumer, consumerClient, closure, null);
		
		_singleItemList.clear();
		_itemCount = 1;
	}

	@Override
	boolean open(ReqMsg reqMsg)
	{
		return super.open(reqMsg);
	}
	
	@Override
	boolean modify(ReqMsg reqMsg)
	{
		StringBuilder temp = _consumer.consumerStrBuilder();
		temp.append("Invalid attempt to modify batch stream. ").append("OmmConsumer name='")
				.append(_consumer.consumerName()).append("'.");

		if (_consumer.loggerClient().isErrorEnabled())
			_consumer.loggerClient()
					.error(_consumer.formatLogMessage(BatchItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

		if (_consumer.hasConsumerErrorClient())
			_consumer.consumerErrorClient().onInvalidUsage(temp.toString());
		else
			throw (_consumer.ommIUExcept().message(temp.toString()));

		return false;
	}

	@Override
	boolean submit(PostMsg postMsg)
	{
		StringBuilder temp = _consumer.consumerStrBuilder();
		temp.append("Invalid attempt to submit PostMsg on batch stream. ").append("OmmConsumer name='")
				.append(_consumer.consumerName()).append("'.");

		if (_consumer.loggerClient().isErrorEnabled())
			_consumer.loggerClient()
					.error(_consumer.formatLogMessage(BatchItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

		if (_consumer.hasConsumerErrorClient())
			_consumer.consumerErrorClient().onInvalidUsage(temp.toString());
		else
			throw (_consumer.ommIUExcept().message(temp.toString()));

		return false;
	}

	@Override
	boolean submit(GenericMsg genericMsg)
	{
		StringBuilder temp = _consumer.consumerStrBuilder();
		temp.append("Invalid attempt to submit GenericMsg on batch stream. ").append("OmmConsumer name='")
				.append(_consumer.consumerName()).append("'.");

		if (_consumer.loggerClient().isErrorEnabled())
			_consumer.loggerClient()
					.error(_consumer.formatLogMessage(BatchItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

		if (_consumer.hasConsumerErrorClient())
			_consumer.consumerErrorClient().onInvalidUsage(temp.toString());
		else
			throw (_consumer.ommIUExcept().message(temp.toString()));

		return false;
	}
	
	@Override
	boolean close()
	{
		StringBuilder temp = _consumer.consumerStrBuilder();
		temp.append("Invalid attempt to close batch stream. ").append("OmmConsumer name='")
				.append(_consumer.consumerName()).append("'.");

		if (_consumer.loggerClient().isErrorEnabled())
			_consumer.loggerClient()
					.error(_consumer.formatLogMessage(BatchItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

		if (_consumer.hasConsumerErrorClient())
			_consumer.consumerErrorClient().onInvalidUsage(temp.toString());
		else
			throw (_consumer.ommIUExcept().message(temp.toString()));

		return false;
	}

	@Override
	int type()
	{
		return ItemType.BATCH_ITEM;
	
	}
	
	SingleItem createSingleItem()
	{
		SingleItem item;
		if ((item = (SingleItem)GlobalPool._singleItemPool.poll()) == null)
		{
			item = new SingleItem(_consumer, _consumerClient, 0, this);
			GlobalPool._singleItemPool.updatePool(item);
		}
		else
			item.reset(_consumer, _consumerClient, 0, this);
		
		return item;
	}
	
	void addBatchItems(int numOfItem )
	{
		SingleItem item;
		for( int i = 0 ; i < numOfItem ; i++ )
		{
			if ((item = (SingleItem)GlobalPool._singleItemPool.poll()) == null)
			{
				item = new SingleItem(_consumer, _consumerClient, _closure, this);
				GlobalPool._singleItemPool.updatePool(item);
			}
			else
				item.reset(_consumer, _consumerClient, _closure, this);
			
			_singleItemList.add( item );
		}
		
		_itemCount = numOfItem;
	}
	
	List<SingleItem> singleItemList()
	{
		return _singleItemList;
	}

	SingleItem singleItem(int streamId)
	{
		int index = streamId - _streamId;
		if (index < 0)
			return null;
	
		return (index == 0) ? this : _singleItemList.get(index-1);
	}

	void decreaseItemCount()
	{
		if ( --_itemCount == 0 )
			this.returnToPool();
	}
}

//TODO TunnelItem
//TODO SubItem

class ClosedStatusClient implements TimeoutClient
{
	private MsgKey 		_rsslMsgKey = CodecFactory.createMsgKey();
	private Buffer 		_statusText =  CodecFactory.createBuffer();
	private String 		_serviceName;
	private int 		_domainType;
	private int 		_streamId;
	private Item 		_item;
	private boolean 	_isPrivateStream; 
	private ConsumerCallbackClient _client;
	
	ClosedStatusClient(ConsumerCallbackClient client, Item item, Msg rsslMsg, String statusText, String serviceName)
	{
		reset(client, item, rsslMsg, statusText, serviceName);
	}
	
	void reset(ConsumerCallbackClient client, Item item, Msg rsslMsg, String statusText, String serviceName)
	{
		_client = client;
		_item = item;
		_statusText.data(statusText);
		_domainType = rsslMsg.domainType();
		_rsslMsgKey.clear();
		_serviceName = serviceName;
		
		if (rsslMsg.msgKey() != null)
			rsslMsg.msgKey().copy(_rsslMsgKey);
		
		switch (rsslMsg.msgClass())
	    {
	     	case MsgClasses.REFRESH :
	           	_isPrivateStream = (rsslMsg.flags() & RefreshMsgFlags.PRIVATE_STREAM) > 0 ? true : false;
	           	break;
	        case MsgClasses.STATUS :
	        	_isPrivateStream = (rsslMsg.flags() & StatusMsgFlags.PRIVATE_STREAM) > 0 ? true : false;
	        	break;
	        case MsgClasses.REQUEST :
	           	_isPrivateStream = (rsslMsg.flags() & RequestMsgFlags.PRIVATE_STREAM) > 0 ? true : false;
	        	break;
	        case MsgClasses.ACK :
	           	_isPrivateStream = (rsslMsg.flags() & AckMsgFlags.PRIVATE_STREAM) > 0 ? true : false;
	        	break;
	        default :
	           	_isPrivateStream = false;
	        	break;
	    }
	}
	
	@Override
	public void handleTimeoutEvent()
	{
		StatusMsg rsslStatusMsg = _client.rsslStatusMsg();

		rsslStatusMsg.msgClass(MsgClasses.STATUS);
		rsslStatusMsg.streamId(_streamId);
		rsslStatusMsg.domainType(_domainType);
		rsslStatusMsg.containerType(DataTypes.NO_DATA);
	
		rsslStatusMsg.applyHasState();
		rsslStatusMsg.state().streamState(StreamStates.CLOSED);
		rsslStatusMsg.state().dataState(DataStates.SUSPECT);
		rsslStatusMsg.state().code(StateCodes.NONE);
		rsslStatusMsg.state().text(_statusText);
		    
		rsslStatusMsg.applyHasMsgKey();
		_rsslMsgKey.copy(rsslStatusMsg.msgKey()); 
		
		if (_isPrivateStream)
			rsslStatusMsg.applyPrivateStream();

		if (_client._statusMsg == null)
			_client._statusMsg = new StatusMsgImpl(true);
		
		_client._statusMsg.decode(rsslStatusMsg, Codec.majorVersion(), Codec.majorVersion(), null);

		_client._statusMsg.service(_serviceName);

		_client._event._item = _item;

		_client._event._item.client().onAllMsg(_client._statusMsg, _client._event);
		_client._event._item.client().onStatusMsg(_client._statusMsg, _client._event);

		_client._event._item.remove();
	}
}
